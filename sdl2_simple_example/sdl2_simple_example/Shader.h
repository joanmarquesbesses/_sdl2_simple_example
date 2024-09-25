#pragma once

#include <string>
#include <unordered_map>

#include "glm/glm.hpp"

using namespace std;

struct ShaderProgramSource {
	string VertexSource;
	string FragmentSource;
};


class Shader {
private:
	string m_FilePath;
	unsigned int m_RendererID;
	unordered_map<string, int> m_UniformLocationCache;

public:
	Shader(const string& filepath);
	~Shader();

	void Bind() const;
	void UnBind() const;

private:
	ShaderProgramSource ParseShader(const std::string& filepath);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::string& VertexShader, const std::string& FragmentShader);
};
