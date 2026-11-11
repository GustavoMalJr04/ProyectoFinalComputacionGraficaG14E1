#pragma once
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Model.h"

// Bosque: un solo árbol repetido en cada posición, juntado en una malla por material
class Bosque
{
public:
	bool cargar(const std::string& carpeta, const std::string& archivoArbol, const std::string& archivoPosiciones)
	{
		// Posiciones de los árboles
		std::vector<glm::mat4> arboles;
		std::ifstream fp(carpeta + "/" + archivoPosiciones);
		if (!fp)
		{
			std::cout << "ERROR::BOSQUE no se encontro " << archivoPosiciones << std::endl;
			return false;
		}
		std::string linea;
		while (std::getline(fp, linea))
		{
			if (linea.empty() || linea[0] == '#') continue;
			std::istringstream s(linea);
			float x, z, angulo, escala;
			if (s >> x >> z >> angulo)
			{
				if (!(s >> escala)) escala = 1.0f;
				glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, z));
				m = glm::rotate(m, glm::radians(angulo), glm::vec3(0.0f, 1.0f, 0.0f));
				m = glm::scale(m, glm::vec3(escala));
				arboles.push_back(m);
			}
		}

		// Árbol base
		std::ifstream fa(carpeta + "/" + archivoArbol);
		if (!fa)
		{
			std::cout << "ERROR::BOSQUE no se encontro " << archivoArbol << std::endl;
			return false;
		}
		std::vector<glm::vec3> v, vn;
		std::vector<glm::vec2> vt;
		std::string mtllib, material;
		std::map<std::string, Grupo> grupos;
		std::vector<std::string> orden;

		while (std::getline(fa, linea))
		{
			std::istringstream s(linea);
			std::string tipo;
			s >> tipo;
			if (tipo == "v") { glm::vec3 p; s >> p.x >> p.y >> p.z; v.push_back(p); }
			else if (tipo == "vt") { glm::vec2 t; s >> t.x >> t.y; vt.push_back(t); }
			else if (tipo == "vn") { glm::vec3 n; s >> n.x >> n.y >> n.z; vn.push_back(n); }
			else if (tipo == "mtllib") s >> mtllib;
			else if (tipo == "usemtl")
			{
				s >> material;
				if (!grupos.count(material)) orden.push_back(material);
			}
			else if (tipo == "f")
			{
				Grupo& g = grupos[material];
				std::string esquina;
				while (s >> esquina)
				{
					auto it = g.unicos.find(esquina);
					if (it != g.unicos.end()) { g.indices.push_back(it->second); continue; }

					size_t p1 = esquina.find('/');
					size_t p2 = esquina.find('/', p1 + 1);
					int a = std::stoi(esquina.substr(0, p1));
					int b = std::stoi(esquina.substr(p1 + 1, p2 - p1 - 1));
					int c = std::stoi(esquina.substr(p2 + 1));
					Vertex vx;
					vx.Position = v[a - 1];
					vx.Normal = vn[c - 1];
					vx.TexCoords = glm::vec2(vt[b - 1].x, 1.0f - vt[b - 1].y);   // igual que aiProcess_FlipUVs

					GLuint i = (GLuint)g.vertices.size();
					g.vertices.push_back(vx);
					g.unicos[esquina] = i;
					g.indices.push_back(i);
				}
			}
		}

		// Texturas
		std::map<std::string, std::string> texturas = leerMtl(carpeta + "/" + mtllib);

		// Repetir el árbol en cada posición
		for (const std::string& nombre : orden)
		{
			Grupo& g = grupos[nombre];
			std::vector<Vertex> todos;
			std::vector<GLuint> indices;
			todos.reserve(g.vertices.size() * arboles.size());
			indices.reserve(g.indices.size() * arboles.size());

			for (const glm::mat4& m : arboles)
			{
				GLuint base = (GLuint)todos.size();
				glm::mat3 giro(m);
				for (Vertex vx : g.vertices)
				{
					vx.Position = glm::vec3(m * glm::vec4(vx.Position, 1.0f));
					vx.Normal = giro * vx.Normal;
					todos.push_back(vx);
				}
				for (GLuint i : g.indices) indices.push_back(base + i);
			}

			std::vector<Texture> tex;
			if (texturas.count(nombre))
			{
				Texture t;
				t.id = TextureFromFile(texturas[nombre].c_str(), carpeta);
				t.type = "texture_diffuse";
				t.path = aiString(texturas[nombre]);
				tex.push_back(t);
			}
			mallas.push_back(Mesh(todos, indices, tex));
		}

		std::cout << "Bosque: " << arboles.size() << " arboles" << std::endl;
		return true;
	}

	void Draw(Shader& shader)
	{
		for (Mesh& m : mallas) m.Draw(shader);
	}

private:
	// Vértices de un material del árbol base
	struct Grupo
	{
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		std::map<std::string, GLuint> unicos;
	};

	std::vector<Mesh> mallas;

	// Textura (map_Kd) de cada material
	static std::map<std::string, std::string> leerMtl(const std::string& ruta)
	{
		std::map<std::string, std::string> res;
		std::ifstream f(ruta);
		std::string linea, actual;
		while (std::getline(f, linea))
		{
			std::istringstream s(linea);
			std::string tipo;
			s >> tipo;
			if (tipo == "newmtl") s >> actual;
			else if (tipo == "map_Kd" && !res.count(actual))
			{
				std::string archivo;
				s >> archivo;
				res[actual] = archivo;
			}
		}
		return res;
	}
};
