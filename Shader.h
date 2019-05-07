#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader
{
public:
	unsigned int shaderProgramID;

	Shader(const char* vertexPath, const char* fragmentPath);

	char* readShaderSource(const char* shaderFile);

	void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);

	void CompileShaders(const char* vertexPath, const char* fragmentPath);

	void use();
	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;

	void setMat4(const std::string &name, glm::mat4 value) const;
	void setVec3(const std::string &name, glm::vec3 value) const;

	GLuint getAttribLocation(const std::string &name);
	void setAttribVec3(GLuint &, unsigned int &) const;

};
#endif
