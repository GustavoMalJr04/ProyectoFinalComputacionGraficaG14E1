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
#include "Mesh.h"
#include "Shader.h"

// Papel picado
class PapelPicado
{
public:

	// Medidas
	float ANCHO = 0.36f;
	float ALTO_FIJO = 0.0f;
	float SEPARACION = 0.2f;
	bool  dibujarCordon = true;

	// Viento
	float amplitud = 0.035f;
	float frecuencia = 12.0f;
	float velocidad = 2.5f;
	float balanceo = 0.12f;

	// Cargar texturas
	bool cargar(const std::string& carpeta = "Imagenes",
		std::vector<std::string> archivos = {
			"pp_rojo.png", "pp_amarillo.png", "pp_morado.png", "pp_verde.png",
			"pp_rosa.png", "pp_naranja.png",  "pp_azul.png",   "pp_cian.png" })
	{
		for (const std::string& a : archivos)
		{
			glm::vec4 recorte;
			float aspecto;
			GLuint tex = cargarTextura(carpeta + "/" + a, recorte, aspecto);
			if (tex == 0) continue;

			float alto = (ALTO_FIJO > 0.0f) ? ALTO_FIJO : ANCHO * aspecto;
			banderas.push_back(Bandera(crearBandera(tex, ANCHO, alto, recorte), alto));
		}

		if (banderas.empty())
		{
			std::cout << "ERROR::PAPEL_PICADO no se cargo ninguna textura (corre git lfs pull)" << std::endl;
			return false;
		}
		texCordon = crearTexturaColor(225, 210, 180);
		return true;
	}

	// Agregar tira
	void agregarTira(glm::vec3 amarreA, glm::vec3 amarreB, float caida = 0.12f, float margen = 0.0f)
	{
		Tira t;
		t.inicio = amarreA;
		t.fin = amarreB;
		t.caida = caida;

		glm::vec3 dir = amarreB - amarreA;
		float largoH = glm::length(glm::vec2(dir.x, dir.z));
		t.sIni = margen / largoH;
		t.sFin = 1.0f - margen / largoH;

		float tramo = largoH * (t.sFin - t.sIni);
		t.n = (int)(tramo / (ANCHO + SEPARACION));
		if (t.n < 1) t.n = 1;

		int k = (int)tiras.size();
		t.desfaseColor = k * 3;
		t.fase = k * 1.3f;

		if (dibujarCordon && texCordon != 0)
			t.cordon = new Mesh(crearCordon(t, texCordon));

		tiras.push_back(t);
	}

	// Tiras del panteón
	void colocarEnPanteon(const glm::mat4& modeloBardas = glm::mat4(1.0f))
	{
		const float xOeste = 15.00f;
		const float xEste = 50.00f;
		const float yAmarre = 2.76f;
		const float margen = 0.55f;

		const float zs[] = {-36.0f, -19.5f, 7.5f, 27.0f, 45.0f };
		const float caidas[] = { 0.15f, 0.14f, 0.14f, 0.16f, 0.15f };

		const int numTiras = sizeof(zs) / sizeof(zs[0]);

		for (int k = 0; k < numTiras; k++)
		{
			glm::vec3 a = glm::vec3(modeloBardas * glm::vec4(xOeste, yAmarre, zs[k], 1.0f));
			glm::vec3 b = glm::vec3(modeloBardas * glm::vec4(xEste, yAmarre, zs[k], 1.0f));
			agregarTira(a, b, caidas[k], margen);
		}
	}

	// Dibujar
	void dibujar(Shader& shader, float tiempo)
	{
		if (banderas.empty()) return;

		GLuint P = shader.Program;
		GLint locModel = glGetUniformLocation(P, "model");
		GLint locFase = glGetUniformLocation(P, "fase");
		GLint locAlto = glGetUniformLocation(P, "altoBandera");
		GLint locOndula = glGetUniformLocation(P, "ondula");
		GLint locTransp = glGetUniformLocation(P, "transparency");

		GLboolean habiaCulling = glIsEnabled(GL_CULL_FACE);
		glDisable(GL_CULL_FACE);
		glUniform1i(locTransp, 1);

		// Cordones
		glUniform1i(locOndula, 0);
		glm::mat4 identidad(1.0f);
		glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(identidad));
		for (Tira& t : tiras)
			if (t.cordon) t.cordon->Draw(shader);

		// Banderitas
		glUniform1i(locOndula, 1);
		glUniform1f(glGetUniformLocation(P, "tiempo"), tiempo);
		glUniform1f(glGetUniformLocation(P, "amplitud"), amplitud);
		glUniform1f(glGetUniformLocation(P, "frecuencia"), frecuencia);
		glUniform1f(glGetUniformLocation(P, "velocidad"), velocidad);
		glUniform1f(glGetUniformLocation(P, "balanceo"), balanceo);

		const int N = (int)banderas.size();
		for (Tira& t : tiras)
		{
			glm::vec3 dir = t.fin - t.inicio;
			float largoH = glm::length(glm::vec2(dir.x, dir.z));
			float rotY = atan2(-dir.z, dir.x);

			for (int i = 0; i < t.n; i++)
			{
				float s = posicionBandera(t, i);
				glm::vec3 p = puntoCordon(t, s);

				float dyds = dir.y - t.caida * 4.0f * (1.0f - 2.0f * s);
				float rotZ = atan(dyds / largoH);

				glm::mat4 model(1.0f);
				model = glm::translate(model, p);
				model = glm::rotate(model, rotY, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, rotZ, glm::vec3(0.0f, 0.0f, 1.0f));
				glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));

				Bandera& b = banderas[(i + t.desfaseColor) % N];
				glUniform1f(locAlto, b.alto);
				glUniform1f(locFase, t.fase + i * 0.7f);

				b.malla.Draw(shader);
			}
		}

		// Restaurar estado
		glUniform1i(locOndula, 0);
		glUniform1i(locTransp, 0);
		if (habiaCulling) glEnable(GL_CULL_FACE);
	}

	// Total de banderitas
	int totalBanderas() const
	{
		int total = 0;
		for (const Tira& t : tiras) total += t.n;
		return total;
	}

	// Punta más baja
	float puntaMasBaja() const
	{
		float minimo = 1e9f;
		const int N = (int)banderas.size();
		for (const Tira& t : tiras)
			for (int i = 0; i < t.n; i++)
			{
				float y = puntoCordon(t, posicionBandera(t, i)).y - banderas[(i + t.desfaseColor) % N].alto;
				if (y < minimo) minimo = y;
			}
		return minimo;
	}

private:

	// Estructura bandera
	struct Bandera
	{
		Mesh malla;
		float alto;
		Bandera(const Mesh& m, float a) : malla(m), alto(a) {}
	};

	// Estructura tira
	struct Tira
	{
		glm::vec3 inicio, fin;
		float caida;
		float sIni, sFin;
		int n;
		int desfaseColor;
		float fase;
		Mesh* cordon = nullptr;
	};

	// Datos
	std::vector<Bandera> banderas;
	std::vector<Tira> tiras;
	GLuint texCordon = 0;

	// Posición de una bandera
	static float posicionBandera(const Tira& t, int i)
	{
		return t.sIni + (t.sFin - t.sIni) * (i + 0.5f) / t.n;
	}

	// Punto del cordón
	static glm::vec3 puntoCordon(const Tira& t, float s)
	{
		glm::vec3 p = t.inicio + (t.fin - t.inicio) * s;
		p.y -= t.caida * 4.0f * s * (1.0f - s);
		return p;
	}

	// Cargar textura
	static GLuint cargarTextura(const std::string& ruta, glm::vec4& recorte, float& aspecto)
	{
		int w, h;
		unsigned char* img = SOIL_load_image(ruta.c_str(), &w, &h, 0, SOIL_LOAD_RGBA);
		if (!img)
		{
			std::cout << "ERROR::PAPEL_PICADO textura no encontrada: " << ruta << std::endl;
			return 0;
		}

		int minX = w, minY = h, maxX = -1, maxY = -1;
		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++)
				if (img[(y * w + x) * 4 + 3] > 25)
				{
					if (x < minX) minX = x;
					if (x > maxX) maxX = x;
					if (y < minY) minY = y;
					if (y > maxY) maxY = y;
				}
		if (maxX < 0) { minX = 0; minY = 0; maxX = w - 1; maxY = h - 1; }

		recorte = glm::vec4((float)minX / w, (float)minY / h, (float)(maxX + 1) / w, (float)(maxY + 1) / h);
		aspecto = (float)(maxY - minY + 1) / (float)(maxX - minX + 1);

		GLuint id;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		SOIL_free_image_data(img);
		return id;
	}

	// Textura de un color
	static GLuint crearTexturaColor(unsigned char r, unsigned char g, unsigned char b)
	{
		unsigned char px[4] = { r, g, b, 255 };
		GLuint id;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		return id;
	}

	// Malla con textura
	static Mesh conTextura(std::vector<Vertex>& v, std::vector<GLuint>& idx, GLuint tex)
	{
		Texture t;
		t.id = tex;
		t.type = "texture_diffuse";
		std::vector<Texture> texturas{ t };
		return Mesh(v, idx, texturas);
	}

	// Crear bandera
	static Mesh crearBandera(GLuint tex, float ancho, float alto, glm::vec4 rec, int columnas = 16, int filas = 12)
	{
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;

		for (int j = 0; j <= filas; j++)
		{
			float fy = (float)j / filas;
			for (int i = 0; i <= columnas; i++)
			{
				float fx = (float)i / columnas;
				Vertex vx;
				vx.Position = glm::vec3((fx - 0.5f) * ancho, -fy * alto, 0.0f);
				vx.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
				vx.TexCoords = glm::vec2(rec.x + fx * (rec.z - rec.x), rec.y + fy * (rec.w - rec.y));
				vertices.push_back(vx);
			}
		}

		for (int j = 0; j < filas; j++)
			for (int i = 0; i < columnas; i++)
			{
				GLuint a = j * (columnas + 1) + i;
				GLuint b = a + 1;
				GLuint c = a + columnas + 1;
				GLuint d = c + 1;
				indices.insert(indices.end(), { a, c, b,  b, c, d });
			}

		return conTextura(vertices, indices, tex);
	}

	// Crear cordón
	static Mesh crearCordon(const Tira& t, GLuint tex, int segmentos = 80, float grosor = 0.008f)
	{
		std::vector<Vertex> v;
		std::vector<GLuint> idx;

		glm::vec3 dir = t.fin - t.inicio;
		glm::vec3 lado = glm::normalize(glm::cross(glm::vec3(dir.x, 0.0f, dir.z), glm::vec3(0.0f, 1.0f, 0.0f)));
		glm::vec3 arriba(0.0f, 1.0f, 0.0f);

		for (int tipo = 0; tipo < 2; tipo++)
		{
			glm::vec3 desplaza = (tipo == 0) ? arriba * grosor : lado * grosor;
			glm::vec3 normal = (tipo == 0) ? lado : arriba;
			GLuint base = (GLuint)v.size();

			for (int j = 0; j <= segmentos; j++)
			{
				float s = (float)j / segmentos;
				glm::vec3 p = puntoCordon(t, s) + glm::vec3(0.0f, grosor, 0.0f);
				Vertex a, b;
				a.Position = p + desplaza;  a.Normal = normal;  a.TexCoords = glm::vec2(0.5f);
				b.Position = p - desplaza;  b.Normal = normal;  b.TexCoords = glm::vec2(0.5f);
				v.push_back(a);
				v.push_back(b);
			}
			for (int j = 0; j < segmentos; j++)
			{
				GLuint a = base + j * 2;
				idx.insert(idx.end(), { a, a + 1, a + 2,  a + 1, a + 3, a + 2 });
			}
		}
		return conTextura(v, idx, tex);
	}
};
