#include <iostream>
#include <cmath>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <utility>
#include <windows.h>
#pragma comment(lib, "winmm.lib")

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Carga de imágenes
#include "stb_image.h"

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// SOIL2 para las texturas de los modelos
#include "SOIL2/SOIL2.h"


// Clases del proyecto
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "PapelPicado.h"   // tiras de papel picado
#include "Texture.h"   // carga del skybox

// Prototipos de funciones
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
void Animation();

// Tamaño de la ventana
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Cámara
Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

// Luz de prueba
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool active;

// Luz de colores activacion por medio de tecla espacio
glm::vec3 pointLightPositions[] = {
	glm::vec3(-20.0f, 3.2f, 4.5f),   // frente a la ofrenda
	glm::vec3(0.0f,0.0f, 0.0f),
	glm::vec3(0.0f,0.0f,  0.0f),
	glm::vec3(0.0f,0.0f, 0.0f)
};

float vertices[] = {
	 -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

// Vértices del skybox
GLfloat skyboxVertices[] = {
	// Posiciones
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

GLuint skyboxIndices[] =
{
	0,1,2,3,
	4,5,6,7,
	8,9,10,11,
	12,13,14,15,
	16,17,18,19,
	20,21,22,23,
	24,25,26,27,
	28,29,30,31,
	32,33,34,35
};

// Posición inicial del esqueleto
glm::vec3 skeletonPos(-25.0f, 0.0f, 55.0f);
float skeletonYaw = 0.0f;   // hacia dónde mira el esqueleto

// Portón del panteón
glm::vec3 portonCentro(32.3f, 0.0f, 50.0f);
float portonRotY = 0.0f;
float portonAncho = 1.57f;
float anguloPorton = 0.0f;
float anguloMax = 90.0f;
float velPorton = 60.0f;
bool  abrirPorton = false;

// Lápidas y cruces de las tumbas
glm::vec3 posLapidas[4] = {
	glm::vec3(37.66f, 0.29f, -27.81f),
	glm::vec3(27.11f, 0.36f, -27.81f),
	glm::vec3(27.14f, 0.35f,  -7.97f),
	glm::vec3(37.68f, 0.25f,  -7.75f)
};
glm::vec3 posCruces[2] = {
	glm::vec3(27.10f, 0.14f, 12.87f),
	glm::vec3(37.62f, 0.40f, 12.95f)
};

// Ofrenda
// El altar mira hacia +Z (hacia el arco) y va en el centro de la explanada del atrio.
const glm::vec3 posAltar(-20.0f, 0.0f, 0.0f);
const float escalaAltar = 0.8f;   // 6.4 m de ancho y 3.6 m de alto
// Medidas del altar ya colocado, para acomodar flores, velas y la colisión
const float altarX0 = -20.0f - 4.0f * 0.8f, altarX1 = -20.0f + 4.0f * 0.8f;
const float altarZ0 = -1.42f * 0.8f, altarZ1 = 3.71f * 0.8f;

// Tumbas del panteón
// Montículo de tierra de cada tumba, medido de piso_panteon.obj: x0, x1, z0, z1
struct Tumba { float x0, x1, z0, z1; };
Tumba tumbas[6] = {
	{ 26.22f, 27.98f, -28.06f, -23.06f },   // lápida (27.11, -27.81)
	{ 36.78f, 38.53f, -28.06f, -23.06f },   // lápida (37.66, -27.81)
	{ 26.26f, 28.01f,  -8.22f,  -3.22f },   // lápida (27.14,  -7.97)
	{ 36.81f, 38.56f,  -8.00f,  -3.00f },   // lápida (37.68,  -7.75)
	{ 26.22f, 27.97f,  12.12f,  17.12f },   // cruz   (27.10,  12.87)
	{ 36.74f, 38.49f,  12.20f,  17.20f }    // cruz   (37.62,  12.95)
};
const float xPasillo = 32.3f;   // pasillo central del panteón, alineado con el portón

// Matrices de los pétalos y las flores
std::vector<glm::mat4> matPetalos;     // una matriz por cada quad de pétalos
std::vector<glm::mat4> matFlores;      // una matriz por cada flor
std::vector<glm::vec3> centrosPetalos;
std::mt19937 rngAdornos(2026);   // semilla fija: los adornos salen igual siempre

// Número al azar entre a y b
float azar(float a, float b)
{
	return std::uniform_real_distribution<float>(a, b)(rngAdornos);
}

// Altura del piso del panteón
// Cada tumba tiene un montículo de tierra, así que el piso no es plano.
// Se lee piso_panteon.obj para colocar flores, pétalos y velas a la altura correcta.
struct TrianguloPiso { glm::vec3 a, b, c; };
std::vector<TrianguloPiso> trianguloPiso;

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

// Regresa la altura del piso en (x, z); fuera del panteón regresa 0.
// Si se pasa 'normal', también regresa la inclinación del piso.
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

// Gira la matriz para que el eje Y del modelo apunte hacia 'n'
glm::mat4 alinearY(glm::mat4 m, glm::vec3 n)
{
	n = glm::normalize(n);
	float c = glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), n);
	if (c > 0.9999f) return m;
	if (c < -0.9999f) return glm::rotate(m, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 eje = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), n));
	return glm::rotate(m, acos(c), eje);
}

// Flor parada en el piso (revisa varios puntos para que no se entierre en las pendientes)
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

// Flor pegada a una pared, mirando hacia 'normal'
void agregarFlorPared(glm::vec3 p, glm::vec3 normal, float escala = 0.4f)
{
	glm::mat4 m(1.0f);
	m = glm::translate(m, p);
	m = alinearY(m, normal);
	m = glm::rotate(m, glm::radians(azar(0.0f, 360.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
	m = glm::scale(m, glm::vec3(escala * azar(0.9f, 1.1f)));
	matFlores.push_back(m);
}

// Estructura de la vela
// vela.obj mide 1 de alto con la base en y = 0; la llama se coloca en la punta (y = sY).
struct Vela
{
	glm::vec3 pos;   // base de la vela, ya sobre el piso
	float sXZ, sY;   // escala del grosor y de la altura
	float fase;      // desfase del parpadeo para que no titilen todas igual
};
std::vector<Vela> velas;

// Tamaño de las velas grande y chica
const float VELA_GRANDE_XZ = 0.32f, VELA_GRANDE_Y = 0.29f;   // ~10 cm de ancho, 29 cm de alto
const float VELA_CHICA_XZ = 0.22f, VELA_CHICA_Y = 0.16f;     // ~7 cm de ancho, 16 cm de alto

// Los adornos más lejos que esta distancia no se dibujan
const float DISTANCIA_ADORNOS = 45.0f;

// Distancia al cuadrado entre dos puntos
float distancia2(const glm::vec3& a, const glm::vec3& b)
{
	glm::vec3 d = a - b;
	return d.x * d.x + d.y * d.y + d.z * d.z;
}

// Agrega una vela grande o chica sobre el piso
void agregarVela(glm::vec3 p, bool grande)
{
	Vela v;
	v.sXZ = (grande ? VELA_GRANDE_XZ : VELA_CHICA_XZ) * azar(0.95f, 1.05f);
	v.sY = (grande ? VELA_GRANDE_Y : VELA_CHICA_Y) * azar(0.9f, 1.1f);
	// Se hunde un centímetro para que no flote en las pendientes
	v.pos = glm::vec3(p.x, alturaPiso(p.x, p.z) - 0.01f, p.z);
	v.fase = azar(0.0f, 6.2832f);
	velas.push_back(v);
}

// Revisa si un punto cae sobre el camino de pétalos
bool sobreSendero(float x, float z, float radio = 0.55f)
{
	for (const glm::vec3& p : centrosPetalos)
		if (glm::length(glm::vec2(x, z) - glm::vec2(p.x, p.z)) < radio) return true;
	return false;
}

// Tramo recto del sendero: pétalos al centro, flores en las orillas
// y una vela entre cada par de flores
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
			+ 0.012f + 0.002f * (float)(matPetalos.size() % 8);   // cada pétalo un poco más alto para evitar z-fighting
		glm::mat4 m(1.0f);
		m = glm::translate(m, p);
		m = alinearY(m, nPiso);   // sigue la inclinación del piso
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
	// Velas entre las flores, intercalando grande y chica
	int k = 0;
	for (float s = pasoFlor; s < largo - 0.3f; s += pasoFlor, k++)
	{
		agregarVela(a + dir * s + lado * orilla, k % 2 == 0);
		agregarVela(a + dir * s - lado * orilla, k % 2 == 1);
	}
}

// Marco de 10 flores y 10 velas alrededor de una tumba
void agregarContornoTumba(const Tumba& t)
{
	const float sep = 0.25f;   // separación del montículo
	const float escala = 0.28f;   // tamaño de las flores del marco
	float x0 = t.x0 - sep, x1 = t.x1 + sep, z0 = t.z0 - sep, z1 = t.z1 + sep;
	float xm = (x0 + x1) * 0.5f;
	float t1 = z0 + (z1 - z0) / 3.0f, t2 = z0 + 2.0f * (z1 - z0) / 3.0f;

	// Posición de las 10 flores: 4 esquinas, 1 en cada lado corto y 2 en cada lado largo
	glm::vec3 marco[10] = {
		glm::vec3(x0, 0.0f, z0), glm::vec3(xm, 0.0f, z0), glm::vec3(x1, 0.0f, z0),
		glm::vec3(x1, 0.0f, t1), glm::vec3(x1, 0.0f, t2), glm::vec3(x1, 0.0f, z1),
		glm::vec3(xm, 0.0f, z1), glm::vec3(x0, 0.0f, z1),
		glm::vec3(x0, 0.0f, t2), glm::vec3(x0, 0.0f, t1)
	};
	for (int i = 0; i < 10; i++)
	{
		agregarFlor(marco[i], escala);
		// Vela entre esta flor y la siguiente
		glm::vec3 medio = (marco[i] + marco[(i + 1) % 10]) * 0.5f;
		agregarVela(medio, i % 2 == 0);
	}
}

// Quita las flores y velas que quedaron encima del camino
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

// Flores en el arco de entrada del atrio
// El hueco del arco tiene lados rectos hasta y = 4 y un medio círculo de radio 2.5.
const glm::vec3 posArco(-20.0f, -0.25f, 50.0f);

// Una flor cada metro alrededor del hueco, en las dos caras del arco
void agregarFloresArco()
{
	const float semiAncho = 2.5f, yRecto = 4.0f, radio = 2.5f;
	const float separacion = 0.3f;   // separación del borde del hueco
	const float r = radio + separacion;
	const float xLado = semiAncho + separacion;
	const float yInicio = 0.5f;   // altura de la primera flor
	const float paso = 1.0f;         // una flor por metro

	const float largoRecto = yRecto - yInicio;
	const float largoCurva = 3.14159265f * r;
	const float total = 2.0f * largoRecto + largoCurva;

	for (float s = 0.0f; s <= total + 0.01f; s += paso)
	{
		glm::vec3 local;
		if (s < largoRecto)   // lado izquierdo
			local = glm::vec3(-xLado, yInicio + s, 0.0f);
		else if (s < largoRecto + largoCurva)   // medio círculo
		{
			float ang = 3.14159265f * (1.0f - (s - largoRecto) / largoCurva);
			local = glm::vec3(r * cos(ang), yRecto + r * sin(ang), 0.0f);
		}
		else   // lado derecho
			local = glm::vec3(xLado, yRecto - (s - largoRecto - largoCurva), 0.0f);

		agregarFlorPared(posArco + local + glm::vec3(0.0f, 0.0f, 0.26f), glm::vec3(0.0f, 0.0f, 1.0f));   // cara de afuera
		agregarFlorPared(posArco + local + glm::vec3(0.0f, 0.0f, -0.26f), glm::vec3(0.0f, 0.0f, -1.0f));   // cara del atrio
	}
}

// Flores al pie de una barda, de A a B, en dos hileras en zigzag.
// Con velas, pone una vela entre cada par de flores.
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

		// Vela entre esta flor y la siguiente
		if (conVelas && s + paso < largo - 0.3f)
			agregarVela(a + dir * (s + paso * 0.5f) + adentro * 0.65f, i % 2 == 0);
	}
}

// Recorre las cuatro bardas del panteón por dentro
void agregarFloresPerimetroPanteon()
{
	const glm::vec3 masX(1.0f, 0.0f, 0.0f), menosX(-1.0f, 0.0f, 0.0f);
	const glm::vec3 masZ(0.0f, 0.0f, 1.0f), menosZ(0.0f, 0.0f, -1.0f);

	// Barda oeste, sin tapar el corredor
	agregarFloresBarda(glm::vec3(15.25f, 0.0f, -49.75f), glm::vec3(15.25f, 0.0f, -5.5f), masX);
	agregarFloresBarda(glm::vec3(15.25f, 0.0f, 5.5f), glm::vec3(15.25f, 0.0f, 49.75f), masX);
	// Barda este
	agregarFloresBarda(glm::vec3(49.75f, 0.0f, -49.75f), glm::vec3(49.75f, 0.0f, 49.75f), menosX);
	// Barda norte
	agregarFloresBarda(glm::vec3(15.25f, 0.0f, -49.75f), glm::vec3(49.75f, 0.0f, -49.75f), masZ);
	// Barda sur, sin tapar el portón
	agregarFloresBarda(glm::vec3(15.25f, 0.0f, 49.75f), glm::vec3(27.3f, 0.0f, 49.75f), menosZ);
	agregarFloresBarda(glm::vec3(37.3f, 0.0f, 49.75f), glm::vec3(49.75f, 0.0f, 49.75f), menosZ);
}

// Flores y velas alternadas al pie de las bardas del atrio, igual que en el panteón
// Bardas del atrio por dentro: oeste x = -49.75, este x = 9.75 (corredor en z = -4.88..4.88),
// norte z = -49.75 y sur z = 49.75 (arco de entrada entre x = -24.25 y -15.75).
void agregarFloresPerimetroAtrio()
{
	const glm::vec3 masX(1.0f, 0.0f, 0.0f), menosX(-1.0f, 0.0f, 0.0f);
	const glm::vec3 masZ(0.0f, 0.0f, 1.0f), menosZ(0.0f, 0.0f, -1.0f);

	// Barda oeste
	agregarFloresBarda(glm::vec3(-49.75f, 0.0f, -49.75f), glm::vec3(-49.75f, 0.0f, 49.75f), masX);
	// Barda este, sin tapar el corredor
	agregarFloresBarda(glm::vec3(9.75f, 0.0f, -49.75f), glm::vec3(9.75f, 0.0f, -5.3f), menosX);
	agregarFloresBarda(glm::vec3(9.75f, 0.0f, 5.3f), glm::vec3(9.75f, 0.0f, 49.75f), menosX);
	// Barda norte, detrás de la iglesia
	agregarFloresBarda(glm::vec3(-49.75f, 0.0f, -49.75f), glm::vec3(9.75f, 0.0f, -49.75f), masZ);
	// Barda sur, sin tapar el arco
	agregarFloresBarda(glm::vec3(-49.75f, 0.0f, 49.75f), glm::vec3(-24.6f, 0.0f, 49.75f), menosZ);
	agregarFloresBarda(glm::vec3(-15.4f, 0.0f, 49.75f), glm::vec3(9.75f, 0.0f, 49.75f), menosZ);
}

// Arriates de flores a los lados del portón del panteón, por dentro
void agregarFloresEntradas()
{
	// Arriates a los lados del portón
	for (int fila = 0; fila < 3; fila++)
	{
		float z = 49.25f - fila * 0.45f;
		float desfase = (fila % 2) * 0.22f;
		for (float x = 27.5f + desfase; x <= 29.7f; x += 0.45f) agregarFlor(glm::vec3(x, 0.0f, z));
		for (float x = 34.9f + desfase; x <= 37.1f; x += 0.45f) agregarFlor(glm::vec3(x, 0.0f, z));
	}
}

// Arma el sendero completo con sus flores y velas (se llama una vez al iniciar)
void construirSendero()
{
	// Altura del piso del panteón
	cargarPisoPanteon("assets/models/edificacion/piso_panteon.obj");

	// Camino de la ofrenda al pasillo central del panteón, pasando por el corredor
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

	// Pasillo central hacia las tumbas
	agregarTramo(glm::vec3(xPasillo, 0.0f, 0.0f), glm::vec3(xPasillo, 0.0f, -25.56f));
	agregarTramo(glm::vec3(xPasillo, 0.0f, 0.0f), glm::vec3(xPasillo, 0.0f, 14.66f));
	// Pasillo central hasta el portón
	agregarTramo(glm::vec3(xPasillo, 0.0f, 14.66f), glm::vec3(xPasillo, 0.0f, 49.2f));

	// Un ramal a cada tumba
	for (const Tumba& t : tumbas)
	{
		float zc = (t.z0 + t.z1) * 0.5f;
		float xFin = (t.x1 < xPasillo) ? t.x1 + 0.6f : t.x0 - 0.6f;
		agregarTramo(glm::vec3(xPasillo, 0.0f, zc), glm::vec3(xFin, 0.0f, zc));
	}

	// Flores y velas alrededor de la ofrenda
	{
		const float sep = 0.4f;   // separación del altar
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

	// Flores y velas en las bardas del panteón
	agregarFloresPerimetroPanteon();
	agregarFloresPerimetroAtrio();

	// Quita lo que quedó encima del camino
	quitarFloresSobreSendero();

	// Marco de cada tumba (después de la limpieza para que quede completo)
	for (const Tumba& t : tumbas)
		agregarContornoTumba(t);

	// Flores del arco del atrio y arriates del portón del panteón
	agregarFloresArco();
	agregarFloresEntradas();

	std::cout << "Sendero: " << matPetalos.size() << " quads de petalos, "
		<< matFlores.size() << " flores, " << velas.size() << " velas" << std::endl;
}

// Velocidad de cambio de la luz de colores
glm::vec3 Light1 = glm::vec3(0);

// Iluminación de noche (se usa junto con Shader/lighting.frag)

// Luz de la luna
const glm::vec3 LUNA_AMBIENTE(0.24f, 0.25f, 0.30f);
const glm::vec3 LUNA_DIFUSA(0.38f, 0.40f, 0.48f);
const glm::vec3 LUNA_ESPECULAR(0.08f, 0.08f, 0.10f);

// Niebla
const glm::vec3 NIEBLA_COLOR(0.02f, 0.025f, 0.05f);
const float NIEBLA_DENSIDAD = 0.014f;

// Luz de las velas
const glm::vec3 COLOR_VELA(1.0f, 0.55f, 0.20f);   // naranja cálido
const float BRILLO_VELAS = 1.3f;   // resplandor de todas las velas
const float INTENSIDAD_LUZ_VELA = 1.2f;   // luz de cada vela cercana
const float ALTURA_LUZ_VELAS = 2.5f;   // altura hasta donde llega el resplandor
const int   MAX_LUCES_VELA = 16;   // igual que en el shader

// Mapa de luz de las velas (textura vista desde arriba, de -60 a 60 m)
const glm::vec2 MAPA_MIN(-60.0f, -60.0f);
const float MAPA_TAM = 120.0f;
const int   MAPA_RES = 512;   // resolución del mapa
const float RADIO_RESPLANDOR = 3.0f;   // alcance de la luz de una vela

bool linterna = false;   // se prende con F

// Faroles
// farol1.obj es solo la cabeza del farol (0.73 de alto, sin textura); se pone sobre
// un poste de hierro hecho con el cubo, con una llama adentro.
struct Farol
{
	glm::vec3 pos;   // base del poste, sobre el piso
};
const int   MAX_FAROLES = 20;   // igual que en el shader
const float ALTURA_POSTE = 2.4f;
const float ESCALA_FAROL = 1.4f;   // cabeza de 1 m de alto
const glm::vec3 COLOR_FAROL(1.0f, 0.78f, 0.45f);   // amarillo cálido
const float INTENSIDAD_FAROL = 1.8f;
const glm::vec3 COLOR_HIERRO(0.10f, 0.09f, 0.08f);

std::vector<Farol> faroles = {
	// Atrio: uno en cada esquina
	{ glm::vec3(-47.0f, 0.0f, -47.0f) }, { glm::vec3(7.0f, 0.0f, -47.0f) },
	{ glm::vec3(-47.0f, 0.0f, 44.0f) }, { glm::vec3(7.0f, 0.0f, 44.0f) },
	// Calle: banqueta junto a las bardas (z = 50.25..53.5), a los lados del arco y del portón
	{ glm::vec3(-26.5f, 0.0f, 52.7f) }, { glm::vec3(-13.5f, 0.0f, 52.7f) },
	{ glm::vec3(27.5f, 0.0f, 52.7f) }, { glm::vec3(37.1f, 0.0f, 52.7f) },
	// Calle: banqueta de enfrente (z = 63.5..67), junto a los árboles
	{ glm::vec3(-40.0f, 0.0f, 64.3f) }, { glm::vec3(-5.0f, 0.0f, 64.3f) },
	{ glm::vec3(25.0f, 0.0f, 64.3f) }
};

// Punto de donde sale la luz del farol (dentro de la cabeza)
glm::vec3 luzFarol(const Farol& f)
{
	return f.pos + glm::vec3(0.0f, ALTURA_POSTE + 0.3f * ESCALA_FAROL, 0.0f);
}

// Parpadeo de una vela (la llama y su luz usan el mismo)
float parpadeoVela(const Vela& v, float tiempo)
{
	return 1.0f + 0.08f * sin(tiempo * 12.0f + v.fase)
		+ 0.04f * sin(tiempo * 23.0f + 2.0f * v.fase);
}

// Crea el mapa de luz de las velas; se calcula una sola vez al iniciar
GLuint crearMapaLuzVelas()
{
	std::vector<float> luz(MAPA_RES * MAPA_RES, 0.0f);
	const float texel = MAPA_TAM / MAPA_RES;
	const int rt = (int)(RADIO_RESPLANDOR / texel) + 1;

	for (const Vela& v : velas)
	{
		float fuerza = v.sXZ / VELA_GRANDE_XZ;   // las chicas alumbran menos
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
				luz[j * MAPA_RES + i] += fuerza * t * t;   // se desvanece hacia la orilla
			}
	}
	// Evita que se queme el color donde hay muchas velas juntas
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

// Manda al shader las velas más cercanas a la cámara como luces.
// Las que están por salir de la lista se apagan poco a poco.
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
		pos[k] = v.pos + glm::vec3(0.0f, v.sY + 0.12f * v.sXZ, 0.0f);   // centro de la llama
		inten[k] = INTENSIDAD_LUZ_VELA * (v.sXZ / VELA_GRANDE_XZ) * parpadeoVela(v, tiempo) * desvanecer;
	}

	glUniform1i(glGetUniformLocation(programa, "numLucesVela"), n);
	if (n > 0)
	{
		glUniform3fv(glGetUniformLocation(programa, "luzVelaPos"), n, glm::value_ptr(pos[0]));
		glUniform1fv(glGetUniformLocation(programa, "luzVelaInt"), n, inten);
	}
}

// Animación del esqueleto
float skelWalkCycle = 0.0f;
bool isWalking = false;

float skelArmAngle = 0.0f;
float skelLegAngle = 0.0f;
float skelForearmAngle = 0.0f;
float skelKneeAngle = 0.0f;

float rotBall = 0.0f;
bool AnimBall = false;
bool AnimDog = false;
float rotDog = 0.0f;
int dogAnim = 0;
float FLegs = 0.0f;
float RLegs = 0.0f;
float head = 0.0f;
float tail = 0.0f;
glm::vec3 dogPos(0.0f, 0.0f, 0.0f);
float dogRot = 0.0f;
bool step = false;

// Tiempo entre cuadros
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Colisiones
struct BoundingBox {
	float minX, maxX;
	float minZ, maxZ;
};

std::vector<BoundingBox> invisibleWalls = {
	{  -120.0f, 120.0f, 80.0f, 85.0f },		//Muro limite invisible norte
	{  -120.0f, 120.0f, -55.0f, -49.8f },	//Muro limite invisible sur
	{  -65.0f, -60.0f, -80.0f, 80.0f },		//Muro limite invisible este
	{  60.0f, 65.0f, -80.0f, 80.0f },		//Muro limite invisible oeste
	{	-50.0f, -49.75f, -60.0f, 50.0f },	//Muro este
	{	49.75f, 50.0f, -60.0f, 50.0f },		//Muro oeste
	{  10.0f, 15.0f,  5.0f,  40.0f },		//Muro interior 1
	{  10.0f, 15.0f, -60.0f,  -4.75f },		//Muro interior 2
	{  -60.0f, -22.9f, 49.90f,  50.00f },	//Muro norte 1
	{  -17.0f, 30.35f, 49.90f,  50.00f },	//Muro norte 2
	{  34.4f, 60.0f, 49.90f,  50.00f },		//Muro norte 3
	{ -33.0f, -6.0f, -45.0f,  -14.0f },		//Iglesia
	{ -23.3f, -16.7f, -1.25f,  3.1f }		// ofrenda
};

// Revisa si el esqueleto choca con alguna pared invisible
bool CheckCollision(glm::vec3 pos)
{
	float skeletonRadius = 0.5f;

	for (const auto& wall : invisibleWalls)
	{
		if (pos.x + skeletonRadius >= wall.minX && pos.x - skeletonRadius <= wall.maxX &&
			pos.z + skeletonRadius >= wall.minZ && pos.z - skeletonRadius <= wall.maxZ)
		{
			return true;
		}
	}
	return false;
}

int main()
{
	// Inicializar GLFW y crear la ventana
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto Dia de Muertos", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	// Inicializar GLEW
	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// Shaders
	Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
	Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");
	Shader SkyBoxshader("Shader/SkyBox.vs", "Shader/SkyBox.frag");

	// Carga de modelos: esqueleto
	Model skeleton((char*)"assets/models/skeleton/skeleton.obj");
	Model craneo((char*)"assets/models/skeleton/craneo.obj");
	Model antebrazoDer((char*)"assets/models/skeleton/antebrazoDer.obj");
	Model antebrazoIzq((char*)"assets/models/skeleton/antebrazoIzq.obj");
	Model brazoDer((char*)"assets/models/skeleton/brazoDer.obj");
	Model brazoIzq((char*)"assets/models/skeleton/brazoIzq.obj");
	Model femurDer((char*)"assets/models/skeleton/femurDer.obj");
	Model femurIzq((char*)"assets/models/skeleton/femurIzq.obj");
	Model manoDer((char*)"assets/models/skeleton/manoDer.obj");
	Model manoIzq((char*)"assets/models/skeleton/manoIzq.obj");
	Model pecho((char*)"assets/models/skeleton/pecho.obj");
	Model pieDer((char*)"assets/models/skeleton/pieDer.obj");
	Model pieIzq((char*)"assets/models/skeleton/pieIzq.obj");
	Model piernaDer((char*)"assets/models/skeleton/piernaDer.obj");
	Model piernaIzq((char*)"assets/models/skeleton/piernaIzq.obj");

	// Carga de modelos: edificación y adornos
	Model arcoEntrada((char*)"assets/models/edificacion/atrio_arco_entrada.obj");
	Model corredor((char*)"assets/models/edificacion/corredor_panteon.obj");
	Model cruzMadera1((char*)"assets/models/edificacion/cruz_pequena.obj");
	Model cruzMadera2((char*)"assets/models/edificacion/cruz_diminuta.obj");
	Model cruzPanteon((char*)"assets/models/edificacion/cruz_panteon.obj");
	Model farol1((char*)"assets/models/edificacion/farol1.obj");
	Model iglesia((char*)"assets/models/edificacion/iglesia.obj");
	Model lapida1((char*)"assets/models/edificacion/lapida.obj");
	Model murosCementerio((char*)"assets/models/edificacion/barda_panteon.obj");
	Model paredesPatio((char*)"assets/models/edificacion/atrio_barda.obj");
	Model petaloCempasuchil((char*)"assets/models/edificacion/petaloCempasuchil.obj");
	Model pisoAdoquin((char*)"assets/models/edificacion/atrio_explanada.obj");
	Model pisoCementerio((char*)"assets/models/edificacion/piso_panteon.obj");
	Model portonDerecha((char*)"assets/models/edificacion/porton_der.obj");
	Model portonIzquierda((char*)"assets/models/edificacion/porton_izq.obj");
	Model portonMarco((char*)"assets/models/edificacion/porton_marco.obj");
	Model altar((char*)"assets/models/edificacion/altar.obj");
	Model papelPicadoAltar((char*)"assets/models/edificacion/papelPicadoAltar.obj");
	Model avenida((char*)"assets/models/edificacion/avenida.obj");
	Model bosque((char*)"assets/models/edificacion/bosque.obj");
	Model cesped((char*)"assets/models/edificacion/cesped.obj");
	Model cempasuchil((char*)"assets/models/edificacion/cempasuchil.obj");
	Model senderoPetalos((char*)"assets/models/edificacion/sendero_petalos.obj");
	Model vela((char*)"assets/models/edificacion/vela.obj");
	Model velaLlama((char*)"assets/models/edificacion/vela_llama.obj");
	Model farol((char*)"assets/models/edificacion/farol1.obj");

	// Papel picado
	glm::mat4 modeloBardas = glm::mat4(1.0f);
	PapelPicado papelPicado;
	papelPicado.cargar("Imagenes");
	// Altura a la que se amarran los cordones del papel picado
	const float yAmarrePapel = 3.5f;

	// Tiras del panteón, de barda a barda
	{
		const float zsPanteon[] = { -36.0f, -19.5f, 7.5f, 27.0f, 45.0f };
		const float caidasPanteon[] = { 0.15f, 0.14f, 0.14f, 0.16f, 0.15f };
		for (int k = 0; k < 5; k++)
		{
			glm::vec3 a = glm::vec3(modeloBardas * glm::vec4(15.0f, yAmarrePapel, zsPanteon[k], 1.0f));
			glm::vec3 b = glm::vec3(modeloBardas * glm::vec4(50.0f, yAmarrePapel, zsPanteon[k], 1.0f));
			papelPicado.agregarTira(a, b, caidasPanteon[k], 0.55f);
		}
	}

	// Tiras del atrio, de barda a barda (algunas en diagonal para esquivar los árboles)
	const glm::vec2 tirasAtrio[] = {
		glm::vec2(-11.5f, -10.5f),   // la más cercana a la iglesia
		glm::vec2(14.0f,  11.0f),
		glm::vec2(27.0f,  29.5f),
		glm::vec2(46.5f,  46.5f)    // junto al arco de entrada
	};
	for (const glm::vec2& t : tirasAtrio)
		papelPicado.agregarTira(glm::vec3(-50.0f, yAmarrePapel, t.x), glm::vec3(10.02f, yAmarrePapel, t.y), 0.15f, 0.55f);

	// Sendero de cempasúchil, flores y velas
	construirSendero();

	// Los postes de los faroles también chocan
	for (const Farol& f : faroles)
		invisibleWalls.push_back({ f.pos.x - 0.25f, f.pos.x + 0.25f, f.pos.z - 0.25f, f.pos.z + 0.25f });

	// Cubo de la luz de prueba
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Skybox
	GLuint skyboxVAO, skyboxVBO, skyboxEBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);

	// Texturas del skybox (orden +X, -X, +Y, -Y, +Z, -Z)
	std::vector<const GLchar*> faces;
	faces.push_back("assets/textures/sky_right.png");
	faces.push_back("assets/textures/sky_left.png");
	faces.push_back("assets/textures/sky_top.png");
	faces.push_back("assets/textures/sky_bottom.png");
	faces.push_back("assets/textures/sky_front.png");
	faces.push_back("assets/textures/sky_back.png");

	stbi_set_flip_vertically_on_load(false);   // los cubemaps no se voltean
	GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);

	SkyBoxshader.Use();
	glUniform1i(glGetUniformLocation(SkyBoxshader.Program, "skybox"), 0);

	// Texturas del material
	lightingShader.Use();
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);

	// Valores fijos de la iluminación de noche
	GLuint mapaLuzVelas = crearMapaLuzVelas();   // se crea después de construirSendero
	glUniform1i(glGetUniformLocation(lightingShader.Program, "mapaVelas"), 5);   // unidad de textura 5
	glUniform2f(glGetUniformLocation(lightingShader.Program, "mapaMin"), MAPA_MIN.x, MAPA_MIN.y);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "mapaTam"), MAPA_TAM);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "alturaLuzVelas"), ALTURA_LUZ_VELAS);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "colorVela"), COLOR_VELA.x, COLOR_VELA.y, COLOR_VELA.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "colorNiebla"), NIEBLA_COLOR.x, NIEBLA_COLOR.y, NIEBLA_COLOR.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "densidadNiebla"), NIEBLA_DENSIDAD);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "emisivo"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "usarColor"), 0);

	// Luz de los faroles (no se mueven, se mandan una sola vez)
	{
		int n = std::min((int)faroles.size(), MAX_FAROLES);
		glm::vec3 posFaroles[MAX_FAROLES];
		for (int i = 0; i < n; i++) posFaroles[i] = luzFarol(faroles[i]);
		glUniform1i(glGetUniformLocation(lightingShader.Program, "numFaroles"), n);
		glUniform3fv(glGetUniformLocation(lightingShader.Program, "farolPos"), n, glm::value_ptr(posFaroles[0]));
		glUniform3f(glGetUniformLocation(lightingShader.Program, "colorFarol"), COLOR_FAROL.x, COLOR_FAROL.y, COLOR_FAROL.z);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "intensidadFarol"), INTENSIDAD_FAROL);
	}

	// Proyección
	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);

	// Música de fondo en bucle
	mciSendString(TEXT("open \"assets/audio/lloronaCancion.mp3\" type mpegvideo alias musicaFondo"), NULL, 0, NULL);
	mciSendString(TEXT("play musicaFondo repeat"), NULL, 0, NULL);

	// Ciclo principal
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		DoMovement();
		Animation();

		float distanceFromSkeleton = 2.5f;
		glm::vec3 camPos = skeletonPos - (camera.GetFront() * distanceFromSkeleton) + glm::vec3(0.0f, 1.8f, 0.0f);
		camera.SetPosition(camPos);

		glClearColor(NIEBLA_COLOR.x, NIEBLA_COLOR.y, NIEBLA_COLOR.z, 1.0f);   // color de fondo de noche
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);

		glm::mat4 modelTemp = glm::mat4(1.0f);

		// Iluminación
		lightingShader.Use();

		glUniform1i(glGetUniformLocation(lightingShader.Program, "diffuse"), 0);

		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

		// Luna
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), LUNA_AMBIENTE.x, LUNA_AMBIENTE.y, LUNA_AMBIENTE.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), LUNA_DIFUSA.x, LUNA_DIFUSA.y, LUNA_DIFUSA.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), LUNA_ESPECULAR.x, LUNA_ESPECULAR.y, LUNA_ESPECULAR.z);

		// Velas: mapa de luz y velas cercanas
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, mapaLuzVelas);
		glActiveTexture(GL_TEXTURE0);
		float pulso = 1.0f + 0.04f * sin(currentFrame * 9.0f) + 0.03f * sin(currentFrame * 17.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "brilloVelas"), BRILLO_VELAS * pulso);
		mandarLucesVelas(lightingShader.Program, camera.GetPosition(), currentFrame);

		// Luz de colores (Espacio)
		glm::vec3 lightColor;
		lightColor.x = abs(sin(glfwGetTime() * Light1.x));
		lightColor.y = abs(sin(glfwGetTime() * Light1.y));
		lightColor.z = sin(glfwGetTime() * Light1.z);

		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 1.0f, 0.2f, 0.2f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.045f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.075f);

		// Linterna de la cámara (F)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
		float lin = linterna ? 1.0f : 0.0f;
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.9f * lin, 0.85f * lin, 0.7f * lin);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.2f * lin, 0.2f * lin, 0.2f * lin);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.09f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.032f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(18.0f)));

		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 5.0f);

		// Matrices de vista y proyección
		glm::mat4 view = camera.GetViewMatrix();

		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glm::mat4 model(1.0f);

		// Escenario
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-20.0f, -0.25f, 50.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
		arcoEntrada.Draw(lightingShader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(12.5f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		corredor.Draw(lightingShader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-20.0f, 0.0f, -30.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		iglesia.Draw(lightingShader);

		model = modeloBardas;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		murosCementerio.Draw(lightingShader);

		model = glm::mat4(1.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		paredesPatio.Draw(lightingShader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-20.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		pisoAdoquin.Draw(lightingShader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.25f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		avenida.Draw(lightingShader);

		model = glm::mat4(1.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		bosque.Draw(lightingShader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.25f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		cesped.Draw(lightingShader);

		model = glm::mat4(1.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		pisoCementerio.Draw(lightingShader);
		portonMarco.Draw(lightingShader);
		// Estos modelos están en el origen y quedarían en medio del atrio;
		// para usarlos hay que darles su posición con glm::translate.
		// cruzPanteon.Draw(lightingShader);
		// cruzMadera1.Draw(lightingShader);
		// cruzMadera2.Draw(lightingShader);
		// lapida1.Draw(lightingShader);
		// farol1.Draw(lightingShader);

		// Lápidas y cruces
		for (int i = 0; i < 4; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, posLapidas[i]);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			lapida1.Draw(lightingShader);
		}
		for (int i = 0; i < 2; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, posCruces[i]);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			cruzMadera1.Draw(lightingShader);
		}

		// Portón: cada hoja gira sobre su bisagra
		glm::mat4 portonBase = glm::mat4(1.0f);
		portonBase = glm::translate(portonBase, portonCentro);
		portonBase = glm::rotate(portonBase, glm::radians(portonRotY), glm::vec3(0.0f, 1.0f, 0.0f));

		model = portonBase;
		model = glm::translate(model, glm::vec3(-portonAncho, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(anguloPorton), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		portonIzquierda.Draw(lightingShader);

		model = portonBase;
		model = glm::translate(model, glm::vec3(portonAncho, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-anguloPorton), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		portonDerecha.Draw(lightingShader);

		// Ofrenda
		model = glm::mat4(1.0f);
		model = glm::translate(model, posAltar);
		model = glm::scale(model, glm::vec3(escalaAltar));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
		altar.Draw(lightingShader);

		// Papel picado del altar
		model = glm::mat4(1.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		model = glm::translate(model, posAltar);
		model = glm::scale(model, glm::vec3(escalaAltar));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
		papelPicadoAltar.Draw(lightingShader);
		glDisable(GL_BLEND);
		glBindVertexArray(0);

		// Sendero, flores y velas (solo lo que está cerca de la cámara)
		glm::vec3 posCam = camera.GetPosition();
		const float dMax2 = DISTANCIA_ADORNOS * DISTANCIA_ADORNOS;

		GLint transpLoc = glGetUniformLocation(lightingShader.Program, "transparency");
		glUniform1i(transpLoc, 1);   // descarta el fondo transparente de los pétalos
		for (const glm::mat4& m : matPetalos)
		{
			if (distancia2(glm::vec3(m[3].x, m[3].y, m[3].z), posCam) > dMax2) continue;
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
			senderoPetalos.Draw(lightingShader);
		}
		glUniform1i(transpLoc, 0);
		for (const glm::mat4& m : matFlores)
		{
			if (distancia2(glm::vec3(m[3].x, m[3].y, m[3].z), posCam) > dMax2) continue;
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
			cempasuchil.Draw(lightingShader);
		}

		// Velas (las llamas se dibujan al final)
		for (const Vela& v : velas)
		{
			if (distancia2(v.pos, posCam) > dMax2) continue;
			glm::mat4 mv = glm::translate(glm::mat4(1.0f), v.pos);
			mv = glm::scale(mv, glm::vec3(v.sXZ, v.sY, v.sXZ));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mv));
			vela.Draw(lightingShader);
		}

		// Faroles: base y poste con el cubo, y la cabeza con farol1.obj, en color hierro
		GLint usarColorLoc = glGetUniformLocation(lightingShader.Program, "usarColor");
		glUniform1i(usarColorLoc, 1);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "colorPlano"), COLOR_HIERRO.x, COLOR_HIERRO.y, COLOR_HIERRO.z);
		for (const Farol& f : faroles)
		{
			glBindVertexArray(VAO);

			glm::mat4 mf = glm::translate(glm::mat4(1.0f), f.pos + glm::vec3(0.0f, 0.1f, 0.0f));
			mf = glm::scale(mf, glm::vec3(0.36f, 0.2f, 0.36f));   // base
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mf));
			glDrawArrays(GL_TRIANGLES, 0, 36);

			mf = glm::translate(glm::mat4(1.0f), f.pos + glm::vec3(0.0f, ALTURA_POSTE * 0.5f, 0.0f));
			mf = glm::scale(mf, glm::vec3(0.1f, ALTURA_POSTE, 0.1f));   // poste
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mf));
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);

			mf = glm::translate(glm::mat4(1.0f), f.pos + glm::vec3(0.0f, ALTURA_POSTE, 0.0f));
			mf = glm::scale(mf, glm::vec3(ESCALA_FAROL));   // cabeza
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mf));
			farol.Draw(lightingShader);
		}
		glUniform1i(usarColorLoc, 0);

		// Papel picado del atrio y del panteón
		papelPicado.dibujar(lightingShader, currentFrame);


		// Esqueleto
		model = glm::mat4(1.0f);
		model = glm::translate(model, skeletonPos);
		model = glm::rotate(model, glm::radians(skeletonYaw), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);

		modelTemp = model;

		// Pecho
		pecho.Draw(lightingShader);

		// Cráneo
		model = modelTemp;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		craneo.Draw(lightingShader);

		// Brazo derecho: gira desde el hombro y el codo
		const glm::vec3 hombroDer(-0.28f, 1.30f, 0.0f);
		const glm::vec3 codoDer(-0.286f, 1.00f, -0.02f);
		const glm::vec3 hombroIzq(0.28f, 1.30f, 0.0f);
		const glm::vec3 codoIzq(0.286f, 1.00f, -0.02f);
		const glm::vec3 caderaDer(-0.10f, 0.90f, 0.0f);
		const glm::vec3 rodillaDer(-0.10f, 0.54f, -0.02f);
		const glm::vec3 caderaIzq(0.10f, 0.90f, 0.0f);
		const glm::vec3 rodillaIzq(0.10f, 0.54f, -0.02f);

		// Hombro derecho
		glm::mat4 brazoDerModel = modelTemp;
		brazoDerModel = glm::translate(brazoDerModel, hombroDer);
		brazoDerModel = glm::rotate(brazoDerModel, glm::radians(skelArmAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		brazoDerModel = glm::translate(brazoDerModel, -hombroDer);
		model = brazoDerModel;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		brazoDer.Draw(lightingShader);

		// Codo derecho
		model = brazoDerModel;
		model = glm::translate(model, codoDer);
		model = glm::rotate(model, glm::radians(skelForearmAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, -codoDer);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		antebrazoDer.Draw(lightingShader);
		manoDer.Draw(lightingShader);

		// Brazo izquierdo
		glm::mat4 brazoIzqModel = modelTemp;
		brazoIzqModel = glm::translate(brazoIzqModel, hombroIzq);
		brazoIzqModel = glm::rotate(brazoIzqModel, glm::radians(-skelArmAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		brazoIzqModel = glm::translate(brazoIzqModel, -hombroIzq);
		model = brazoIzqModel;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		brazoIzq.Draw(lightingShader);

		model = brazoIzqModel;
		model = glm::translate(model, codoIzq);
		model = glm::rotate(model, glm::radians(-skelForearmAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, -codoIzq);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		antebrazoIzq.Draw(lightingShader);
		manoIzq.Draw(lightingShader);

		// Pierna derecha: gira desde la cadera y la rodilla
		glm::mat4 piernaDerModel = modelTemp;
		piernaDerModel = glm::translate(piernaDerModel, caderaDer);
		piernaDerModel = glm::rotate(piernaDerModel, glm::radians(-skelLegAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		piernaDerModel = glm::translate(piernaDerModel, -caderaDer);
		model = piernaDerModel;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		femurDer.Draw(lightingShader);

		model = piernaDerModel;
		model = glm::translate(model, rodillaDer);
		model = glm::rotate(model, glm::radians(skelKneeAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, -rodillaDer);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		piernaDer.Draw(lightingShader);
		pieDer.Draw(lightingShader);

		// Pierna izquierda
		glm::mat4 piernaIzqModel = modelTemp;
		piernaIzqModel = glm::translate(piernaIzqModel, caderaIzq);
		piernaIzqModel = glm::rotate(piernaIzqModel, glm::radians(skelLegAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		piernaIzqModel = glm::translate(piernaIzqModel, -caderaIzq);
		model = piernaIzqModel;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		femurIzq.Draw(lightingShader);

		model = piernaIzqModel;
		model = glm::translate(model, rodillaIzq);
		model = glm::rotate(model, glm::radians(-skelKneeAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, -rodillaIzq);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		piernaIzq.Draw(lightingShader);
		pieIzq.Draw(lightingShader);

		// Resto del esqueleto
		model = modelTemp;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		skeleton.Draw(lightingShader);

		// Cubo de la luz de prueba
		lampShader.Use();
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// Solo se dibuja si DIBUJAR_LAMPARA es true
		const bool DIBUJAR_LAMPARA = false;
		if (DIBUJAR_LAMPARA)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, pointLightPositions[0]);
			model = glm::scale(model, glm::vec3(0.2f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		}

		// Skybox (al final de la escena)
		glDepthFunc(GL_LEQUAL);   // solo se pinta donde no hay geometría
		SkyBoxshader.Use();
		glm::mat4 skyView = glm::mat4(glm::mat3(camera.GetViewMatrix()));   // sin traslación: el cielo sigue a la cámara
		glUniformMatrix4fv(glGetUniformLocation(SkyBoxshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(skyView));
		glUniformMatrix4fv(glGetUniformLocation(SkyBoxshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);   // valor normal

		// Llamas de las velas
		// Van después del skybox para que no las tape; usan blending y no escriben profundidad.
		lightingShader.Use();
		modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint emisivoLoc = glGetUniformLocation(lightingShader.Program, "emisivo");
		glUniform1i(emisivoLoc, 1);   // la llama brilla sola
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);   // mezcla aditiva: se ve luminosa
		glDepthMask(GL_FALSE);
		for (const Vela& v : velas)
		{
			if (distancia2(v.pos, posCam) > dMax2) continue;
			float flick = parpadeoVela(v, currentFrame);   // mismo parpadeo que su luz
			float vaiven = 4.0f * sin(currentFrame * 3.0f + v.fase);   // vaivén en grados

			glm::mat4 l = glm::translate(glm::mat4(1.0f), v.pos + glm::vec3(0.0f, v.sY, 0.0f));
			l = glm::rotate(l, glm::radians(vaiven), glm::vec3(0.0f, 0.0f, 1.0f));
			l = glm::scale(l, glm::vec3(v.sXZ, v.sXZ * flick, v.sXZ));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(l));
			velaLlama.Draw(lightingShader);
		}
		// Llama dentro de cada farol
		for (const Farol& f : faroles)
		{
			float flick = 1.0f + 0.05f * sin(currentFrame * 10.0f + f.pos.x);
			glm::mat4 l = glm::translate(glm::mat4(1.0f), f.pos + glm::vec3(0.0f, ALTURA_POSTE + 0.12f * ESCALA_FAROL, 0.0f));
			l = glm::scale(l, glm::vec3(1.3f, 1.3f * flick, 1.3f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(l));
			velaLlama.Draw(lightingShader);
		}
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glUniform1i(emisivoLoc, 0);
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);

		glfwSwapBuffers(window);
	}

	// Liberar el skybox
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);
	glDeleteBuffers(1, &skyboxEBO);
	glDeleteTextures(1, &cubemapTexture);

	// Cerrar la música
	mciSendString(TEXT("close musicaFondo"), NULL, 0, NULL);

	glfwTerminate();
	return 0;
}

// Movimiento del esqueleto y la cámara
void DoMovement()
{
	GLfloat cameraSpeed = 5.0f * deltaTime;

	glm::vec3 front = camera.GetFront();
	front.y = 0.0f;
	front = glm::normalize(front);

	glm::vec3 right = camera.GetRight();
	right.y = 0.0f;
	right = glm::normalize(right);


	// Ángulo hacia donde mira la cámara
	float targetYaw = glm::degrees(atan2(front.x, front.z));

	// El esqueleto gira poco a poco hacia ese ángulo
	float rotationSpeed = 10.0f;   // más alto = gira más rápido

	// Evita giros raros al pasar de 360 a 0 grados
	float diff = targetYaw - skeletonYaw;
	while (diff < -180.0f) diff += 360.0f;
	while (diff > 180.0f) diff -= 360.0f;

	skeletonYaw += diff * glm::min(1.0f, rotationSpeed * deltaTime);

	glm::vec3 nextPos = skeletonPos;
	bool isMoving = false;

	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
	{
		nextPos += front * cameraSpeed;
		isMoving = true;
	}
	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
	{
		nextPos -= front * cameraSpeed;
		isMoving = true;
	}
	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
	{
		nextPos -= right * cameraSpeed;
		isMoving = true;
	}
	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
	{
		nextPos += right * cameraSpeed;
		isMoving = true;
	}

	if (isMoving)
	{
		if (!CheckCollision(nextPos))
		{
			skeletonPos = nextPos;
			isWalking = true;
		}
		else
		{
			isWalking = false;
		}
	}
	else
	{
		isWalking = false;
	}

	// Mover la luz de prueba
	if (keys[GLFW_KEY_T]) { pointLightPositions[0].x += 0.01f; }
	if (keys[GLFW_KEY_G]) { pointLightPositions[0].x -= 0.01f; }
	if (keys[GLFW_KEY_Y]) { pointLightPositions[0].y += 0.01f; }
	if (keys[GLFW_KEY_H]) { pointLightPositions[0].y -= 0.01f; }
	if (keys[GLFW_KEY_U]) { pointLightPositions[0].z -= 0.1f; }
	if (keys[GLFW_KEY_J]) { pointLightPositions[0].z += 0.01f; }
}

// Teclado
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}

	if (keys[GLFW_KEY_SPACE])
	{
		active = !active;
		if (active)
		{
			Light1 = glm::vec3(0.2f, 0.8f, 1.0f);
		}
		else
		{
			Light1 = glm::vec3(0);
		}
	}
	if (keys[GLFW_KEY_N])
	{
		AnimBall = !AnimBall;
	}
	if (keys[GLFW_KEY_B])
	{
		dogAnim = 1;
	}

	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		linterna = !linterna;
	}

	if (key == GLFW_KEY_O && action == GLFW_PRESS)
	{
		abrirPorton = !abrirPorton;
	}
}

// Animaciones
void Animation()
{
	if (isWalking)
	{
		skelWalkCycle += deltaTime * 7.0f;

		// Brazos y piernas alternan al caminar
		skelArmAngle = sin(skelWalkCycle) * 15.0f;
		skelLegAngle = sin(skelWalkCycle) * 16.0f;

		// Flexión del codo y la rodilla
		skelForearmAngle = (0.5f - 0.5f * cos(skelWalkCycle)) * 10.0f;
		skelKneeAngle = (0.5f - 0.5f * cos(skelWalkCycle + 3.14159265f)) * 12.0f;
	}
	else
	{
		skelArmAngle = 0.0f;
		skelLegAngle = 0.0f;
		skelForearmAngle = 0.0f;
		skelKneeAngle = 0.0f;
	}

	float objetivo = abrirPorton ? anguloMax : 0.0f;
	if (anguloPorton < objetivo)
		anguloPorton = glm::min(anguloPorton + velPorton * deltaTime, objetivo);
	else if (anguloPorton > objetivo)
		anguloPorton = glm::max(anguloPorton - velPorton * deltaTime, objetivo);
}

// Mouse
void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;

	lastX = xPos;
	lastY = yPos;

	GLfloat sensitivityMultiplier = 2.5f;
	xOffset *= sensitivityMultiplier;
	yOffset *= sensitivityMultiplier;

	camera.ProcessMouseMovement(xOffset, yOffset);
}