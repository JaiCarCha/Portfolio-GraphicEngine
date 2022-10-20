#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include <vector>
#include <string>
#include <map>
#include "Camera.h"
#include "LightBase.h"
#include "DrawableObject.h"
#include "DeferredShading.h"
#include "Shape.h"
#include "Mesh.h"
#include "Model.h"
#include "Framebuffer.h"
#include "FramebufferDebug.h"
#include "Cubemap.h"
#include "ShadowMap.h"
#include "Scene.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"


// Using
using std::cout;
using std::endl;
using glm::mat4;
using glm::vec3;
template<class T> using vector = std::vector<T>;


int WINDOW_WIDTH = 1600, WINDOW_HEIGHT = 900;

Camera* camera; // Main camera

// Delta time info
float lastFrame = 0.f;
float deltaTime = 0.f;

// Mouse info
bool firstMouse = true;
float lastX, lastY;

// Scene info
int skyboxID = 0; // Current skybox
int modelID = 0; // Current model


#pragma region Utility functions
// GLFW framebuffer size listener. 
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	WINDOW_WIDTH = width;
	WINDOW_HEIGHT = height;

	// Set the viewport size with the new width and height if the window size change
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	// First callback
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // Inverted: Y ranges bottom to top
	lastX = xpos;
	lastY = ypos;

	camera->cameraMouse(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera->cameraScroll(yoffset);
}

// Key listener
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS);

	// If press ESCAPE, close the window
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Increase camera speed if pressed
	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
		camera->cameraKeyboard(Movement::SPEED2, deltaTime);
	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
		camera->cameraKeyboard(Movement::SPEED1, deltaTime);

	// Swap skyboxes
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		skyboxID = 0;
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		skyboxID = 1;
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		skyboxID = 2;

	// Swap models
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		modelID = (modelID + 1) % int(Scene::sceneObjects.size());
}

// Key listener by polling, needed for a smooth camera movement
void processInput(GLFWwindow* window)
{

	// Camera movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera->cameraKeyboard(Movement::FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera->cameraKeyboard(Movement::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera->cameraKeyboard(Movement::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera->cameraKeyboard(Movement::RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera->cameraKeyboard(Movement::UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera->cameraKeyboard(Movement::DOWN, deltaTime);

}

int glfwConfig(GLFWwindow*& window)
{
	// GLFW init
	glfwInit();
	// OpenGL context (version 3.3)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	// OpenGL core profile
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// GLFW window
	glfwWindowHint(GLFW_SAMPLES, 4);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Motor grafico creado por Jaime Carmona Chavero", NULL, NULL);

	if (window == NULL)
	{
		cout << "ERROR: Failed to create a window" << endl;
		glfwTerminate();
		return -1;
	}

	// Set the current context with the previously created window
	glfwMakeContextCurrent(window);

	// Getting OpenGL functions with GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
		return -1;
	}

	// GLFW listener. When the framebuffer (the window) size change, execute the given function (framebuffer_size_callback)
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Hide cursor and capture it when focus
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, key_callback);

	// GLFW listener. When the mouse moves, execute the given function (mouse_callback)
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	return 0;
}

void errorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* userParam)
{
	const char* _source;
	const char* _type;
	const char* _severity;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:
		_source = "API";
		break;

	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		_source = "WINDOW SYSTEM";
		break;

	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		_source = "SHADER COMPILER";
		break;

	case GL_DEBUG_SOURCE_THIRD_PARTY:
		_source = "THIRD PARTY";
		break;

	case GL_DEBUG_SOURCE_APPLICATION:
		_source = "APPLICATION";
		break;

	case GL_DEBUG_SOURCE_OTHER:
		_source = "UNKNOWN";
		break;

	default:
		_source = "UNKNOWN";
		break;
	}

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		_type = "ERROR";
		break;

	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		_type = "DEPRECATED BEHAVIOR";
		break;

	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		_type = "UDEFINED BEHAVIOR";
		break;

	case GL_DEBUG_TYPE_PORTABILITY:
		_type = "PORTABILITY";
		break;

	case GL_DEBUG_TYPE_PERFORMANCE:
		_type = "PERFORMANCE";
		break;

	case GL_DEBUG_TYPE_OTHER:
		_type = "OTHER";
		break;

	case GL_DEBUG_TYPE_MARKER:
		_type = "MARKER";
		break;

	default:
		_type = "UNKNOWN";
		break;
	}

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		_severity = "HIGH";
		break;

	case GL_DEBUG_SEVERITY_MEDIUM:
		_severity = "MEDIUM";
		break;

	case GL_DEBUG_SEVERITY_LOW:
		_severity = "LOW";
		break;

	case GL_DEBUG_SEVERITY_NOTIFICATION:
		_severity = "NOTIFICATION";
		break;

	default:
		_severity = "UNKNOWN";
		break;
	}

	printf("%d: %s of %s severity, raised from %s: %s\n\n",
		id, _type, _severity, _source, msg);
}

void glConfig()
{
	// Default openGL canvas size. NOTE: this isn't the window size
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	glEnable(GL_MULTISAMPLE);

	// Depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL); // Less equal in order to make the skybox pass the depth test. (Default: Less)

	// Stencil
	glEnable(GL_STENCIL_TEST);

	// Blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Culling
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_FRONT);

	// GL log
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(errorCallback, NULL);

	// Apply a filter between faces in a cubemap in order to avoid artifacts in low-res skyboxes (be able to see the union between skybox faces)
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void calculateDeltaTime()
{
	// Calculate delta time each frame
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}
#pragma endregion 

#pragma region Testing functions

DirectionalLight generateDirectLight()
{
	DirectionalLight dLight = Scene::createDirectionalLight(glm::normalize(vec3(1.f, -1.f, -1.f)), vec3(1.0f));

	return dLight;
}

vector<SpotLight> generateSpotLights()
{
	vector<SpotLight> spotLights;

	SpotLight defaultSL = Scene::createSpotLight(vec3(0.f, 0.5f, 0.f), glm::normalize(vec3(0.f, 0.f, -1.f)), 45.f, 0, vec3(0.0f));
	spotLights.push_back(defaultSL);

	/*SpotLight sLight2 = Scene::createSpotLight(vec3(-7.f, 0.5f, -12.f), glm::normalize(vec3(1.f, 0.f, 0.f)), 45.f, 0, vec3(0.f));
	spotLights.push_back(sLight2);*/

	return spotLights;
}

vector<PointLight> generatePointLights()
{
	vector<PointLight> pointLights;

	PointLight red = Scene::createPointLight(vec3(-5.f, 0.5f, 0.2f), -1, vec3(0.0f,0.0f,0.0f));
	pointLights.push_back(red);
	/*PointLight blue = Scene::createPointLight(vec3(5.f, 0.5f, 0.2f), -1, vec3(0.1f, 0.f, 0.3f));
	pointLights.push_back(blue);*/
	/*PointLight white = Scene::createPointLight(vec3(0.f, 0.f, 5.f), -1, vec3(0.5f, 0.5f, 0.5f));
	pointLights.push_back(white);*/

	return pointLights;
}

// Spawn here 
vector<DrawableObject*> generateSceneObjects()
{
	vector<DrawableObject*> sceneObj;

	// Models
	DrawableObject* model1 = Scene::createModel("Models/Camera/Camera.obj");
	model1->transformation.translation = vec3(0.f, -1.f, 0.f);

	DrawableObject* model2 = Scene::createModel("Models/Sword/Sword.obj");
	model2->transformation.translation = vec3(0.f, 0.2f, 0.f);

	DrawableObject* model3 = Scene::createModel("Models/Knight/Knight.obj");
	model3->transformation.translation = vec3(0.f, -2.f, 0.f);
	model3->transformation.rotation = vec3(-90.0f, 0.0f, 0.0f);

	DrawableObject* model4 = Scene::createModel("Models/TV/TV.obj");
	model4->transformation.translation = vec3(0.f, 0.f, 0.f);
	model4->transformation.scale = vec3(0.5f, 0.5f, 0.5f);


	// Obj1
	/*Texture obj1Diff("textures/bricks2.jpg", "texture_diffuse");
	Texture obj1Spec("textures/bricks2.jpg", "texture_specular");*/
	//Texture earthDiff("textures/2k_earth_daymap.jpg", "texture_diffuse");
	//Texture earthSpec("textures/2k_earth_specular_map.jpg", "texture_specular");
	//Texture obj1Norm("textures/bricks2_normal1.jpg", "texture_normal");
	//Texture obj1Depth("textures/bricks2_disp.jpg", "texture_depth");

	// IRON
	Texture obj1Diff("textures/rustediron/rustediron2_basecolor.png", "texture_base");
	Texture obj1Spec("textures/rustediron/rustediron2_metallic.png", "texture_metallic");
	Texture obj1Norm("textures/rustediron/rustediron2_normal.png", "texture_normal");
	Texture obj1Roug("textures/rustediron/rustediron2_roughness.png", "texture_roughness");

	vector<float> vert1;
	vector<unsigned int> ind1;
	vector<Texture> textures1;
	textures1.push_back(obj1Diff);
	textures1.push_back(obj1Spec);
	textures1.push_back(obj1Norm);
	textures1.push_back(obj1Roug);

	Shape::generateSphere(1., 32, 32, vert1, ind1);

	DrawableObject* obj1 = Scene::createMesh(vert1, ind1, textures1);
	obj1->transformation.translation = vec3(0.f, 0.f, 0.f);

	// Gold
	Texture obj2Diff("textures/gold/gold-scuffed_basecolor-boosted.png", "texture_base");
	Texture obj2Spec("textures/gold/gold-scuffed_metallic.png", "texture_metallic");
	Texture obj2Norm("textures/gold/gold-scuffed_normal.png", "texture_normal");
	Texture obj2Roug("textures/gold/gold-scuffed_roughness.png", "texture_roughness");
	vector<Texture> textures2;
	vector<float> vert2;
	vector<unsigned int> ind2;


	textures2.push_back(obj2Diff);
	textures2.push_back(obj2Spec);
	textures2.push_back(obj2Norm);
	textures2.push_back(obj2Roug);

	//Shape::generateCube(1, 1, 1, vert1, ind1);
	Shape::generateSphere(1., 32, 32, vert2, ind2);

	DrawableObject* obj2 = Scene::createMesh(vert2, ind2, textures2,glm::vec3(1.f,0.7f,0.6f));
	obj2->transformation.translation = vec3(0.f, 0.f, 0.f);

	//// Obj2
	//Texture obj2Diff("textures/wood.png", "texture_diffuse");
	//Texture obj2Spec("textures/wood.png", "texture_specular");
	//Texture obj2Norm("textures/toy_box_normal.png", "texture_normal");
	//Texture obj2Depth("textures/toy_box_disp.png", "texture_depth");

	//vector<float> vert2;
	//vector<unsigned int> ind2;
	//vector<Texture> textures2;
	////textures2.push_back(obj2Diff);
	//textures2.push_back(obj2Spec);
	////textures2.push_back(obj2Norm);
	////textures2.push_back(obj2Depth);

	////Shape::generateCube(1, 1, 1, vert2, ind2);
	//Shape::generateSphere(1, 32, 32, vert2, ind2);

	////DrawableObject* obj2 = new Mesh(vert2, ind2, textures2);
	//DrawableObject* obj2 = Scene::createMesh(vert2, ind2, textures2, glm::vec3(1.0f, 0.f, 0.f));
	//obj2->transformation.translation = vec3(2.f, 0.f, -8.f);

	//// Floor
	//vector<float> vert3;
	//vector<unsigned int> ind3;
	//vector<Texture> textures3;

	//Texture floorDiff("textures/wall.jpg", "texture_diffuse");
	//Texture floorSpec("textures/wall.jpg", "texture_specular");

	//textures3.push_back(floorDiff);
	//textures3.push_back(floorSpec);

	//Shape::generateCube(100, 0.1f, 100, vert3, ind3, 10, 10);

	////Mesh floor(vert2, ind2, textures2);
	////DrawableObject* floor = new Mesh(vert3, ind3, textures3);
	//DrawableObject* floor = Scene::createMesh(vert3, ind3, textures3);
	//floor->transformation.translation = vec3(0.f, -2.f, -5.f);
	
	//sceneObj.push_back(obj1);
	//sceneObj.push_back(obj2);
	//sceneObj.push_back(floor);

	return sceneObj;
}

#pragma endregion


void drawScene(Shader& sh, vector<DrawableObject*> obj, DirectionalLight dLight, vector<SpotLight> sLight, vector<PointLight> pLight)
{
	sh.use();

	sh.addDirectionalLight(dLight);
	sh.addSpotLight(sLight);
	sh.addPointLight(pLight);
	sh.addCamera(*camera);

	for (int i = 0; i < obj.size(); i++)
	{
		sh.setTransform(obj[i]->transformation);
		obj[i]->Draw(&sh);
	}
}

int main()
{

	GLFWwindow* window;
	if (glfwConfig(window) == -1) return -1;

	glConfig();

#pragma region Shaders, camera, lights and cubemap
	Camera cam(vec3(0.f, 0.f, 3.f), vec3(0.0f, 0.0f, -1.f));
	camera = &cam;

	ShadowMap::init(1024 * 8, 1024 * 8);

	generateDirectLight();
	generateSpotLights();
	generatePointLights();
	generateSceneObjects();

	std::string sLightSizeStr = std::to_string(Scene::spotLights.size());
	std::string pLightSizeStr = std::to_string(Scene::pointLights.size());


	std::map<std::string, const char*> defineValues;
	defineValues.insert(std::pair<std::string, const char*>("MAX_SPOT_LIGHT", sLightSizeStr.c_str()));
	defineValues.insert(std::pair<std::string, const char*>("MAX_POINT_LIGHT", pLightSizeStr.c_str()));
	//Shader shader("vsStandard.vert", "fsStandard.frag", "", defineValues);
	Shader shader("vsStandard.vert", "fsPBR.frag", "", defineValues);

	Scene::createSkybox("textures/Arches_E_PineTree_3k.hdr", ".hdr");
	Scene::createSkybox("textures/Ice_Lake_Ref.hdr", ".hdr");
	Scene::createSkybox("textures/Chelsea_Stairs_3k.hdr", ".hdr");
	
	Framebuffer hdr;

	// Testing viewports
	FramebufferDebug fbDebug1(glm::vec2(0.f), glm::vec2(200.f * (16.f / 9.f), 200.f), glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT));
	FramebufferDebug fbDebug2(glm::vec2(200.f * (16.f / 9.f), 0.f), glm::vec2(200.f * (16.f / 9.f), 200.f), glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT));

#pragma endregion

	vector<DrawableObject*> model; // What model want to see in the scene

	// (Main) Render loop. Prevent closing the window until glfwWindowShouldClose returns true
	while (!glfwWindowShouldClose(window))
	{		

		calculateDeltaTime();

		// Check if any key has pressed/released
		processInput(window);

		// If glClear() is called, use the input color
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

		model.push_back(Scene::sceneObjects[modelID]);
		// Generate shadows before draw a scene
		Scene::generateShadows(model);


		// Normal rendering
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, hdr.fboID);
		//hdr.bindFramebuffer();
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		
		
		Scene::drawScene(hdr.fboID, shader, *camera, Scene::skyboxes[skyboxID], model);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		hdr.draw(*camera, model);

		model.clear();


		fbDebug1.draw(hdr.ssao->gBuffer->gNormal);
		fbDebug2.draw(hdr.ssao->gBuffer->gColorSpec);
		//fbDebug3.draw(hdr.ssao->ssaoColorBuffer);

		

		

		// Swap front (what the user see) and back (what the opengl draw) buffers to avoid tearing/flickering
		glfwSwapBuffers(window);
		//glfwSwapInterval(1);
		// Check if any events are triggered (like framebuffer size)
		glfwPollEvents();

		//checkErrors();
	}


	// If the while ends, close glfw (clear resources) and finish the main execution

	glfwTerminate();
	return 0;
}