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
#include "Skybox.h"

using namespace std;

GLuint screenWidth = 800, screenHeight = 600;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

Camera camera(glm::vec3(0.0f, 3.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

GLuint FurTexture::fur_textureId = 0;
GLuint FurTexture::fin_textureId = 0;
const int FUR_DIM = 1024;
const float FUR_DENSITY = 0.7f;
const int FUR_LAYERS = 20;
const float FUR_HEIGHT = 0.03f;
const int GRASS_LAYERS = 30;
const float GRASS_HEIGHT = 0.8f;

enum RabbitType {
	Bunny, FurBunny, VertexBunny, GraftalBunny, ArtBunny, Dump
} rabbitType;

bool animation = true;
bool useSpotLight = false;

const glm::vec3 pointLightPositions[] = {
	glm::vec3(2.3f, -1.6f, -3.0f),
	glm::vec3(-1.7f, 3.0f, 1.0f)
};

glm::vec3 gravity(0.0f, -FUR_HEIGHT, 0.0f);
glm::vec3 rabbitPostion(2.0f, 0.0f, 2.0f);

template<class T>
void shader_draw(Shader shader, GLfloat furHeight, glm::vec3 disp, T & ourModel, glm::mat4 model)
{
	shader.Use();

	glUniform3f(glGetUniformLocation(shader.Program, "displacement"), disp.x, disp.y, disp.z);
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniform1f(glGetUniformLocation(shader.Program, "furLength"), furHeight);

	glUniform3f(glGetUniformLocation(shader.Program, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
	// Point light 1
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].ambient"), 0.5f, 0.5f, 0.5f);
	//glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].diffuse"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].diffuse"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].specular"), 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].constant"), 1.0f);
	// glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].linear"), 0.009f);
	// glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].quadratic"), 0.0032f);
	glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].linear"), 0.009f);
	glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].quadratic"), 0.0032f);
	// Point light 2
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].ambient"), 0.5f, 0.5f, 0.5f);
	//glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].ambient"), 0.5f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].diffuse"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].specular"), 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].constant"), 1.0f);
	// glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].linear"), 0.009f);
	// glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].quadratic"), 0.0032f);
	glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].linear"), 0.009f);
	glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].quadratic"), 0.0032f);

	// spot light
	glUniform1i(glGetUniformLocation(shader.Program, "useSpotLight"), useSpotLight);
	glUniform3f(glGetUniformLocation(shader.Program, "spotLight.position"), camera.Position.x, camera.Position.y, camera.Position.z);
	glUniform3f(glGetUniformLocation(shader.Program, "spotLight.direction"), camera.Front.x, camera.Front.y, camera.Front.z);
	glUniform1f(glGetUniformLocation(shader.Program, "spotLight.cutoff"), glm::cos(glm::radians(0.5f)));
	glUniform1f(glGetUniformLocation(shader.Program, "spotLight.outCutoff"), glm::cos(glm::radians(1.5f)));
	glUniform3f(glGetUniformLocation(shader.Program, "spotLight.ambient"), 0.5f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(shader.Program, "spotLight.diffuse"), 0.0f, 10.0f, 100.0f);
	glUniform1f(glGetUniformLocation(shader.Program, "spotLight.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(shader.Program, "spotLight.linear"), 0.009f);
	glUniform1f(glGetUniformLocation(shader.Program, "spotLight.quadratic"), 0.0032f);


	glUniform1f(glGetUniformLocation(shader.Program, "metallic"), 0.5f);
	glUniform1f(glGetUniformLocation(shader.Program, "roughness"), 0.5f);
	glUniform1f(glGetUniformLocation(shader.Program, "ao"), 1.0f);

	// glm::mat4 model(1.0f);
	// model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	rabbitType = FurBunny;

	FurTexture fur(FUR_DIM, FUR_DIM, FUR_LAYERS, FUR_DENSITY);

	Model bunny("Object/bunny/bunny.obj");
	GraftalModel graftalsBunny(bunny, FUR_HEIGHT);
	Model furBunny(bunny, true, FUR_LAYERS, FUR_HEIGHT);

	Model p("Object/plane/plane.obj");
	Model panel(p, true, GRASS_LAYERS, GRASS_HEIGHT);

	Shader shader("Shader/Rabbit.vert", "Shader/Rabbit.frag");
	Shader furShader("Shader/FurRabbit.vert", "Shader/FurRabbit.frag");
	Shader grassShader("Shader/Grass.vert", "Shader/Grass.frag");
	Shader vertexFurShader("Shader/VertexFurRabbit.vert", "Shader/VertexFurRabbit.frag", "Shader/VertexFurRabbit.geom");
	Shader graftalsShader("Shader/GraftalsRabbit.vert", "Shader/GraftalsRabbit.frag", "Shader/GraftalsRabbit.geom");
	Shader artOutlineShader("Shader/ArtOutlineRabbit.vert", "Shader/ArtOutlineRabbit.frag", "Shader/ArtOutlineRabbit.geom");
	Shader artShader("Shader/ArtRabbit.vert", "Shader/ArtRabbit.frag", "Shader/ArtRabbit.geom");
	Shader skyboxShader("Shader/skybox.vert", "Shader/skybox.frag");

	Skybox skybox;
	vector<const GLchar*> faces;
	faces.push_back("images/right.jpg");
	faces.push_back("images/left.jpg");
	faces.push_back("images/top.jpg");
	faces.push_back("images/bottom.jpg");
	faces.push_back("images/back.jpg");
	faces.push_back("images/front.jpg");
	skybox.loadCubemap(faces); 
	skybox.Bind();

	// Model lightBulb("Object/lamp/file.obj");
	while (!glfwWindowShouldClose(window)) {
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glm::vec3 furForce(sin(glm::radians(currentFrame * 50.0f)) * FUR_HEIGHT, 0.0f, 0.0f);
		glm::vec3 grassForce(sin(glm::radians(currentFrame * 50.0f)) * GRASS_HEIGHT, 0.0f, 0.0f);
		glm::vec3 disp = animation ? gravity + furForce : glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 dispGrass = animation ? grassForce : glm::vec3(0.0f, 0.0f, 0.0f);

		glfwPollEvents();
		Do_Movement();

		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		skyboxShader.Use();
		glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		skybox.Draw(skyboxShader);
		

		glm::mat4 model(1.0f);
		model = glm::translate(model, rabbitPostion);
		model = glm::translate(model, glm::vec3(0.1f, -0.2f, -0.15f));

		if (rabbitType == Bunny) {
			shader.Use();
			glUniform1i(glGetUniformLocation(shader.Program, "artDraw"), 0);
			shader_draw(shader, FUR_HEIGHT, disp, bunny, model);
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.1f, 0.35f, 0.1f));
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.1f));
			shader_draw(shader, GRASS_HEIGHT, dispGrass, panel, model);

			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.1f, 0.35f, 0.1f));
			model = glm::scale(model, glm::vec3(0.1f));
			shader_draw(shader, GRASS_HEIGHT, dispGrass, panel, model);

		}
		else if (rabbitType == FurBunny) {
			furShader.Use();
			shader_draw(furShader, FUR_HEIGHT, disp, furBunny, model);

			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.1f, 0.35f, 0.1f));
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.1f));
			grassShader.Use();
			glUniform3f(glGetUniformLocation(grassShader.Program, "rabbitPostion"), rabbitPostion.x, rabbitPostion.y, rabbitPostion.z);
			glUniform3f(glGetUniformLocation(shader.Program, "displacement"), disp.x, disp.y, disp.z);
			shader_draw(grassShader, GRASS_HEIGHT, dispGrass, panel, model);

			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.1f, 0.34f, 0.1f));
			model = glm::scale(model, glm::vec3(0.1f));
			shader_draw(furShader, GRASS_HEIGHT, dispGrass, panel, model);
		}
		else if (rabbitType == VertexBunny) {
			shader.Use();
			glUniform1i(glGetUniformLocation(shader.Program, "artDraw"), 0);
			shader_draw(shader, FUR_HEIGHT, disp, bunny, model);
			shader_draw(vertexFurShader, FUR_HEIGHT, disp, bunny, model);
		}
		else if (rabbitType == GraftalBunny) {
			// shader.Use();
			// glUniform1i(glGetUniformLocation(shader.Program, "artDraw"), 0);
			// shader_draw(shader, currentFrame, bunny);
			shader_draw(graftalsShader, FUR_HEIGHT, disp, bunny, model);
		}
		else if (rabbitType == ArtBunny) {
			float oldY = gravity.y;
			gravity.y = 0.0f;
			shader.Use();
			glUniform1i(glGetUniformLocation(shader.Program, "artDraw"), 1);
			shader_draw(shader, FUR_HEIGHT, disp, bunny, model);

			artShader.Use();
			glUniform1i(glGetUniformLocation(artShader.Program, "lodLevel"), 1);
			artOutlineShader.Use();
			glUniform1i(glGetUniformLocation(artOutlineShader.Program, "lodLevel"), 1);
			shader_draw(artShader, FUR_HEIGHT, disp, graftalsBunny, model);
			shader_draw(artOutlineShader, FUR_HEIGHT, disp, graftalsBunny, model);

			artShader.Use();
			glUniform1i(glGetUniformLocation(artShader.Program, "lodLevel"), 2);
			artOutlineShader.Use();
			glUniform1i(glGetUniformLocation(artOutlineShader.Program, "lodLevel"), 2);
			shader_draw(artShader, FUR_HEIGHT, disp, graftalsBunny, model);
			shader_draw(artOutlineShader, FUR_HEIGHT, disp, graftalsBunny, model);

			gravity.y = oldY;
		}

		// for (GLuint i = 0; i < 2; i++) {
		// 	model = glm::mat4(1.0f);
		// 	model = glm::translate(model, pointLightPositions[i]);
		// 	model = glm::scale(model, glm::vec3(0.001f));
		// 	shader_draw(shader, currentFrame, lightBulb, model);
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
	if (keys[GLFW_KEY_Z])
		rabbitPostion.x += 0.002f;
	if (keys[GLFW_KEY_X])
		rabbitPostion.x -= 0.002f;

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (action == GLFW_RELEASE && key == GLFW_KEY_M) {
		rabbitType = RabbitType((int)rabbitType + 1);
		if (rabbitType == Dump)
			rabbitType = Bunny;
	}
	if (action == GLFW_RELEASE && key == GLFW_KEY_L) {
		useSpotLight = !useSpotLight;
	}
	if (action == GLFW_RELEASE && key == GLFW_KEY_N)
		animation = !animation;
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