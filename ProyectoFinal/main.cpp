#include <iostream>
#include <cmath>
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

glm::vec3 Light1 = glm::vec3(0);

//Animacion del Esqueleto
float skelWalkCycle = 0.0f;
bool isWalking = false;

float skelArmAngle = 0.0f;
float skelLegAngle = 0.0f;
float skelForearmAngle = 0.0f;
float skelKneeAngle = 0.0f; // Flexión de rodillas independiente de los brazos

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

	glm::mat4 modeloBardas = glm::mat4(1.0f);
	PapelPicado papelPicado;
	papelPicado.cargar("Imagenes");
	papelPicado.colocarEnPanteon(modeloBardas);

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

		camera.SetPosition(skeletonPos + glm::vec3(0.0f, 2.0f, -0.5f));

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

		glfwSwapBuffers(window);
	}

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

	skeletonYaw = glm::degrees(atan2(front.x, front.z));

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