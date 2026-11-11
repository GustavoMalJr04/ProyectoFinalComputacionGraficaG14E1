#pragma once

// Librerías
#include <vector>
#include <string>
#include <cmath>
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SOIL2/SOIL2.h"

// Fichas culturales
class FichasCulturales
{
public:

	// Ajustes
	float ALTURA_MARCADOR = 2.4f;
	float TAM_MARCADOR = 0.7f;
	float ALTO_FICHA = 1.75f;
	float OSCURECER = 0.65f;

	// Cargar
	bool cargar(const std::string& carpeta = "Imagenes")
	{
		this->carpeta = carpeta;
		compilarShader();
		crearQuad();

		// Textura blanca
		unsigned char blanco[4] = { 255, 255, 255, 255 };
		glGenTextures(1, &texBlanca);
		glBindTexture(GL_TEXTURE_2D, texBlanca);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blanco);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		return programa != 0;
	}

	// Agregar punto de interés
	void agregarPunto(const std::string& nombre, glm::vec3 pos, float radio,
		const std::vector<std::string>& archivos)
	{
		Punto p;
		p.nombre = nombre;
		p.pos = pos;
		p.radio = radio;
		for (const std::string& a : archivos)
		{
			Ficha f;
			if (cargarTextura(carpeta + "/" + a, f)) p.fichas.push_back(f);
		}
		if (p.fichas.empty())
		{
			std::cout << "[Fichas] El punto " << nombre << " no tiene fichas validas" << std::endl;
			return;
		}
		puntos.push_back(p);
	}

	// Actualizar
	void actualizar(glm::vec3 posJugador, float dt)
	{
		int antes = cercano;
		cercano = -1;
		float mejor = 1e9f;
		for (int i = 0; i < (int)puntos.size(); i++)
		{
			glm::vec2 d(posJugador.x - puntos[i].pos.x, posJugador.z - puntos[i].pos.z);
			float dist = glm::length(d);
			if (dist < puntos[i].radio && dist < mejor) { mejor = dist; cercano = i; }
		}
		if (cercano != antes && cercano >= 0)
			std::cout << "[Fichas] " << puntos[cercano].nombre << ": presiona Q para ver la ficha" << std::endl;

		// Cerrar si se aleja
		if (abiertoEn >= 0 && cercano != abiertoEn) cerrar();
	}

	// Teclas
	void tecla(int key)
	{
		// Q abre o pasa a la siguiente
		if (key == GLFW_KEY_Q)
		{
			if (abiertoEn < 0)
			{
				if (cercano >= 0) { abiertoEn = cercano; pagina = 0; }
			}
			else
			{
				pagina++;
				if (pagina >= (int)puntos[abiertoEn].fichas.size()) cerrar();
			}
		}
		// E cierra
		else if (key == GLFW_KEY_E)
		{
			cerrar();
		}
	}

	// Ficha abierta
	bool abierta() const { return abiertoEn >= 0; }

	// Dibujar marcadores
	void dibujarMarcadores(const glm::mat4& view, const glm::mat4& projection,
		glm::vec3 posCam, float tiempo)
	{
		if (puntos.empty()) return;
		empezar();
		glEnable(GL_DEPTH_TEST);

		for (int i = 0; i < (int)puntos.size(); i++)
		{
			const Punto& p = puntos[i];
			const Ficha& f = p.fichas[0];
			bool cerca = (i == cercano);

			glm::vec3 centro = p.pos + glm::vec3(0.0f, ALTURA_MARCADOR + 0.12f * sinf(tiempo * 2.0f + i), 0.0f);
			float yaw = atan2f(posCam.x - centro.x, posCam.z - centro.z);
			float escala = TAM_MARCADOR;

			glm::mat4 m(1.0f);
			m = glm::translate(m, centro);
			m = glm::rotate(m, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
			m = glm::scale(m, glm::vec3(escala / f.aspecto, escala, 1.0f));

			glm::vec3 tinte = cerca ? glm::vec3(1.25f, 1.1f, 0.8f) : glm::vec3(0.85f);
			dibujarQuad(projection * view * m, f.tex, tinte, 1.0f);
		}
		terminar();
	}

	// Dibujar ficha en pantalla
	void dibujarHUD(int ancho, int alto, float tiempo)
	{
		if (abiertoEn < 0 || alto <= 0) return;
		empezar();
		glDisable(GL_DEPTH_TEST);
		float pantalla = (float)ancho / (float)alto;
		const Ficha& f = puntos[abiertoEn].fichas[pagina];

		// Fondo oscuro
		dibujarQuad(glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)), texBlanca, glm::vec3(0.0f), OSCURECER);

		// Ficha centrada
		float h = ALTO_FICHA;
		float w = h / f.aspecto / pantalla;
		if (w > 1.9f) { h *= 1.9f / w; w = 1.9f; }
		glm::mat4 m = glm::scale(glm::mat4(1.0f), glm::vec3(w, h, 1.0f));
		dibujarQuad(m, f.tex, glm::vec3(1.0f), 1.0f);

		glEnable(GL_DEPTH_TEST);
		terminar();
	}

private:

	// Estructuras
	struct Ficha { GLuint tex = 0; float aspecto = 1.0f; };
	struct Punto
	{
		std::string nombre;
		glm::vec3 pos;
		float radio;
		std::vector<Ficha> fichas;
	};

	// Estado
	std::string carpeta;
	std::vector<Punto> puntos;
	int cercano = -1;
	int abiertoEn = -1;
	int pagina = 0;

	// OpenGL
	GLuint programa = 0, vao = 0, vbo = 0, texBlanca = 0;
	GLint locMvp = -1, locTinte = -1, locAlfa = -1, locTex = -1;
	GLint programaAnterior = 0;
	GLboolean habiaCulling = GL_FALSE;

	// Cerrar
	void cerrar() { abiertoEn = -1; pagina = 0; }


	// Cargar textura
	bool cargarTextura(const std::string& ruta, Ficha& f)
	{
		int w, h;
		unsigned char* img = SOIL_load_image(ruta.c_str(), &w, &h, 0, SOIL_LOAD_RGBA);
		if (!img)
		{
			std::cout << "[Fichas] ERROR: no se encontro " << ruta << std::endl;
			return false;
		}
		glGenTextures(1, &f.tex);
		glBindTexture(GL_TEXTURE_2D, f.tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		SOIL_free_image_data(img);
		f.aspecto = (float)h / (float)w;
		return true;
	}

	// Crear quad
	void crearQuad()
	{
		float v[] = {
			-0.5f, -0.5f, 0.0f, 1.0f,
			 0.5f, -0.5f, 1.0f, 1.0f,
			 0.5f,  0.5f, 1.0f, 0.0f,
			-0.5f, -0.5f, 0.0f, 1.0f,
			 0.5f,  0.5f, 1.0f, 0.0f,
			-0.5f,  0.5f, 0.0f, 0.0f,
		};
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);
	}

	// Compilar etapa del shader
	GLuint compilarEtapa(GLenum tipo, const char* src)
	{
		GLuint s = glCreateShader(tipo);
		glShaderSource(s, 1, &src, nullptr);
		glCompileShader(s);
		GLint ok;
		glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
		if (!ok)
		{
			char log[512];
			glGetShaderInfoLog(s, 512, nullptr, log);
			std::cout << "[Fichas] ERROR shader: " << log << std::endl;
		}
		return s;
	}

	// Compilar shader
	void compilarShader()
	{
		const char* vs =
			"#version 330 core\n"
			"layout (location = 0) in vec2 aPos;\n"
			"layout (location = 1) in vec2 aUV;\n"
			"uniform mat4 mvp;\n"
			"out vec2 uv;\n"
			"void main() { uv = aUV; gl_Position = mvp * vec4(aPos, 0.0, 1.0); }\n";
		const char* fs =
			"#version 330 core\n"
			"in vec2 uv;\n"
			"uniform sampler2D tex;\n"
			"uniform vec3 tinte;\n"
			"uniform float alfa;\n"
			"out vec4 color;\n"
			"void main() {\n"
			"  vec4 c = texture(tex, uv) * vec4(tinte, alfa);\n"
			"  if (c.a < 0.01) discard;\n"
			"  color = c;\n"
			"}\n";
		GLuint v = compilarEtapa(GL_VERTEX_SHADER, vs);
		GLuint f = compilarEtapa(GL_FRAGMENT_SHADER, fs);
		programa = glCreateProgram();
		glAttachShader(programa, v);
		glAttachShader(programa, f);
		glLinkProgram(programa);
		glDeleteShader(v);
		glDeleteShader(f);
		locMvp = glGetUniformLocation(programa, "mvp");
		locTinte = glGetUniformLocation(programa, "tinte");
		locAlfa = glGetUniformLocation(programa, "alfa");
		locTex = glGetUniformLocation(programa, "tex");
	}

	// Empezar dibujo
	void empezar()
	{
		glGetIntegerv(GL_CURRENT_PROGRAM, &programaAnterior);
		glUseProgram(programa);
		glUniform1i(locTex, 0);
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		habiaCulling = glIsEnabled(GL_CULL_FACE);
		glDisable(GL_CULL_FACE);
		glBindVertexArray(vao);
	}

	// Terminar dibujo
	void terminar()
	{
		glBindVertexArray(0);
		glDisable(GL_BLEND);
		if (habiaCulling) glEnable(GL_CULL_FACE);
		glUseProgram(programaAnterior);
	}

	// Dibujar quad
	void dibujarQuad(const glm::mat4& mvp, GLuint tex, glm::vec3 tinte, float alfa)
	{
		glUniformMatrix4fv(locMvp, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniform3f(locTinte, tinte.x, tinte.y, tinte.z);
		glUniform1f(locAlfa, alfa);
		glBindTexture(GL_TEXTURE_2D, tex);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
};
