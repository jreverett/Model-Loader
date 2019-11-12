#include "ModelLoader.h"
#include "LoadObj.h"
#include "Shader.h"


/*******************************************************
FILETYPES:

.obj - Wavefront OBJ & MTL
.dae - Collada DAE
.fbx - Autodesk FBX (proprietary)
.3ds - Autodesk 3DS (proprietary)

********************************************************/


///////////////////////////////////////////////////
// DataTypes
enum class WindowStatus {
	FOCUSED,
	NOT_FOCUSED
};

struct displayObj {
	unsigned int VAO;
	unsigned int texture;
};


///////////////////////////////////////////////////
// Forward Declarations
void display(GLFWwindow* window, std::string modelPath, std::vector<Mesh> meshVector);

void processInput(GLFWwindow* window);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow* window, double xPos, double yPos);
void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
void setUniformMatrix(Shader shaders, glm::mat4 matrix, const char* uniformName);
void onWindowResize(GLFWwindow* window, int width, int height);
void printWelcomeAscii();


///////////////////////////////////////////////////
// Global Vars (Default Program Behaviour)
// window properties
const char* DEFAULT_SCR_TITLE = "JE - ModelLoader";
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
double lastX = 400;
double lastY = 300;
double yaw = -90.0f;
double pitch = 0.0f;
double fov = 45.0f;
bool firstMouse = true;
WindowStatus windowStatus = WindowStatus::FOCUSED;

// rendering
bool captureMouse = true;
bool wireframe = false;

// timing
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
bool awaitingRelease = false;

// user feedback
bool displayAscii = true;


int main()
{
	/*if (displayAscii)
		printWelcomeAscii();*/
	displayAscii = false;
	/*
	///////////////////////////////////////////////////
	// Get user input (file/folder directory)
	std::cout << "Enter the location of a model file or folder of models to continue..." << std::endl;
	std::cout << "(press 'Escape' to close an open model)" << std::endl;
	std::string modelPath; std::cin >> modelPath;
	*/
	std::string modelPath = R"(D:\source\repos\SOFT356\Model Loader\Test Files\Creeper-obj\creeper.obj)";
	//std::string modelPath = R"(D:\source\repos\SOFT356\Model Loader\Test Files\LowPolyBoat-obj\low_poly_boat.obj)";

	///////////////////////////////////////////////////
	// Read Model
	std::regex pattern(".[a-z0-9]+$", std::regex_constants::icase);
	std::smatch fileExtension;
	std::regex_search(modelPath, fileExtension, pattern);
	
	std::vector<Mesh> meshVector;

	// Init opengl
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, DEFAULT_SCR_TITLE, NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, onWindowResize);

	glewInit();

	if (fileExtension[0] == ".obj") {
		meshVector = loadObj(modelPath);
	} 
	else if (fileExtension[0] == ".dae") {
		// TODO: load a .dae file
		// loadDae(modelPath);
	}
	else if (fileExtension[0] == ".fbx") {
		// TODO: load a .fbx file
		// loadFbx(modelPath);
	}
	else if (fileExtension[0] == ".3ds") {
		// TODO: load a .3ds file
		// load3ds(modelPath);
	}
	else {
		system("cls");

		printWelcomeAscii();
		std::cout << std::endl;
		std::cout << "ERROR->" << __FUNCTION__ << ": Unsupported file type" << std::endl;
		std::cout << "Try using the supported file types:" << std::endl;
		std::cout << "  - .obj" << std::endl;
		std::cout << std::endl;

		glfwTerminate();

		main();
	}

	display(window, modelPath, meshVector); // TODO: make this compatable with the DAE loader return
}


void display(GLFWwindow* window, std::string modelPath, std::vector<Mesh> meshVector) {
	// Attach input callbacks
	glfwSetKeyCallback(window, keyCallback); // only used to check key releases
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetScrollCallback(window, scrollCallback);

	// Record mouse input
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	glEnable(GL_DEPTH_TEST);

	// Build and compile Shader program
	Shader shaders("shaders/shader.vs", "shaders/shader.fs");

	// Setup buffers
	for (int i = 0; i < meshVector.size(); i++)	{
		meshVector[i].draw(shaders);
	}
	
	while (!glfwWindowShouldClose(window))
	{
		// Frame timer logic
		float currFrame = (float)glfwGetTime();
		deltaTime = currFrame - lastFrame;
		lastFrame = currFrame;

		// Check inputs
		processInput(window);

		glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Model transformations
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);

		float angleDelta = (float)glfwGetTime() * 0.4f;

		//model = glm::rotate(model, angleDelta, glm::vec3(1.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(0.005f, 0.005f, 0.005f));
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		projection = glm::perspective(glm::radians((float)fov), (float)(SCR_WIDTH/SCR_HEIGHT), 0.1f, 100.0f);

		// Select shaders
		shaders.use();
		setUniformMatrix(shaders, model, "model");
		setUniformMatrix(shaders, view, "view");
		setUniformMatrix(shaders, projection, "projection");
		glUniform1i(glGetUniformLocation(shaders.ID, "theTexture"), 0);

		for (unsigned int i = 0; i < meshVector.size(); i++) {
			meshVector[i].draw(shaders);
		}
		glDrawArrays(GL_TRIANGLES, 0, (GLint)meshVector[0].objData.vertices.size());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//glDeleteBuffers(BufferValue::NUM_BUFFERS, buffers);

	glfwTerminate();

	//main();
}


void processInput(GLFWwindow* window) {
	if ((glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) && !awaitingRelease) {
		firstMouse = true;
		captureMouse = !captureMouse;
		captureMouse ? windowStatus = WindowStatus::FOCUSED : windowStatus = WindowStatus::NOT_FOCUSED;
		captureMouse ? glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED) : glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		awaitingRelease = true;
	}

	if (windowStatus == WindowStatus::NOT_FOCUSED)
		return;

	float cameraSpeed = 2.5f * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
		displayAscii = true;
		system("cls");
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		cameraPos += cameraUp * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		cameraPos -= cameraUp * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !awaitingRelease) {
		wireframe = !wireframe;
		wireframe ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		awaitingRelease = true;
	}
}


void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (GLFW_KEY_1 && action == GLFW_RELEASE)
		awaitingRelease = false;
	if (GLFW_KEY_2 && action == GLFW_RELEASE)
		awaitingRelease = false;
}


void mouseCallback(GLFWwindow* window, double xPos, double yPos) {
	if (windowStatus == WindowStatus::NOT_FOCUSED)
		return;

	if (firstMouse) {
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	// Calculate mouse offset
	double xOffset = xPos - lastX;
	double yOffset = lastY - yPos;
	lastX = xPos;
	lastY = yPos;

	float sensitivity = 0.05f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	// Add offset values to camera values
	yaw += xOffset;
	pitch += yOffset;

	// Add pitch constraints
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	// Calculate direction vector
	glm::vec3 cameraDir;
	cameraDir.x = cos(glm::radians((float)pitch)) *  cos(glm::radians((float)yaw));
	cameraDir.y = sin(glm::radians((float)pitch));
	cameraDir.z = cos(glm::radians((float)pitch)) * sin(glm::radians((float)yaw));
	cameraFront = glm::normalize(cameraDir);
}


void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
	// Limit fov between 1.0f and 45.0f
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yOffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}


void setUniformMatrix(Shader shaders, glm::mat4 matrix, const char* uniformName) {
	unsigned int loc = glGetUniformLocation(shaders.ID, uniformName);
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
}


void onWindowResize(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}


void printWelcomeAscii() {
	std::cout << R"(
	##################################################################################################
	#                                                                                                #
	#          ,___          .-;'                                                                    #
	#         `"-.`\_...._/`.`                                                                       #
	#      ,      \        /      __  __           _      _   _                     _                #
	#   .-' ',    / ()   ()\     |  \/  |         | |    | | | |                   | |               #
	#  `'._   \  /()    .  (|    | \  / | ___   __| | ___| | | |     ___   __ _  __| | ___ _ __      #
	#      > .' ;,     -'-  /    | |\/| |/ _ \ / _` |/ _ \ | | |    / _ \ / _` |/ _` |/ _ \ '__|     #
	#     / <   |;,     __.;     | |  | | (_) | (_| |  __/ | | |___| (_) | (_| | (_| |  __/ |        #
	#     '-.'-.|  , \    , \    |_|  |_|\___/ \__,_|\___|_| |______\___/ \__,_|\__,_|\___|_|        #
	#        `>.|;, \_)    \_)                                                                       #
	#         `-;     ,    /                                                                         #
	#            \    /   <                                                                          #
	#             '. <`'-,_)                                                                         #
	#          jgs '._)                                                                              #
	#                                                                                                #
	##################################################################################################
	)" << '\n';
}