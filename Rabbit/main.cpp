#include <string>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>
#include <SOIL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

using namespace std;

GLuint screenWidth = 800, screenHeight = 600;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

GLuint FurTexture::fur_textureId = 0;
GLuint FurTexture::fin_textureId = 0;
const int FUR_DIM = 512;
const float FUR_DENSITY = 0.8f;
const int FUR_LAYERS = 30;
// const float FUR_HEIGHT = 0.5f;
const float FUR_HEIGHT = 0.09f;

// FUR_BUNNY FUR_ROOM VERTEX_BUNNY ART_BUNNY BUNNY GRAFTALS_BUNNY
#define ART_BUNNY


const glm::vec3 pointLightPositions[] = {
	glm::vec3(2.3f, -1.6f, -3.0f),
	glm::vec3(-1.7f, 0.9f, 1.0f)
};

const glm::vec3 gravity(0.0f, -FUR_HEIGHT, 0.0f);
// const glm::vec3 gravity(0.0f, 0.0f, 0.0f);

template<class T>
void shader_draw(Shader shader, GLfloat & currentFrame, T & ourModel)
{
	shader.Use();

	// glm::vec3 force(sin(glm::radians(currentFrame * 50.0f)) * 50.0f, 0.0f, 0.0f);
	glm::vec3 force(sin(glm::radians(currentFrame * 50.0f)) * FUR_HEIGHT, 0.0f, 0.0f);
	// glm::vec3 disp = gravity + force;
	glm::vec3 disp = gravity + force;

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glUniform3f(glGetUniformLocation(shader.Program, "displacement"), disp.x, disp.y, disp.z);
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniform1f(glGetUniformLocation(shader.Program, "furLength"), FUR_HEIGHT);

	glUniform3f(glGetUniformLocation(shader.Program, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
	// Point light 1
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
	// glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].ambient"), 0.3f, 0.3f, 0.3f);
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].diffuse"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].specular"), 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].constant"), 1.0f);
	// glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].linear"), 0.009f);
	// glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].quadratic"), 0.0032f);
	glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].linear"), 0.009f);
	glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].quadratic"), 0.0032f);
	// Point light 2
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
	// glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].ambient"), 0.3f, 0.3f, 0.3f);
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].diffuse"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].specular"), 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].constant"), 1.0f);
	// glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].linear"), 0.009f);
	// glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].quadratic"), 0.0032f);
	glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].linear"), 0.009f);
	glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].quadratic"), 0.0032f);

	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
	// model = glm::scale(model, glm::vec3(0.005f));
	// model = glm::scale(model, glm::vec3(10.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	ourModel.Draw(shader);
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Rabbit", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewExperimental = GL_TRUE;
	glewInit();
	
	glViewport(0, 0, screenWidth, screenHeight);

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	// glEnable(GL_ARB_arrays_of_arrays);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	FurTexture fur(FUR_DIM, FUR_DIM, FUR_LAYERS, FUR_DENSITY);

	

#ifdef FUR_BUNNY
	Model furBunny("Object/bunny/bunny.obj", true, FUR_LAYERS, FUR_HEIGHT);
	Shader furShader("Shader/FurRabbit.vert", "Shader/FurRabbit.frag");
#else 
#ifdef FUR_ROOM
	Shader furShader("Shader/FurRabbit.vert", "Shader/FurRabbit.frag");
	Model ourModel("Object/room/room.obj", true, FUR_LAYERS, FUR_HEIGHT, false);
#else
	Model bunny("Object/bunny/bunny.obj");
	Shader shader("Shader/Rabbit.vert", "Shader/Rabbit.frag");
	GraftalModel graftalsBunny = GraftalModel("Object/bunny/bunny.obj", FUR_HEIGHT);
#ifdef GRAFTALS_BUNNY
	// GraftalModel graftalsBunny = GraftalModel("Object/room/room.obj", FUR_HEIGHT);
	Shader graftalsShader("Shader/GraftalsRabbit.vert", "Shader/GraftalsRabbit.frag", "Shader/GraftalsRabbit.geom");
#else
	// Model bunny("Object/room/room.obj");
	// bunny.SetFurTexture(true);
#ifdef VERTEX_BUNNY
	// bunny.SetFurTexture(true);
	Shader vertexFurShader("Shader/VertexFurRabbit.vert", "Shader/VertexFurRabbit.frag", "Shader/VertexFurRabbit.geom");
#else
#ifdef ART_BUNNY
	Shader artOutlineShader("Shader/ArtOutlineRabbit.vert", "Shader/ArtOutlineRabbit.frag", "Shader/ArtOutlineRabbit.geom");
	Shader artShader("Shader/ArtRabbit.vert", "Shader/ArtRabbit.frag", "Shader/ArtRabbit.geom");
#else
#endif
#endif
#endif
#endif
#endif

	// Model lightBulb("Object/lamp/file.obj");

	while (!glfwWindowShouldClose(window)) {
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		Do_Movement();

		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// shader_draw(shader, currentFrame, bunny);
		// shader_draw(vertexFurShader, currentFrame, bunny);
#ifdef FUR_BUNNY
		shader_draw(furShader, currentFrame, furBunny);
#else
#ifdef FUR_ROOM
		shader_draw(furShader, currentFrame, ourModel);
#else
		shader_draw(shader, currentFrame, bunny);
#ifdef GRAFTALS_BUNNY
		graftalsShader.Use();
		glUniform1i(glGetUniformLocation(graftalsShader.Program, "lodLevel"), 1);
		shader_draw(graftalsShader, currentFrame, graftalsBunny);
		glUniform1i(glGetUniformLocation(graftalsShader.Program, "lodLevel"), 2);
		shader_draw(graftalsShader, currentFrame, graftalsBunny);
#else
#ifdef VERTEX_BUNNY
		shader_draw(vertexFurShader, currentFrame, bunny);
#else
#ifdef ART_BUNNY
		artShader.Use();
		glUniform1i(glGetUniformLocation(artShader.Program, "lodLevel"), 1);
		artOutlineShader.Use();
		glUniform1i(glGetUniformLocation(artOutlineShader.Program, "lodLevel"), 1);
		shader_draw(artShader, currentFrame, graftalsBunny);
		shader_draw(artOutlineShader, currentFrame, graftalsBunny);

		artShader.Use();
		glUniform1i(glGetUniformLocation(artShader.Program, "lodLevel"), 2);
		artOutlineShader.Use();
		glUniform1i(glGetUniformLocation(artOutlineShader.Program, "lodLevel"), 2);
		shader_draw(artShader, currentFrame, graftalsBunny);
		shader_draw(artOutlineShader, currentFrame, graftalsBunny);
#else
#endif
#endif
#endif
#endif
#endif
	
		// glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		// glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		// for (GLuint i = 0; i < 2; i++) {
		// 	model = glm::mat4(1.0f);
		// 	model = glm::translate(model, pointLightPositions[i]);
		// 	model = glm::scale(model, glm::vec3(0.001f));
		// 	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		// 	lightBulb.Draw(shader);
		// }

		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;
}

#pragma region "User input"

void Do_Movement() {
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = (GLfloat)xpos;
		lastY = (GLfloat)ypos;
		firstMouse = false;
	}

	GLfloat xoffset = (GLfloat)xpos - lastX;
	GLfloat yoffset = lastY - (GLfloat)ypos;

	lastX = (GLfloat)xpos;
	lastY = (GLfloat)ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll((GLfloat)yoffset);
}

#pragma endregion
