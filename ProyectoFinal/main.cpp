// Librerías
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

// stb_image
#include "stb_image.h"

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// SOIL2
#include "SOIL2/SOIL2.h"

// Clases del proyecto
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "PapelPicado.h"
#include "Texture.h"
#include "MariposaKeyframes.h"
#include "Humo.h"
#include "FichasCulturales.h"
#include "Bosque.h"
#include "Sendero.h"
#include "Iluminacion.h"

// Prototipos
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void FramebufferSizeCallback(GLFWwindow* window, int ancho, int alto);
void MouseButtonCallback(GLFWwindow* window, int boton, int accion, int mods);
void FocusCallback(GLFWwindow* window, int enfocada);
void DoMovement();
void Animation();

// Ventana
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Cámara
Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool mouseCapturado = false;

// Fichas culturales
FichasCulturales fichas;
bool firstMouse = true;

// Plano lejano de la proyección (el escenario mide ~130 m)
const float PLANO_LEJANO = 200.0f;

// Vértices del cubo (se usa para los postes de los faroles)
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

// Índices del skybox
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

// Esqueleto
glm::vec3 skeletonPos(-25.0f, 0.0f, 55.0f);
float skeletonYaw = 0.0f;

// Portón del panteón
glm::vec3 portonCentro(32.3f, 0.0f, 50.0f);
float portonRotY = 0.0f;
float portonAncho = 1.57f;
float anguloPorton = 0.0f;
float anguloMax = 90.0f;
float velPorton = 60.0f;
bool  abrirPorton = false;

// Lápidas y cruces
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

// Animación del esqueleto
float skelWalkCycle = 0.0f;
bool isWalking = false;

float skelArmAngle = 0.0f;
float skelLegAngle = 0.0f;
float skelForearmAngle = 0.0f;
float skelKneeAngle = 0.0f;

// Salto
float velocidadSalto = 0.0f;
bool enSuelo = true;
const float FUERZA_SALTO = 5.0f;   // ~85 cm de alto
const float GRAVEDAD = 15.0f;

// Mariposas
const int NUM_MARIPOSAS = 2;
const char* RUTAS_MARIPOSA[NUM_MARIPOSAS] = {
	"assets/models/mariposa/vuelo_iglesia.txt",
	"assets/models/mariposa/vuelo_altar.txt"
};
MariposaKeyframes* mariposas[NUM_MARIPOSAS] = { nullptr, nullptr };

// Tiempo
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Estructura de colisión
struct BoundingBox {
	float minX, maxX;
	float minZ, maxZ;
};

// Paredes invisibles
std::vector<BoundingBox> invisibleWalls = {
	{ -120.0f, 120.0f,  80.0f,  85.0f },	// límite de la calle (lado de los árboles)
	{ -120.0f, 120.0f, -55.0f, -49.8f },	// barda norte (detrás de la iglesia y del panteón)
	{  -65.0f, -60.0f, -80.0f,  80.0f },	// límite oeste del mapa
	{   60.0f,  65.0f, -80.0f,  80.0f },	// límite este del mapa
	{  -50.0f, -49.75f, -60.0f, 50.0f },	// barda oeste del atrio
	{   49.75f, 50.0f, -60.0f,  50.0f },	// barda este del panteón
	{   10.0f,  15.0f,   5.0f,  40.0f },	// barda entre atrio y panteón (sur del corredor)
	{   10.0f,  15.0f, -60.0f, -4.75f },	// barda entre atrio y panteón (norte del corredor)
	{  -60.0f, -22.9f,  49.90f, 50.00f },	// barda sur 1 (oeste del arco)
	{  -17.0f,  30.35f, 49.90f, 50.00f },	// barda sur 2 (entre el arco y el portón)
	{   34.4f,  60.0f,  49.90f, 50.00f },	// barda sur 3 (este del portón)
	{  -33.0f,  -6.0f, -45.0f, -14.0f },	// iglesia
	{  -23.3f, -16.7f,  -1.25f,  3.1f }	// ofrenda
};

// Hueco del portón del panteón: solo bloquea cuando el portón está cerrado
const BoundingBox HUECO_PORTON = { 30.35f, 34.4f, 49.90f, 50.00f };
const float ANGULO_PASO_PORTON = 60.0f;   // a partir de este ángulo ya se puede pasar

// Revisa si un círculo de radio r en pos toca una caja
bool TocaCaja(const glm::vec3& pos, const BoundingBox& caja, float r)
{
	return pos.x + r >= caja.minX && pos.x - r <= caja.maxX &&
		pos.z + r >= caja.minZ && pos.z - r <= caja.maxZ;
}

// Colisión
bool CheckCollision(glm::vec3 pos)
{
	const float skeletonRadius = 0.5f;

	for (const auto& wall : invisibleWalls)
	{
		if (TocaCaja(pos, wall, skeletonRadius))
			return true;
	}

	// Portón cerrado. Si el esqueleto ya estaba en el hueco cuando se cerró,
	// se le deja salir para que no se quede atorado.
	if (anguloPorton < ANGULO_PASO_PORTON &&
		TocaCaja(pos, HUECO_PORTON, skeletonRadius) &&
		!TocaCaja(skeletonPos, HUECO_PORTON, skeletonRadius))
	{
		return true;
	}
	return false;
}

int main()
{
	// Ventana
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
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetWindowFocusCallback(window, FocusCallback);

	// GLEW
	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// Shaders
	Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
	Shader SkyBoxshader("Shader/skybox.vs", "Shader/skybox.frag");
	Shader humoShader("Shader/humo.vs", "Shader/humo.frag");

	// Carga de modelos: esqueleto
	Model skeleton((char*)"assets/models/skeleton/skeleton.obj");
	Model craneo((char*)"assets/models/skeleton/craneo.obj");
	Model antebrazoDer((char*)"assets/models/skeleton/anteBrazoDer.obj");
	Model antebrazoIzq((char*)"assets/models/skeleton/anteBrazoIzq.obj");
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

	// Carga de modelos: escenario
	Model arcoEntrada((char*)"assets/models/edificacion/atrio_arco_entrada.obj");
	Model corredor((char*)"assets/models/edificacion/corredor_panteon.obj");
	Model cruzMadera1((char*)"assets/models/edificacion/cruz_pequena.obj");
	Model iglesia((char*)"assets/models/edificacion/iglesia.obj");
	Model lapida1((char*)"assets/models/edificacion/lapida.obj");
	Model murosCementerio((char*)"assets/models/edificacion/barda_panteon.obj");
	Model paredesPatio((char*)"assets/models/edificacion/atrio_barda.obj");
	Model pisoAdoquin((char*)"assets/models/edificacion/atrio_explanada.obj");
	Model pisoCementerio((char*)"assets/models/edificacion/piso_panteon.obj");
	Model portonDerecha((char*)"assets/models/edificacion/porton_der.obj");
	Model portonIzquierda((char*)"assets/models/edificacion/porton_izq.obj");
	Model portonMarco((char*)"assets/models/edificacion/porton_marco.obj");
	Model altar((char*)"assets/models/edificacion/altar.obj");
	Model papelPicadoAltar((char*)"assets/models/edificacion/papelPicadoAltar.obj");
	Model copalero((char*)"assets/models/edificacion/copalero.obj");

	// Humo del copal
	Humo humoCopal(origenHumoCopal, 1.5f, 200, 40.0f);
	humoCopal.colorNiebla = NIEBLA_COLOR;
	humoCopal.densidadNiebla = NIEBLA_DENSIDAD;

	// Carga de modelos: calle y adornos
	Model avenida((char*)"assets/models/edificacion/avenida.obj");
	Model cesped((char*)"assets/models/edificacion/cesped.obj");
	Model cempasuchil((char*)"assets/models/edificacion/cempasuchil.obj");
	Model senderoPetalos((char*)"assets/models/edificacion/sendero_petalos.obj");
	Model vela((char*)"assets/models/edificacion/vela.obj");
	Model velaLlama((char*)"assets/models/edificacion/vela_llama.obj");
	Model farol((char*)"assets/models/edificacion/farol1.obj");

	// Bosque
	Bosque bosque;
	bosque.cargar("assets/models/edificacion", "arbol_bosque.obj", "bosque_arboles.txt");

	// Carga de modelos: mariposa
	Model mariposaCuerpo((char*)"assets/models/mariposa/cuerpo.obj");
	Model mariposaAlaIzq((char*)"assets/models/mariposa/ala_izq.obj");
	Model mariposaAlaDer((char*)"assets/models/mariposa/ala_der.obj");

	// Mariposas
	MariposaKeyframes mariposaIglesia(&mariposaCuerpo, &mariposaAlaIzq, &mariposaAlaDer);
	MariposaKeyframes mariposaAltar(&mariposaCuerpo, &mariposaAlaIzq, &mariposaAlaDer);
	mariposas[0] = &mariposaIglesia;
	mariposas[1] = &mariposaAltar;

	// Tamaño y velocidad de las mariposas
	mariposaIglesia.escala = 0.30f;
	mariposaIglesia.pasosEntreCuadros = 120;
	mariposaIglesia.pasosEntreAleteos = 5;
	mariposaAltar.escala = 0.15f;
	mariposaAltar.pasosEntreCuadros = 90;
	mariposaAltar.pasosEntreAleteos = 6;

	// Cargar keyframes
	for (int i = 0; i < NUM_MARIPOSAS; i++)
	{
		if (mariposas[i]->CargarDesdeArchivo(RUTAS_MARIPOSA[i]))
			mariposas[i]->ReproducirDesde(0);
	}

	// Papel picado
	glm::mat4 modeloBardas = glm::mat4(1.0f);
	PapelPicado papelPicado;
	papelPicado.cargar("Imagenes");
	const float yAmarrePapel = 3.5f;

	{
		// Tiras del panteón
		const float zsPanteon[] = { -36.0f, -19.5f, 7.5f, 27.0f, 45.0f };
		const float caidasPanteon[] = { 0.15f, 0.14f, 0.14f, 0.16f, 0.15f };
		for (int k = 0; k < 5; k++)
		{
			glm::vec3 a = glm::vec3(modeloBardas * glm::vec4(15.0f, yAmarrePapel, zsPanteon[k], 1.0f));
			glm::vec3 b = glm::vec3(modeloBardas * glm::vec4(50.0f, yAmarrePapel, zsPanteon[k], 1.0f));
			papelPicado.agregarTira(a, b, caidasPanteon[k], 0.55f);
		}
	}

	// Tiras del atrio
	const glm::vec2 tirasAtrio[] = {
		glm::vec2(-11.5f, -10.5f),
		glm::vec2(14.0f,  11.0f),
		glm::vec2(27.0f,  29.5f),
		glm::vec2(46.5f,  46.5f)
	};
	for (const glm::vec2& t : tirasAtrio)
		papelPicado.agregarTira(glm::vec3(-50.0f, yAmarrePapel, t.x), glm::vec3(10.02f, yAmarrePapel, t.y), 0.15f, 0.55f);

	// Sendero
	construirSendero();

	// Fichas culturales
	fichas.cargar("Imagenes");
	fichas.agregarPunto("Ofrenda", glm::vec3(posAltar.x, 0.0f, altarZ1 + 3.0f), 3.5f, {
		"FIcha_Niveles.png", "FIcha_Retrato.png", "Ficha_Flor.png", "FIcha_Veladoras.png",
		"FIcha_Copal.png", "FIcha_Agua.png", "FIcha_Sal.png", "FIcha_Pan.png",
		"FIcha_Calavera.png", "FIcha_Mariposa.png" });
	fichas.agregarPunto("Papel picado", glm::vec3(xPasillo, alturaPiso(xPasillo, 30.0f), 30.0f), 3.0f, { "FIcha_Papel.png" });
	fichas.agregarPunto("Xoloitzcuintle", glm::vec3(xPasillo, alturaPiso(xPasillo, 45.0f), 45.0f), 3.0f, { "FIcha_Xolo.png" });

	// Colisión de faroles
	for (const Farol& f : faroles)
		invisibleWalls.push_back({ f.pos.x - 0.25f, f.pos.x + 0.25f, f.pos.z - 0.25f, f.pos.z + 0.25f });

	// Colisión de las tumbas (montículo de tierra con su lápida o cruz)
	for (const Tumba& t : tumbas)
		invisibleWalls.push_back({ t.x0, t.x1, t.z0, t.z1 });

	// Cubo
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

	// Texturas del skybox
	std::vector<const GLchar*> faces;
	faces.push_back("assets/textures/sky_right.png");
	faces.push_back("assets/textures/sky_left.png");
	faces.push_back("assets/textures/sky_top.png");
	faces.push_back("assets/textures/sky_bottom.png");
	faces.push_back("assets/textures/sky_front.png");
	faces.push_back("assets/textures/sky_back.png");

	stbi_set_flip_vertically_on_load(false);
	GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);

	SkyBoxshader.Use();
	glUniform1i(glGetUniformLocation(SkyBoxshader.Program, "skybox"), 0);

	// Texturas del material
	lightingShader.Use();
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);

	// Iluminación de noche
	GLuint mapaLuzVelas = configurarNoche(lightingShader.Program);

	// El shader tiene una luz puntual que no se usa: se deja apagada.
	// constant = 1 evita dividir entre cero en la atenuación.
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.0f, 0.0f, 0.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.0f);

	// Música
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
		fichas.actualizar(skeletonPos, deltaTime);

		// Cámara en tercera persona
		float distanceFromSkeleton = 2.5f;
		glm::vec3 camPos = skeletonPos - (camera.GetFront() * distanceFromSkeleton) + glm::vec3(0.0f, 1.8f, 0.0f);
		camera.SetPosition(camPos);

		// Proyección con el tamaño actual de la ventana
		float aspecto = (SCREEN_HEIGHT > 0) ? (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT : 1.0f;
		glm::mat4 projection = glm::perspective(camera.GetZoom(), aspecto, 0.1f, PLANO_LEJANO);

		glClearColor(NIEBLA_COLOR.x, NIEBLA_COLOR.y, NIEBLA_COLOR.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);

		glm::mat4 modelTemp = glm::mat4(1.0f);

		// Iluminación
		lightingShader.Use();

		glUniform1i(glGetUniformLocation(lightingShader.Program, "diffuse"), 0);

		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

		// Luna, velas y linterna
		actualizarLuces(lightingShader.Program, camera.GetPosition(), camera.GetFront(), currentFrame, mapaLuzVelas);

		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 5.0f);

		// Vista y proyección
		glm::mat4 view = camera.GetViewMatrix();

		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// Escenario
		glm::mat4 model(1.0f);

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
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
		bosque.Draw(lightingShader);
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.25f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		cesped.Draw(lightingShader);

		model = glm::mat4(1.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		pisoCementerio.Draw(lightingShader);
		portonMarco.Draw(lightingShader);

		// Cruz del altar
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-20.0f, 3.0f, -0.25f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		cruzMadera1.Draw(lightingShader);

		// Lápidas
		for (int i = 0; i < 4; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, posLapidas[i]);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			lapida1.Draw(lightingShader);
		}

		// Cruces
		for (int i = 0; i < 2; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, posCruces[i]);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			cruzMadera1.Draw(lightingShader);
		}

		// Portón
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

		// Copalero
		model = glm::mat4(1.0f);
		model = glm::translate(model, posCopalero);
		model = glm::scale(model, glm::vec3(escalaCopalero));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		copalero.Draw(lightingShader);

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

		// Pétalos, flores y velas
		dibujarAdornos(lightingShader, modelLoc, senderoPetalos, cempasuchil, vela, camera.GetPosition());

		// Faroles
		dibujarFaroles(lightingShader, modelLoc, VAO, farol);

		// Papel picado
		papelPicado.dibujar(lightingShader, currentFrame);

		// Mariposas
		for (int i = 0; i < NUM_MARIPOSAS; i++)
		{
			mariposas[i]->Actualizar(deltaTime);
			mariposas[i]->Dibujar(lightingShader);
		}

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

		// Articulaciones
		const glm::vec3 hombroDer(-0.28f, 1.30f, 0.0f);
		const glm::vec3 codoDer(-0.286f, 1.00f, -0.02f);
		const glm::vec3 hombroIzq(0.28f, 1.30f, 0.0f);
		const glm::vec3 codoIzq(0.286f, 1.00f, -0.02f);
		const glm::vec3 caderaDer(-0.10f, 0.90f, 0.0f);
		const glm::vec3 rodillaDer(-0.10f, 0.54f, -0.02f);
		const glm::vec3 caderaIzq(0.10f, 0.90f, 0.0f);
		const glm::vec3 rodillaIzq(0.10f, 0.54f, -0.02f);

		// Brazo derecho
		glm::mat4 brazoDerModel = modelTemp;
		brazoDerModel = glm::translate(brazoDerModel, hombroDer);
		brazoDerModel = glm::rotate(brazoDerModel, glm::radians(skelArmAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		brazoDerModel = glm::translate(brazoDerModel, -hombroDer);
		model = brazoDerModel;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		brazoDer.Draw(lightingShader);

		// Antebrazo derecho
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

		// Antebrazo izquierdo
		model = brazoIzqModel;
		model = glm::translate(model, codoIzq);
		model = glm::rotate(model, glm::radians(-skelForearmAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, -codoIzq);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		antebrazoIzq.Draw(lightingShader);
		manoIzq.Draw(lightingShader);

		// Pierna derecha
		glm::mat4 piernaDerModel = modelTemp;
		piernaDerModel = glm::translate(piernaDerModel, caderaDer);
		piernaDerModel = glm::rotate(piernaDerModel, glm::radians(-skelLegAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		piernaDerModel = glm::translate(piernaDerModel, -caderaDer);
		model = piernaDerModel;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		femurDer.Draw(lightingShader);

		// Rodilla derecha
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

		// Rodilla izquierda
		model = piernaIzqModel;
		model = glm::translate(model, rodillaIzq);
		model = glm::rotate(model, glm::radians(-skelKneeAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, -rodillaIzq);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		piernaIzq.Draw(lightingShader);
		pieIzq.Draw(lightingShader);

		// Cuerpo del esqueleto
		model = modelTemp;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		skeleton.Draw(lightingShader);

		// Skybox
		glDepthFunc(GL_LEQUAL);
		SkyBoxshader.Use();
		glm::mat4 skyView = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(SkyBoxshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(skyView));
		glUniformMatrix4fv(glGetUniformLocation(SkyBoxshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		// Llamas de velas y faroles
		lightingShader.Use();
		dibujarLlamas(lightingShader, velaLlama, camera.GetPosition(), currentFrame);

		// Humo del copal
		humoCopal.Actualizar(deltaTime, currentFrame);
		humoCopal.Dibujar(humoShader.Program, view, projection, camera.GetPosition(), currentFrame);

		// Fichas culturales
		fichas.dibujarMarcadores(view, projection, camera.GetPosition(), currentFrame);
		fichas.dibujarHUD(SCREEN_WIDTH, SCREEN_HEIGHT, currentFrame);

		glfwSwapBuffers(window);
	}

	// Liberar recursos
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);
	glDeleteBuffers(1, &skyboxEBO);
	glDeleteTextures(1, &cubemapTexture);

	mciSendString(TEXT("close musicaFondo"), NULL, 0, NULL);

	glfwTerminate();
	return 0;
}

// Movimiento
void DoMovement()
{
	if (fichas.abierta()) { isWalking = false; return; }

	GLfloat cameraSpeed = 5.0f * deltaTime;

	glm::vec3 front = camera.GetFront();
	front.y = 0.0f;
	front = glm::normalize(front);

	glm::vec3 right = camera.GetRight();
	right.y = 0.0f;
	right = glm::normalize(right);

	// Rotación del esqueleto
	float targetYaw = glm::degrees(atan2(front.x, front.z));

	float rotationSpeed = 10.0f;

	float diff = targetYaw - skeletonYaw;
	while (diff < -180.0f) diff += 360.0f;
	while (diff > 180.0f) diff -= 360.0f;

	skeletonYaw += diff * glm::min(1.0f, rotationSpeed * deltaTime);

	// Desplazamiento
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

	// Saltar
	if (keys[GLFW_KEY_SPACE] && enSuelo)
	{
		velocidadSalto = FUERZA_SALTO;
		enSuelo = false;
	}
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

	// Linterna
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		linterna = !linterna;
	}

	// Fichas culturales
	if (action == GLFW_PRESS && (key == GLFW_KEY_Q || key == GLFW_KEY_E))
	{
		fichas.tecla(key);
	}

	// Portón
	if (key == GLFW_KEY_O && action == GLFW_PRESS)
	{
		abrirPorton = !abrirPorton;
	}

	// Mariposas: detener o reanudar el vuelo
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
	{
		for (int i = 0; i < NUM_MARIPOSAS; i++)
			if (mariposas[i] != nullptr) mariposas[i]->PausarOReanudar();
	}
}

// Animaciones
void Animation()
{
	if (isWalking)
	{
		skelWalkCycle += deltaTime * 7.0f;

		skelArmAngle = sin(skelWalkCycle) * 15.0f;
		skelLegAngle = sin(skelWalkCycle) * 16.0f;

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

	// Salto
	if (!enSuelo)
	{
		velocidadSalto -= GRAVEDAD * deltaTime;
		skeletonPos.y += velocidadSalto * deltaTime;
		if (skeletonPos.y <= 0.0f)
		{
			skeletonPos.y = 0.0f;
			velocidadSalto = 0.0f;
			enSuelo = true;
		}
	}

	// Portón
	float objetivo = abrirPorton ? anguloMax : 0.0f;
	if (anguloPorton < objetivo)
		anguloPorton = glm::min(anguloPorton + velPorton * deltaTime, objetivo);
	else if (anguloPorton > objetivo)
		anguloPorton = glm::max(anguloPorton - velPorton * deltaTime, objetivo);
}

// Mouse
void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (!mouseCapturado) return;

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

// Cambio de tamaño de la ventana
void FramebufferSizeCallback(GLFWwindow* window, int ancho, int alto)
{
	SCREEN_WIDTH = ancho;
	SCREEN_HEIGHT = alto;
	glViewport(0, 0, ancho, alto);
}

// Capturar el mouse al dar clic en la ventana
void MouseButtonCallback(GLFWwindow* window, int boton, int accion, int mods)
{
	if (boton == GLFW_MOUSE_BUTTON_LEFT && accion == GLFW_PRESS && !mouseCapturado)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		mouseCapturado = true;
		firstMouse = true;
	}
}

// Soltar el mouse al cambiar de ventana
void FocusCallback(GLFWwindow* window, int enfocada)
{
	if (!enfocada)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		mouseCapturado = false;
	}
}