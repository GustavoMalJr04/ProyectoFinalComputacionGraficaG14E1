#pragma once

// Librerías
#include <vector>
#include <string>
#include <random>
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

// Ofrenda
const glm::vec3 posAltar(-20.0f, 0.0f, 0.0f);
const float escalaAltar = 0.8f;
const float altarX0 = -20.0f - 4.0f * 0.8f, altarX1 = -20.0f + 4.0f * 0.8f;
const float altarZ0 = -1.42f * 0.8f, altarZ1 = 3.71f * 0.8f;

// Copalero
const float escalaCopalero = 2.0f;
const glm::vec3 posCopalero(posAltar.x, 4.5f * escalaAltar, posAltar.z + 0.20f);
const glm::vec3 origenHumoCopal = posCopalero + glm::vec3(0.0f, 0.11f * escalaCopalero, 0.0f);

// Estructura tumba
struct Tumba { float x0, x1, z0, z1; };
Tumba tumbas[6] = {
	{ 26.22f, 27.98f, -28.06f, -23.06f },
	{ 36.78f, 38.53f, -28.06f, -23.06f },
	{ 26.26f, 28.01f,  -8.22f,  -3.22f },
	{ 36.81f, 38.56f,  -8.00f,  -3.00f },
	{ 26.22f, 27.97f,  12.12f,  17.12f },
	{ 36.74f, 38.49f,  12.20f,  17.20f }
};
const float xPasillo = 32.3f;

// Adornos
std::vector<glm::mat4> matPetalos;
std::vector<glm::mat4> matFlores;
std::vector<glm::vec3> centrosPetalos;
std::mt19937 rngAdornos(2026);

// Número aleatorio
float azar(float a, float b)
{
	return std::uniform_real_distribution<float>(a, b)(rngAdornos);
}

// Estructura triángulo del piso
struct TrianguloPiso { glm::vec3 a, b, c; };
std::vector<TrianguloPiso> trianguloPiso;

// Cargar piso del panteón
void cargarPisoPanteon(const char* ruta)
{
	std::ifstream archivo(ruta);
	if (!archivo)
	{
		std::cout << "ERROR::SENDERO no se encontro " << ruta << " (las flores quedan en y = 0)" << std::endl;
		return;
	}
	std::vector<glm::vec3> v;
	std::string linea;
	while (std::getline(archivo, linea))
	{
		if (linea.size() > 2 && linea[0] == 'v' && linea[1] == ' ')
		{
			std::istringstream s(linea.substr(2));
			glm::vec3 p;
			s >> p.x >> p.y >> p.z;
			v.push_back(p);
		}
		else if (linea.size() > 2 && linea[0] == 'f' && linea[1] == ' ')
		{
			std::istringstream s(linea.substr(2));
			std::string tok;
			std::vector<int> idx;
			while (s >> tok)
			{
				int i = std::stoi(tok.substr(0, tok.find('/')));
				idx.push_back(i > 0 ? i - 1 : (int)v.size() + i);
			}
			for (size_t k = 1; k + 1 < idx.size(); k++)
			{
				TrianguloPiso t;
				t.a = v[idx[0]];  t.b = v[idx[k]];  t.c = v[idx[k + 1]];
				trianguloPiso.push_back(t);
			}
		}
	}
}

// Altura del piso
float alturaPiso(float x, float z, glm::vec3* normal = nullptr)
{
	float mejor = 0.0f;
	bool hay = false;
	glm::vec3 nMejor(0.0f, 1.0f, 0.0f);
	for (const TrianguloPiso& t : trianguloPiso)
	{
		float d = (t.b.z - t.c.z) * (t.a.x - t.c.x) + (t.c.x - t.b.x) * (t.a.z - t.c.z);
		if (fabs(d) < 1e-9f) continue;
		float l1 = ((t.b.z - t.c.z) * (x - t.c.x) + (t.c.x - t.b.x) * (z - t.c.z)) / d;
		float l2 = ((t.c.z - t.a.z) * (x - t.c.x) + (t.a.x - t.c.x) * (z - t.c.z)) / d;
		float l3 = 1.0f - l1 - l2;
		if (l1 < -1e-4f || l2 < -1e-4f || l3 < -1e-4f) continue;
		float y = l1 * t.a.y + l2 * t.b.y + l3 * t.c.y;
		if (!hay || y > mejor)
		{
			mejor = y;
			hay = true;
			nMejor = glm::normalize(glm::cross(t.b - t.a, t.c - t.a));
			if (nMejor.y < 0.0f) nMejor = -nMejor;
		}
	}
	if (normal) *normal = nMejor;
	return mejor;
}

// Alinear eje Y
glm::mat4 alinearY(glm::mat4 m, glm::vec3 n)
{
	n = glm::normalize(n);
	float c = glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), n);
	if (c > 0.9999f) return m;
	if (c < -0.9999f) return glm::rotate(m, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 eje = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), n));
	return glm::rotate(m, acos(c), eje);
}

// Agregar flor
void agregarFlor(glm::vec3 p, float escala = 0.28f)
{
	float r = 0.45f * escala;
	float y = alturaPiso(p.x, p.z);
	y = glm::max(y, alturaPiso(p.x + r, p.z));
	y = glm::max(y, alturaPiso(p.x - r, p.z));
	y = glm::max(y, alturaPiso(p.x, p.z + r));
	y = glm::max(y, alturaPiso(p.x, p.z - r));

	glm::mat4 m(1.0f);
	m = glm::translate(m, glm::vec3(p.x, y, p.z));
	m = glm::rotate(m, glm::radians(azar(0.0f, 360.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
	m = glm::rotate(m, glm::radians(azar(-8.0f, 8.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
	m = glm::scale(m, glm::vec3(escala * azar(0.8f, 1.2f)));
	matFlores.push_back(m);
}

// Agregar flor en pared
void agregarFlorPared(glm::vec3 p, glm::vec3 normal, float escala = 0.4f)
{
	glm::mat4 m(1.0f);
	m = glm::translate(m, p);
	m = alinearY(m, normal);
	m = glm::rotate(m, glm::radians(azar(0.0f, 360.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
	m = glm::scale(m, glm::vec3(escala * azar(0.9f, 1.1f)));
	matFlores.push_back(m);
}

// Estructura vela
struct Vela
{
	glm::vec3 pos;
	float sXZ, sY;
	float fase;
};
std::vector<Vela> velas;

// Tamaño de las velas
const float VELA_GRANDE_XZ = 0.32f, VELA_GRANDE_Y = 0.29f;
const float VELA_CHICA_XZ = 0.22f, VELA_CHICA_Y = 0.16f;

// Distancia de dibujado
const float DISTANCIA_ADORNOS = 45.0f;

// Distancia al cuadrado
float distancia2(const glm::vec3& a, const glm::vec3& b)
{
	glm::vec3 d = a - b;
	return d.x * d.x + d.y * d.y + d.z * d.z;
}

// Agregar vela
void agregarVela(glm::vec3 p, bool grande)
{
	Vela v;
	v.sXZ = (grande ? VELA_GRANDE_XZ : VELA_CHICA_XZ) * azar(0.95f, 1.05f);
	v.sY = (grande ? VELA_GRANDE_Y : VELA_CHICA_Y) * azar(0.9f, 1.1f);
	v.pos = glm::vec3(p.x, alturaPiso(p.x, p.z) - 0.01f, p.z);
	v.fase = azar(0.0f, 6.2832f);
	velas.push_back(v);
}

// Revisar si está sobre el sendero
bool sobreSendero(float x, float z, float radio = 0.55f)
{
	for (const glm::vec3& p : centrosPetalos)
		if (glm::length(glm::vec2(x, z) - glm::vec2(p.x, p.z)) < radio) return true;
	return false;
}

// Tramo del sendero
void agregarTramo(glm::vec3 a, glm::vec3 b)
{
	const float paso = 0.35f, dispersion = 0.35f, pasoFlor = 1.5f, orilla = 1.0f;
	glm::vec3 d = b - a;  d.y = 0.0f;
	float largo = glm::length(d);
	glm::vec3 dir = d / largo;
	glm::vec3 lado = glm::normalize(glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f)));

	int n = (int)(largo / paso);
	for (int i = 0; i <= n; i++)
	{
		glm::vec3 p = a + dir * (i * paso) + lado * azar(-dispersion, dispersion);
		glm::vec3 nPiso;
		p.y = alturaPiso(p.x, p.z, &nPiso)
			+ 0.012f + 0.002f * (float)(matPetalos.size() % 8);
		glm::mat4 m(1.0f);
		m = glm::translate(m, p);
		m = alinearY(m, nPiso);
		m = glm::rotate(m, glm::radians(azar(0.0f, 360.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
		m = glm::scale(m, glm::vec3(azar(0.9f, 1.2f), 1.0f, azar(0.9f, 1.2f)));
		matPetalos.push_back(m);
		centrosPetalos.push_back(p);
	}
	for (float s = pasoFlor * 0.5f; s < largo; s += pasoFlor)
	{
		agregarFlor(a + dir * s + lado * (orilla + azar(-0.05f, 0.05f)));
		agregarFlor(a + dir * s - lado * (orilla + azar(-0.05f, 0.05f)));
	}
	int k = 0;
	for (float s = pasoFlor; s < largo - 0.3f; s += pasoFlor, k++)
	{
		agregarVela(a + dir * s + lado * orilla, k % 2 == 0);
		agregarVela(a + dir * s - lado * orilla, k % 2 == 1);
	}
}

// Contorno de tumba
void agregarContornoTumba(const Tumba& t)
{
	const float sep = 0.25f;
	const float escala = 0.28f;
	float x0 = t.x0 - sep, x1 = t.x1 + sep, z0 = t.z0 - sep, z1 = t.z1 + sep;
	float xm = (x0 + x1) * 0.5f;
	float t1 = z0 + (z1 - z0) / 3.0f, t2 = z0 + 2.0f * (z1 - z0) / 3.0f;

	glm::vec3 marco[10] = {
		glm::vec3(x0, 0.0f, z0), glm::vec3(xm, 0.0f, z0), glm::vec3(x1, 0.0f, z0),
		glm::vec3(x1, 0.0f, t1), glm::vec3(x1, 0.0f, t2), glm::vec3(x1, 0.0f, z1),
		glm::vec3(xm, 0.0f, z1), glm::vec3(x0, 0.0f, z1),
		glm::vec3(x0, 0.0f, t2), glm::vec3(x0, 0.0f, t1)
	};
	for (int i = 0; i < 10; i++)
	{
		agregarFlor(marco[i], escala);
		glm::vec3 medio = (marco[i] + marco[(i + 1) % 10]) * 0.5f;
		agregarVela(medio, i % 2 == 0);
	}
}

// Quitar adornos sobre el sendero
void quitarFloresSobreSendero(float radio = 0.55f)
{
	std::vector<glm::mat4> quedan;
	for (const glm::mat4& f : matFlores)
		if (!sobreSendero(f[3].x, f[3].z, radio)) quedan.push_back(f);
	matFlores = quedan;

	std::vector<Vela> velasQuedan;
	for (const Vela& v : velas)
		if (!sobreSendero(v.pos.x, v.pos.z, radio)) velasQuedan.push_back(v);
	velas = velasQuedan;
}

// Arco de entrada
const glm::vec3 posArco(-20.0f, -0.25f, 50.0f);

// Flores del arco
void agregarFloresArco()
{
	const float semiAncho = 2.5f, yRecto = 4.0f, radio = 2.5f;
	const float separacion = 0.3f;
	const float r = radio + separacion;
	const float xLado = semiAncho + separacion;
	const float yInicio = 0.5f;
	const float paso = 1.0f;

	const float largoRecto = yRecto - yInicio;
	const float largoCurva = 3.14159265f * r;
	const float total = 2.0f * largoRecto + largoCurva;

	for (float s = 0.0f; s <= total + 0.01f; s += paso)
	{
		glm::vec3 local;
		if (s < largoRecto)
			local = glm::vec3(-xLado, yInicio + s, 0.0f);
		else if (s < largoRecto + largoCurva)
		{
			float ang = 3.14159265f * (1.0f - (s - largoRecto) / largoCurva);
			local = glm::vec3(r * cos(ang), yRecto + r * sin(ang), 0.0f);
		}
		else
			local = glm::vec3(xLado, yRecto - (s - largoRecto - largoCurva), 0.0f);

		agregarFlorPared(posArco + local + glm::vec3(0.0f, 0.0f, 0.26f), glm::vec3(0.0f, 0.0f, 1.0f));
		agregarFlorPared(posArco + local + glm::vec3(0.0f, 0.0f, -0.26f), glm::vec3(0.0f, 0.0f, -1.0f));
	}
}

// Flores y velas de una barda
void agregarFloresBarda(glm::vec3 a, glm::vec3 b, glm::vec3 adentro, bool conVelas = true, float paso = 1.2f, float escala = 0.28f)
{
	glm::vec3 d = b - a;
	float largo = glm::length(d);
	glm::vec3 dir = d / largo;
	int i = 0;
	for (float s = 0.4f; s < largo - 0.3f; s += paso, i++)
	{
		float fondo = (i % 2 == 0) ? 0.45f : 0.85f;
		agregarFlor(a + dir * (s + azar(-0.15f, 0.15f)) + adentro * fondo, escala);

		if (conVelas && s + paso < largo - 0.3f)
			agregarVela(a + dir * (s + paso * 0.5f) + adentro * 0.65f, i % 2 == 0);
	}
}

// Perímetro del panteón
void agregarFloresPerimetroPanteon()
{
	const glm::vec3 masX(1.0f, 0.0f, 0.0f), menosX(-1.0f, 0.0f, 0.0f);
	const glm::vec3 masZ(0.0f, 0.0f, 1.0f), menosZ(0.0f, 0.0f, -1.0f);

	agregarFloresBarda(glm::vec3(15.25f, 0.0f, -49.75f), glm::vec3(15.25f, 0.0f, -5.5f), masX);
	agregarFloresBarda(glm::vec3(15.25f, 0.0f, 5.5f), glm::vec3(15.25f, 0.0f, 49.75f), masX);
	agregarFloresBarda(glm::vec3(49.75f, 0.0f, -49.75f), glm::vec3(49.75f, 0.0f, 49.75f), menosX);
	agregarFloresBarda(glm::vec3(15.25f, 0.0f, -49.75f), glm::vec3(49.75f, 0.0f, -49.75f), masZ);
	agregarFloresBarda(glm::vec3(15.25f, 0.0f, 49.75f), glm::vec3(27.3f, 0.0f, 49.75f), menosZ);
	agregarFloresBarda(glm::vec3(37.3f, 0.0f, 49.75f), glm::vec3(49.75f, 0.0f, 49.75f), menosZ);
}

// Perímetro del atrio
void agregarFloresPerimetroAtrio()
{
	const glm::vec3 masX(1.0f, 0.0f, 0.0f), menosX(-1.0f, 0.0f, 0.0f);
	const glm::vec3 masZ(0.0f, 0.0f, 1.0f), menosZ(0.0f, 0.0f, -1.0f);

	agregarFloresBarda(glm::vec3(-49.75f, 0.0f, -49.75f), glm::vec3(-49.75f, 0.0f, 49.75f), masX);
	agregarFloresBarda(glm::vec3(9.75f, 0.0f, -49.75f), glm::vec3(9.75f, 0.0f, -5.3f), menosX);
	agregarFloresBarda(glm::vec3(9.75f, 0.0f, 5.3f), glm::vec3(9.75f, 0.0f, 49.75f), menosX);
	agregarFloresBarda(glm::vec3(-49.75f, 0.0f, -49.75f), glm::vec3(9.75f, 0.0f, -49.75f), masZ);
	agregarFloresBarda(glm::vec3(-49.75f, 0.0f, 49.75f), glm::vec3(-24.6f, 0.0f, 49.75f), menosZ);
	agregarFloresBarda(glm::vec3(-15.4f, 0.0f, 49.75f), glm::vec3(9.75f, 0.0f, 49.75f), menosZ);
}

// Arriates del portón
void agregarFloresEntradas()
{
	for (int fila = 0; fila < 3; fila++)
	{
		float z = 49.25f - fila * 0.45f;
		float desfase = (fila % 2) * 0.22f;
		for (float x = 27.5f + desfase; x <= 29.7f; x += 0.45f) agregarFlor(glm::vec3(x, 0.0f, z));
		for (float x = 34.9f + desfase; x <= 37.1f; x += 0.45f) agregarFlor(glm::vec3(x, 0.0f, z));
	}
}

// Construir sendero
void construirSendero()
{
	// Piso del panteón
	cargarPisoPanteon("assets/models/edificacion/piso_panteon.obj");

	// Ruta de la ofrenda al panteón
	glm::vec3 ruta[] = {
		glm::vec3(posAltar.x, 0.0f, altarZ1 + 0.9f),
		glm::vec3(-13.0f, 0.0f, 4.6f),
		glm::vec3(-5.0f, 0.0f, 3.8f),
		glm::vec3(3.0f, 0.0f, 2.0f),
		glm::vec3(9.0f, 0.0f, 0.3f),
		glm::vec3(15.0f, 0.0f, 0.0f),
		glm::vec3(24.0f, 0.0f, 0.0f),
		glm::vec3(xPasillo, 0.0f, 0.0f)
	};
	const int numPuntosRuta = sizeof(ruta) / sizeof(ruta[0]);
	for (int i = 0; i < numPuntosRuta - 1; i++) agregarTramo(ruta[i], ruta[i + 1]);

	// Pasillo central
	agregarTramo(glm::vec3(xPasillo, 0.0f, 0.0f), glm::vec3(xPasillo, 0.0f, -25.56f));
	agregarTramo(glm::vec3(xPasillo, 0.0f, 0.0f), glm::vec3(xPasillo, 0.0f, 14.66f));
	agregarTramo(glm::vec3(xPasillo, 0.0f, 14.66f), glm::vec3(xPasillo, 0.0f, 49.2f));

	// Ramales a las tumbas
	for (const Tumba& t : tumbas)
	{
		float zc = (t.z0 + t.z1) * 0.5f;
		float xFin = (t.x1 < xPasillo) ? t.x1 + 0.6f : t.x0 - 0.6f;
		agregarTramo(glm::vec3(xPasillo, 0.0f, zc), glm::vec3(xFin, 0.0f, zc));
	}

	// Ofrenda
	{
		const float sep = 0.4f;
		float xIzq = altarX0 - sep, xDer = altarX1 + sep, zFrente = altarZ1 + sep;
		int k = 0;
		for (float z = altarZ0 + 0.2f; z <= zFrente + 0.01f; z += 0.7f, k++)
		{
			agregarFlor(glm::vec3(xIzq, 0.0f, z));
			agregarFlor(glm::vec3(xDer, 0.0f, z));
			if (z + 0.7f <= zFrente + 0.01f)
			{
				agregarVela(glm::vec3(xIzq, 0.0f, z + 0.35f), k % 2 == 0);
				agregarVela(glm::vec3(xDer, 0.0f, z + 0.35f), k % 2 == 1);
			}
		}
		k = 0;
		for (float x = xIzq + 0.7f; x < xDer - 0.3f; x += 0.7f, k++)
		{
			agregarFlor(glm::vec3(x, 0.0f, zFrente));
			agregarVela(glm::vec3(x + 0.35f, 0.0f, zFrente), k % 2 == 0);
		}
	}

	// Perímetros
	agregarFloresPerimetroPanteon();
	agregarFloresPerimetroAtrio();

	// Limpieza
	quitarFloresSobreSendero();

	// Tumbas
	for (const Tumba& t : tumbas)
		agregarContornoTumba(t);

	// Arco y portón
	agregarFloresArco();
	agregarFloresEntradas();

	std::cout << "Sendero: " << matPetalos.size() << " quads de petalos, "
		<< matFlores.size() << " flores, " << velas.size() << " velas" << std::endl;
}

// Dibujar pétalos, flores y velas
void dibujarAdornos(Shader& shader, GLint modelLoc, Model& petalos, Model& flor, Model& velaModelo, glm::vec3 posCam)
{
	const float dMax2 = DISTANCIA_ADORNOS * DISTANCIA_ADORNOS;

	// Pétalos
	GLint transpLoc = glGetUniformLocation(shader.Program, "transparency");
	glUniform1i(transpLoc, 1);
	for (const glm::mat4& m : matPetalos)
	{
		if (distancia2(glm::vec3(m[3].x, m[3].y, m[3].z), posCam) > dMax2) continue;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
		petalos.Draw(shader);
	}

	// Flores
	glUniform1i(transpLoc, 0);
	for (const glm::mat4& m : matFlores)
	{
		if (distancia2(glm::vec3(m[3].x, m[3].y, m[3].z), posCam) > dMax2) continue;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
		flor.Draw(shader);
	}

	// Velas
	for (const Vela& v : velas)
	{
		if (distancia2(v.pos, posCam) > dMax2) continue;
		glm::mat4 mv = glm::translate(glm::mat4(1.0f), v.pos);
		mv = glm::scale(mv, glm::vec3(v.sXZ, v.sY, v.sXZ));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mv));
		velaModelo.Draw(shader);
	}
}
