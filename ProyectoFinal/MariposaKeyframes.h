#pragma once
// =====================================================================
//  MariposaKeyframes.h
//  Mariposa monarca animada por CUADROS CLAVE (keyframes)
//  Proyecto Día de Muertos - Equipo 1 - Computación Gráfica, FI UNAM
// ---------------------------------------------------------------------
//  ¿QUÉ HACE ESTE ARCHIVO?
//
//  1) VUELO (cuadros clave de trayectoria)
//     Cada cuadro clave guarda una "foto" de la mariposa:
//        posición (X, Y, Z) + giro en Y + inclinación en X.
//     Entre un cuadro clave y el siguiente se hace INTERPOLACIÓN LINEAL:
//        incremento = (valorDestino - valorOrigen) / pasosEntreCuadros
//     y en cada paso se suma ese incremento. Al llegar al último cuadro
//     clave regresa al primero, así el vuelo se repite sin fin.
//
//  2) ALETEO (cuadros clave de las alas)
//     De 2 a 8 poses (ángulo de las alas). También se interpolan,
//     pero con muy pocos pasos, para que aletee rápido.
//
//  ARCHIVO DE CUADROS CLAVE (assets/models/mariposa/vuelo_*.txt)
//     # comentario
//     ala -20                    <- cuadro clave del aleteo
//     ala 70
//     -1.00 5.00 -27.00 180 0    <- cuadro clave del vuelo: x y z giroY inclinacionX
//     CargarDesdeArchivo() lo lee y GuardarEnArchivo() lo escribe.
//
//  3) MODELADO JERÁRQUICO
//     cuerpo (padre)  ->  ala izquierda y ala derecha (hijas)
//     La matriz de cada ala = matriz del cuerpo × rotación de la bisagra,
//     por eso las alas siguen al cuerpo a donde vaya.
//
//  REQUISITOS DE LOS .obj (Blender o los de respaldo):
//     - Cabeza hacia -Z (el "frente" de OpenGL)
//     - Ala derecha en x > 0, ala izquierda en x < 0
//     - La bisagra de las alas sobre el eje Z (x = 0, y = 0)
// =====================================================================

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

// ---------------------------------------------------------------------
//  Constantes
// ---------------------------------------------------------------------
const int   MAX_CUADROS_CLAVE      = 40;     // máximo de cuadros clave del vuelo
const int   MAX_POSES_ALA          = 8;      // máximo de cuadros clave del aleteo
const float DISTANCIA_FRENTE_CAMARA = 1.0f;  // a cuántos metros de la cámara se guarda un cuadro

// ---------------------------------------------------------------------
//  Un cuadro clave del vuelo
// ---------------------------------------------------------------------
struct CuadroClave
{
    float posicionX;      // metros
    float posicionY;      // metros (altura)
    float posicionZ;      // metros
    float giroY;          // grados. 0 = mira hacia -Z, 90 = mira hacia -X
    float inclinacionX;   // grados. + = nariz arriba, - = nariz abajo
};

// ---------------------------------------------------------------------
//  Una pose de las alas
// ---------------------------------------------------------------------
struct PoseAla
{
    float anguloGrados;   // + = ala levantada, - = ala bajada
};

// =====================================================================
//  Clase principal
// =====================================================================
class MariposaKeyframes
{
public:
    // ------------------- Parámetros que puedes ajustar -------------------
    int   pasosEntreCuadros     = 120;    // pasos para ir de un cuadro clave al siguiente (120 = 2 s)
    int   pasosEntreAleteos     = 5;      // pasos entre "abajo" y "arriba" (5 → unos 6 aleteos por segundo)
    float pasosPorSegundo       = 60.0f;  // ritmo fijo de la animación (no depende de los FPS)
    float escala                = 0.10f;  // tamaño final del modelo
    float correccionGiroGrados  = 0.0f;   // pon 180 si la mariposa vuela "de reversa"
    float bisagraX              = 0.0f;   // si la bisagra del ala no quedó en x = 0
    glm::vec3 desplazamiento    = glm::vec3(0.0f); // se suma a la trayectoria (para que varias
                                                   // mariposas compartan el mismo archivo sin encimarse)

    // ------------------- Constructor -------------------
    // Recibe los tres modelos ya cargados (se pueden compartir entre varias mariposas)
    MariposaKeyframes(Model* modeloCuerpo, Model* modeloAlaIzquierda, Model* modeloAlaDerecha)
        : cuerpo(modeloCuerpo), alaIzquierda(modeloAlaIzquierda), alaDerecha(modeloAlaDerecha)
    {
        ConfigurarAleteo(-15.0f, 75.0f);
    }

    // ------------------- Aleteo -------------------
    // Ángulos (en grados) de las dos poses de las alas
    void ConfigurarAleteo(float anguloAbajo, float anguloArriba)
    {
        totalPosesAla = 0;
        AgregarPoseAla(anguloAbajo);
        AgregarPoseAla(anguloArriba);
        ReiniciarAleteo();
    }

    // Agrega un cuadro clave del aleteo (ángulo de las alas en grados)
    void AgregarPoseAla(float anguloGrados)
    {
        if (totalPosesAla >= MAX_POSES_ALA) return;
        posesAla[totalPosesAla].anguloGrados = anguloGrados;
        totalPosesAla++;
    }

    // Empieza el aleteo desde la primera pose
    void ReiniciarAleteo()
    {
        if (totalPosesAla < 2) ConfigurarAleteoPorDefecto();
        poseAlaActual     = 0;
        pasoAlaActual     = 0;
        anguloAlaActual   = posesAla[0].anguloGrados;
        CalcularIncrementoAla();
    }

    // ------------------- Cuadros clave -------------------
    // Agrega un cuadro clave escrito a mano en main.cpp
    void AgregarCuadroClave(float x, float y, float z, float giroY, float inclinacionX)
    {
        if (totalCuadrosClave >= MAX_CUADROS_CLAVE)
        {
            printf("[Mariposa] Ya no caben mas cuadros clave (maximo %d)\n", MAX_CUADROS_CLAVE);
            return;
        }
        cuadrosClave[totalCuadrosClave] = { x, y, z, giroY, inclinacionX };
        totalCuadrosClave++;

        // Con el primer cuadro clave la mariposa ya tiene dónde aparecer
        if (totalCuadrosClave == 1) ColocarEnCuadroClave(0);
    }

    // Guarda un cuadro clave 1 m frente a la cámara, mirando a donde mira la cámara.
    // Imprime en la consola la línea de C++ lista para copiar a main.cpp.
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

    // Borra todos los cuadros clave (para grabar unos nuevos)
    void BorrarCuadrosClave()
    {
        totalCuadrosClave = 0;
        reproduciendo = false;
        printf("[Mariposa] Cuadros clave borrados\n");
    }

    // Imprime todos los cuadros clave actuales (para copiarlos a main.cpp)
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

    int  TotalCuadrosClave() const { return totalCuadrosClave; }
    bool EstaReproduciendo() const { return reproduciendo; }

    // ------------------- Archivo de cuadros clave -------------------
    // Formato: una línea por cuadro clave ->  x  y  z  giroY  inclinacionX
    // Las líneas que empiezan con '#' son comentarios.

    // Guarda los cuadros clave actuales en un archivo de texto
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

    // Borra los cuadros actuales y carga los del archivo. Si estaba volando, sigue volando.
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

            if (linea.compare(0, 3, "ala") == 0)          // cuadro clave del aleteo
            {
                std::string etiqueta;
                float angulo;
                if (datos >> etiqueta >> angulo)
                {
                    if (posesLeidas == 0) totalPosesAla = 0;   // el archivo reemplaza las poses por defecto
                    AgregarPoseAla(angulo);
                    posesLeidas++;
                }
                continue;
            }

            float x, y, z, giro, inclinacion;               // cuadro clave del vuelo
            if (datos >> x >> y >> z >> giro >> inclinacion)
                AgregarCuadroClave(x, y, z, giro, inclinacion);
        }
        ReiniciarAleteo();
        printf("[Mariposa] %s: %d cuadros de vuelo, %d de aleteo\n", ruta, totalCuadrosClave, totalPosesAla);

        if (seguiaVolando && totalCuadrosClave >= 2) ReproducirDesde(0);
        return totalCuadrosClave >= 2;
    }

    // Arranca el vuelo desde un cuadro clave cualquiera (sirve para que
    // varias mariposas con la misma ruta vayan repartidas y no juntas)
    void ReproducirDesde(int cuadroInicial)
    {
        if (totalCuadrosClave < 2) return;
        cuadroActual = cuadroInicial % totalCuadrosClave;
        pasoActual   = 0;
        acumuladorTiempo = 0.0f;
        CalcularIncrementos();
        reproduciendo = true;
    }

    // ------------------- Reproducción -------------------
    // Inicia la animación desde el primer cuadro clave, o la detiene si ya estaba corriendo
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
        CalcularIncrementos();          // también coloca la mariposa en el cuadro 0
        reproduciendo = true;
        printf("[Mariposa] Animacion iniciada con %d cuadros clave\n", totalCuadrosClave);
    }

    // Llamar UNA vez por frame, con el deltaTime del ciclo principal
    void Actualizar(float deltaTime)
    {
        if (!reproduciendo) return;

        acumuladorTiempo += deltaTime;
        if (acumuladorTiempo > 0.25f) acumuladorTiempo = 0.25f;  // si el programa se trabó, no dar un salto enorme

        const float duracionPaso = 1.0f / pasosPorSegundo;
        while (acumuladorTiempo >= duracionPaso)
        {
            AvanzarUnPaso();
            acumuladorTiempo -= duracionPaso;
        }
    }

    // ------------------- Dibujo -------------------
    // Llamar con el shader de iluminación activo (lightingShader.Use() ya hecho),
    // DESPUÉS de los objetos opacos y ANTES de las llamas.
    void Dibujar(Shader& shader)
    {
        if (totalCuadrosClave == 0) return;   // todavía no tiene posición

        GLuint programa   = shader.Program;
        GLint  ubicModelo = glGetUniformLocation(programa, "model");

        // Las alas son PNG con transparencia: se descartan los píxeles invisibles
        // y se desactiva el culling para ver el ala por arriba y por abajo.
        glUniform1i(glGetUniformLocation(programa, "transparency"), 1);
        glUniform1f(glGetUniformLocation(programa, "specularStrength"), 0.05f);
        GLboolean cullingEstabaActivo = glIsEnabled(GL_CULL_FACE);
        glDisable(GL_CULL_FACE);

        // ---------- CUERPO (padre) ----------
        // Orden de lectura: primero escala, luego inclina, luego gira, al final traslada.
        glm::mat4 matrizCuerpo(1.0f);
        matrizCuerpo = glm::translate(matrizCuerpo, glm::vec3(posicionX, posicionY, posicionZ) + desplazamiento);
        matrizCuerpo = glm::rotate(matrizCuerpo, glm::radians(giroY + correccionGiroGrados), glm::vec3(0.0f, 1.0f, 0.0f));
        matrizCuerpo = glm::rotate(matrizCuerpo, glm::radians(inclinacionX), glm::vec3(1.0f, 0.0f, 0.0f));
        matrizCuerpo = glm::scale(matrizCuerpo, glm::vec3(escala));

        glUniformMatrix4fv(ubicModelo, 1, GL_FALSE, glm::value_ptr(matrizCuerpo));
        cuerpo->Draw(shader);

        float anguloAla = glm::radians(anguloAlaActual);

        // ---------- ALA DERECHA (hija, x > 0) ----------
        // Ángulo positivo alrededor de Z levanta las puntas que están en x > 0
        glm::mat4 matrizAlaDerecha = matrizCuerpo;
        matrizAlaDerecha = glm::translate(matrizAlaDerecha, glm::vec3(bisagraX, 0.0f, 0.0f));
        matrizAlaDerecha = glm::rotate(matrizAlaDerecha, anguloAla, glm::vec3(0.0f, 0.0f, 1.0f));
        matrizAlaDerecha = glm::translate(matrizAlaDerecha, glm::vec3(-bisagraX, 0.0f, 0.0f));
        glUniformMatrix4fv(ubicModelo, 1, GL_FALSE, glm::value_ptr(matrizAlaDerecha));
        alaDerecha->Draw(shader);

        // ---------- ALA IZQUIERDA (hija, x < 0) ----------
        // Para levantar puntas que están en x < 0 el ángulo va con signo contrario
        glm::mat4 matrizAlaIzquierda = matrizCuerpo;
        matrizAlaIzquierda = glm::translate(matrizAlaIzquierda, glm::vec3(-bisagraX, 0.0f, 0.0f));
        matrizAlaIzquierda = glm::rotate(matrizAlaIzquierda, -anguloAla, glm::vec3(0.0f, 0.0f, 1.0f));
        matrizAlaIzquierda = glm::translate(matrizAlaIzquierda, glm::vec3(bisagraX, 0.0f, 0.0f));
        glUniformMatrix4fv(ubicModelo, 1, GL_FALSE, glm::value_ptr(matrizAlaIzquierda));
        alaIzquierda->Draw(shader);

        // Dejar el estado como estaba para el resto de la escena
        if (cullingEstabaActivo) glEnable(GL_CULL_FACE);
        glUniform1i(glGetUniformLocation(programa, "transparency"), 0);
        glUniform1f(glGetUniformLocation(programa, "specularStrength"), 0.15f);
    }

private:
    // ------------------- Modelos -------------------
    Model* cuerpo;
    Model* alaIzquierda;
    Model* alaDerecha;

    // ------------------- Cuadros clave del vuelo -------------------
    CuadroClave cuadrosClave[MAX_CUADROS_CLAVE];
    int  totalCuadrosClave = 0;
    int  cuadroActual      = 0;     // de cuál cuadro clave va saliendo
    int  pasoActual        = 0;     // cuántos pasos lleva en el tramo actual
    bool reproduciendo     = false;
    float acumuladorTiempo = 0.0f;

    // Estado actual (lo que se dibuja en pantalla)
    float posicionX = 0.0f, posicionY = 0.0f, posicionZ = 0.0f;
    float giroY = 0.0f, inclinacionX = 0.0f;

    // Cuánto cambia cada valor en un paso
    float incrementoX = 0.0f, incrementoY = 0.0f, incrementoZ = 0.0f;
    float incrementoGiroY = 0.0f, incrementoInclinacionX = 0.0f;

    // ------------------- Cuadros clave de las alas -------------------
    PoseAla posesAla[MAX_POSES_ALA];
    int   totalPosesAla     = 0;
    int   poseAlaActual     = 0;
    int   pasoAlaActual     = 0;
    float anguloAlaActual   = 0.0f;
    float incrementoAla     = 0.0f;

    // ------------------- Funciones internas -------------------

    // Diferencia de ángulos por el camino más corto.
    // Ejemplo: de 170° a -170° son 20°, no -340° (si no, daría una vuelta completa).
    static float DiferenciaAnguloCorta(float desde, float hasta)
    {
        float diferencia = hasta - desde;
        while (diferencia > 180.0f)  diferencia -= 360.0f;
        while (diferencia < -180.0f) diferencia += 360.0f;
        return diferencia;
    }

    // Pone la mariposa exactamente en el cuadro clave 'indice'
    void ColocarEnCuadroClave(int indice)
    {
        posicionX    = cuadrosClave[indice].posicionX;
        posicionY    = cuadrosClave[indice].posicionY;
        posicionZ    = cuadrosClave[indice].posicionZ;
        giroY        = cuadrosClave[indice].giroY;
        inclinacionX = cuadrosClave[indice].inclinacionX;
    }

    // INTERPOLACIÓN: calcula cuánto avanzar en cada paso para ir
    // del cuadro clave actual al siguiente.
    void CalcularIncrementos()
    {
        int siguiente = (cuadroActual + 1) % totalCuadrosClave;   // después del último, el 0
        ColocarEnCuadroClave(cuadroActual);                       // así no se acumula error

        const CuadroClave& destino = cuadrosClave[siguiente];
        incrementoX            = (destino.posicionX - posicionX) / pasosEntreCuadros;
        incrementoY            = (destino.posicionY - posicionY) / pasosEntreCuadros;
        incrementoZ            = (destino.posicionZ - posicionZ) / pasosEntreCuadros;
        incrementoGiroY        = DiferenciaAnguloCorta(giroY, destino.giroY) / pasosEntreCuadros;
        incrementoInclinacionX = (destino.inclinacionX - inclinacionX) / pasosEntreCuadros;
    }

    void ConfigurarAleteoPorDefecto()
    {
        totalPosesAla = 0;
        AgregarPoseAla(-15.0f);
        AgregarPoseAla(75.0f);
    }

    void CalcularIncrementoAla()
    {
        int siguientePose = (poseAlaActual + 1) % totalPosesAla;
        incrementoAla = (posesAla[siguientePose].anguloGrados - posesAla[poseAlaActual].anguloGrados)
                        / pasosEntreAleteos;
    }

    // Un paso de la animación (se llama pasosPorSegundo veces por segundo)
    void AvanzarUnPaso()
    {
        // ---------- Vuelo ----------
        if (pasoActual >= pasosEntreCuadros)
        {
            // Terminó el tramo: el siguiente cuadro clave pasa a ser el de origen
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

        // ---------- Aleteo ----------
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
