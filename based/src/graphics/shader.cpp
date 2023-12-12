#include "graphics/shader.h"
#include "graphics/helpers.h"
#include "log.h"
#include "glad/glad.h"
#include "external/glm/gtc/type_ptr.hpp"

#include <fstream>
#include <sstream>

#include "app.h"
#include "engine.h"
#include "based/core/profiler.h"
#include "based/core/basedtime.h"

namespace based::graphics
{
	static ShaderGlobals mGlobals;
	static std::vector<unsigned int> mBufferIds;

	Shader::Shader(const std::string& vertex, const std::string& fragment)
		: mVertexShader(vertex)
		, mFragmentShader(fragment)
	{
		PROFILE_FUNCTION();
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

		unsigned int globalsIndex = glGetUniformBlockIndex(mProgramId, "Globals");
		if (globalsIndex != GL_INVALID_INDEX)
			glUniformBlockBinding(mProgramId, globalsIndex, 0); BASED_CHECK_GL_ERROR;

		glDeleteShader(vertexShaderId); BASED_CHECK_GL_ERROR;
		glDeleteShader(fragmentShaderId); BASED_CHECK_GL_ERROR;
	}

	Shader::Shader(const Shader& other) : Shader(other.mVertexShader, other.mFragmentShader)
	{
	}

	Shader::~Shader()
	{
		glUseProgram(0); BASED_CHECK_GL_ERROR;
		glDeleteProgram(mProgramId); BASED_CHECK_GL_ERROR;
	}

	void Shader::InitializeUniformBuffers()
	{
		unsigned int globalsUbo;
		glGenBuffers(1, &globalsUbo); BASED_CHECK_GL_ERROR;
		glBindBuffer(GL_UNIFORM_BUFFER, globalsUbo); BASED_CHECK_GL_ERROR;
		glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderGlobals), NULL, GL_STATIC_DRAW); BASED_CHECK_GL_ERROR;
		glBindBuffer(GL_UNIFORM_BUFFER, 0); BASED_CHECK_GL_ERROR;
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, globalsUbo); BASED_CHECK_GL_ERROR;
		mBufferIds.emplace_back(globalsUbo);
	}

	void Shader::UpdateUniformBuffers()
	{
		mGlobals.proj = Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()->GetProjectionMatrix();
		mGlobals.view = Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()->GetViewMatrix();
		mGlobals.eyePos = glm::vec4(Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()->GetTransform().Position, 1.f);
		mGlobals.eyeForward = glm::vec4(Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()->GetForward(), 1.f);
		mGlobals.time = based::core::Time::GetTime();

		glBindBuffer(GL_UNIFORM_BUFFER, mBufferIds[0]); BASED_CHECK_GL_ERROR;
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderGlobals), &mGlobals); BASED_CHECK_GL_ERROR;
		glBindBuffer(GL_UNIFORM_BUFFER, 0); BASED_CHECK_GL_ERROR;
	}

	Shader* Shader::LoadShader(const std::string& vsPath, const std::string& fsPath)
	{
		PROFILE_FUNCTION();
		std::ifstream vsFile(vsPath);
		std::ifstream fsFile(fsPath);

		BASED_ASSERT(vsFile && fsFile, "Could not load shaders! Make sure your path is correct!");

		if (vsFile && fsFile)
		{
			std::string temp = "";
			std::string vertexSource = "";
			while (std::getline(vsFile, temp))
			{
				vertexSource = vertexSource.append(temp).append("\n");
			}
			vsFile.close();

			std::string fragSource = "";
			while (std::getline(fsFile, temp))
			{
				fragSource = fragSource.append(temp).append("\n");
			}
			fsFile.close();

			std::set<std::string> alreadyIncluded = {};
			vertexSource = PreprocessShader(vertexSource, "#include ", "Assets/shaders/", alreadyIncluded);
			alreadyIncluded = {};
			fragSource = PreprocessShader(fragSource, "#include ", "Assets/shaders/", alreadyIncluded);

			return new Shader(vertexSource, fragSource);
		}
		
		return nullptr;
	}

	std::string Shader::PreprocessShader(const std::string source, const std::string includeIdentifier,
		const std::string includeSearchDir, std::set<std::string>& alreadyIncluded)
	{
		// Shader preprocessing code by Grayson Clark: https://github.com/FaultyPine/tiny_engine/blob/master/engine/src/render/shader.cpp
		static bool isRecursiveCall = false;
		std::string fullSourceCode = "";
		std::string lineBuffer;
		std::istringstream stream = std::istringstream(source);
		// TODO: custom strings
		while (std::getline(stream, lineBuffer)) {
			// if include is in this line
			if (lineBuffer.find(includeIdentifier) != lineBuffer.npos && lineBuffer.find("//") == lineBuffer.npos) {
				// Remove the include identifier, this will cause the path to remain
				lineBuffer.erase(0, includeIdentifier.size());
				lineBuffer = lineBuffer.erase(0, 1);
				lineBuffer.erase(lineBuffer.size() - 1, 1); // remove ""
				// The include path is relative to the current shader file path
				std::string path = includeSearchDir + lineBuffer;
				// if we haven't already included this in the compilation unit
				if (alreadyIncluded.count(path) == 0)
				{
					std::string nextFile;
					if (ReadEntireFile(path.c_str(), nextFile)) {
						alreadyIncluded.insert(path);
						// recursively process included file
						isRecursiveCall = true;
						std::string recursiveShaderSource = PreprocessShader(nextFile, includeIdentifier, 
							includeSearchDir, alreadyIncluded);
						fullSourceCode += recursiveShaderSource;
					}
					else {
						BASED_ERROR("Failed to open shader include: {}", path);
					}
				}

				// don't add the actual "#include blah.blah" line in the final source
				continue;
			}

			fullSourceCode += lineBuffer + '\n';
		}

		// null terminate the very end of the file
		if (!isRecursiveCall)
			fullSourceCode += '\0';

		return fullSourceCode;
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

	void Shader::UpdateShaderPointLighting(Shader* shader, glm::vec3 objectPos)
	{
		const entt::registry& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
		const auto lightsView = registry.view<scene::Enabled, scene::Transform, scene::PointLight>();
		std::vector<scene::PointLight> pointLights;

		for (const auto entity : lightsView)
		{
			scene::Transform trans = registry.get<scene::Transform>(entity);
			scene::PointLight light = registry.get<scene::PointLight>(entity);
			light.position = trans.Position;
			pointLights.emplace_back(light);
		}

		std::sort(pointLights.begin(), pointLights.end(),
			[objectPos](const scene::PointLight& lhs, const scene::PointLight& rhs)
			{
				return glm::distance(objectPos, lhs.position) < glm::distance(objectPos, rhs.position);
			}
		);

		for (int i = 0; i < pointLights.size() && i < 8; i++)
		{
			const scene::PointLight light = pointLights[i];
			shader->SetUniformFloat3("pointLights[" + std::to_string(i) + "].position", light.position);
			shader->SetUniformFloat("pointLights[" + std::to_string(i) + "].constant", light.constant);
			shader->SetUniformFloat("pointLights[" + std::to_string(i) + "].linear", light.linear);
			shader->SetUniformFloat("pointLights[" + std::to_string(i) + "].quadratic", light.quadratic);
			shader->SetUniformFloat("pointLights[" + std::to_string(i) + "].intensity", light.intensity);
			shader->SetUniformFloat3("pointLights[" + std::to_string(i) + "].color", light.color);
		}
	}

	void Shader::UpdateShaderDirectionalLighting(Shader* shader)
	{
		const entt::registry& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
		const auto lightsView = registry.view<scene::Enabled, scene::Transform, scene::DirectionalLight>();

		// Return if there are no directional lights
		if (lightsView.begin() == lightsView.end()) return;

		for (const auto entity : lightsView)
		{
			scene::Transform trans = registry.get<scene::Transform>(entity);
			scene::DirectionalLight light = registry.get<scene::DirectionalLight>(entity);
			light.direction = trans.Rotation;

			shader->SetUniformFloat3("directionalLight.direction", light.direction);
			shader->SetUniformFloat3("directionalLight.color", light.color);
			break;
		}
	}
}
