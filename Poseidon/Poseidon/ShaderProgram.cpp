#pragma once
#include "ShaderProgram.h"

ShaderProgram::ShaderProgram()
{
}

ShaderProgram::ShaderProgram(const char* computeFilePath)
{
	this->program_id = glCreateProgram();

	GLuint computeShader = createShader(computeFilePath, ShaderType::COMPUTE);

	glAttachShader(program_id, computeShader);
	glLinkProgram(program_id);

	int success;
	char infoLog[512];
	glGetProgramiv(this->program_id, GL_LINK_STATUS, &success);
	if (!success)
	{

		glGetProgramInfoLog(this->program_id, 512, NULL, infoLog);
		std::cout << "ERROR: Shader Linking failed \n" << infoLog << std::endl;
	}

	glDeleteShader(computeShader);
}

ShaderProgram::ShaderProgram(const char* vertexFilePath, const char* fragmentFilePath)
{
	this->program_id = glCreateProgram();

	GLuint vertexShader = createShader(vertexFilePath, ShaderType::VERTEX);
	GLuint fragmentShader = createShader(fragmentFilePath, ShaderType::FRAGMENT);

	glAttachShader(program_id, vertexShader);
	glAttachShader(program_id, fragmentShader);
	glLinkProgram(program_id);

	int success;
	char infoLog[512];
	glGetProgramiv(this->program_id, GL_LINK_STATUS, &success);
	if (!success)
	{

		glGetProgramInfoLog(this->program_id, 512, NULL, infoLog);
		std::cout << "ERROR: Shader Linking failed \n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

ShaderProgram::~ShaderProgram()
{
	
}

unsigned int ShaderProgram::getID()
{
	return this->program_id;
}


void ShaderProgram::dispatchCompute(int width, int height, int depth)
{
	glDispatchCompute(width, height, depth);
}

void ShaderProgram::bind()
{
	glUseProgram(this->program_id);
}

void ShaderProgram::unbind()
{
	glUseProgram(0);
}

GLuint ShaderProgram::createShader(const char* filePath, ShaderType type) {

	GLenum shaderType;
	switch (type)
	{
	case ShaderType::VERTEX:
		shaderType = GL_VERTEX_SHADER;
		break;
	case ShaderType::COMPUTE:
		shaderType = GL_COMPUTE_SHADER;
		break;
	case ShaderType::FRAGMENT:
		shaderType = GL_FRAGMENT_SHADER;
		break;
	default:
		break;
	}

	std::string absoluteFilePath = std::filesystem::absolute(filePath).u8string();
	std::ifstream filestream(absoluteFilePath, std::ios::in);

	std::string content;

	std::string line = "";
	while (!filestream.eof())
	{
		std::getline(filestream, line);
		content.append(line + "\n");
	}
	filestream.close();
	const char* shaderSource = content.c_str();

	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderSource, NULL);

	glCompileShader(shader);
	GLint succeded;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &succeded);

	if (succeded == GL_FALSE)
	{
		//logging
		GLint logSize;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
		GLchar* message = new char[logSize];
		glGetShaderInfoLog(shader, logSize, NULL, message);
		std::cout << "compile error in Shader program with id " << this->program_id << "...";
		std::cout << message << std::endl;
		delete[] message;
	}

	return shader;
}

/* 1.binds the shader program to the current context,
	2.calls dispatch compute with the given x y z dimensions (runs with attached shader program)
	3.unbinds the shader program, (depth has default value 1 if not given)  */
void ShaderProgram::compute(int width, int height, int depth ) {
	bind();
	dispatchCompute(width, height, depth);
	unbind();
};

unsigned int ShaderProgram::GetUniformLocation(const std::string& name)
{
	unsigned int location = glGetUniformLocation(this->program_id, name.c_str());
	if (location == -1)
		std::cout << "No active uniform variable with name " << name << " found." << std::endl;

	return location;
}

void ShaderProgram::SetUniform1i(const std::string& name, int value)
{
	glProgramUniform1i(this->program_id, GetUniformLocation(name), value);
}

void ShaderProgram::SetUniform1f(const std::string& name, float value)
{
	glProgramUniform1f(this->program_id, GetUniformLocation(name), value);
}

void ShaderProgram::SetUniform1fv(const std::string& name, glm::vec2 vector)
{
	glProgramUniform2f(this->getID(), GetUniformLocation(name), vector.x, vector.y);
}

/** pingpong val should be either 0 or 1, is_vertical should be 0 or 1 and controls whether it is horizontal or vertical fft
stage is the stage of the fft being computed */
void ShaderProgram::updateButterflyComputeUniforms(int pingpong_val, int is_vertical_fft,int stage) {
	SetUniform1i("pingpong_index", pingpong_val);
	SetUniform1i("direction", is_vertical_fft);
	SetUniform1i("stage", stage);
}
