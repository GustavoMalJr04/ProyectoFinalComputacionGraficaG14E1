#pragma once

// Librerías
#include <cstdio>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Model.h"
#include "Shader.h"

// Constantes
const int   MAX_CUADROS_CLAVE      = 40;
const int   MAX_POSES_ALA          = 8;
const float DISTANCIA_FRENTE_CAMARA = 1.0f;

// Estructura cuadro clave
struct CuadroClave
{
    float posicionX;
    float posicionY;
    float posicionZ;
    float giroY;
    float inclinacionX;
};

// Estructura pose de ala
struct PoseAla
{
    float anguloGrados;
};

// Mariposa por keyframes
class MariposaKeyframes
{
public:

    // Parámetros
    int   pasosEntreCuadros     = 120;
    int   pasosEntreAleteos     = 5;
    float pasosPorSegundo       = 60.0f;
    float escala                = 0.10f;
	float correccionGiroGrados = 0.0f;
    float bisagraX              = 0.0f;
    glm::vec3 desplazamiento    = glm::vec3(0.0f);

    // Constructor
    MariposaKeyframes(Model* modeloCuerpo, Model* modeloAlaIzquierda, Model* modeloAlaDerecha)
        : cuerpo(modeloCuerpo), alaIzquierda(modeloAlaIzquierda), alaDerecha(modeloAlaDerecha)
    {
        ConfigurarAleteo(-15.0f, 75.0f);
    }

    // Configurar aleteo
    void ConfigurarAleteo(float anguloAbajo, float anguloArriba)
    {
        totalPosesAla = 0;
        AgregarPoseAla(anguloAbajo);
        AgregarPoseAla(anguloArriba);
        ReiniciarAleteo();
    }

    // Agregar pose de ala
    void AgregarPoseAla(float anguloGrados)
    {
        if (totalPosesAla >= MAX_POSES_ALA) return;
        posesAla[totalPosesAla].anguloGrados = anguloGrados;
        totalPosesAla++;
    }

    // Reiniciar aleteo
    void ReiniciarAleteo()
    {
        if (totalPosesAla < 2) ConfigurarAleteoPorDefecto();
        poseAlaActual     = 0;
        pasoAlaActual     = 0;
        anguloAlaActual   = posesAla[0].anguloGrados;
        CalcularIncrementoAla();
    }

    // Agregar cuadro clave
    void AgregarCuadroClave(float x, float y, float z, float giroY, float inclinacionX)
    {
        if (totalCuadrosClave >= MAX_CUADROS_CLAVE)
        {
            printf("[Mariposa] Ya no caben mas cuadros clave (maximo %d)\n", MAX_CUADROS_CLAVE);
            return;
        }
        cuadrosClave[totalCuadrosClave] = { x, y, z, giroY, inclinacionX };
        totalCuadrosClave++;

        if (totalCuadrosClave == 1) ColocarEnCuadroClave(0);
    }

    // Guardar cuadro desde la cámara
    void GuardarCuadroDesdeCamara(glm::vec3 posicionCamara, glm::vec3 frenteCamara)
    {
        glm::vec3 punto = posicionCamara + frenteCamara * DISTANCIA_FRENTE_CAMARA;
        float giro        = glm::degrees(std::atan2(-frenteCamara.x, -frenteCamara.z));
        float inclinacion = glm::degrees(std::asin(glm::clamp(frenteCamara.y, -1.0f, 1.0f)));
        inclinacion       = glm::clamp(inclinacion, -30.0f, 30.0f);

        AgregarCuadroClave(punto.x, punto.y, punto.z, giro, inclinacion);
        printf("mariposa.AgregarCuadroClave(%.2ff, %.2ff, %.2ff, %.1ff, %.1ff);   // cuadro %d\n",
               punto.x, punto.y, punto.z, giro, inclinacion, totalCuadrosClave - 1);
    }

    // Borrar cuadros clave
    void BorrarCuadrosClave()
    {
        totalCuadrosClave = 0;
        reproduciendo = false;
        printf("[Mariposa] Cuadros clave borrados\n");
    }

    // Imprimir cuadros clave
    void ImprimirCuadrosClave() const
    {
        printf("// ---- %d cuadros clave de la mariposa ----\n", totalCuadrosClave);
        for (int i = 0; i < totalCuadrosClave; i++)
        {
            const CuadroClave& c = cuadrosClave[i];
            printf("mariposa.AgregarCuadroClave(%.2ff, %.2ff, %.2ff, %.1ff, %.1ff);   // cuadro %d\n",
                   c.posicionX, c.posicionY, c.posicionZ, c.giroY, c.inclinacionX, i);
        }
    }

    // Consultas
    int  TotalCuadrosClave() const { return totalCuadrosClave; }
    bool EstaReproduciendo() const { return reproduciendo; }

    // Guardar en archivo
    bool GuardarEnArchivo(const char* ruta) const
    {
        std::ofstream archivo(ruta);
        if (!archivo.is_open())
        {
            printf("[Mariposa] No se pudo crear el archivo %s\n", ruta);
            return false;
        }
        archivo << "# Cuadros clave de la mariposa monarca\n";
        archivo << "# ala <angulo>  -> cuadro clave del aleteo (grados, + = arriba)\n";
        for (int i = 0; i < totalPosesAla; i++)
            archivo << "ala " << posesAla[i].anguloGrados << "\n";
        archivo << "# x  y  z  giroY(grados)  inclinacionX(grados)  -> cuadro clave del vuelo\n";
        for (int i = 0; i < totalCuadrosClave; i++)
        {
            const CuadroClave& c = cuadrosClave[i];
            archivo << c.posicionX << " " << c.posicionY << " " << c.posicionZ << " "
                    << c.giroY << " " << c.inclinacionX << "\n";
        }
        printf("[Mariposa] %d cuadros clave guardados en %s\n", totalCuadrosClave, ruta);
        return true;
    }

    // Cargar desde archivo
    bool CargarDesdeArchivo(const char* ruta)
    {
        std::ifstream archivo(ruta);
        if (!archivo.is_open())
        {
            printf("[Mariposa] No se encontro el archivo %s\n", ruta);
            return false;
        }
        bool seguiaVolando = reproduciendo;
        totalCuadrosClave = 0;
        reproduciendo = false;
        int posesLeidas = 0;

        std::string linea;
        while (std::getline(archivo, linea))
        {
            if (linea.empty() || linea[0] == '#' || linea[0] == '\r') continue;
            std::istringstream datos(linea);

            if (linea.compare(0, 3, "ala") == 0)
            {
                std::string etiqueta;
                float angulo;
                if (datos >> etiqueta >> angulo)
                {
                    if (posesLeidas == 0) totalPosesAla = 0;
                    AgregarPoseAla(angulo);
                    posesLeidas++;
                }
                continue;
            }

            float x, y, z, giro, inclinacion;
            if (datos >> x >> y >> z >> giro >> inclinacion)
                AgregarCuadroClave(x, y, z, giro, inclinacion);
        }
        ReiniciarAleteo();
        printf("[Mariposa] %s: %d cuadros de vuelo, %d de aleteo\n", ruta, totalCuadrosClave, totalPosesAla);

        if (seguiaVolando && totalCuadrosClave >= 2) ReproducirDesde(0);
        return totalCuadrosClave >= 2;
    }

    // Reproducir desde un cuadro
    void ReproducirDesde(int cuadroInicial)
    {
        if (totalCuadrosClave < 2) return;
        cuadroActual = cuadroInicial % totalCuadrosClave;
        pasoActual   = 0;
        acumuladorTiempo = 0.0f;
        CalcularIncrementos();
        reproduciendo = true;
    }

    // Reproducir o detener
    void ReproducirODetener()
    {
        if (reproduciendo)
        {
            reproduciendo = false;
            printf("[Mariposa] Animacion detenida\n");
            return;
        }
        if (totalCuadrosClave < 2)
        {
            printf("[Mariposa] Se necesitan al menos 2 cuadros clave (hay %d)\n", totalCuadrosClave);
            return;
        }
        cuadroActual = 0;
        pasoActual   = 0;
        acumuladorTiempo = 0.0f;
        CalcularIncrementos();
        reproduciendo = true;
        printf("[Mariposa] Animacion iniciada con %d cuadros clave\n", totalCuadrosClave);
    }

    // Pausar o reanudar
    void PausarOReanudar()
    {
        if (totalCuadrosClave < 2) return;
        reproduciendo = !reproduciendo;
    }

    // Actualizar
    void Actualizar(float deltaTime)
    {
        if (!reproduciendo) return;

        acumuladorTiempo += deltaTime;
        if (acumuladorTiempo > 0.25f) acumuladorTiempo = 0.25f;

        const float duracionPaso = 1.0f / pasosPorSegundo;
        while (acumuladorTiempo >= duracionPaso)
        {
            AvanzarUnPaso();
            acumuladorTiempo -= duracionPaso;
        }
    }

    // Dibujar
    void Dibujar(Shader& shader)
    {
        if (totalCuadrosClave == 0) return;

        GLuint programa   = shader.Program;
        GLint  ubicModelo = glGetUniformLocation(programa, "model");

        glUniform1i(glGetUniformLocation(programa, "transparency"), 1);
        glUniform1f(glGetUniformLocation(programa, "specularStrength"), 0.05f);
        GLboolean cullingEstabaActivo = glIsEnabled(GL_CULL_FACE);
        glDisable(GL_CULL_FACE);

        // Cuerpo
        glm::mat4 matrizCuerpo(1.0f);
        matrizCuerpo = glm::translate(matrizCuerpo, glm::vec3(posicionX, posicionY, posicionZ) + desplazamiento);
        matrizCuerpo = glm::rotate(matrizCuerpo, glm::radians(giroY + correccionGiroGrados), glm::vec3(0.0f, 1.0f, 0.0f));
        matrizCuerpo = glm::rotate(matrizCuerpo, glm::radians(inclinacionX), glm::vec3(1.0f, 0.0f, 0.0f));
        matrizCuerpo = glm::scale(matrizCuerpo, glm::vec3(escala));

        glUniformMatrix4fv(ubicModelo, 1, GL_FALSE, glm::value_ptr(matrizCuerpo));
        cuerpo->Draw(shader);

        float anguloAla = glm::radians(anguloAlaActual);

        // Ala derecha
        glm::mat4 matrizAlaDerecha = matrizCuerpo;
        matrizAlaDerecha = glm::translate(matrizAlaDerecha, glm::vec3(bisagraX, 0.0f, 0.0f));
        matrizAlaDerecha = glm::rotate(matrizAlaDerecha, anguloAla, glm::vec3(0.0f, 0.0f, 1.0f));
        matrizAlaDerecha = glm::translate(matrizAlaDerecha, glm::vec3(-bisagraX, 0.0f, 0.0f));
        glUniformMatrix4fv(ubicModelo, 1, GL_FALSE, glm::value_ptr(matrizAlaDerecha));
        alaDerecha->Draw(shader);

        // Ala izquierda
        glm::mat4 matrizAlaIzquierda = matrizCuerpo;
        matrizAlaIzquierda = glm::translate(matrizAlaIzquierda, glm::vec3(-bisagraX, 0.0f, 0.0f));
        matrizAlaIzquierda = glm::rotate(matrizAlaIzquierda, -anguloAla, glm::vec3(0.0f, 0.0f, 1.0f));
        matrizAlaIzquierda = glm::translate(matrizAlaIzquierda, glm::vec3(bisagraX, 0.0f, 0.0f));
        glUniformMatrix4fv(ubicModelo, 1, GL_FALSE, glm::value_ptr(matrizAlaIzquierda));
        alaIzquierda->Draw(shader);

        // Restaurar estado
        if (cullingEstabaActivo) glEnable(GL_CULL_FACE);
        glUniform1i(glGetUniformLocation(programa, "transparency"), 0);
        glUniform1f(glGetUniformLocation(programa, "specularStrength"), 0.15f);
    }

private:

    // Modelos
    Model* cuerpo;
    Model* alaIzquierda;
    Model* alaDerecha;

    // Cuadros clave del vuelo
    CuadroClave cuadrosClave[MAX_CUADROS_CLAVE];
    int  totalCuadrosClave = 0;
    int  cuadroActual      = 0;
    int  pasoActual        = 0;
    bool reproduciendo     = false;
    float acumuladorTiempo = 0.0f;

    // Estado actual
    float posicionX = 0.0f, posicionY = 0.0f, posicionZ = 0.0f;
    float giroY = 0.0f, inclinacionX = 0.0f;

    // Incrementos
    float incrementoX = 0.0f, incrementoY = 0.0f, incrementoZ = 0.0f;
    float incrementoGiroY = 0.0f, incrementoInclinacionX = 0.0f;

    // Cuadros clave de las alas
    PoseAla posesAla[MAX_POSES_ALA];
    int   totalPosesAla     = 0;
    int   poseAlaActual     = 0;
    int   pasoAlaActual     = 0;
    float anguloAlaActual   = 0.0f;
    float incrementoAla     = 0.0f;

    // Diferencia de ángulos
    static float DiferenciaAnguloCorta(float desde, float hasta)
    {
        float diferencia = hasta - desde;
        while (diferencia > 180.0f)  diferencia -= 360.0f;
        while (diferencia < -180.0f) diferencia += 360.0f;
        return diferencia;
    }

    // Colocar en cuadro clave
    void ColocarEnCuadroClave(int indice)
    {
        posicionX    = cuadrosClave[indice].posicionX;
        posicionY    = cuadrosClave[indice].posicionY;
        posicionZ    = cuadrosClave[indice].posicionZ;
        giroY        = cuadrosClave[indice].giroY;
        inclinacionX = cuadrosClave[indice].inclinacionX;
    }

    // Interpolación
    void CalcularIncrementos()
    {
        int siguiente = (cuadroActual + 1) % totalCuadrosClave;
        ColocarEnCuadroClave(cuadroActual);

        const CuadroClave& destino = cuadrosClave[siguiente];
        incrementoX            = (destino.posicionX - posicionX) / pasosEntreCuadros;
        incrementoY            = (destino.posicionY - posicionY) / pasosEntreCuadros;
        incrementoZ            = (destino.posicionZ - posicionZ) / pasosEntreCuadros;
        incrementoGiroY        = DiferenciaAnguloCorta(giroY, destino.giroY) / pasosEntreCuadros;
        incrementoInclinacionX = (destino.inclinacionX - inclinacionX) / pasosEntreCuadros;
    }

    // Aleteo por defecto
    void ConfigurarAleteoPorDefecto()
    {
        totalPosesAla = 0;
        AgregarPoseAla(-15.0f);
        AgregarPoseAla(75.0f);
    }

    // Incremento del ala
    void CalcularIncrementoAla()
    {
        int siguientePose = (poseAlaActual + 1) % totalPosesAla;
        incrementoAla = (posesAla[siguientePose].anguloGrados - posesAla[poseAlaActual].anguloGrados)
                        / pasosEntreAleteos;
    }

    // Avanzar un paso
    void AvanzarUnPaso()
    {
        // Vuelo
        if (pasoActual >= pasosEntreCuadros)
        {
            cuadroActual = (cuadroActual + 1) % totalCuadrosClave;
            pasoActual = 0;
            CalcularIncrementos();
        }
        else
        {
            posicionX    += incrementoX;
            posicionY    += incrementoY;
            posicionZ    += incrementoZ;
            giroY        += incrementoGiroY;
            inclinacionX += incrementoInclinacionX;
            pasoActual++;
        }

        // Aleteo
        if (pasoAlaActual >= pasosEntreAleteos)
        {
            poseAlaActual   = (poseAlaActual + 1) % totalPosesAla;
            anguloAlaActual = posesAla[poseAlaActual].anguloGrados;
            pasoAlaActual   = 0;
            CalcularIncrementoAla();
        }
        else
        {
            anguloAlaActual += incrementoAla;
            pasoAlaActual++;
        }
    }
};
