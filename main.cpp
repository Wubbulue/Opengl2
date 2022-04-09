#include <chrono>
#include <memory>
#include "line.h"
#include "VBO.h"
#include "VAO.h"
#include "minecraft.h"
#include "PerlinNoise.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Camera.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



//callled on window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
unsigned int SCR_WIDTH = 1200;
unsigned int SCR_HEIGHT = 1000;

float mixAmount = 0.5f;
float fov = 45.0f;
float camHeight = 0.0f;
const float mouseSensitivity = 0.1f;


Camera camera;

Chunk chunk(123489u);
bool wireframe = false;

const float lineLength = 30.0f;
std::vector<std::unique_ptr<Line>> cameraLines;





void checkCompilation(const char* shaderName, unsigned int shader) {

	int  success;
	char infoLog[512];


	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::" << shaderName << "::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{

		glm::vec3 start = camera.position;
		glm::vec3 end = start+(camera.direction * lineLength);
		cameraLines.emplace_back(std::make_unique<Line>(start, end, glm::vec3(1.0f, 0.0f, 0.0f)));

		//Logging::funcTime("Eliminate ray intersect", std::bind(&Chunk::eliminateRayIntersection, chunk, std::placeholders::_1, std::placeholders::_2), start, camera.direction);
		auto t1 = std::chrono::high_resolution_clock::now();

		Box3 box(glm::vec3(0.0f,0.0f,0.0f),glm::vec3(64.0f,64.0f,64.0f));
		chunk.eliminateRayIntersection(start, camera.direction);
		auto t2 = std::chrono::high_resolution_clock::now();

		// floating-point duration: no duration_cast needed
		std::chrono::duration<double, std::milli> fp_ms = t2 - t1;

		// integral duration: requires duration_cast
		auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
		
		std::cout << "Took " << int_ms.count() << " miliseconds to elimnate blocks" << std::endl;


	}
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {

	static float lastX = SCR_WIDTH / 2;
	static float lastY = SCR_HEIGHT / 2;

	static bool firstMouse = true;

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;
	camera.ProcessMouseMovement(xoffset, yoffset);

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.processSroll(float(yoffset));
}


//will only get called when a key is pressed
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_N && action == GLFW_PRESS) //regenerate chunks
	{
		chunk.regenerate();
	}
	else if (key == GLFW_KEY_T && action == GLFW_PRESS) //toggle wireframes
	{
		wireframe = !wireframe;
		if (wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS) //Print position
	{
		printf("Position:%f,%f,%f \n",camera.position.x,camera.position.y,camera.position.z);
	}
	else if (key == GLFW_KEY_C && action == GLFW_PRESS) //Clear lines
	{
		cameraLines.clear();
	}
	else if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
		chunk.pickBlock(camera.position);
	}
}

//relatively rotates something around a point
void rotateAboutPoint(glm::mat4 &mat,float rotationAmount, float xOffset, float yOffset) {
		mat = glm::translate(mat, glm::vec3(xOffset,yOffset, 0.0f));
		mat = glm::rotate(mat, rotationAmount, glm::vec3(0.0f, 0.0f, 1.0f));
		mat = glm::translate(mat, glm::vec3(-xOffset,-yOffset, 0.0f));
}


int main()
{


	//camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));
	camera.position.y = 50.0f;


	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//set input callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window,mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);



	//enable z buffer!
	glEnable(GL_DEPTH_TEST);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



	Shader lineShader("C:/Programming_projects/Open-GL/shaders/vert_line.glsl", "C:/Programming_projects/Open-GL/shaders/frag_line.glsl");

	//create a shader program from a vert and frag path
	Shader shaderTexture("C:/Programming_projects/Open-GL/shaders/vert_texture.glsl", "C:/Programming_projects/Open-GL/shaders/frag_texture.glsl");
	shaderTexture.use();

	Shader diffuseShader("C:/Programming_projects/Open-GL/shaders/vert_diffuse.glsl", "C:/Programming_projects/Open-GL/shaders/frag_diffuse.glsl");

	unsigned int dirtTexture, stoneTexture;

	{
		//--------------------------TEXTURES---------------------------------------------------

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// load and create a texture 
		// -------------------------
		// Dirt
		// ---------
		glGenTextures(1, &dirtTexture);
		glBindTexture(GL_TEXTURE_2D, dirtTexture);
		// load image, create texture and generate mipmaps
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
		unsigned char* data = stbi_load("C:/Programming_projects/Open-GL/textures/dirt.jpg", &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);

		// Stone
		// ---------
		glGenTextures(1, &stoneTexture);
		glBindTexture(GL_TEXTURE_2D, stoneTexture);
		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// load image, create texture and generate mipmaps
		data = stbi_load("C:/Programming_projects/Open-GL/textures/stone.jpg", &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);

		// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
		// -------------------------------------------------------------------------------------------
		shaderTexture.use(); // don't forget to activate/use the shader before setting uniforms!

		// either set it manually like so:
		shaderTexture.setInt("texture", 0);
		//-------------------------------------------------------------------------------------------
	}



	//--------------------GEOMETERY AND STUFF-------------------------------------------
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------

	float lineVerts[] = {
	0.0f,50.0f,0.0f,
	10.0f,50.0f,0.0f
	};

	float vertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	glm::vec3 cubePositions[] = {
	glm::vec3(0.0f,  0.0f,  0.0f),
	glm::vec3(2.0f,  5.0f, -15.0f),
	glm::vec3(-1.5f, -2.2f, -2.5f),
	glm::vec3(-3.8f, -2.0f, -12.3f),
	glm::vec3(2.4f, -0.4f, -3.5f),
	glm::vec3(-1.7f,  3.0f, -7.5f),
	glm::vec3(1.3f, -2.0f, -2.5f),
	glm::vec3(1.5f,  2.0f, -2.5f),
	glm::vec3(1.5f,  0.2f, -1.5f),
	glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	VBO lineVBO(lineVerts,sizeof(lineVerts));
	VAO lineVAO;
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO); // we can also generate multiple VAOs or buffers at the same time
	glGenBuffers(1, &VBO);


	// line setup
	// --------------------
	lineVAO.Bind();
	lineVAO.LinkAttrib(lineVBO,0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);


	// first triangle setup
	// --------------------
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	//COORDINATE TRANSFORMATIONS
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	

	chunk.empty();
	chunk.populateBlocks();

	// render loop
	// -----------
	float lastTime=glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//calc frame rate
		float timeValue = glfwGetTime();
		float elapsedTime = timeValue - lastTime;
		float frameRate = 1 / elapsedTime;
		lastTime = timeValue;

		camera.moveCamera(window,elapsedTime);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, dirtTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, stoneTexture);

		shaderTexture.use();


		//update coordiante transformations
		projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		view = camera.view();

		shaderTexture.setMat4("model", model);
		shaderTexture.setMat4("view", view);
		shaderTexture.setMat4("projection", projection);


		glBindVertexArray(VAO);

		for (unsigned int x = 0; x < CHUNK_LENGTH; x++) {
			
			for (unsigned int z = 0; z < CHUNK_LENGTH; z++) {
				for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {

					auto blockType = chunk.blocks[x][z][y].type;
					if (blockType == BlockType::Air || !chunk.isBlockAdjacentToAir(x, y, z)){
						continue;
					}
					glm::vec3 position(float(x), float(y), float(z));
					model = glm::mat4(1.0f);
					model = glm::translate(model, glm::vec3(float(x),float(y),float(z)));
					shaderTexture.setMat4("model", model);

					if (blockType == BlockType::Dirt) {
						shaderTexture.setInt("texture", 0);
					} else if (blockType == BlockType::Stone) {
						shaderTexture.setInt("texture", 1);
					} 

					glDrawArrays(GL_TRIANGLES, 0, 36);
				}
			}
		}



		model = glm::mat4(1.0f);
		glm::mat4 MVP = projection*view*model;
		for (int i = 0; i < cameraLines.size(); i++) {
			cameraLines[i]->setMVP(MVP);
			cameraLines[i]->draw();
		}



		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	//glDeleteBuffers(1, &EBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	SCR_HEIGHT = height;
	SCR_WIDTH = width;
	glViewport(0, 0, width, height);
}