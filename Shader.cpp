#include "Shader.h"

char* Shader::readShaderSource(const char* shaderFile) {
	FILE* fp = fopen(shaderFile, "rb"); //!->Why does binary flag "RB" work and not "R"... wierd msvc thing?

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}

void Shader::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

void Shader::CompileShaders(const char* vertexPath, const char* fragmentPath)
{
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	AddShader(shaderProgramID, vertexPath, GL_VERTEX_SHADER);
	AddShader(shaderProgramID, fragmentPath, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };
	glLinkProgram(shaderProgramID);
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(shaderProgramID);
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}
}

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	CompileShaders(vertexPath, fragmentPath);
}

void Shader::use()
{
	glUseProgram(shaderProgramID);
}
void Shader::setBool(const std::string &name, bool value) const
{
	glUniform1i(glGetUniformLocation(shaderProgramID, name.c_str()), (int)value);
}
void Shader::setInt(const std::string &name, int value) const
{
	glUniform1i(glGetUniformLocation(shaderProgramID, name.c_str()), value);
}
void Shader::setFloat(const std::string &name, float value) const
{
	glUniform1fv(glGetUniformLocation(shaderProgramID, name.c_str()), 1, &value);
}

void Shader::setMat4(const std::string &name, glm::mat4 value) const
{
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

void Shader::setVec3(const std::string &name, glm::vec3 value) const
{
	glUniform3fv(glGetUniformLocation(shaderProgramID, name.c_str()), 1, &value[0]);
}

GLuint Shader::getAttribLocation(const std::string &name)
{
	return glGetAttribLocation(shaderProgramID, name.c_str());
}

void Shader::setAttribVec3(GLuint &AttricLocatin, unsigned int &VBO) const
{
	glEnableVertexAttribArray(AttricLocatin);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(AttricLocatin, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}