#include "graphics/shader.h"
#include "graphics/helpers.h"
#include "log.h"
#include "glad/glad.h"
#include "external/glm/gtc/type_ptr.hpp"

namespace based::graphics
{
	Shader::Shader(const std::string& vertex, const std::string& fragment)
	{
		mProgramId = glCreateProgram(); BASED_CHECK_GL_ERROR;

		int status = GL_FALSE;
		char errorLog[512];

		uint32_t vertexShaderId = glCreateShader(GL_VERTEX_SHADER); BASED_CHECK_GL_ERROR;
		{
			const GLchar* glSource = vertex.c_str();
			glShaderSource(vertexShaderId, 1, &glSource, NULL); BASED_CHECK_GL_ERROR;
			glCompileShader(vertexShaderId); BASED_CHECK_GL_ERROR;
			glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &status); BASED_CHECK_GL_ERROR;
			if (status != GL_TRUE)
			{
				glGetShaderInfoLog(vertexShaderId, sizeof(errorLog), NULL, errorLog); BASED_CHECK_GL_ERROR;
				BASED_ERROR("Vertex Shader compilation error: {}", errorLog);
				glDeleteShader(vertexShaderId); BASED_CHECK_GL_ERROR;
			}
			else
			{
				glAttachShader(mProgramId, vertexShaderId); BASED_CHECK_GL_ERROR;
			} 
		}

		uint32_t fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER); BASED_CHECK_GL_ERROR;
		if (status == GL_TRUE)
		{
			const GLchar* glSource = fragment.c_str();
			glShaderSource(fragmentShaderId, 1, &glSource, NULL); BASED_CHECK_GL_ERROR;
			glCompileShader(fragmentShaderId); BASED_CHECK_GL_ERROR;
			glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &status); BASED_CHECK_GL_ERROR;
			if (status != GL_TRUE)
			{
				glGetShaderInfoLog(fragmentShaderId, sizeof(errorLog), NULL, errorLog); BASED_CHECK_GL_ERROR;
				BASED_ERROR("Fragment Shader compilation error: {}", errorLog);
			}
			else
			{
				glAttachShader(mProgramId, fragmentShaderId); BASED_CHECK_GL_ERROR;
			}
		}

		BASED_ASSERT(status == GL_TRUE, "Error compiling shader");
		if (status == GL_TRUE)
		{
			glLinkProgram(mProgramId); BASED_CHECK_GL_ERROR;
			glValidateProgram(mProgramId); BASED_CHECK_GL_ERROR;
			glGetProgramiv(mProgramId, GL_LINK_STATUS, &status); BASED_CHECK_GL_ERROR;
			if (status != GL_TRUE)
			{
				glGetProgramInfoLog(mProgramId, sizeof(errorLog), NULL, errorLog); BASED_CHECK_GL_ERROR;
				BASED_ERROR("Shader link error: {}", errorLog); 
				glDeleteProgram(mProgramId); BASED_CHECK_GL_ERROR;
				mProgramId = -1;
			}
		}

		glDeleteShader(vertexShaderId); BASED_CHECK_GL_ERROR;
		glDeleteShader(fragmentShaderId); BASED_CHECK_GL_ERROR;
	}

	Shader::~Shader()
	{
		glUseProgram(0); BASED_CHECK_GL_ERROR;
		glDeleteProgram(mProgramId); BASED_CHECK_GL_ERROR;
	}

	void Shader::Bind()
	{
		glUseProgram(mProgramId); BASED_CHECK_GL_ERROR;
	} 

	void Shader::Unbind()
	{
		glUseProgram(0); BASED_CHECK_GL_ERROR;
	}

	void Shader::SetUniformInt(const std::string& name, int val)
	{
		glUseProgram(mProgramId); BASED_CHECK_GL_ERROR;
		glUniform1i(GetUniformLocation(name), val); BASED_CHECK_GL_ERROR;
	}

	void Shader::SetUniformFloat(const std::string& name, float val)
	{
		glUseProgram(mProgramId); BASED_CHECK_GL_ERROR;
		glUniform1f(GetUniformLocation(name), val); BASED_CHECK_GL_ERROR;
	}

	void Shader::SetUniformFloat2(const std::string& name, float val1, float val2)
	{
		glUseProgram(mProgramId); BASED_CHECK_GL_ERROR;
		glUniform2f(GetUniformLocation(name), val1, val2); BASED_CHECK_GL_ERROR;
	}

	void Shader::SetUniformFloat2(const std::string& name, const glm::vec2& val)
	{
		SetUniformFloat2(name, val.x, val.y);
	}

	void Shader::SetUniformFloat3(const std::string& name, float val1, float val2, float val3)
	{
		glUseProgram(mProgramId); BASED_CHECK_GL_ERROR;
		glUniform3f(GetUniformLocation(name), val1, val2, val3); BASED_CHECK_GL_ERROR;
	}

	void Shader::SetUniformFloat3(const std::string& name, const glm::vec3& val)
	{
		SetUniformFloat3(name, val.x, val.y, val.z);
	}

	void Shader::SetUniformFloat4(const std::string& name, float val1, float val2, float val3, float val4)
	{
		glUseProgram(mProgramId); BASED_CHECK_GL_ERROR;
		glUniform4f(GetUniformLocation(name), val1, val2, val3, val4); BASED_CHECK_GL_ERROR;
	}

	void Shader::SetUniformFloat4(const std::string& name, const glm::vec4& val)
	{
		SetUniformFloat4(name, val.x, val.y, val.z, val.w);
	}

	void Shader::SetUniformMat3(const std::string& name, const glm::mat3& mat)
	{
		glUseProgram(mProgramId); BASED_CHECK_GL_ERROR;
		glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat)); BASED_CHECK_GL_ERROR;
	}

	void Shader::SetUniformMat4(const std::string& name, const glm::mat4& mat)
	{
		glUseProgram(mProgramId); BASED_CHECK_GL_ERROR;
		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat)); BASED_CHECK_GL_ERROR;
	}

	int Shader::GetUniformLocation(const std::string& name)
	{
		auto it = mUniformLocations.find(name);
		if (it == mUniformLocations.end())
		{
			mUniformLocations[name] = glGetUniformLocation(mProgramId, name.c_str()); BASED_CHECK_GL_ERROR;
		}

		return mUniformLocations[name];
	}
}