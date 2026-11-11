// Humo.h — Sistema de partículas para el humo del copal 
// Billboards instanciados + textura procedural en el shader para no necesita imagen
#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cmath>

struct ParticulaHumo {
    glm::vec3 pos;
    glm::vec3 vel;
    float vida;      // segundos vividos
    float vidaMax;   // segundos que dura
    float semilla;   // 0..1, da variación a cada bocanada
};

class Humo {
public:
    glm::vec3 origen;        // punto de donde sale el humo (centro del copal encendido)
    float escala;            // ajusta todo a las unidades de tu escena
    int   maxParticulas;
    float tasaEmision;       // partículas por segundo
    glm::vec3 color = glm::vec3(0.55f, 0.52f, 0.55f);   // gris tenue: la escena es de noche
    glm::vec3 colorNiebla = glm::vec3(0.0f);   // igual que la niebla de lighting.frag
    float densidadNiebla = 0.0f;

    Humo(glm::vec3 origen_, float escala_ = 1.0f, int maxP = 200, float tasa = 40.0f)
        : origen(origen_), escala(escala_), maxParticulas(maxP), tasaEmision(tasa)
    {
        particulas.reserve(maxP);
        datos.reserve(maxP * 6);

        // Quad unitario centrado (2 triángulos)
        float quad[] = { -0.5f,-0.5f,  0.5f,-0.5f,  0.5f, 0.5f,
                         -0.5f,-0.5f,  0.5f, 0.5f, -0.5f, 0.5f };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBOquad);
        glGenBuffers(1, &VBOinst);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBOquad);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        // Por instancia: pos.xyz, tamaño, alfa, semilla  (6 floats)
        glBindBuffer(GL_ARRAY_BUFFER, VBOinst);
        glBufferData(GL_ARRAY_BUFFER, maxP * 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glVertexAttribDivisor(1, 1);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(4 * sizeof(float)));
        glVertexAttribDivisor(2, 1);

        glBindVertexArray(0);
    }

    ~Humo() {
        glDeleteBuffers(1, &VBOquad);
        glDeleteBuffers(1, &VBOinst);
        glDeleteVertexArrays(1, &VAO);
    }

    // dt = deltaTime del frame, t = glfwGetTime()
    void Actualizar(float dt, float t) {
        // Emisión continua
        acumulador += dt * tasaEmision;
        while (acumulador >= 1.0f && (int)particulas.size() < maxParticulas) {
            Emitir();
            acumulador -= 1.0f;
        }
        if (acumulador > 1.0f) acumulador = 1.0f;

        for (size_t i = 0; i < particulas.size(); ) {
            ParticulaHumo& p = particulas[i];
            p.vida += dt;
            if (p.vida >= p.vidaMax) {           // muere: se reemplaza por la última
                particulas[i] = particulas.back();
                particulas.pop_back();
                continue;
            }
            float k = p.vida / p.vidaMax;         // 0 al nacer, 1 al morir

            // Turbulencia: el humo se ondula más mientras más sube
            glm::vec3 viento(std::sin(t * 1.3f + p.semilla * 6.2831f),
                             0.0f,
                             std::cos(t * 0.9f + p.semilla * 4.0f));
            p.vel += viento * (0.18f * k * escala) * dt;
            p.vel.y *= (1.0f - 0.25f * dt);      // pierde fuerza al enfriarse
            p.pos += p.vel * dt;
            ++i;
        }
    }

    // programa = humoShader.Program
    void Dibujar(GLuint programa, const glm::mat4& view, const glm::mat4& projection,
                 const glm::vec3& camPos, float t)
    {
        if (particulas.empty()) return;

        // Ordenar de atrás hacia adelante para que la transparencia se vea bien
        std::sort(particulas.begin(), particulas.end(),
            [&](const ParticulaHumo& a, const ParticulaHumo& b) {
                glm::vec3 da = a.pos - camPos, db = b.pos - camPos;
                return glm::dot(da, da) > glm::dot(db, db);
            });

        datos.clear();
        for (const ParticulaHumo& p : particulas) {
            float k = p.vida / p.vidaMax;
            float tam = escala * (0.06f + 0.45f * k);             // crece al subir
            float entrada = (std::min)(k / 0.15f, 1.0f);            // aparece suave
            float alfa = entrada * (1.0f - k) * 0.55f;            // y se desvanece
            datos.insert(datos.end(), { p.pos.x, p.pos.y, p.pos.z, tam, alfa, p.semilla });
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBOinst);
        glBufferSubData(GL_ARRAY_BUFFER, 0, datos.size() * sizeof(float), datos.data());

        glUseProgram(programa);
        glUniformMatrix4fv(glGetUniformLocation(programa, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(programa, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1f(glGetUniformLocation(programa, "tiempo"), t);
        glUniform3fv(glGetUniformLocation(programa, "colorHumo"), 1, glm::value_ptr(color));
        glUniform3fv(glGetUniformLocation(programa, "colorNiebla"), 1, glm::value_ptr(colorNiebla));
        glUniform1f(glGetUniformLocation(programa, "densidadNiebla"), densidadNiebla);
        glUniform3fv(glGetUniformLocation(programa, "viewPos"), 1, glm::value_ptr(camPos));

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);   // el humo no tapa a otras partículas en el z-buffer

        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)particulas.size());
        glBindVertexArray(0);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);     // quítalo si tu escena deja el blending activo siempre
    }

private:
    std::vector<ParticulaHumo> particulas;
    std::vector<float> datos;
    GLuint VAO = 0, VBOquad = 0, VBOinst = 0;
    float acumulador = 0.0f;

    static float Aleatorio(float a, float b) {
        return a + (b - a) * (std::rand() / (float)RAND_MAX);
    }

    void Emitir() {
        ParticulaHumo p;
        p.pos = origen + glm::vec3(Aleatorio(-0.02f, 0.02f), 0.0f, Aleatorio(-0.02f, 0.02f)) * escala;
        p.vel = glm::vec3(Aleatorio(-0.02f, 0.02f), Aleatorio(0.25f, 0.40f), Aleatorio(-0.02f, 0.02f)) * escala;
        p.vida = 0.0f;
        p.vidaMax = Aleatorio(2.5f, 4.0f);
        p.semilla = Aleatorio(0.0f, 1.0f);
        particulas.push_back(p);
    }
};
