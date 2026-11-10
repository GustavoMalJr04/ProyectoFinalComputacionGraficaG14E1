#include <iostream>
#include <cmath>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include <string>
#include <windows.h>
#pragma comment(lib, "winmm.lib")

/// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include "stb_image.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Load Models
#include "SOIL2/SOIL2.h"


// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "PapelPicado.h"   // PAPEL PICADO: tiras con ondulacion en el panteon
#include "Texture.h"       // SKYBOX: TextureLoading::LoadCubemap

// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
void Animation();

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool active;

// Positions of the point lights
glm::vec3 pointLightPositions[] = {
	glm::vec3(0.0f,2.0f, 0.0f),
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

// ---------------- SKYBOX ----------------
GLfloat skyboxVertices[] = {
	// Positions
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

// Posición base del esqueleto en el mundo
glm::vec3 skeletonPos(-25.0f, 0.0f, 55.0f);
float skeletonYaw = 0.0f; // Ángulo de orientación guiado por el mouse/cámara

// ---------------- PORTON DEL PANTEON ----------------
glm::vec3 portonCentro(32.3f, 0.0f, 50.0f);
float portonRotY = 0.0f;
float portonAncho = 1.57f;
float anguloPorton = 0.0f;
float anguloMax = 90.0f;
float velPorton = 60.0f;
bool  abrirPorton = false;

// ---------------- TUMBAS DEL PANTEON ----------------
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

// ---------------- SENDERO DE CEMPASUCHIL (ofrenda -> tumbas) ----------------
// Montículos de tierra medidos de piso_panteon.obj: x0, x1, z0, z1
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

std::vector<glm::mat4> matPetalos;     // una matriz por cada quad de pétalos
std::vector<glm::mat4> matFlores;      // una matriz por cada flor
std::vector<glm::vec3> centrosPetalos;
std::mt19937 rngAdornos(2026);         // semilla fija: el sendero sale igual en cada ejecución

float azar(float a, float b)
{
	return std::uniform_real_distribution<float>(a, b)(rngAdornos);
}

// ---------------- ALTURA DEL PISO DEL PANTEON ----------------
// El piso del panteón no es plano: cada tumba tiene un montículo cuyas faldas
// se extienden varios metros. Para que flores y pétalos no queden enterrados se
// lee piso_panteon.obj y se busca la altura real del piso en (x, z).
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

// Altura del piso en (x, z). Fuera del panteón regresa 0 (atrio y corredor son planos).
// Si se pasa 'normal', ahí se guarda la inclinación del piso en ese punto.
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

// Flor parada en el piso. Toma la altura más alta bajo la flor para que en
// las faldas de los montículos no se entierre de un lado.
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

// Flor pegada a una pared (o encima de algo), con su eje Y mirando hacia 'normal'
void agregarFlorPared(glm::vec3 p, glm::vec3 normal, float escala = 0.4f)
{
	glm::mat4 m(1.0f);
	m = glm::translate(m, p);
	m = alinearY(m, normal);
	m = glm::rotate(m, glm::radians(azar(0.0f, 360.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
	m = glm::scale(m, glm::vec3(escala * azar(0.9f, 1.1f)));
	matFlores.push_back(m);
}

// ---------------- VELAS ----------------
// vela.obj: base en y = 0, borde de cera en y = 1.0, radio 0.15 (partes "cuerpo" y "mecha").
// vela_llama.obj: origen en la base de la llama; se pone en la punta (y = sY).
// Se intercalan una grande y una chica.
struct Vela
{
	glm::vec3 pos;   // base de la vela, ya sobre el piso
	float sXZ, sY;   // escala del grosor y de la altura
	float fase;      // desfase del parpadeo para que no titilen todas igual
};
std::vector<Vela> velas;

const float VELA_GRANDE_XZ = 0.40f, VELA_GRANDE_Y = 0.38f;   // ~12 cm de ancho, 38 cm de alto
const float VELA_CHICA_XZ = 0.28f, VELA_CHICA_Y = 0.21f;     // ~8 cm de ancho, 21 cm de alto

// Flores, pétalos y velas más lejos que esto de la cámara no se dibujan (ahorra FPS)
const float DISTANCIA_ADORNOS = 45.0f;

float distancia2(const glm::vec3& a, const glm::vec3& b)
{
	glm::vec3 d = a - b;
	return d.x * d.x + d.y * d.y + d.z * d.z;
}

void agregarVela(glm::vec3 p, bool grande)
{
	Vela v;
	v.sXZ = (grande ? VELA_GRANDE_XZ : VELA_CHICA_XZ) * azar(0.95f, 1.05f);
	v.sY = (grande ? VELA_GRANDE_Y : VELA_CHICA_Y) * azar(0.9f, 1.1f);
	// Asentada en el piso; un centímetro abajo para que en las faldas no flote de un lado
	v.pos = glm::vec3(p.x, alturaPiso(p.x, p.z) - 0.01f, p.z);
	v.fase = azar(0.0f, 6.2832f);
	velas.push_back(v);
}

// ¿El punto (x, z) cae sobre el tapete de pétalos?
bool sobreSendero(float x, float z, float radio = 0.55f)
{
	for (const glm::vec3& p : centrosPetalos)
		if (glm::length(glm::vec2(x, z) - glm::vec2(p.x, p.z)) < radio) return true;
	return false;
}

// Tramo recto de pétalos de A a B, con flores en las dos orillas
// y una vela en cada espacio entre flores (grande y chica intercaladas)
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
			+ 0.012f + 0.002f * (float)(matPetalos.size() % 8);   // capas: evita z-fighting
		glm::mat4 m(1.0f);
		m = glm::translate(m, p);
		m = alinearY(m, nPiso);                                  // el tapete se recarga en la falda del montículo
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
	// Velas a la mitad entre dos flores; de un lado empieza la grande y del otro la chica
	int k = 0;
	for (float s = pasoFlor; s < largo - 0.3f; s += pasoFlor, k++)
	{
		agregarVela(a + dir * s + lado * orilla, k % 2 == 0);
		agregarVela(a + dir * s - lado * orilla, k % 2 == 1);
	}
}

// Marco de 10 flores alrededor del montículo de una tumba, como las tumbas
// adornadas del Día de Muertos:
//   4 esquinas + 1 a la mitad de cada lado corto + 2 en cada lado largo = 10
void agregarContornoTumba(const Tumba& t)
{
	const float sep = 0.25f;     // qué tan afuera del montículo queda el marco
	const float escala = 0.45f;  // más grandes que las del sendero para que el marco se note
	float x0 = t.x0 - sep, x1 = t.x1 + sep, z0 = t.z0 - sep, z1 = t.z1 + sep;
	float xm = (x0 + x1) * 0.5f;
	float t1 = z0 + (z1 - z0) / 3.0f, t2 = z0 + 2.0f * (z1 - z0) / 3.0f;

	// Las 10 flores en orden alrededor del marco:
	// 4 esquinas + 1 a la mitad de cada lado corto + 2 en cada lado largo (a 1/3 y 2/3)
	glm::vec3 marco[10] = {
		glm::vec3(x0, 0.0f, z0), glm::vec3(xm, 0.0f, z0), glm::vec3(x1, 0.0f, z0),
		glm::vec3(x1, 0.0f, t1), glm::vec3(x1, 0.0f, t2), glm::vec3(x1, 0.0f, z1),
		glm::vec3(xm, 0.0f, z1), glm::vec3(x0, 0.0f, z1),
		glm::vec3(x0, 0.0f, t2), glm::vec3(x0, 0.0f, t1)
	};
	for (int i = 0; i < 10; i++)
	{
		agregarFlor(marco[i], escala);
		// Una vela en el espacio entre esta flor y la siguiente: 10 velas, grande y chica intercaladas
		glm::vec3 medio = (marco[i] + marco[(i + 1) % 10]) * 0.5f;
		agregarVela(medio, i % 2 == 0);
	}
}

// Quita las flores y velas que quedaron encima del camino (cruces, ramales, frente del altar)
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

// ---------------- FLORES EN EL ARCO DE ENTRADA ----------------
// atrio_arco_entrada.obj se dibuja en (-20, -0.25, 50). Hueco del arco (coordenadas locales):
// lados rectos en x = ±2.5 de y = 0 a 4, y medio círculo de centro (0, 4) y radio 2.5 (cima en 6.5).
// El arco mide 0.5 de grueso (caras en z = ±0.25).
const glm::vec3 posArco(-20.0f, -0.25f, 50.0f);

// Una flor cada metro siguiendo el contorno del hueco, en las dos caras del arco
void agregarFloresArco()
{
	const float semiAncho = 2.5f, yRecto = 4.0f, radio = 2.5f;
	const float separacion = 0.3f;   // qué tan afuera del borde del hueco quedan las flores
	const float r = radio + separacion;
	const float xLado = semiAncho + separacion;
	const float yInicio = 0.5f;      // primera flor a medio metro del suelo
	const float paso = 1.0f;         // una flor por metro

	const float largoRecto = yRecto - yInicio;
	const float largoCurva = 3.14159265f * r;
	const float total = 2.0f * largoRecto + largoCurva;

	for (float s = 0.0f; s <= total + 0.01f; s += paso)
	{
		glm::vec3 local;
		if (s < largoRecto)                                   // lado izquierdo, subiendo
			local = glm::vec3(-xLado, yInicio + s, 0.0f);
		else if (s < largoRecto + largoCurva)                 // medio círculo, de izquierda a derecha
		{
			float ang = 3.14159265f * (1.0f - (s - largoRecto) / largoCurva);
			local = glm::vec3(r * cos(ang), yRecto + r * sin(ang), 0.0f);
		}
		else                                                  // lado derecho, bajando
			local = glm::vec3(xLado, yRecto - (s - largoRecto - largoCurva), 0.0f);

		agregarFlorPared(posArco + local + glm::vec3(0.0f, 0.0f, 0.26f), glm::vec3(0.0f, 0.0f, 1.0f));    // cara de afuera (+Z)
		agregarFlorPared(posArco + local + glm::vec3(0.0f, 0.0f, -0.26f), glm::vec3(0.0f, 0.0f, -1.0f));  // cara del atrio (-Z)
	}
}

// ---------------- FLORES EN EL PERIMETRO DEL PANTEON ----------------
// Caras interiores de barda_panteon.obj (model = identidad):
//   oeste x = 15.25 (hueco del corredor z = -5..5), este x = 49.75,
//   norte z = -49.75, sur z = 49.75 (portón entre x = 29.9 y 34.7).
// Arriate al pie de la barda de A a B: dos hileras en zigzag, una flor cada 1.2 m.
void agregarFloresBarda(glm::vec3 a, glm::vec3 b, glm::vec3 adentro)
{
	const float paso = 1.2f;
	glm::vec3 d = b - a;
	float largo = glm::length(d);
	glm::vec3 dir = d / largo;
	int i = 0;
	for (float s = 0.4f; s < largo - 0.3f; s += paso, i++)
	{
		float fondo = (i % 2 == 0) ? 0.45f : 0.85f;
		agregarFlor(a + dir * (s + azar(-0.15f, 0.15f)) + adentro * fondo, 0.34f);

		// Vela en el espacio hasta la siguiente flor, entre las dos hileras
		if (s + paso < largo - 0.3f)
			agregarVela(a + dir * (s + paso * 0.5f) + adentro * 0.65f, i % 2 == 0);
	}
}

void agregarFloresPerimetroPanteon()
{
	const glm::vec3 masX(1.0f, 0.0f, 0.0f), menosX(-1.0f, 0.0f, 0.0f);
	const glm::vec3 masZ(0.0f, 0.0f, 1.0f), menosZ(0.0f, 0.0f, -1.0f);

	// Barda oeste (dos tramos, sin tapar el corredor)
	agregarFloresBarda(glm::vec3(15.25f, 0.0f, -49.75f), glm::vec3(15.25f, 0.0f, -5.5f), masX);
	agregarFloresBarda(glm::vec3(15.25f, 0.0f, 5.5f), glm::vec3(15.25f, 0.0f, 49.75f), masX);
	// Barda este
	agregarFloresBarda(glm::vec3(49.75f, 0.0f, -49.75f), glm::vec3(49.75f, 0.0f, 49.75f), menosX);
	// Barda norte
	agregarFloresBarda(glm::vec3(15.25f, 0.0f, -49.75f), glm::vec3(49.75f, 0.0f, -49.75f), masZ);
	// Barda sur, dejando libre el portón y los arriates de la entrada
	agregarFloresBarda(glm::vec3(15.25f, 0.0f, 49.75f), glm::vec3(27.3f, 0.0f, 49.75f), menosZ);
	agregarFloresBarda(glm::vec3(37.3f, 0.0f, 49.75f), glm::vec3(49.75f, 0.0f, 49.75f), menosZ);
}

// ---------------- FLORES EN LAS ENTRADAS DEL PANTEON ----------------
// Portón (barda sur): pilares de x = 29.9..30.7 y 33.9..34.7, alto 3.25,
// caras en z = 49.65 (adentro) y 50.35 (afuera). Hojas del portón entre 30.7 y 33.9.
// Corredor (barda oeste): hueco z = -5..5; la barda del panteón tiene su cara
// interior en x = 15.25 y la del atrio su cara en x = 9.75.
void agregarFloresEntradas()
{
	// Arriates a los dos lados del portón, por dentro: 3 hileras densas
	for (int fila = 0; fila < 3; fila++)
	{
		float z = 49.25f - fila * 0.45f;
		float desfase = (fila % 2) * 0.22f;
		for (float x = 27.5f + desfase; x <= 29.7f; x += 0.45f) agregarFlor(glm::vec3(x, 0.0f, z), 0.38f);
		for (float x = 34.9f + desfase; x <= 37.1f; x += 0.45f) agregarFlor(glm::vec3(x, 0.0f, z), 0.38f);
	}

	// Pilares del portón: flores subiendo por las dos caras y una grande encima
	const float xPilar[2] = { 30.3f, 34.3f };
	for (int k = 0; k < 2; k++)
	{
		for (float y = 0.5f; y <= 2.9f; y += 0.6f)
		{
			agregarFlorPared(glm::vec3(xPilar[k], y, 50.36f), glm::vec3(0.0f, 0.0f, 1.0f), 0.38f);
			agregarFlorPared(glm::vec3(xPilar[k], y, 49.64f), glm::vec3(0.0f, 0.0f, -1.0f), 0.38f);
		}
		agregarFlorPared(glm::vec3(xPilar[k], 3.25f, 50.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.6f);
	}

	// Entrada por el corredor: flores subiendo a los dos lados del hueco,
	// del lado del panteón (mirando a +X) y del lado del atrio (mirando a -X)
	const float zLado[2] = { -5.4f, 5.4f };
	for (int k = 0; k < 2; k++)
		for (float y = 0.5f; y <= 2.6f; y += 0.5f)
		{
			agregarFlorPared(glm::vec3(15.26f, y, zLado[k]), glm::vec3(1.0f, 0.0f, 0.0f), 0.35f);
			agregarFlorPared(glm::vec3(9.74f, y, zLado[k]), glm::vec3(-1.0f, 0.0f, 0.0f), 0.35f);
		}
}

void construirSendero()
{
	// 0. Leer el piso del panteón para conocer su altura (montículos de las tumbas)
	cargarPisoPanteon("assets/models/edificacion/piso_panteon.obj");

	// 1. Frente de la ofrenda -> corredor -> pasillo central del panteón
	glm::vec3 ruta[] = {
		glm::vec3(0.0f, 0.0f, 2.6f),  glm::vec3(5.0f, 0.0f, 2.2f),
		glm::vec3(9.0f, 0.0f, 0.3f),  glm::vec3(15.0f, 0.0f, 0.0f),
		glm::vec3(24.0f, 0.0f, 0.0f), glm::vec3(xPasillo, 0.0f, 0.0f)
	};
	for (int i = 0; i < 5; i++) agregarTramo(ruta[i], ruta[i + 1]);

	// 2. Pasillo central hacia las tumbas de los dos lados
	agregarTramo(glm::vec3(xPasillo, 0.0f, 0.0f), glm::vec3(xPasillo, 0.0f, -25.56f));
	agregarTramo(glm::vec3(xPasillo, 0.0f, 0.0f), glm::vec3(xPasillo, 0.0f, 14.66f));
	// ...y sigue hasta el portón, para que la entrada principal también tenga tapete
	agregarTramo(glm::vec3(xPasillo, 0.0f, 14.66f), glm::vec3(xPasillo, 0.0f, 49.2f));

	// 3. Un ramal a cada tumba (llega a 0.6 m del montículo)
	for (const Tumba& t : tumbas)
	{
		float zc = (t.z0 + t.z1) * 0.5f;
		float xFin = (t.x1 < xPasillo) ? t.x1 + 0.6f : t.x0 - 0.6f;
		agregarTramo(glm::vec3(xPasillo, 0.0f, zc), glm::vec3(xFin, 0.0f, zc));
	}

	// 4. Flores a los lados y al frente de la ofrenda (altar: x -2..2, z -0.7..1.9)
	for (float z = -0.6f; z <= 2.0f; z += 0.45f)
	{
		agregarFlor(glm::vec3(-2.35f, 0.0f, z));
		agregarFlor(glm::vec3(2.35f, 0.0f, z));
	}
	for (float x = -2.0f; x <= 2.01f; x += 0.45f)
		agregarFlor(glm::vec3(x, 0.0f, 2.3f));

	// 5. Arriates al pie de toda la barda del panteón
	agregarFloresPerimetroPanteon();

	quitarFloresSobreSendero();

	// 6. Marco de 10 flores en cada una de las 6 tumbas (después de la limpieza
	//    para que siempre queden las 10 completas, aunque toquen el ramal)
	for (const Tumba& t : tumbas)
		agregarContornoTumba(t);

	// 7. Flores en el arco del atrio y en las dos entradas del panteón
	agregarFloresArco();
	agregarFloresEntradas();

	std::cout << "Sendero: " << matPetalos.size() << " quads de petalos, "
		<< matFlores.size() << " flores, " << velas.size() << " velas" << std::endl;
}

glm::vec3 Light1 = glm::vec3(0);

//Animacion del Esqueleto
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

// Deltatime
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

//Estructuras para las colisiones
struct BoundingBox {
	float minX, maxX;
	float minZ, maxZ;
};

std::vector<BoundingBox> invisibleWalls = {
	{  -120.0f, 120.0f, 80.0f, 85.0f },
	{  -120.0f, 120.0f, -55.0f, -49.8f },
	{	-50.0f, -49.75f, -60.0f, 50.0f },
	{	49.75f, 50.0f, -60.0f, 50.0f },
	{  -65.0f, -60.0f, -80.0f, 80.0f },
	{  60.0f, 65.0f, -80.0f, 80.0f },
	{  10.0f, 15.0f,  5.0f,  40.0f },
	{  10.0f, 15.0f, -60.0f,  -4.75f }
};

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

	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
	Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");
	Shader SkyBoxshader("Shader/SkyBox.vs", "Shader/SkyBox.frag");

	// Modelos
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

	// Edificación
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

	glm::mat4 modeloBardas = glm::mat4(1.0f);
	PapelPicado papelPicado;
	papelPicado.cargar("Imagenes");
	// Altura de amarre de TODOS los cordones. Las bardas miden 2.75; antes se amarraban
	// a 2.76 y las banderitas quedaban muy bajas. Súbelo o bájalo aquí.
	const float yAmarrePapel = 3.5f;

	// ---------------- PAPEL PICADO EN EL PANTEON ----------------
	// Mismas tiras que papelPicado.colocarEnPanteon() (PapelPicado.h), pero a yAmarrePapel.
	// Barda oeste x = 15.0, barda este x = 50.0; se salta z = -5..5 (corredor) y los árboles.
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

	// ---------------- PAPEL PICADO EN EL ATRIO (frente a la iglesia) ----------------
	// atrio_barda.obj (model = identidad): barda oeste x = -50.0, barda este x = 10.02.
	// Cada par es (z en la barda oeste, z en la barda este). Algunas tiras van un poco
	// diagonales para esquivar las copas de bosque.obj que asoman sobre las bardas
	// y el hueco del corredor (z = -5..5 en la barda este).
	const glm::vec2 tirasAtrio[] = {
		glm::vec2(-11.5f, -10.5f),   // la más cercana a la fachada (z = -13.4)
		glm::vec2(14.0f,  11.0f),
		glm::vec2(27.0f,  29.5f),
		glm::vec2(46.5f,  46.5f)    // junto al arco de entrada
	};
	for (const glm::vec2& t : tirasAtrio)
		papelPicado.agregarTira(glm::vec3(-50.0f, yAmarrePapel, t.x), glm::vec3(10.02f, yAmarrePapel, t.y), 0.15f, 0.55f);

	// ---------------- SENDERO DE CEMPASUCHIL ----------------
	construirSendero();

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

	// ---------------- SKYBOX: VAO, VBO, EBO ----------------
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

	// ---------------- SKYBOX: texturas (orden +X, -X, +Y, -Y, +Z, -Z) ----------------
	std::vector<const GLchar*> faces;
	faces.push_back("assets/textures/sky_right.png");
	faces.push_back("assets/textures/sky_left.png");
	faces.push_back("assets/textures/sky_top.png");
	faces.push_back("assets/textures/sky_bottom.png");
	faces.push_back("assets/textures/sky_front.png");
	faces.push_back("assets/textures/sky_back.png");

	stbi_set_flip_vertically_on_load(false);   // los cubemaps NO se voltean
	GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);

	SkyBoxshader.Use();
	glUniform1i(glGetUniformLocation(SkyBoxshader.Program, "skybox"), 0);

	lightingShader.Use();
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);

	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);

	// Abrir y reproducir el MP3 en bucle (repeat)
	mciSendString(TEXT("open \"assets/audio/lloronaCancion.mp3\" type mpegvideo alias musicaFondo"), NULL, 0, NULL);
	mciSendString(TEXT("play musicaFondo repeat"), NULL, 0, NULL);

	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		DoMovement();
		Animation();

		camera.SetPosition(skeletonPos + glm::vec3(0.0f, 2.0f, -0.25f));

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);

		glm::mat4 modelTemp = glm::mat4(1.0f);

		lightingShader.Use();

		glUniform1i(glGetUniformLocation(lightingShader.Program, "diffuse"), 0);

		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.6f, 0.6f, 0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.6f, 0.6f, 0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.3f, 0.3f, 0.3f);

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

		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.2f, 0.2f, 0.8f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.2f, 0.2f, 0.8f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.3f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.7f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(18.0f)));

		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 5.0f);

		glm::mat4 view = camera.GetViewMatrix();

		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glm::mat4 model(1.0f);

		// Dibujar Entorno
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
		cruzPanteon.Draw(lightingShader);
		cruzMadera1.Draw(lightingShader);
		cruzMadera2.Draw(lightingShader);
		lapida1.Draw(lightingShader);
		farol1.Draw(lightingShader);

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

		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
		altar.Draw(lightingShader);

		model = glm::mat4(1.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
		papelPicadoAltar.Draw(lightingShader);
		glDisable(GL_BLEND);
		glBindVertexArray(0);

		// ---------------- SENDERO DE CEMPASUCHIL, FLORES Y VELAS ----------------
		// Solo se dibuja lo que está a menos de DISTANCIA_ADORNOS de la cámara:
		// más lejos se ve como un punto y dibujarlo solo baja los FPS.
		glm::vec3 posCam = camera.GetPosition();
		const float dMax2 = DISTANCIA_ADORNOS * DISTANCIA_ADORNOS;

		GLint transpLoc = glGetUniformLocation(lightingShader.Program, "transparency");
		glUniform1i(transpLoc, 1);   // descarta el fondo transparente de petalos_sendero.png
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

		// Cuerpo de las velas (cera + mecha). Las llamas van al final del cuadro, con blending.
		for (const Vela& v : velas)
		{
			if (distancia2(v.pos, posCam) > dMax2) continue;
			glm::mat4 mv = glm::translate(glm::mat4(1.0f), v.pos);
			mv = glm::scale(mv, glm::vec3(v.sXZ, v.sY, v.sXZ));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mv));
			vela.Draw(lightingShader);
		}

		papelPicado.dibujar(lightingShader, currentFrame);


		model = glm::mat4(1.0f);
		model = glm::translate(model, skeletonPos);
		model = glm::rotate(model, glm::radians(skeletonYaw), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);

		modelTemp = model;

		// 1. PECHO / TORSO
		pecho.Draw(lightingShader);

		// 2. CRÁNEO
		model = modelTemp;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		craneo.Draw(lightingShader);

		// 3. BRAZO DERECHO: rotación desde hombro y codo (no desde el origen del modelo)
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

		// Codo derecho: el antebrazo hereda el movimiento del hombro
		model = brazoDerModel;
		model = glm::translate(model, codoDer);
		model = glm::rotate(model, glm::radians(skelForearmAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, -codoDer);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		antebrazoDer.Draw(lightingShader);
		manoDer.Draw(lightingShader);

		// 4. BRAZO IZQUIERDO
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

		// 5. PIERNA DERECHA: movimiento desde la cadera y flexión desde la rodilla
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

		// 6. PIERNA IZQUIERDA
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

		// 7. ESQUELETO GENERAL BASE
		model = modelTemp;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		skeleton.Draw(lightingShader);

		// Lamp object
		lampShader.Use();
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		model = glm::mat4(1.0f);
		model = glm::translate(model, pointLightPositions[0]);
		model = glm::scale(model, glm::vec3(0.2f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		// ---------------- SKYBOX (siempre al final de la escena) ----------------
		glDepthFunc(GL_LEQUAL);   // el skybox queda a profundidad 1.0 y solo se pinta donde no hay geometria
		SkyBoxshader.Use();
		glm::mat4 skyView = glm::mat4(glm::mat3(camera.GetViewMatrix()));   // sin traslacion: el cielo sigue a la camara
		glUniformMatrix4fv(glGetUniformLocation(SkyBoxshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(skyView));
		glUniformMatrix4fv(glGetUniformLocation(SkyBoxshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);     // regresar al valor normal

		// ---------------- LLAMAS DE LAS VELAS (lo último del cuadro) ----------------
		// Van después del skybox para que el cielo no las tape. No escriben profundidad
		// (glDepthMask false) pero sí respetan la de la escena: una barda las tapa.
		lightingShader.Use();
		modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint ambienteLoc = glGetUniformLocation(lightingShader.Program, "dirLight.ambient");
		glUniform3f(ambienteLoc, 1.6f, 1.6f, 1.6f);   // la llama brilla sola, no depende de la luna
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);   // suma de color: se ve luminosa
		glDepthMask(GL_FALSE);
		for (const Vela& v : velas)
		{
			if (distancia2(v.pos, posCam) > dMax2) continue;
			// Parpadeo: dos senos con desfase propio para que cada vela titile distinto
			float flick = 1.0f + 0.08f * sin(currentFrame * 12.0f + v.fase)
				+ 0.04f * sin(currentFrame * 23.0f + 2.0f * v.fase);
			float vaiven = 4.0f * sin(currentFrame * 3.0f + v.fase);   // grados

			glm::mat4 l = glm::translate(glm::mat4(1.0f), v.pos + glm::vec3(0.0f, v.sY, 0.0f));
			l = glm::rotate(l, glm::radians(vaiven), glm::vec3(0.0f, 0.0f, 1.0f));
			l = glm::scale(l, glm::vec3(v.sXZ, v.sXZ * flick, v.sXZ));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(l));
			velaLlama.Draw(lightingShader);
		}
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glUniform3f(ambienteLoc, 0.6f, 0.6f, 0.6f);   // regresar la luz ambiente de la luna
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);

		glfwSwapBuffers(window);
	}

	// Liberar recursos del skybox
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);
	glDeleteBuffers(1, &skyboxEBO);
	glDeleteTextures(1, &cubemapTexture);

	// Detener y cerrar la música de fondo al salir
	mciSendString(TEXT("close musicaFondo"), NULL, 0, NULL);

	glfwTerminate();
	return 0;
}

void DoMovement()
{
	GLfloat cameraSpeed = 5.0f * deltaTime;

	glm::vec3 front = camera.GetFront();
	front.y = 0.0f;
	front = glm::normalize(front);

	glm::vec3 right = camera.GetRight();
	right.y = 0.0f;
	right = glm::normalize(right);


	// 1. Obtenemos el ángulo hacia donde apunta la cámara actualmente
	float targetYaw = glm::degrees(atan2(front.x, front.z));

	// 2. Interpolamos suavemente (Lerp) para que el esqueleto rote de forma retardada/más lenta que la cámara
	float rotationSpeed = 10.0f; // Sube este número si quieres que gire más rápido, bájalo para más inercia

	// Manejo seguro del salto de ángulo en 360 grados para evitar giros raros
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

	if (keys[GLFW_KEY_T]) { pointLightPositions[0].x += 0.01f; }
	if (keys[GLFW_KEY_G]) { pointLightPositions[0].x -= 0.01f; }
	if (keys[GLFW_KEY_Y]) { pointLightPositions[0].y += 0.01f; }
	if (keys[GLFW_KEY_H]) { pointLightPositions[0].y -= 0.01f; }
	if (keys[GLFW_KEY_U]) { pointLightPositions[0].z -= 0.1f; }
	if (keys[GLFW_KEY_J]) { pointLightPositions[0].z += 0.01f; }
}

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

	if (key == GLFW_KEY_O && action == GLFW_PRESS)
	{
		abrirPorton = !abrirPorton;
	}
}

void Animation()
{
	if (isWalking)
	{
		skelWalkCycle += deltaTime * 7.0f;

		// Brazos y piernas alternan como en una caminata natural.
		skelArmAngle = sin(skelWalkCycle) * 15.0f;
		skelLegAngle = sin(skelWalkCycle) * 16.0f;

		// Flexión suave del codo y de la rodilla durante el paso.
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