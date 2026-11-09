#pragma once
// =====================================================================
//  PapelPicado.h  (Equipo 1 - Día de Muertos)
//
//  Cuelga tiras de papel picado dentro del panteón, de la barda oeste a la
//  barda este de barda_panteon.obj. Cada tira es un cordón apoyado sobre las
//  bardas, con caída parabólica, del que cuelgan banderitas con las texturas
//  Imagenes/pp_*.png intercaladas. La ondulación la hace Shader/lighting.vs
//  cuando el uniform "ondula" vale 1.
//
//  Uso en main.cpp:
//      #include "PapelPicado.h"                       // después de "Model.h"
//      PapelPicado papelPicado;                       // antes del while
//      papelPicado.cargar("Imagenes");
//      papelPicado.colocarEnPanteon();
//      papelPicado.dibujar(lightingShader, currentFrame);   // dentro del while
// =====================================================================
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

class PapelPicado
{
public:
	// ---------------- Medidas (metros) ----------------
	// Tamaño real de una banderita de papel de china: unos 36 x 30 cm.
	float ANCHO = 0.36f;        // ancho de cada banderita
	float ALTO_FIJO = 0.0f;     // 0 = cada bandera respeta la proporción de su PNG; > 0 = todas miden esto de alto
	float SEPARACION = 0.2f;   // hueco entre banderitas
	bool  dibujarCordon = true;

	// ---------------- Viento (se mandan al vertex shader) ----------------
	float amplitud = 0.035f;  // cuánto se separa la punta del plano
	float frecuencia = 12.0f;   // ondas por metro a lo ancho de la bandera
	float velocidad = 2.5f;    // rapidez del viento
	float balanceo = 0.12f;   // vaivén tipo péndulo

	// Orden de colores intercalado: ningún tono queda junto a uno parecido.
	bool cargar(const std::string& carpeta = "Imagenes",
		std::vector<std::string> archivos = {
			"pp_rojo.png", "pp_amarillo.png", "pp_morado.png", "pp_verde.png",
			"pp_rosa.png", "pp_naranja.png",  "pp_azul.png",   "pp_cian.png" })
	{
		for (const std::string& a : archivos)
		{
			glm::vec4 recorte;   // u0, v0, u1, v1 de la parte con papel del PNG
			float aspecto;       // alto / ancho de esa parte
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
		texCordon = crearTexturaColor(225, 210, 180);   // cordón de ixtle
		return true;
	}

	// Un cordón amarrado en dos puntos (amarreA, amarreB).
	// Las banderitas solo se cuelgan a partir de "margen" metros de cada amarre,
	// para que no atraviesen las bardas.
	// caida = cuánto baja el cordón en el centro.
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
		t.desfaseColor = k * 3;    // 3 y 8 no tienen divisores comunes: cada tira empieza en otro color
		t.fase = k * 1.3f;

		if (dibujarCordon && texCordon != 0)
			t.cordon = new Mesh(crearCordon(t, texCordon));

		tiras.push_back(t);
	}

	//  tiras dentro del panteón, de barda a barda.
	// Medidas de assets/models/edificacion/barda_panteon.obj (se dibuja con model = identidad):
	//   barda oeste  x = 14.75 .. 15.25   (hueco del corredor en z = -5 .. 5)
	//   barda este   x = 49.75 .. 50.25
	//   interior     z = -49.75 .. 49.75  (portón en la barda sur, z = 50, x = 30.5 .. 34.5)
	//   altura       y = 2.75
	// El cordón se apoya en el centro de la parte de arriba de cada barda.
	// modeloBardas = la misma matriz model con la que dibujan murosCementerio.
	void colocarEnPanteon(const glm::mat4& modeloBardas = glm::mat4(1.0f))
	{
		const float xOeste = 15.00f;   // centro de la barda oeste
		const float xEste = 50.00f;   // centro de la barda este
		const float yAmarre = 2.76f;    // encima de la barda (mide 2.75)
		const float margen = 0.55f;    // 0.25 de medio grosor de barda + 0.30 libres

		// Se salta z = -5 .. 5 para dejar libre la entrada del corredor.
		// -19.5, -12 y 7.5 esquivan las copas de los árboles de bosque.obj que
		// entran hasta 1.4 m al panteón junto al corredor (x = 11 .. 16.7).
		const float zs[] = {-36.0f, -19.5f, 7.5f, 27.0f, 45.0f };
		// Cada cordón cuelga un poco distinto, como en la realidad
		const float caidas[] = { 0.15f, 0.14f, 0.14f, 0.16f, 0.15f };

		const int numTiras = sizeof(zs) / sizeof(zs[0]);   // cuántas tiras hay en el arreglo

		for (int k = 0; k < numTiras; k++)
		{
			glm::vec3 a = glm::vec3(modeloBardas * glm::vec4(xOeste, yAmarre, zs[k], 1.0f));
			glm::vec3 b = glm::vec3(modeloBardas * glm::vec4(xEste, yAmarre, zs[k], 1.0f));
			agregarTira(a, b, caidas[k], margen);
		}
	}

	// Llamar con lightingShader en uso, DESPUÉS de dibujar los objetos opacos.
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
		glDisable(GL_CULL_FACE);           // el papel se ve por los dos lados
		glUniform1i(locTransp, 1);         // descarta los huecos del picado

		// ---- Cordones (no ondulan; ya están en coordenadas de mundo) ----
		glUniform1i(locOndula, 0);
		glm::mat4 identidad(1.0f);
		glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(identidad));
		for (Tira& t : tiras)
			if (t.cordon) t.cordon->Draw(shader);

		// ---- Banderitas ----
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
			float rotY = atan2(-dir.z, dir.x);            // eje X local a lo largo del cordón

			for (int i = 0; i < t.n; i++)
			{
				float s = posicionBandera(t, i);           // 0..1 a lo largo del cordón
				glm::vec3 p = puntoCordon(t, s);

				// Inclinación del cordón en ese punto: la bandera sigue la curva
				float dyds = dir.y - t.caida * 4.0f * (1.0f - 2.0f * s);
				float rotZ = atan(dyds / largoH);

				glm::mat4 model(1.0f);
				model = glm::translate(model, p);
				model = glm::rotate(model, rotY, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, rotZ, glm::vec3(0.0f, 0.0f, 1.0f));
				glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));

				Bandera& b = banderas[(i + t.desfaseColor) % N];   // colores intercalados
				glUniform1f(locAlto, b.alto);
				glUniform1f(locFase, t.fase + i * 0.7f);   // la onda "viaja" a lo largo del cordón

				b.malla.Draw(shader);
			}
		}

		// Dejar el estado como estaba para el resto de la escena
		glUniform1i(locOndula, 0);
		glUniform1i(locTransp, 0);
		if (habiaCulling) glEnable(GL_CULL_FACE);
	}

	int totalBanderas() const
	{
		int total = 0;
		for (const Tira& t : tiras) total += t.n;
		return total;
	}

	// Altura (y) de la punta más baja en reposo: sirve para comprobar que la
	// cámara (y = 2.0 sobre el esqueleto) pase por debajo sin atravesarlas.
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
	struct Bandera
	{
		Mesh malla;
		float alto;
		Bandera(const Mesh& m, float a) : malla(m), alto(a) {}
	};

	struct Tira
	{
		glm::vec3 inicio, fin;     // amarres del cordón
		float caida;
		float sIni, sFin;          // tramo del cordón donde hay banderas (0..1)
		int n;
		int desfaseColor;
		float fase;
		Mesh* cordon = nullptr;
	};

	std::vector<Bandera> banderas;
	std::vector<Tira> tiras;
	GLuint texCordon = 0;

	static float posicionBandera(const Tira& t, int i)
	{
		return t.sIni + (t.sFin - t.sIni) * (i + 0.5f) / t.n;
	}

	// Punto del cordón: recta entre amarres menos una parábola de caída
	static glm::vec3 puntoCordon(const Tira& t, float s)
	{
		glm::vec3 p = t.inicio + (t.fin - t.inicio) * s;
		p.y -= t.caida * 4.0f * s * (1.0f - s);
		return p;
	}

	// Carga el PNG en RGBA y calcula el rectángulo donde realmente hay papel
	// (los pp_*.png miden 1024x1024 y tienen margen transparente).
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

		// La fila 0 del PNG (el doblez que va en el cordón) queda en v = 0
		recorte = glm::vec4((float)minX / w, (float)minY / h, (float)(maxX + 1) / w, (float)(maxY + 1) / h);
		aspecto = (float)(maxY - minY + 1) / (float)(maxX - minX + 1);

		GLuint id;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
		glGenerateMipmap(GL_TEXTURE_2D);
		// CLAMP para que el borde de arriba no tome color del margen de abajo
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		SOIL_free_image_data(img);
		return id;
	}

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

	static Mesh conTextura(std::vector<Vertex>& v, std::vector<GLuint>& idx, GLuint tex)
	{
		Texture t;
		t.id = tex;
		t.type = "texture_diffuse";      // Mesh::Draw la liga a la unidad 0 (material.diffuse)
		std::vector<Texture> texturas{ t };
		return Mesh(v, idx, texturas);
	}

	// Rejilla columnas x filas. Origen en el centro del borde superior (el cordón);
	// la bandera cuelga hacia -Y. Las UV usan solo la parte con papel del PNG.
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

	// Cordón como dos listones delgados en cruz (uno vertical y uno horizontal)
	// para que se vea de lado y desde abajo. Ya en coordenadas de mundo.
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
				glm::vec3 p = puntoCordon(t, s) + glm::vec3(0.0f, grosor, 0.0f);   // el doblez de la bandera queda colgado
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