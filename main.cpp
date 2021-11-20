/*
Student Information
Student ID:1155124488
Student Name: LYU An
*/

#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"
#include "Dependencies/glm/glm/glm.hpp"
#include "Dependencies/glm/glm/gtc/matrix_transform.hpp"

#include "Shader.h"
#include "Texture.h"

#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

// screen setting
const int SCR_WIDTH = 1000;
const int SCR_HEIGHT = 1000;
Shader myShader;
GLuint vaoID[4];
GLuint vboID[4];
GLfloat rotation_y = 0.0f;
glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 endPoint = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ambientLight(0.3f, 0.3f, 0.3f);
glm::vec3 lightPosition(0.0f, 1.0f, 0.0f);
size_t size[4];
Texture penguin_1;
Texture penguin_2;
Texture snow_1;
Texture snow_2;
Texture earth_1;
Texture cottage_1;
GLuint slot_1 = 0;
GLuint slot_2 = 0;
//struct for storing the movement of object

struct MouseController {
	bool LEFT_BUTTON = false;
	bool RIGHT_BUTTON = false;
	double MOUSE_Clickx = 0.0, MOUSE_Clicky = 0.0;
	double MOUSE_X = 0.0, MOUSE_Y = 0.0;
	int click_time = glfwGetTime();
};
MouseController mouseCtl;
// struct for storing the obj file
struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
};


struct Model {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

Model loadOBJ(const char* objPath)
{
	// function to load the obj file
	// Note: this simple function cannot load all obj files.

	struct V {
		// struct for identify if a vertex has showed up
		unsigned int index_position, index_uv, index_normal;
		bool operator == (const V& v) const {
			return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
		}
		bool operator < (const V& v) const {
			return (index_position < v.index_position) ||
				(index_position == v.index_position && index_uv < v.index_uv) ||
				(index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
		}
	};

	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::map<V, unsigned int> temp_vertices;

	Model model;
	unsigned int num_vertices = 0;

	std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

	std::ifstream file;
	file.open(objPath);

	// Check for Error
	if (file.fail()) {
		std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
		exit(1);
	}

	while (!file.eof()) {
		// process the object file
		char lineHeader[128];
		file >> lineHeader;

		if (strcmp(lineHeader, "v") == 0) {
			// geometric vertices
			glm::vec3 position;
			file >> position.x >> position.y >> position.z;
			temp_positions.push_back(position);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			// texture coordinates
			glm::vec2 uv;
			file >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			// vertex normals
			glm::vec3 normal;
			file >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			// Face elements
			V vertices[3];
			for (int i = 0; i < 3; i++) {
				char ch;
				file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
			}

			// Check if there are more than three vertices in one face.
			std::string redundency;
			std::getline(file, redundency);
			if (redundency.length() >= 5) {
				std::cerr << "There may exist some errors while load the obj file. Error content: [" << redundency << " ]" << std::endl;
				std::cerr << "Please note that we only support the faces drawing with triangles. There are more than three vertices in one face." << std::endl;
				std::cerr << "Your obj file can't be read properly by our simple parser :-( Try exporting with other options." << std::endl;
				exit(1);
			}

			for (int i = 0; i < 3; i++) {
				if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
					// the vertex never shows before
					Vertex vertex;
					vertex.position = temp_positions[vertices[i].index_position - 1];
					vertex.uv = temp_uvs[vertices[i].index_uv - 1];
					vertex.normal = temp_normals[vertices[i].index_normal - 1];

					model.vertices.push_back(vertex);
					model.indices.push_back(num_vertices);
					temp_vertices[vertices[i]] = num_vertices;
					num_vertices += 1;
				}
				else {
					// reuse the existing vertex
					unsigned int index = temp_vertices[vertices[i]];
					model.indices.push_back(index);
				}
			} // for
		} // else if
		else {
			// it's not a vertex, texture coordinate, normal or face
			char stupidBuffer[1024];
			file.getline(stupidBuffer, 1024);
		}
	}
	file.close();

	std::cout << "There are " << num_vertices << " vertices in the obj file.\n" << std::endl;
	return model;
}

void get_OpenGL_info()
{
	// OpenGL information
	const GLubyte* name = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* glversion = glGetString(GL_VERSION);
	std::cout << "OpenGL company: " << name << std::endl;
	std::cout << "Renderer name: " << renderer << std::endl;
	std::cout << "OpenGL version: " << glversion << std::endl;
}

void sendDataToOpenGL()
{
	//TODO
	//Load objects and bind to VAO and VBO
	Model penguin = loadOBJ("resources/penguin/penguin.obj");
	Model snow = loadOBJ("resources/snow/snow.obj");
	Model earth = loadOBJ("resources/earth/earth.obj");
	Model cottage = loadOBJ("resources/cottage/cottage.obj");
	penguin_1.setupTexture("resources/penguin/penguin_01.png");
	penguin_2.setupTexture("resources/penguin/penguin_02.png");
	snow_1.setupTexture("resources/snow/snow_01.jpg");
	snow_2.setupTexture("resources/snow/snow_02.jpg");
	earth_1.setupTexture("resources/earth/Textures/Diffuse_2K.png");
	cottage_1.setupTexture("resources/cottage/cottage_textures/cottage_diffuse.png");


	glGenVertexArrays(4, vaoID);
	// import vertices and indices of penguin
	glBindVertexArray(vaoID[0]);

	glGenBuffers(1, &vboID[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
	glBufferData(GL_ARRAY_BUFFER, penguin.vertices.size() * sizeof(Vertex), &penguin.vertices[0], GL_STATIC_DRAW);
	
	GLuint ebo_penguin;
	glGenBuffers(1, &ebo_penguin);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_penguin);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, penguin.indices.size() * sizeof(unsigned int), &penguin.indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	size[0] = penguin.indices.size();
		//import UV coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	//import data of snow field
	glBindVertexArray(vaoID[1]);

	glGenBuffers(1, &vboID[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vboID[1]);
	glBufferData(GL_ARRAY_BUFFER, snow.vertices.size() * sizeof(Vertex), &snow.vertices[0], GL_STATIC_DRAW);

	GLuint ebo_snow;
	glGenBuffers(1, &ebo_snow);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_snow);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, snow.indices.size() * sizeof(unsigned int), &snow.indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	size[1] = snow.indices.size();
		//import uv
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	glBindVertexArray(vaoID[2]);
	glGenBuffers(1, &vboID[2]);
	glBindBuffer(GL_ARRAY_BUFFER, vboID[2]);
	glBufferData(GL_ARRAY_BUFFER, earth.vertices.size() * sizeof(Vertex), &earth.vertices[0], GL_STATIC_DRAW);

	GLuint ebo_earth;
	glGenBuffers(1, &ebo_earth);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_earth);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, earth.indices.size() * sizeof(unsigned int), &earth.indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	size[2] = earth.indices.size();
	//import uv
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	glBindVertexArray(vaoID[3]);
	glGenBuffers(1, &vboID[3]);
	glBindBuffer(GL_ARRAY_BUFFER, vboID[3]);
	glBufferData(GL_ARRAY_BUFFER, cottage.vertices.size() * sizeof(Vertex), &cottage.vertices[0], GL_STATIC_DRAW);

	GLuint ebo_cottage;
	glGenBuffers(1, &ebo_cottage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_cottage);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cottage.indices.size() * sizeof(unsigned int), &cottage.indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	size[3] = cottage.indices.size();
	//import uv
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}

void initializedGL(void) //run only once
{
	if (glewInit() != GLEW_OK) {
		std::cout << "GLEW not OK." << std::endl;
	}

	get_OpenGL_info();
	sendDataToOpenGL();
	myShader.setupShader("VertexShaderCode.glsl", "FragmentShaderCode.glsl");
	myShader.use();

	//TODO: set up the camera parameters	
	//TODO: set up the vertex shader and fragment shader

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void paintGL(void)  //always run
{
	glClearColor(0.2f, 0.2f, 0.4f, 0.5f); //specify the background color, this is just an example
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//TODO:
	// 	glBindVertexArray(vaoID[4]);
	//Set lighting information, such as position and color of lighting source
	myShader.setVec3("ambientLight", ambientLight);
	myShader.setVec3("lightPosition", lightPosition);
	glm::vec3 eyePosition(0.0f, 0.0f, 0.2f);
	myShader.setVec3("eyePositionWorld", eyePosition);
	//Set transformation matrix
	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
	modelTransformMatrix = glm::translate(modelTransformMatrix, translation);
	modelTransformMatrix = glm::rotate(modelTransformMatrix, glm::radians(rotation_y), glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("modelTransformMatrix", modelTransformMatrix);

	glm::mat4 DefaultTransformMatrix = glm::mat4(1.0f);
	DefaultTransformMatrix = glm::translate(DefaultTransformMatrix, glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("defaultTransformMatrix", DefaultTransformMatrix);

	glm::mat4 earthMatrix = glm::mat4(1.0f);
	earthMatrix = glm::scale(earthMatrix, glm::vec3(0.2f, 0.2f, 0.2f));
	earthMatrix = glm::translate(earthMatrix, glm::vec3(1.0f, 5.0f, -40.0f));
	myShader.setMat4("earthMatrix", earthMatrix);

	glm::mat4 cottageMatrix = glm::mat4(1.0f);
	cottageMatrix = glm::scale(cottageMatrix, glm::vec3(0.05f, 0.05f, 0.05f));
	cottageMatrix = glm::translate(cottageMatrix, glm::vec3(-20.0f, 0.0f,-50.0f));
	cottageMatrix = glm::rotate(cottageMatrix, glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("cottageMatrix", cottageMatrix);
	glm::vec3 viewPoint = glm::vec3(0.0f, 2.0f, 6.0f);
	glm::mat4 view = glm::lookAt(viewPoint,
		endPoint,
		glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("view", view);
	int movable = 1;
	myShader.setInt("movable", movable);
	//Bind different textures
	glm::mat4 projection = glm::perspective(glm::radians(30.0f), 1.0f, 0.1f, 20.0f);
	myShader.setMat4("projection", projection);
	int independent = 0;
	myShader.setInt("independent", independent);



	penguin_1.bind(0);
	penguin_2.bind(1);
	myShader.setInt("sampler1", slot_1);
	glBindVertexArray(vaoID[0]);
	glDrawElements(GL_TRIANGLES, size[0], GL_UNSIGNED_INT, 0);
	penguin_1.unbind();
	penguin_2.unbind();

	snow_1.bind(0);
	snow_2.bind(1);
	myShader.setInt("sampler1", slot_2);
	movable = 0;
	myShader.setInt("movable", movable);
	glBindVertexArray(vaoID[1]);
	glDrawElements(GL_TRIANGLES, size[1], GL_UNSIGNED_INT, 0);
	snow_1.unbind();
	snow_2.unbind();

	earth_1.bind(0);
	myShader.setInt("sampler1", 0);
	movable = 2;
	myShader.setInt("movable", movable);
	independent = 1;
	myShader.setInt("independent", independent);
	glBindVertexArray(vaoID[2]);
	glDrawElements(GL_TRIANGLES, size[2], GL_UNSIGNED_INT, 0);
	earth_1.unbind();

	cottage_1.bind(0);
	movable = 3;
	myShader.setInt("movable", movable);
	myShader.setInt("sampler1", 0);
	independent = 0;
	myShader.setInt("independent", independent);
	glBindVertexArray(vaoID[3]);
	glDrawElements(GL_TRIANGLES, size[3], GL_UNSIGNED_INT, 0);
	cottage_1.unbind();

	cottageMatrix = glm::mat4(1.0f);
	cottageMatrix = glm::scale(cottageMatrix, glm::vec3(0.05f, 0.05f, 0.05f));
	cottageMatrix = glm::translate(cottageMatrix, glm::vec3(20.0f, 0.0f, -50.0f));
	cottageMatrix = glm::rotate(cottageMatrix, glm::radians(-60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("cottageMatrix", cottageMatrix);

	cottage_1.bind(0);
	myShader.setInt("sampler1", 0);
	glDrawElements(GL_TRIANGLES, size[3], GL_UNSIGNED_INT, 0);
	cottage_1.unbind();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// Sets the mouse-button callback for the current window
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		mouseCtl.LEFT_BUTTON = true;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		mouseCtl.LEFT_BUTTON = false;
	}
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
	// Sets the cursor position callback for the current window
	if (mouseCtl.LEFT_BUTTON) {
		mouseCtl.MOUSE_X = float((x - (float)SCR_WIDTH / 2) / (float)SCR_WIDTH) * 2;
		mouseCtl.MOUSE_Y = float(0 - (y - (float)SCR_HEIGHT / 2) / (float)SCR_HEIGHT) * 2;
		endPoint = endPoint + glm::vec3(0.0f, (mouseCtl.MOUSE_Y - mouseCtl.MOUSE_Clicky)/10, 0.0f);
		endPoint = endPoint + glm::vec3((mouseCtl.MOUSE_X - mouseCtl.MOUSE_Clickx) / 10, 0.0f, 0.0f);
	}
	else {
		mouseCtl.MOUSE_Clickx = float((x - (float)SCR_WIDTH / 2) / (float)SCR_WIDTH) * 2;
		mouseCtl.MOUSE_Clicky = float(0 - (y - (float)SCR_HEIGHT / 2) / (float)SCR_HEIGHT) * 2;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Sets the scoll callback for the current window.

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Sets the Keyboard callback for the current window.
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		glm::vec3 diff = glm::vec3(0.2f * sin(glm::radians(rotation_y)),0.0f,0.2f * cos(glm::radians(rotation_y)));
		translation = translation + diff;
		//trace.points.push_back(translation.x);
		//trace.points.push_back(translation.y);
		//trace.points.push_back(translation.z);
		//trace.size += 3;
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		glm::vec3 diff = glm::vec3(-0.2f * sin(glm::radians(rotation_y)), 0.0f, -0.2f * cos(glm::radians(rotation_y)));
		translation = translation + diff;
		//trace.points.push_back(translation.x);
		//trace.points.push_back(translation.y);
		//trace.points.push_back(translation.z);
		//trace.size += 3;
	}
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		rotation_y += 10.0f;
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		rotation_y += -10.0f;
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		slot_1 = 0;
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		slot_1 = 1;
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		slot_2 = 0;
	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
		slot_2 = 1;
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		if (ambientLight.x <= 0.9) {
			ambientLight = ambientLight + glm::vec3(0.1f, 0.1f, 0.1f);
		}
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		if (ambientLight.x >=0.2) {
			ambientLight = ambientLight - glm::vec3(0.1f, 0.1f, 0.1f);
		}
	}
	if (key == GLFW_KEY_I && action == GLFW_PRESS)
		lightPosition = lightPosition + glm::vec3(0.0f, 0.0f, -0.2f);
	if (key == GLFW_KEY_K && action == GLFW_PRESS)
		lightPosition = lightPosition + glm::vec3(0.0f, 0.0f, 0.2f);
	if (key == GLFW_KEY_J && action == GLFW_PRESS)
		lightPosition = lightPosition + glm::vec3(-0.2f, 0.0f, 0.0f);
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
		lightPosition = lightPosition + glm::vec3(0.2f, 0.0f, 0.0f);
}


int main(int argc, char* argv[])
{
	GLFWwindow* window;

	/* Initialize the glfw */
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	/* glfw: configure; necessary for MAC */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 2", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/*register callback functions*/
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);                                                                
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	initializedGL();

	while (!glfwWindowShouldClose(window)) {
		/* Render here */
		paintGL();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}






