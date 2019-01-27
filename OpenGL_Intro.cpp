#include <iostream>

#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include "engine\engine.h"
#include "engine\managers\shaderManager\shaderManager.h"
#include "core\logging.h"

#include "core\messageManager.h"
#include <thread>

void resize(GLFWwindow* pWindow, int width, int height) {
	glViewport(0, 0, width, height);
}
void processInput(GLFWwindow* pWindow) {
	if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(pWindow, true);
	}
	if (glfwGetKey(pWindow, GLFW_KEY_1) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(pWindow, GLFW_KEY_2) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void cbt1(geManagers::sMessage& msg) { std::cout << "1] Got Message with id " << msg.iUniqueMessageId << std::endl; }
void cbt2(geManagers::sMessage& msg) { std::cout << "2] Got Message with id " << msg.iUniqueMessageId << std::endl; }
void cbt3(geManagers::sMessage& msg) { std::cout << "3] Got Message with id " << msg.iUniqueMessageId << std::endl; }
void cbt4(geManagers::sMessage& msg) { std::cout << "4] Got Message with id " << msg.iUniqueMessageId << std::endl; }


void test() {
	geManagers::MessageManager *MsgPumpTest = new geManagers::MessageManager();

	MsgPumpTest->RegisterEvent("ExampleEvent1", 1);
	MsgPumpTest->RegisterEvent("ExampleEvent2", 2);
	MsgPumpTest->RegisterEvent("ExampleEvent3", 3);
	MsgPumpTest->RegisterEvent("ExampleEvent4", 4);

	std::thread t(&geManagers::MessageManager::Start, MsgPumpTest);

	MsgPumpTest->Subscribe(1, cbt1);
	MsgPumpTest->Subscribe(2, cbt2);
	MsgPumpTest->Subscribe(3, cbt3);
	MsgPumpTest->Subscribe(4, cbt4);
	
	int i = 0;
	geManagers::sMessage myMessage{ i };
	while (i<10) {
		Sleep(1);
		myMessage.iMessageId = i;
		int target = (rand() % 4) + 1;
		MsgPumpTest->QueueMessage(target, myMessage);
		i++;
	}

	while (MsgPumpTest->Pending()) { Sleep(1); };
	MsgPumpTest->Die();
	t.join();
}

int main(){

	test();

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow* gWindow = glfwCreateWindow(800, 600, "Learning OpenGL", nullptr, nullptr);
	if (gWindow == nullptr) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(gWindow);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwTerminate();
		return -2;
	}

	core::Shader_Manager::Init();
	core::Shader_Manager* pShader = core::Shader_Manager::Get();
	LOG_IF_ERROR(!pShader->CreateProgram("default","./programs/defaultVertexShader.glsl","./programs/defaultFragmentShader.glsl"), "Failed to create program!");
	
	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(gWindow, resize);

	float verts[] = {
	 0.5f,  0.5f, 0.0f,  // top right
	 0.5f, -0.5f, 0.0f,  // bottom right
	-0.5f, -0.5f, 0.0f,  // bottom left
	-0.5f,  0.5f, 0.0f   // top left 
	};
	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	unsigned int VAO,VBO,EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	while (!glfwWindowShouldClose(gWindow)) {
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		pShader->UseShader("default");
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		processInput(gWindow);
		
		glfwSwapBuffers(gWindow);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glfwTerminate();

	return 0;
}
