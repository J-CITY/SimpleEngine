#include <iostream>

//glew
//#define GLEW_STATIC
#include <GL/glew.h>

//sfml
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "math.h"
using namespace std;


#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

//imgui
#include "3rd/imgui/imgui.h"
#include "3rd/imgui/imgui-SFML.h"

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

/*
int main() {
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.majorVersion = 4;
	settings.minorVersion = 3;
	settings.attributeFlags = sf::ContextSettings::Core;

	sf::RenderWindow window(sf::VideoMode(800, 600, 32), "First Window", sf::Style::Titlebar|sf::Style::Close);
	//window.setFramerateLimit(60);
	window.setActive();


	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit()) {
		cout << "Error:: glew not init =(" << endl;
		return -1;
	}

	ImGui::SFML::Init(window);
	//window.resetGLStates();
	//window.pushGLStates();

	
	
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	//glBindVertexArray(VertexArrayID);

	GLuint programID = LoadShaders("shader.vs", "shader.fs");
	
	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 0.0f,  1.0f, 0.0f,
	};
	
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	

	sf::Color bgColor;
	float color[3] = { 0.f, 0.f, 0.f };
	sf::Clock deltaClock;
	//bool show_test_window = true;
	bool show_another_window = true;
	ImVec4 clear_color = ImColor(114, 144, 154);

	bool isGo = true;
	while (isGo) {
		sf::Event windowEvent;
		while (window.pollEvent(windowEvent)) {
			ImGui::SFML::ProcessEvent(windowEvent);
			switch (windowEvent.type)
			{
			case sf::Event::Closed:
				isGo = false;
				break;
			default:
				break;
			}
		}
		


		ImGui::SFML::Update(window, deltaClock.restart());

		ImGui::Begin("Sample window"); // создаём окно

		// Инструмент выбора цвета
		if (ImGui::ColorEdit3("Background color", color)) {
			// код вызывается при изменении значения, поэтому всё
			// обновляется автоматически
			bgColor.r = static_cast<sf::Uint8>(color[0] * 255.f);
			bgColor.g = static_cast<sf::Uint8>(color[1] * 255.f);
			bgColor.b = static_cast<sf::Uint8>(color[2] * 255.f);
		}

		char s[100] = "ImGui + SFML = <3";
		ImGui::InputText("Window title", s, 255);

		if (ImGui::Button("Update window title")) {
			// этот код выполняется, когда юзер жмёт на кнопку
			// здесь можно было бы написать 
			// if(ImGui::InputText(...))
			window.setTitle("!!!!!!");
		}
		ImGui::End(); // end window
		
		//..............................................
		//window.clear();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		//ImGui::SFML::Render(window);
		//window.pushGLStates();
		
		// Use our shader
		glUseProgram(programID);


		glBindVertexArray(VertexArrayID);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);


	
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle
		glBindVertexArray(0);

		glDisableVertexAttribArray(0);
		
		glUseProgram(0);
		

		window.pushGLStates();
		ImGui::SFML::Render(window);
		window.popGLStates();


		window.display();
	}

	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID);

	ImGui::SFML::Shutdown();

	window.close();
	return 0;
}
*/


int main() {
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.majorVersion = 4;
	settings.minorVersion = 3;
	settings.attributeFlags = sf::ContextSettings::Core;

	sf::RenderWindow window(sf::VideoMode(800, 600, 32), "First Window", sf::Style::Titlebar | sf::Style::Close);
	//window.setFramerateLimit(60);
	window.setActive();


	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit()) {
		cout << "Error:: glew not init =(" << endl;
		return -1;
	}

	ImGui::SFML::Init(window);
	//window.resetGLStates();
	//window.pushGLStates();



	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	//glBindVertexArray(VertexArrayID);

	GLuint programID = LoadShaders("shader.vs", "shader.fs");



	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	auto Projection = Mathgl::perspective(Mathgl::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	auto View = Mathgl::lookAt(
		Mathgl::Matrix<float>::Vec3(4, 3, -3), // Camera is at (4,3,-3), in World Space
		Mathgl::Matrix<float>::Vec3(0, 0, 0), // and looks at the origin
		Mathgl::Matrix<float>::Vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	// Model matrix : an identity matrix (model will be at the origin)
	auto Model = Mathgl::Matrix<float>::Mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	auto MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

	static const GLfloat g_vertex_buffer_data [] = {
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		 1.0f,-1.0f,-1.0f,
		 1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		 1.0f,-1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0f,-1.0f,-1.0f,
		 1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f,-1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0f,-1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		 1.0f,-1.0f, 1.0f
	};

	// One color for each vertex. They were generated randomly.
	static const GLfloat g_color_buffer_data [] = {
		0.583f,  0.771f,  0.014f,
		0.609f,  0.115f,  0.436f,
		0.327f,  0.483f,  0.844f,
		0.822f,  0.569f,  0.201f,
		0.435f,  0.602f,  0.223f,
		0.310f,  0.747f,  0.185f,
		0.597f,  0.770f,  0.761f,
		0.559f,  0.436f,  0.730f,
		0.359f,  0.583f,  0.152f,
		0.483f,  0.596f,  0.789f,
		0.559f,  0.861f,  0.639f,
		0.195f,  0.548f,  0.859f,
		0.014f,  0.184f,  0.576f,
		0.771f,  0.328f,  0.970f,
		0.406f,  0.615f,  0.116f,
		0.676f,  0.977f,  0.133f,
		0.971f,  0.572f,  0.833f,
		0.140f,  0.616f,  0.489f,
		0.997f,  0.513f,  0.064f,
		0.945f,  0.719f,  0.592f,
		0.543f,  0.021f,  0.978f,
		0.279f,  0.317f,  0.505f,
		0.167f,  0.620f,  0.077f,
		0.347f,  0.857f,  0.137f,
		0.055f,  0.953f,  0.042f,
		0.714f,  0.505f,  0.345f,
		0.783f,  0.290f,  0.734f,
		0.722f,  0.645f,  0.174f,
		0.302f,  0.455f,  0.848f,
		0.225f,  0.587f,  0.040f,
		0.517f,  0.713f,  0.338f,
		0.053f,  0.959f,  0.120f,
		0.393f,  0.621f,  0.362f,
		0.673f,  0.211f,  0.457f,
		0.820f,  0.883f,  0.371f,
		0.982f,  0.099f,  0.879f
	};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

	sf::Color bgColor;
	float color[3] = {0.f, 0.f, 0.f};
	sf::Clock deltaClock;
	//bool show_test_window = true;
	bool show_another_window = true;
	ImVec4 clear_color = ImColor(114, 144, 154);

	bool isGo = true;
	while (isGo) {
		sf::Event windowEvent;
		while (window.pollEvent(windowEvent)) {
			ImGui::SFML::ProcessEvent(windowEvent);
			switch (windowEvent.type) {
			case sf::Event::Closed:
				isGo = false;
				break;
			default:
				break;
			}
		}



		ImGui::SFML::Update(window, deltaClock.restart());

		ImGui::Begin("Sample window"); // создаём окно

		// Инструмент выбора цвета
		if (ImGui::ColorEdit3("Background color", color)) {
			// код вызывается при изменении значения, поэтому всё
			// обновляется автоматически
			bgColor.r = static_cast<sf::Uint8>(color[0] * 255.f);
			bgColor.g = static_cast<sf::Uint8>(color[1] * 255.f);
			bgColor.b = static_cast<sf::Uint8>(color[2] * 255.f);
		}

		char s[100] = "ImGui + SFML = <3";
		ImGui::InputText("Window title", s, 255);

		if (ImGui::Button("Update window title")) {
			// этот код выполняется, когда юзер жмёт на кнопку
			// здесь можно было бы написать 
			// if(ImGui::InputText(...))
			window.setTitle("!!!!!!");
		}
		ImGui::End(); // end window

		//..............................................
		//window.clear();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//ImGui::SFML::Render(window);
		//window.pushGLStates();

		// Use our shader
		glUseProgram(programID);


		glBindVertexArray(VertexArrayID);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);



		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 12*3); // 3 indices starting at 0 -> 1 triangle
		glBindVertexArray(0);

		glDisableVertexAttribArray(0);

		glUseProgram(0);


		window.pushGLStates();
		ImGui::SFML::Render(window);
		window.popGLStates();


		window.display();
	}

	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID);

	ImGui::SFML::Shutdown();

	window.close();
	return 0;
}
