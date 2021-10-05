#pragma once
#include <iostream>
#include <filesystem>
#include <sstream>
#include <fstream>
#include "Debug.h"
#include "OpenGLimports.h"

enum class ShaderType {
	VERTEX = 0, COMPUTE = 1, FRAGMENT = 2
};

#include "Debug.h"
#include "OpenGLimports.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

class ShaderProgram
{
public:
	ShaderProgram();
	ShaderProgram(const char* computeFilePath);
	ShaderProgram(const char* vertexFilePath, const char* fragmentFilePath);
	~ShaderProgram();

	unsigned int getID();


	void bind();
	void unbind();

	unsigned int GetUniformLocation(const std::string& name);
	void SetUniform1i(const std::string& name, int value);
	void SetUniform1f(const std::string& name, float value);
	void SetUniform1fv(const std::string& name, glm::vec2 vector);


	// all compute shaders
	void dispatchCompute(int width, int height, int depth);
	void compute(int width, int height, int depth = 1);


	// butterfly compute shader
	void updateButterflyComputeUniforms(int pingpong_val, int is_vertical_fft, int stage);


private:
	unsigned int program_id;
	GLuint createShader(const char* filePath, ShaderType type);

};