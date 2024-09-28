#include <GL/glew.h>
#include <chrono>
#include <thread>
#include <exception>
#include <glm/glm.hpp>
#include <SDL2/SDL_events.h>
#include "MyWindow.h"
#include <json/json.h>
#include <fstream>
#include <string>
#include <iostream>
#include "Shader.h"

using namespace std;

using hrclock = chrono::high_resolution_clock;
using u8vec4 = glm::u8vec4;
using ivec2 = glm::ivec2;
using vec3 = glm::dvec3;
using vec4 = glm::dvec4;

static const ivec2 WINDOW_SIZE(512, 512);
static const unsigned int FPS = 60;
static const auto FRAME_DT = 1.0s / FPS;

float angleX = 0.0f;
float angleY = 0.0f;

int lastMouseX = 0, lastMouseY = 0;
bool mouseDragging = false;

unsigned int vaID;
unsigned int vbID;
unsigned int ibID;

struct Triangle {
	vec3 vertex;
	vec3 vertex1;
	vec3 vertex2;
	vec4 color;
};

static void init_openGL() {
	glewInit();
	if (!GLEW_VERSION_3_0) throw exception("OpenGL 3.0 API is not available.");
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.5, 0.5, 0.5, 1.0);
}

static void draw_triangle(const u8vec4& color, const vec3& center, double size) {
	glColor4ub(color.r, color.g, color.b, color.a);
	glBegin(GL_TRIANGLES);
	glVertex3d(center.x, center.y + size, center.z);
	glVertex3d(center.x - size, center.y - size, center.z);
	glVertex3d(center.x + size, center.y - size, center.z);
	glEnd();
}

static void load_triangle() {

	float positions[] = { // index
		-0.5, -0.5, 0.0, // 0
		 0.5, -0.5, 0.0, // 1
		 0.5,  0.5, 0.0, // 2
		-0.5,  0.5, 0.0  // 3
	};

	unsigned int indices[] = {
		   0, 1, 2, 2, 3, 0
	};

	glCreateVertexArrays(1, &vaID);
	glBindVertexArray(vaID);

	glGenBuffers(1, &vbID);
	glBindBuffer(GL_ARRAY_BUFFER, vbID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

	glEnableVertexArrayAttrib(vbID, 0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	/*glEnableVertexArrayAttrib(vbID, 1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (const void*)12);*/

	glGenBuffers(1, &ibID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);
}

static void display_func(Json::Value &triangle) {

	//camera
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, static_cast<double>(WINDOW_SIZE.x / WINDOW_SIZE.y), 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 2.0, 10.0, 0.0, 2.0, 0.0, 0.0, 1.0, 0.0);

	glRotatef(angleX, 1.0f, 0.0f, 0.0f);
	glRotatef(angleY, 0.0f, 1.0f, 0.0f);

	vec3 a(-1, 0, 0);
	vec3 b(1, 0, 0);
	vec3 c(1, 2, 0);
	vec3 d(-1, 2, 0);

	//front
	glBegin(GL_TRIANGLE_FAN);
	glColor4ub(255, 0, 0, 255);
	glVertex3d(a.x, a.y, a.z);
	glVertex3d(b.x, b.y, b.z);
	glVertex3d(c.x, c.y, c.z);
	glVertex3d(d.x, d.y, d.z);
	glEnd();

	//left
	glBegin(GL_TRIANGLE_FAN);
	glColor4ub(0, 255, 0, 255);
	glVertex3d(d.x, d.y, d.z);
	glVertex3d(d.x, d.y, d.z - 2);
	glVertex3d(a.x, a.y, a.z - 2);
	glVertex3d(a.x, a.y, a.z);
	glEnd();

	//down
	glBegin(GL_TRIANGLE_FAN);
	glColor4ub(0, 0, 255, 255);
	glVertex3d(b.x, b.y, b.z - 2);
	glVertex3d(b.x, b.y, b.z);
	glVertex3d(a.x, a.y, a.z);
	glVertex3d(a.x, a.y, a.z - 2);
	glEnd();

	//right
	glBegin(GL_TRIANGLE_FAN);
	glColor4ub(255, 255, 0, 255);
	glVertex3d(c.x, c.y, c.z);
	glVertex3d(c.x, c.y, c.z - 2);
	glVertex3d(b.x, b.y, b.z - 2);
	glVertex3d(b.x, b.y, b.z);
	glEnd();

	//up
	glBegin(GL_TRIANGLE_FAN);
	glColor4ub(255, 0, 255, 255);
	glVertex3d(d.x, d.y, d.z);
	glVertex3d(c.x, c.y, c.z);
	glVertex3d(c.x, c.y, c.z - 2);
	glVertex3d(d.x, d.y, d.z - 2);
	glEnd();

	//behind
	glBegin(GL_TRIANGLE_FAN);
	glColor4ub(0, 255, 255, 255);
	glVertex3d(a.x, a.y, a.z - 2);
	glVertex3d(b.x, b.y, b.z - 2);
	glVertex3d(c.x, c.y, c.z - 2);
	glVertex3d(d.x, d.y, d.z - 2);
	glEnd();
}

static bool processEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		cout << event.type << endl;
		switch (event.type) {
		case SDL_QUIT:
			return false;
		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_LEFT) {
				mouseDragging = true;
				lastMouseX = event.button.x;
				lastMouseY = event.button.y;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_LEFT) {
				mouseDragging = false;
			}
			break;
		case SDL_MOUSEMOTION:
			if (mouseDragging) {
				int deltaX = event.motion.x - lastMouseX;
				int deltaY = event.motion.y - lastMouseY;

				angleX += deltaY * 0.5f;
				angleY += deltaX * 0.5f;

				lastMouseX = event.motion.x;
				lastMouseY = event.motion.y;
			}
			break;
		}
	}
	return true;
}

int main(int argc, char** argv) {
	MyWindow window("SDL2 Simple Example", WINDOW_SIZE.x, WINDOW_SIZE.y);

	ifstream file("data.json", ifstream::binary);

	Json::Value jsonData;
	Json::CharReaderBuilder readerBuilder;
	string errs;

	Json::parseFromStream(readerBuilder, file, &jsonData, &errs);	

	init_openGL();

	load_triangle();

	std::unique_ptr<Shader> m_Shader;
	m_Shader = std::make_unique<Shader>("Basic.shader");

	SDL_Event event;

	while (processEvents()) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		const auto t0 = hrclock::now();
		display_func(jsonData["triangle"]);

		//probant amb vertexbuffer, vertexarray i shader
		m_Shader->Bind();
		//m_Shader->SetUniform1f("u_Angle", );
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		m_Shader->UnBind();

		window.swapBuffers();
		const auto t1 = hrclock::now();
		const auto dt = t1 - t0;
		if(dt<FRAME_DT) this_thread::sleep_for(FRAME_DT - dt);
	}

	return 0;
}