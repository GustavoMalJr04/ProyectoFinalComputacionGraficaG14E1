#pragma once

// Librerías
#include <vector>
#include <string>
#include <algorithm>
#include <utility>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Model.h"
#include "Sendero.h"

// Luna
const glm::vec3 LUNA_AMBIENTE(0.24f, 0.25f, 0.30f);
const glm::vec3 LUNA_DIFUSA(0.38f, 0.40f, 0.48f);
const glm::vec3 LUNA_ESPECULAR(0.08f, 0.08f, 0.10f);

// Niebla
const glm::vec3 NIEBLA_COLOR(0.02f, 0.025f, 0.05f);
const float NIEBLA_DENSIDAD = 0.014f;

// Luz de velas
const glm::vec3 COLOR_VELA(1.0f, 0.55f, 0.20f);
const float BRILLO_VELAS = 1.3f;
const float INTENSIDAD_LUZ_VELA = 1.2f;
const float ALTURA_LUZ_VELAS = 2.5f;
const int   MAX_LUCES_VELA = 16;

// Mapa de luz de velas
const glm::vec2 MAPA_MIN(-60.0f, -60.0f);
const float MAPA_TAM = 120.0f;
const int   MAPA_RES = 512;
const float RADIO_RESPLANDOR = 3.0f;

// Linterna
bool linterna = false;

// Estructura farol
struct Farol
{
	glm::vec3 pos;
};

// Faroles
const int   MAX_FAROLES = 20;
const float ALTURA_POSTE = 2.4f;
const float ESCALA_FAROL = 1.4f;
const glm::vec3 COLOR_FAROL(1.0f, 0.78f, 0.45f);
const float INTENSIDAD_FAROL = 1.8f;
const glm::vec3 COLOR_HIERRO(0.10f, 0.09f, 0.08f);

// Posición de faroles
std::vector<Farol> faroles = {
	{ glm::vec3(-47.0f, 0.0f, -47.0f) }, { glm::vec3(7.0f, 0.0f, -47.0f) },
	{ glm::vec3(-47.0f, 0.0f, 44.0f) }, { glm::vec3(7.0f, 0.0f, 44.0f) },
	{ glm::vec3(-26.5f, 0.0f, 52.7f) }, { glm::vec3(-13.5f, 0.0f, 52.7f) },
	{ glm::vec3(27.5f, 0.0f, 52.7f) }, { glm::vec3(37.1f, 0.0f, 52.7f) },
	{ glm::vec3(-40.0f, 0.0f, 64.3f) }, { glm::vec3(-5.0f, 0.0f, 64.3f) },
	{ glm::vec3(25.0f, 0.0f, 64.3f) }
};

// Luz del farol
glm::vec3 luzFarol(const Farol& f)
{
	return f.pos + glm::vec3(0.0f, ALTURA_POSTE + 0.3f * ESCALA_FAROL, 0.0f);
}

// Parpadeo de vela
float parpadeoVela(const Vela& v, float tiempo)
{
	return 1.0f + 0.08f * sin(tiempo * 12.0f + v.fase)
		+ 0.04f * sin(tiempo * 23.0f + 2.0f * v.fase);
}

// Crear mapa de luz
GLuint crearMapaLuzVelas()
{
	std::vector<float> luz(MAPA_RES * MAPA_RES, 0.0f);
	const float texel = MAPA_TAM / MAPA_RES;
	const int rt = (int)(RADIO_RESPLANDOR / texel) + 1;

	for (const Vela& v : velas)
	{
		float fuerza = v.sXZ / VELA_GRANDE_XZ;
		int cx = (int)((v.pos.x - MAPA_MIN.x) / texel);
		int cz = (int)((v.pos.z - MAPA_MIN.y) / texel);
		for (int j = cz - rt; j <= cz + rt; j++)
			for (int i = cx - rt; i <= cx + rt; i++)
			{
				if (i < 0 || j < 0 || i >= MAPA_RES || j >= MAPA_RES) continue;
				float wx = MAPA_MIN.x + (i + 0.5f) * texel - v.pos.x;
				float wz = MAPA_MIN.y + (j + 0.5f) * texel - v.pos.z;
				float r = sqrt(wx * wx + wz * wz);
				if (r >= RADIO_RESPLANDOR) continue;
				float t = 1.0f - r / RADIO_RESPLANDOR;
				luz[j * MAPA_RES + i] += fuerza * t * t;
			}
	}
	for (float& l : luz) l = 1.0f - exp(-l);

	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, MAPA_RES, MAPA_RES, 0, GL_RED, GL_FLOAT, luz.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	return id;
}

// Luces de velas cercanas
void mandarLucesVelas(GLuint programa, glm::vec3 posCam, float tiempo)
{
	std::vector<std::pair<float, int>> orden;
	orden.reserve(velas.size());
	for (int i = 0; i < (int)velas.size(); i++)
		orden.push_back(std::make_pair(distancia2(velas[i].pos, posCam), i));

	int n = std::min((int)orden.size(), MAX_LUCES_VELA);
	std::partial_sort(orden.begin(), orden.begin() + n, orden.end());
	float dCorte = ((int)orden.size() > n) ? sqrt(orden[n].first) : 1e6f;

	glm::vec3 pos[MAX_LUCES_VELA];
	float inten[MAX_LUCES_VELA];
	for (int k = 0; k < n; k++)
	{
		const Vela& v = velas[orden[k].second];
		float d = sqrt(orden[k].first);
		float desvanecer = glm::clamp((dCorte - d) / 2.0f, 0.0f, 1.0f);
		pos[k] = v.pos + glm::vec3(0.0f, v.sY + 0.12f * v.sXZ, 0.0f);
		inten[k] = INTENSIDAD_LUZ_VELA * (v.sXZ / VELA_GRANDE_XZ) * parpadeoVela(v, tiempo) * desvanecer;
	}

	glUniform1i(glGetUniformLocation(programa, "numLucesVela"), n);
	if (n > 0)
	{
		glUniform3fv(glGetUniformLocation(programa, "luzVelaPos"), n, glm::value_ptr(pos[0]));
		glUniform1fv(glGetUniformLocation(programa, "luzVelaInt"), n, inten);
	}
}

// Configurar noche
GLuint configurarNoche(GLuint programa)
{
	GLuint mapaLuzVelas = crearMapaLuzVelas();
	glUniform1i(glGetUniformLocation(programa, "mapaVelas"), 5);
	glUniform2f(glGetUniformLocation(programa, "mapaMin"), MAPA_MIN.x, MAPA_MIN.y);
	glUniform1f(glGetUniformLocation(programa, "mapaTam"), MAPA_TAM);
	glUniform1f(glGetUniformLocation(programa, "alturaLuzVelas"), ALTURA_LUZ_VELAS);
	glUniform3f(glGetUniformLocation(programa, "colorVela"), COLOR_VELA.x, COLOR_VELA.y, COLOR_VELA.z);
	glUniform3f(glGetUniformLocation(programa, "colorNiebla"), NIEBLA_COLOR.x, NIEBLA_COLOR.y, NIEBLA_COLOR.z);
	glUniform1f(glGetUniformLocation(programa, "densidadNiebla"), NIEBLA_DENSIDAD);
	glUniform1i(glGetUniformLocation(programa, "emisivo"), 0);
	glUniform1i(glGetUniformLocation(programa, "usarColor"), 0);

	{
		// Luz de faroles
		int n = std::min((int)faroles.size(), MAX_FAROLES);
		glm::vec3 posFaroles[MAX_FAROLES];
		for (int i = 0; i < n; i++) posFaroles[i] = luzFarol(faroles[i]);
		glUniform1i(glGetUniformLocation(programa, "numFaroles"), n);
		glUniform3fv(glGetUniformLocation(programa, "farolPos"), n, glm::value_ptr(posFaroles[0]));
		glUniform3f(glGetUniformLocation(programa, "colorFarol"), COLOR_FAROL.x, COLOR_FAROL.y, COLOR_FAROL.z);
		glUniform1f(glGetUniformLocation(programa, "intensidadFarol"), INTENSIDAD_FAROL);
	}
	return mapaLuzVelas;
}

// Actualizar luces
void actualizarLuces(GLuint programa, glm::vec3 posCam, glm::vec3 frente, float tiempo, GLuint mapaLuzVelas)
{
	// Luna
	glUniform3f(glGetUniformLocation(programa, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
	glUniform3f(glGetUniformLocation(programa, "dirLight.ambient"), LUNA_AMBIENTE.x, LUNA_AMBIENTE.y, LUNA_AMBIENTE.z);
	glUniform3f(glGetUniformLocation(programa, "dirLight.diffuse"), LUNA_DIFUSA.x, LUNA_DIFUSA.y, LUNA_DIFUSA.z);
	glUniform3f(glGetUniformLocation(programa, "dirLight.specular"), LUNA_ESPECULAR.x, LUNA_ESPECULAR.y, LUNA_ESPECULAR.z);

	// Velas
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, mapaLuzVelas);
	glActiveTexture(GL_TEXTURE0);
	float pulso = 1.0f + 0.04f * sin(tiempo * 9.0f) + 0.03f * sin(tiempo * 17.0f);
	glUniform1f(glGetUniformLocation(programa, "brilloVelas"), BRILLO_VELAS * pulso);
	mandarLucesVelas(programa, posCam, tiempo);

	// Linterna
	glUniform3f(glGetUniformLocation(programa, "spotLight.position"), posCam.x, posCam.y, posCam.z);
	glUniform3f(glGetUniformLocation(programa, "spotLight.direction"), frente.x, frente.y, frente.z);
	float lin = linterna ? 1.0f : 0.0f;
	glUniform3f(glGetUniformLocation(programa, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(programa, "spotLight.diffuse"), 0.9f * lin, 0.85f * lin, 0.7f * lin);
	glUniform3f(glGetUniformLocation(programa, "spotLight.specular"), 0.2f * lin, 0.2f * lin, 0.2f * lin);
	glUniform1f(glGetUniformLocation(programa, "spotLight.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programa, "spotLight.linear"), 0.09f);
	glUniform1f(glGetUniformLocation(programa, "spotLight.quadratic"), 0.032f);
	glUniform1f(glGetUniformLocation(programa, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
	glUniform1f(glGetUniformLocation(programa, "spotLight.outerCutOff"), glm::cos(glm::radians(18.0f)));
}

// Dibujar faroles
void dibujarFaroles(Shader& shader, GLint modelLoc, GLuint cuboVAO, Model& farol)
{
	GLint usarColorLoc = glGetUniformLocation(shader.Program, "usarColor");
	glUniform1i(usarColorLoc, 1);
	glUniform3f(glGetUniformLocation(shader.Program, "colorPlano"), COLOR_HIERRO.x, COLOR_HIERRO.y, COLOR_HIERRO.z);
	for (const Farol& f : faroles)
	{
		glBindVertexArray(cuboVAO);

		// Base
		glm::mat4 mf = glm::translate(glm::mat4(1.0f), f.pos + glm::vec3(0.0f, 0.1f, 0.0f));
		mf = glm::scale(mf, glm::vec3(0.36f, 0.2f, 0.36f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mf));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Poste
		mf = glm::translate(glm::mat4(1.0f), f.pos + glm::vec3(0.0f, ALTURA_POSTE * 0.5f, 0.0f));
		mf = glm::scale(mf, glm::vec3(0.1f, ALTURA_POSTE, 0.1f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mf));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		// Cabeza
		mf = glm::translate(glm::mat4(1.0f), f.pos + glm::vec3(0.0f, ALTURA_POSTE, 0.0f));
		mf = glm::scale(mf, glm::vec3(ESCALA_FAROL));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mf));
		farol.Draw(shader);
	}
	glUniform1i(usarColorLoc, 0);
}

// Dibujar llamas
void dibujarLlamas(Shader& shader, Model& llama, glm::vec3 posCam, float tiempo)
{
	const float dMax2 = DISTANCIA_ADORNOS * DISTANCIA_ADORNOS;
	GLint modelLoc = glGetUniformLocation(shader.Program, "model");
	GLint emisivoLoc = glGetUniformLocation(shader.Program, "emisivo");
	glUniform1i(emisivoLoc, 1);
	glUniform1i(glGetUniformLocation(shader.Program, "transparency"), 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE);

	// Llamas de velas
	for (const Vela& v : velas)
	{
		if (distancia2(v.pos, posCam) > dMax2) continue;
		float flick = parpadeoVela(v, tiempo);
		float vaiven = 4.0f * sin(tiempo * 3.0f + v.fase);

		glm::mat4 l = glm::translate(glm::mat4(1.0f), v.pos + glm::vec3(0.0f, v.sY, 0.0f));
		l = glm::rotate(l, glm::radians(vaiven), glm::vec3(0.0f, 0.0f, 1.0f));
		l = glm::scale(l, glm::vec3(v.sXZ, v.sXZ * flick, v.sXZ));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(l));
		llama.Draw(shader);
	}

	// Llamas de faroles
	for (const Farol& f : faroles)
	{
		float flick = 1.0f + 0.05f * sin(tiempo * 10.0f + f.pos.x);
		glm::mat4 l = glm::translate(glm::mat4(1.0f), f.pos + glm::vec3(0.0f, ALTURA_POSTE + 0.12f * ESCALA_FAROL, 0.0f));
		l = glm::scale(l, glm::vec3(1.3f, 1.3f * flick, 1.3f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(l));
		llama.Draw(shader);
	}
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glUniform1i(emisivoLoc, 0);
	glUniform1i(glGetUniformLocation(shader.Program, "transparency"), 0);
}
