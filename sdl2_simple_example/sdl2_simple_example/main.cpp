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

static void load_triangle(const Triangle& triangle) {

	float positions[] = {
		triangle.vertex.x, triangle.vertex.y, triangle.vertex.z, triangle.color.x, triangle.color.y, triangle.color.z, triangle.color.w,
		triangle.vertex1.x, triangle.vertex1.y, triangle.vertex1.z, triangle.color.x, triangle.color.y, triangle.color.z, triangle.color.w,
		triangle.vertex2.x, triangle.vertex2.y, triangle.vertex2.z, triangle.color.x, triangle.color.y, triangle.color.z, triangle.color.w
	};

	unsigned int vaID;
	glCreateVertexArrays(1, &vaID);
	glBindVertexArray(vaID);

	unsigned int vbID;
	glGenBuffers(1, &vbID);
	glBindBuffer(GL_ARRAY_BUFFER, vbID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

	glEnableVertexArrayAttrib(vbID, 0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, 0);

	glEnableVertexArrayAttrib(vbID, 1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (const void*)12);

	glBindVertexArray(vaID);
}

static void make_triangle(Json::Value& data) {

	Triangle triangle;

	triangle.vertex.x = data["position"][0].asFloat();
	triangle.vertex.y = data["position"][1].asFloat();
	triangle.vertex.z = data["position"][2].asFloat();
	triangle.vertex1.x = data["position"][3].asFloat();
	triangle.vertex1.y = data["position"][4].asFloat();
	triangle.vertex1.z = data["position"][5].asFloat();
	triangle.vertex2.x = data["position"][6].asFloat();
	triangle.vertex2.y = data["position"][7].asFloat();
	triangle.vertex2.z = data["position"][8].asFloat();
	triangle.color.x = data["color"][0].asFloat();
	triangle.color.y = data["color"][1].asFloat();
	triangle.color.z = data["color"][2].asFloat();
	triangle.color.w = data["color"][3].asFloat();

	load_triangle(triangle);

}

static void display_func(Json::Value &triangle) {
	u8vec4 color = { triangle["color"][0].asUInt(), triangle["color"][1].asUInt(),
		triangle["color"][2].asUInt(), triangle["color"][3].asUInt() };

	vec3 pos = { triangle["center"][0].asFloat(), triangle["center"][1].asFloat(), triangle["center"][2].asFloat() };

	draw_triangle(color, pos, triangle["size"].asFloat());
}

static bool processEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			return false;
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

	make_triangle(jsonData["triangle4"]);

	std::unique_ptr<Shader> m_Shader;
	m_Shader = std::make_unique<Shader>("Basic.shader");

	while (processEvents()) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		const auto t0 = hrclock::now();
		display_func(jsonData["triangle"]);
		display_func(jsonData["triangle1"]);
		display_func(jsonData["triangle2"]);
		display_func(jsonData["triangle3"]);

		//probant amb vertexbuffer, vertexarray i shader
		m_Shader->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 3);
		m_Shader->UnBind();

		window.swapBuffers();
		const auto t1 = hrclock::now();
		const auto dt = t1 - t0;
		if(dt<FRAME_DT) this_thread::sleep_for(FRAME_DT - dt);
	}

	return 0;
}