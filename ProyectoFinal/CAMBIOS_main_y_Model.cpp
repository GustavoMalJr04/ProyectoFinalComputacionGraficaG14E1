// =====================================================================
//  Cambios para usar el nuevo lighting.frag + skybox
//  (copiar cada bloque donde indica; lo demás del main se queda igual)
// =====================================================================


// ---------------------------------------------------------------------
// 1) Model.h  ->  reemplazar TextureFromFile completo
//    Antes cargaba SOIL_LOAD_RGB: se perdía la transparencia de
//    papel picado, pétalos, alas, llama y humo. Ahora carga RGBA
//    y avisa en consola si una textura no se encuentra.
// ---------------------------------------------------------------------
GLint TextureFromFile(const char *path, string directory)
{
	string filename = directory + '/' + string(path);
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height;
	unsigned char *image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
	if (!image)
	{
		cout << "ERROR::TEXTURA no encontrada: " << filename << endl;
		return textureID;
	}

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);

	return textureID;
}

// Model.h, en loadModel(): agregar aiProcess_GenSmoothNormals.
// Si un OBJ de 3ds Max se exporta sin normales, sin esto el programa truena.
const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);


// ---------------------------------------------------------------------
// 2) main.cpp  ->  variables globales (junto a pointLightPositions)
//    Posiciones de ejemplo: se ajustan cuando esté colocada la escena.
//    flicker[i] lo actualiza el script de parpadeo (320492806).
// ---------------------------------------------------------------------
const int MAX_LUCES = 16;
int numLuces = 4;
glm::vec3 posLuces[MAX_LUCES] = {
	glm::vec3( 0.0f, 1.2f, -4.0f),   // altar, nivel 1
	glm::vec3( 0.8f, 0.9f, -4.0f),   // altar, nivel 2
	glm::vec3(-0.8f, 0.9f, -4.0f),   // altar, nivel 2
	glm::vec3( 6.0f, 2.5f, -8.0f),   // farol del panteón
};
bool esFarol[MAX_LUCES] = { false, false, false, true };
float flicker[MAX_LUCES];            // 1.0 = intensidad normal


// ---------------------------------------------------------------------
// 3) main.cpp  ->  antes del while
// ---------------------------------------------------------------------
// BORRAR estas dos líneas (nombres incorrectos, no hacían nada):
//   glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
//   glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);

// CAMBIAR la proyección: glm::perspective espera radianes, no grados.
glm::mat4 projection = glm::perspective(glm::radians(camera.GetZoom()), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 200.0f);

for (int i = 0; i < MAX_LUCES; i++) flicker[i] = 1.0f;

// --- Skybox ---
Shader skyboxShader("Shader/skybox.vs", "Shader/skybox.frag");

float skyboxVertices[] = {
	-1, 1,-1, -1,-1,-1,  1,-1,-1,  1,-1,-1,  1, 1,-1, -1, 1,-1,
	-1,-1, 1, -1,-1,-1, -1, 1,-1, -1, 1,-1, -1, 1, 1, -1,-1, 1,
	 1,-1,-1,  1,-1, 1,  1, 1, 1,  1, 1, 1,  1, 1,-1,  1,-1,-1,
	-1,-1, 1, -1, 1, 1,  1, 1, 1,  1, 1, 1,  1,-1, 1, -1,-1, 1,
	-1, 1,-1,  1, 1,-1,  1, 1, 1,  1, 1, 1, -1, 1, 1, -1, 1,-1,
	-1,-1,-1, -1,-1, 1,  1,-1,-1,  1,-1,-1, -1,-1, 1,  1,-1, 1
};
GLuint skyVAO, skyVBO;
glGenVertexArrays(1, &skyVAO);
glGenBuffers(1, &skyVBO);
glBindVertexArray(skyVAO);
glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
glBindVertexArray(0);

// Orden obligatorio: +X, -X, +Y, -Y, +Z, -Z. Las caras NO se voltean.
const char* caras[6] = {
	"assets/textures/skybox/sky_right.png",  "assets/textures/skybox/sky_left.png",
	"assets/textures/skybox/sky_top.png",    "assets/textures/skybox/sky_bottom.png",
	"assets/textures/skybox/sky_front.png",  "assets/textures/skybox/sky_back.png"
};
GLuint cubemapTexture;
glGenTextures(1, &cubemapTexture);
glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
for (int i = 0; i < 6; i++)
{
	int w, h;
	unsigned char* data = SOIL_load_image(caras[i], &w, &h, 0, SOIL_LOAD_RGB);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		SOIL_free_image_data(data);
	}
	else std::cout << "ERROR::SKYBOX no encontrado: " << caras[i] << std::endl;
}
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


// ---------------------------------------------------------------------
// 4) main.cpp  ->  dentro del while, REEMPLAZAR todo desde
//    "// Directional light" hasta "material.shininess" (incluye el
//    spotLight azul de la cámara, que se elimina)
// ---------------------------------------------------------------------
GLuint P = lightingShader.Program;

// Luna: luz fría y tenue. La dirección apunta desde la luna del skybox.
glUniform3f(glGetUniformLocation(P, "moon.direction"), -0.45f, -0.42f, 0.79f);
glUniform3f(glGetUniformLocation(P, "moon.ambient"),   0.06f, 0.07f, 0.12f);
glUniform3f(glGetUniformLocation(P, "moon.diffuse"),   0.18f, 0.20f, 0.30f);
glUniform3f(glGetUniformLocation(P, "moon.specular"),  0.10f, 0.10f, 0.15f);

// Veladoras y faroles: luz cálida con atenuación
glUniform1i(glGetUniformLocation(P, "numPointLights"), numLuces);
for (int i = 0; i < numLuces; i++)
{
	std::string b = "pointLights[" + std::to_string(i) + "].";
	float f = flicker[i];
	glUniform3fv(glGetUniformLocation(P, (b + "position").c_str()), 1, glm::value_ptr(posLuces[i]));
	glUniform3f(glGetUniformLocation(P, (b + "ambient").c_str()),  0.05f * f, 0.025f * f, 0.01f * f);
	glUniform3f(glGetUniformLocation(P, (b + "diffuse").c_str()),  1.00f * f, 0.55f * f, 0.20f * f);
	glUniform3f(glGetUniformLocation(P, (b + "specular").c_str()), 0.40f * f, 0.25f * f, 0.10f * f);
	glUniform1f(glGetUniformLocation(P, (b + "constant").c_str()), 1.0f);
	// Veladora: alcance ~7 m. Farol: alcance ~20 m.
	glUniform1f(glGetUniformLocation(P, (b + "linear").c_str()),    esFarol[i] ? 0.22f : 0.70f);
	glUniform1f(glGetUniformLocation(P, (b + "quadratic").c_str()), esFarol[i] ? 0.20f : 1.80f);
}

// Material y niebla
glUniform1f(glGetUniformLocation(P, "material_shininess"), 16.0f);
glUniform1f(glGetUniformLocation(P, "specularStrength"), 0.15f);
glUniform3f(glGetUniformLocation(P, "fogColor"), 0.06f, 0.06f, 0.13f);
glUniform1f(glGetUniformLocation(P, "fogDensity"), 0.03f);
glUniform1i(glGetUniformLocation(P, "emissive"), 0);
glUniform1f(glGetUniformLocation(P, "emissiveStrength"), 1.5f);

// Y cambiar glClearColor por el color de la niebla:
glClearColor(0.06f, 0.06f, 0.13f, 1.0f);


// ---------------------------------------------------------------------
// 5) Cómo dibujar cada tipo de objeto
// ---------------------------------------------------------------------
// Opaco (iglesia, bardas, tumbas, props): igual que ahora, transparency = 0.

// Con alfa (papel picado, pétalos, alas): AL FINAL, después de los opacos
glUniform1i(glGetUniformLocation(P, "transparency"), 1);
glDisable(GL_CULL_FACE);           // que se vean por ambos lados
// papelPicado.Draw(lightingShader);
glUniform1i(glGetUniformLocation(P, "transparency"), 0);

// Llamas (emisivas, brillo aditivo)
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE);
glDepthMask(GL_FALSE);
glUniform1i(glGetUniformLocation(P, "emissive"), 1);
glUniform1i(glGetUniformLocation(P, "transparency"), 1);
// llama.Draw(lightingShader);
glUniform1i(glGetUniformLocation(P, "emissive"), 0);
glUniform1i(glGetUniformLocation(P, "transparency"), 0);
glDepthMask(GL_TRUE);
glDisable(GL_BLEND);


// ---------------------------------------------------------------------
// 6) Skybox: dibujarlo al final del frame, antes de glfwSwapBuffers
// ---------------------------------------------------------------------
glDepthFunc(GL_LEQUAL);
skyboxShader.Use();
glm::mat4 skyView = glm::mat4(glm::mat3(camera.GetViewMatrix()));   // quita la traslación
glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(skyView));
glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
glUniform1i(glGetUniformLocation(skyboxShader.Program, "skybox"), 0);
glBindVertexArray(skyVAO);
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
glDrawArrays(GL_TRIANGLES, 0, 36);
glBindVertexArray(0);
glDepthFunc(GL_LESS);
