#define GLEW_STATIC

#include <glew.h>
#include <glfw3.h>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <map>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MATRIX_INIT.h"
#include "SHADER.h"
#include "CAMERA.h"

// Window dimensions
//const GLuint WIDTH = 800, HEIGHT = 600;
const GLuint WIDTH = 1440, HEIGHT = 900;

//for parallax mapping
float heightScale = 0.1;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

//Камера
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];//буфер нажатых клавиш
GLfloat deltaTime = 0.0f;    // Время, прошедшее между последним и текущим кадром
GLfloat lastFrame = 0.0f;      // Время вывода последнего кадра

//для мыши
// Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right (due to how Eular angles work) so we initially rotate a bit to the left.
GLfloat yaw = -90.0f;//движения при фиксированной оси OY
//pitch - движения при фиксированной оси OZ
GLfloat pitch = 0.0f;//хранение углов камеры для мышиx

GLfloat lastX = WIDTH / 2.0;//последние координаты мыши
GLfloat lastY = HEIGHT / 2.0;

//Lighting attributes
//glm::vec3 lightPos(0.5f, 1.0f, 0.3f);//Положение источника света
glm::vec3 lightPos(3.0f, 4.0f, -1.0f);

// meshes
unsigned int planeVAO;

//GLfloat fov =  45.0f;//угол обзора камеры

//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void key_callback(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void Do_movement();
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(std::vector<std::string> faces);

void renderScene(Shader &shader);
void renderCube();
void renderQuad();


GLFWwindow* global_window;

struct windows_struct
{
	float dist;
	glm::vec3 w;
};
bool compareByDistance(const windows_struct &a, const windows_struct &b)
{
	return a.dist > b.dist;
}

int main()
{
	//СОЗДАНИЕ ОКНА

	int screenWidth, screenHeight;

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "CG", nullptr, nullptr);
	global_window = window;
	{
		glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

		if (nullptr == window)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return -1;
		}

		glfwMakeContextCurrent(window);
		//glfwSetKeyCallback(window, key_callback);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetScrollCallback(window, scroll_callback);
		glewExperimental = GL_TRUE;

		if (GLEW_OK != glewInit())
		{
			std::cout << "Failed to initialize GLEW" << std::endl;
			return EXIT_FAILURE;
		}

		glViewport(0, 0, screenWidth, screenHeight);//преобразуем координаты отсечения от -1.0 до 1.0 в область экранных координат
	}

	//say to OpenGL to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//ВКЛЮЧАЕМ БУФЕР ПРОВЕРКИ ГЛУБИНЫ
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_ALWAYS); // always pass the depth test (same effect as glDisable(GL_DEPTH_TEST))
	//glEnable(GL_STENCIL_TEST);

	/*glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);//включаем трафаретное тестирование
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	*/
	//test of matrix operation
	/*
	glm::vec4 vec(1.0f,0.0f,0.0f,1.0f);
	glm::mat4 trans;//по-умолчанию это единичная матрица 4х4
	matrix_init4(trans);//забили диагонали единицами
	trans = glm::translate(trans, glm::vec3(1.0f, 1.0f, 0.0f));//превратили нашу единичную матрицу в матрицу трансформации
	//translate - Builds a translation 4 * 4 matrix created from a vector of 3 components
	vec=trans*vec;//trans теперь это матрица сдвига на (x+1,y+1,z+0)
	std::cout <<vec.x << vec.y << vec.z << std::endl;
	 */

	 // Build and compile our shader program
	 //Shader ourShader("shader.vs","shader.frag");

	 // configure global opengl state
	 // -----------------------------
	glEnable(GL_DEPTH_TEST);//включили режим смешивания и выбрали его параметры
	glEnable(GL_BLEND);//включаем смешивание цветов(для создания полупрозрачных билбордов)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glBlendColor(0.5f,0.5f,0.5f,0.5f);
	//glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR);

	/*
	 !!!
	 порядок действий при рендере сцены, содержащей как непрозрачные, так и прозрачные объекты, выглядит следующим образом:

	 Вывести все непрозрачные объекты.
	 Отсортировать прозрачные объекты по удалению.
	 Нарисовать прозрачные объекты в отсортированном порядке.
	 */

	 // configure global opengl state
	 // -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);//включаем смешивание цветов(для создания полупрозрачных билбордов)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//формула для смешивания цветов

	// build and compile shaders
	// -------------------------
	Shader pm_shader("shaders/parallax_mapping.vs", "shaders/parallax_mapping.frag");

	Shader shadowShader("shaders/shadow_mapping.vs", "shaders/shadow_mapping.frag");
	Shader simpleDepthShader("shaders/shadow_mapping_depth.vs", "shaders/shadow_mapping_depth.frag");

	Shader billbordShader("shaders/blending.vs", "shaders/blending.frag");

	Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.frag");

	Shader reflectShader("shaders/reflection.vs", "shaders/reflection.frag");

	Shader refractShader("shaders/refraction.vs", "shaders/refraction.frag");


	//load textures for normal_mapping
	//unsigned int diffuseMap = loadTexture("res/images/brickwall.jpg");
	//unsigned int normalMap  = loadTexture("res/images/brickwall_normal.jpg");
	//в текстуре brickwall_normal мы шифруем в rgb вектора нормалей

	//load dextures for parallax_mapping
	unsigned int diffuseMap = loadTexture("my tex 2/pm_orig.jpg");
	unsigned int normalMap = loadTexture("my tex 2/pm_normal.png");
	unsigned int heightMap = loadTexture("my tex 2/pm_disp.png");

	//unsigned int diffuseMap = loadTexture("res/images/bricks2.jpg");
	//unsigned int normalMap = loadTexture("res/images/bricks2_normal.jpg");
	//unsigned int heightMap = loadTexture("res/images/bricks2_disp.jpg");


	// !!! BEGIN BILLBORDS
	//координаты куба
	float cube_for_billbords_Vertices[] = {
		// positions          // texture Coords
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
	float billbord_vertices[] = //билборд
	{
		// positions         // texture Coords (swapped y coordinates because
											// texture is flipped upside down)
		0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
		0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
		1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

		0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
		1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
		1.0f,  0.5f,  0.0f,  1.0f,  0.0f
	};

	// cube VAO
	unsigned int cube_for_billbord_VAO, cube_for_billbord_VBO;
	glGenVertexArrays(1, &cube_for_billbord_VAO);
	glGenBuffers(1, &cube_for_billbord_VBO);
	glBindVertexArray(cube_for_billbord_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, cube_for_billbord_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_for_billbords_Vertices), &cube_for_billbords_Vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	// billbord VAO
	unsigned int billbordVAO, billbordVBO;
	glGenVertexArrays(1, &billbordVAO);
	glGenBuffers(1, &billbordVBO);
	glBindVertexArray(billbordVAO);
	glBindBuffer(GL_ARRAY_BUFFER, billbordVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(billbord_vertices), billbord_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);

	//unsigned int cube_for_billbord_Texture =loadTexture("res/images/brickwall_normal.jpg");
	unsigned int cube_for_billbord_Texture = loadTexture("my tex 2/neon2_cube.jpg");
	unsigned int billbordTexture = loadTexture("res/images/window.png");

	// transparent window locations
	// --------------------------------
	std::vector<glm::vec3> windows //billbord coords
	{
		glm::vec3(-1.5f, 0.0f, -0.48f),
		glm::vec3(1.5f, 2.0f, 0.51f),
		glm::vec3(0.0f, 0.0f, 0.7f),
		glm::vec3(-0.3f, 0.0f, -2.3f),
		glm::vec3(0.5f, 0.0f, -0.6f)
	};

	// shader configuration
	// --------------------
	billbordShader.Use();
	billbordShader.setInt("texture1", 0);

	// !!! END BILLBORDS

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float planeVertices[] = {
		// positions            // normals         // texcoords
		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		 25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};
	float cubeVertices[] = {
		// координаты        // нормали
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

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
	float skyboxVertices[] = {
		// positions
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

	// !!! BEGIN shadows VAO, depth_map FBO

	// plane VAO
	unsigned int planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	// load textures
	// -------------
	unsigned int woodTexture = loadTexture("res/images/floor_tiles.jpg");//wood.png");

	//создаём кадровый буфер для рисования карты глубины
	// -----------------------

	unsigned int depthMapFBO;//созали кадровый буфер для рисования карты глубины
	glGenFramebuffers(1, &depthMapFBO);

	//создаём 2д текстуру, чтобы использовать её качестве буфера глубины для кадрового буфера
	// create depth texture
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	unsigned int depthMap;//текстура для заполнения глубиной
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	// GL_DEPTH_COMPONENT - т.к. нас интереуют только значения глубины (а не цвета r,g,b,a)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//GL_CLAMP_TO_BORDER - Теперь, если мы читаем значение из карты глубины по координатам вне интервала [0,1], мы будем получать в ответ глубину 1.0, из за чего значение shadow в шейдере будет 0.0.

	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };//чтобы удаленные места сцены не затенялись
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	//Присоединяем текстуру глубины к кадровому буферу в качестве буфера глубины
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);// укзываем, что не собираемся рендерить цвет
	glReadBuffer(GL_NONE);// укзываем, что не собираемся рендерить цвет

	glBindFramebuffer(GL_FRAMEBUFFER, 0);//Переключаемся обратно на буфер по умолчянию



	// !!! END shadows VAO, depth_map FBO
	//reflect VAO
	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));


	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


	/*
	 lesson's skybox
	std::vector<std::string> faces
	   {
		   "skybox/right.jpg",
		   "skybox/left.jpg",
		   "skybox/top.jpg",
		   "skybox/bottom.jpg",
		   "skybox/front.jpg",
		   "skybox/back.jpg"
	   };
	*/
	/*
	std::vector<std::string> faces
	{

		"myskybox/left.png", "myskybox/right.png",
		"myskybox/top.png",
		"myskybox/bottom.png",
		"myskybox/front.png",
		"myskybox/back.png"
	};
	*/
	// +X (right)
	// -X (left)
	// +Y (top)
	// -Y (bottom)
	// +Z (front)
	// -Z (back)
	/*std::vector<std::string> faces
	{

		"myskybox2/posx.jpg",
		"myskybox2/negx.jpg",
		"myskybox2/posy.jpg",
		"myskybox2/negy.jpg",
		"myskybox2/posz.jpg",
		"myskybox2/negz.jpg"
	};*/

	unsigned int cubeTexture = loadTexture("res/images/wood.png");

	std::vector<std::string> faces
	{
		"skybox/front.jpg",
		"skybox/back.jpg",
		"skybox/top.jpg",
		"skybox/bottom.jpg",
		"skybox/right.jpg",
		"skybox/left.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(faces);


	// PM shader configuration
	// --------------------
	pm_shader.Use();
	pm_shader.setInt("diffuseMap", 0);
	pm_shader.setInt("normalMap", 1);
	pm_shader.setInt("depthMap", 2);

	// SHADOW shader configuration
	// --------------------
	shadowShader.Use();
	shadowShader.setInt("diffuseTexture", 0);
	shadowShader.setInt("shadowMap", 1);

	reflectShader.Use();
	reflectShader.setInt("skybox", 0);

	refractShader.Use();
	refractShader.setInt("skybox", 0);

	skyboxShader.Use();
	skyboxShader.setInt("skybox", 0);

	// render loop
	// -----------

	while (!glfwWindowShouldClose(window))
	{

		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		glfwPollEvents();//обабатываем события
		key_callback(window);
		//Do_movement();//обрабатываем перемещения(нажатие WASD)

		// change light position over time
		//lightPos.x = sin(glfwGetTime()) * 3.0f;
		//lightPos.z = cos(glfwGetTime()) * 2.0f;
		//lightPos.y = 5.0 + cos(glfwGetTime()) * 1.0f;

		/*
		!!!
		порядок действий при рендере сцены, содержащей как непрозрачные, так и прозрачные объекты, выглядит следующим образом:

		Вывести все непрозрачные объекты.
		Отсортировать прозрачные объекты по удалению.
		Нарисовать прозрачные объекты в отсортированном порядке.
		*/

		//сортируем окна по удаленности и будем выводить от дальнего к ближнему, чтобы через ближние было видно дальние
		// sort the transparent windows before rendering
		// ---------------------------------------------



		// render
		// ------
		// make sure we clear the framebuffer's content
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//lightPos.y=(3.5f+1.1*sin(( glfwGetTime())));
		lightPos.x = (3.0f + 0.4*sin((glfwGetTime())));


		// configure view/projection matrices
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		// !!! BEGIN SHADOWs CODE

		// 1. Сначала рисуем карту глубины
		// 1. render depth of scene to texture (from light's perspective)
		// B первом проходе мы используем матрицы проекции и вида для рисования сцены с точки зрения источника света
		// --------------------------------------------------------------
		glm::mat4 lightProjection;
		glm::mat4 lightSpaceMatrix;
		glm::mat4 lightView;//куда смотрит источник света

		matrix_init4(lightProjection); matrix_init4(lightView); matrix_init4(lightSpaceMatrix);

		// Мы будет использовать ортографическую матрицу проекции для источника света (в ней нет переспектиных искажений)
		float near_plane = 1.0f, far_plane = 7.5f;//задаем расстояние между которым распростроняется свет

		//использовать ортографическую матрицу проекции для источника света (в ней нет переспектиных искажений)
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		//первый вектор — расположение камеры, второй — куда она смотрит, третий — направление взгляда вверх

		lightSpaceMatrix = lightProjection * lightView;//Используем её вместо матриц вида и проекции обычной камеры
		// render scene from light's point of view
		simpleDepthShader.Use();
		simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTexture);
		renderScene(simpleDepthShader);//рисование в буфер глубины
		glBindFramebuffer(GL_FRAMEBUFFER, 0);//переключаемся на кадровый буфер по умолчанию

		// reset viewport
		glViewport(0, 0, WIDTH, HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 2. Рисуем сцену как обычно с тенями (используя карту глубины)
		// 2. render scene as normal using the generated depth/shadow map
		// --------------------------------------------------------------
		glViewport(0, 0, WIDTH, HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shadowShader.Use();
		//glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		//glm::mat4 view = camera.GetViewMatrix();
		shadowShader.setMat4("projection", projection);
		shadowShader.setMat4("view", view);
		// set light uniforms
		shadowShader.setVec3("viewPos", camera.Position);
		shadowShader.setVec3("lightPos", lightPos);
		shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		renderScene(shadowShader);


		// !!! END SHADOWs CODE



		// !!! BEGIN PARALLAX_MAPPING
		pm_shader.Use();

		pm_shader.setMat4("projection", projection);
		pm_shader.setMat4("view", view);

		// render normal-mapped quad
		glm::mat4 model = glm::mat4(1.0f);
		//model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f),
		  //                  glm::normalize(glm::vec3(1.0, 0.0, 1.0))); // rotate the quad to show normal mapping from multiple directions
		//model = glm::rotate(model, glm::radians( -60.0f),
						 // glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 1.0f));//увеличили стену
		pm_shader.setMat4("model", model);
		pm_shader.setVec3("viewPos", camera.Position);
		pm_shader.setVec3("lightPos", lightPos);
		pm_shader.setFloat("heightScale", heightScale); // adjust with Q and E keys
		//std::cout << heightScale << std::endl;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMap);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, heightMap);
		renderQuad();

		// render light source (simply re-renders a smaller plane at the light's position for debugging/visualization)
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.1f));
		pm_shader.setMat4("model", model);
		renderQuad();

		// !!! END PARALLAX_MAPPING

		// draw skybox as last
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		//это надо, чтобы уменьшить нагрузку на вычисления skybox и ненужные части не рендерить

		skyboxShader.Use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);

		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default


		//reflection
		reflectShader.Use();
		model = glm::mat4(1.0f);
		view = camera.GetViewMatrix();
		model = glm::translate(model, glm::vec3(-3.0, 2.0, 0.5));
		model = glm::rotate(model, glm::radians((float)glfwGetTime() * 30.0f), glm::normalize(glm::vec3(-0.75, 0.5, -1.0)));
		model = glm::scale(model, glm::vec3(0.7f));
		reflectShader.setMat4("model", model);
		reflectShader.setMat4("view", view);
		reflectShader.setMat4("projection", projection);
		reflectShader.setVec3("cameraPos", camera.Position);
		glBindVertexArray(cubeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		//refraction 
		refractShader.Use();
		model = glm::mat4(1.0f);
		view = camera.GetViewMatrix();
		model = glm::translate(model, glm::vec3(-4.0, 3.0, 0.5));
		model = glm::rotate(model, glm::radians((float)glfwGetTime() * 30.0f), glm::normalize(glm::vec3(-0.75, 0.5, -1.0)));
		model = glm::scale(model, glm::vec3(0.7f));
		refractShader.setMat4("model", model);
		refractShader.setMat4("view", view);
		refractShader.setMat4("projection", projection);
		refractShader.setVec3("cameraPos", camera.Position);
		glBindVertexArray(cubeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		// !!! BEGIN BILLBORDS

		/*
			   !!!
			   порядок действий при рендере сцены, содержащей как непрозрачные, так и прозрачные объекты, выглядит следующим образом:

			   Вывести все непрозрачные объекты.
			   Отсортировать прозрачные объекты по удалению.
			   Нарисовать прозрачные объекты в отсортированном порядке.
			   */

			   //сортируем окна по удаленности и будем выводить от дальнего к ближнему, чтобы через ближние было видно дальние
			   // sort the transparent windows before rendering
			   // ---------------------------------------------



		/*std::vector<windows_struct> sortwindows(windows.size());
		for (unsigned int i = 0; i < windows.size(); i++)
		{
			float distance = glm::length(camera.Position - windows[i]);
			windows_struct curwind;
			curwind.dist = distance;
			curwind.w = windows[i];

			sortwindows.push_back(curwind);
		}
		std::sort(sortwindows.begin(), sortwindows.end(), compareByDistance);*/

		std::multimap<float, glm::vec3> sorted;
		for (unsigned int i = 0; i < windows.size(); i++)
		{
			float distance = glm::length(camera.Position - windows[i]);
			sorted.insert(std::make_pair(distance, windows[i]));
		}

		// render
		// ------

		// set uniforms
		billbordShader.Use();

		//glm::mat4 model ; matrix_init4(model);
		//glm::mat4 view ;  matrix_init4(view);
		//glm::mat4 projection; matrix_init4(projection);

		model = glm::mat4(1.0f);
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		billbordShader.setMat4("view", view);
		billbordShader.setMat4("projection", projection);

		billbordShader.Use();
		billbordShader.setMat4("view", view);
		billbordShader.setMat4("projection", projection);




		// cubes
		glBindVertexArray(cube_for_billbord_VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cube_for_billbord_Texture);
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -2.0f));
		billbordShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, 0.0f, -2.0f));
		billbordShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-4.0f, 0.0f, -4.0f));
		billbordShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(4.0f, 0.0f, -4.0f));
		billbordShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-6.0f, 0.0f, -6.0f));
		billbordShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(6.0f, 0.0f, -6.0f));
		billbordShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// windows (from furthest to nearest)
		glBindVertexArray(billbordVAO);
		glBindTexture(GL_TEXTURE_2D, billbordTexture);

		//выводим окна от дальнего к ближнему

		for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, it->second);
			billbordShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		/*for (unsigned int i = 0; i < sortwindows.size(); i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, sortwindows[i].w);
			billbordShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}*/



		// !!! END BILLBORDS




		/*

		// set uniforms
		shader.Use();

		glm::mat4 model ; matrix_init4(model);
		glm::mat4 view ;  matrix_init4(view);
		glm::mat4 projection; matrix_init4(projection);

		model = glm::mat4(1.0f);
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH/ (float)HEIGHT, 0.1f, 100.0f);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		shader.Use();
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		/*
		 шаги, необходимые для вывода сцены в текстуру:

		 1. Привязать наш объект буфера кадра как текущий и вывести сцену обычным образом.
		 2. Привязать буфер кадра по умолчанию.
		 3. Вывести полноэкранный квад с наложением текстуры из буфера цвета нашего объекта кадрового буфера.

		 */
		 /*
		 // cubes
		 glBindVertexArray(cubeVAO);
		 glActiveTexture(GL_TEXTURE0);
		 glBindTexture(GL_TEXTURE_2D, cubeTexture);
		 model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
		 shader.setMat4("model", model);
		 glDrawArrays(GL_TRIANGLES, 0, 36);
		 model = glm::mat4(1.0f);
		 model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
		 shader.setMat4("model", model);
		 glDrawArrays(GL_TRIANGLES, 0, 36);


		 // draw skybox as last
		 glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		 //это надо, чтобы уменьшить нагрузку на вычисления skybox и ненужные части не рендерить

		 skyboxShader.Use();
		 view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		 skyboxShader.setMat4("view", view);
		 skyboxShader.setMat4("projection", projection);

		 // skybox cube
		 glBindVertexArray(skyboxVAO);
		 glActiveTexture(GL_TEXTURE0);
		 glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		 glDrawArrays(GL_TRIANGLES, 0, 36);
		 glBindVertexArray(0);
		 glDepthFunc(GL_LESS); // set depth function back to default
		  */
		  // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		  // -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------


	glDeleteVertexArrays(1, &planeVAO);
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteBuffers(1, &skyboxVAO);

	glfwTerminate();

	return 0;
}

// renders a 1x1 quad in NDC with manually calculated tangent vectors
// ------------------------------------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		// позиции

		glm::vec3 pos1(-1.0f, 1.0f, -3.0f);
		glm::vec3 pos2(-1.0f, -1.0f, -3.0f);
		glm::vec3 pos3(1.0f, -1.0f, -3.0f);
		glm::vec3 pos4(1.0f, 1.0f, -3.0f);


		// текстурные координаты
		glm::vec2 uv1(0.0f, 1.0f);
		glm::vec2 uv2(0.0f, 0.0f);
		glm::vec2 uv3(1.0f, 0.0f);
		glm::vec2 uv4(1.0f, 1.0f);

		// вектор нормали
		glm::vec3 nm(0.0f, 0.0f, 1.0f);
		//glm::vec3 nm(0.0f,1.0f,0.0f);

		// calculate tangent/bitangent vectors of both triangles
		glm::vec3 tangent1, bitangent1;
		glm::vec3 tangent2, bitangent2;
		// triangle 1
		// ----------
		glm::vec3 edge1 = pos2 - pos1;//вектор стороны треуголника E1
		glm::vec3 edge2 = pos3 - pos1;//вектор стороны треуголника E2

		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent1 = glm::normalize(tangent1);

		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent1 = glm::normalize(bitangent1);

		// triangle 2
		// ----------
		edge1 = pos3 - pos1;
		edge2 = pos4 - pos1;
		deltaUV1 = uv3 - uv1;
		deltaUV2 = uv4 - uv1;

		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent2 = glm::normalize(tangent2);


		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent2 = glm::normalize(bitangent2);


		float quadVertices[] = {
			// positions            // normal         // texcoords  // tangent                          // bitangent
			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
		};
		// configure plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}




// renders the 3D scene
// --------------------

void renderScene(Shader &shader)
{
	// floor
	glm::mat4 model = glm::mat4(1.0f);
	shader.setMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// cubes
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 1.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMat4("model", model);
	renderCube();
}


// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}
/*
// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
*/



// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		if (heightScale > 0.0f)
			heightScale -= 0.0005f;
		else
			heightScale = 0.0f;
	}
	else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		if (heightScale < 1.0f)
			heightScale += 0.0005f;
		else
			heightScale = 1.0f;
	}
}


bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;  // Обратный порядок вычитания потому что оконные Y-координаты возрастают с верху вниз

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}