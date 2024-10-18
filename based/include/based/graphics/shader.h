#pragma once

#define LOAD_SHADER(vs, fs) std::shared_ptr<graphics::Shader>(graphics::Shader::LoadShader(vs, fs))

namespace based::graphics
{
	struct ShaderGlobals
	{
		glm::mat4 proj;
		glm::mat4 view;
		glm::vec4 eyePos;
		glm::vec4 eyeForward;
		float time;
	};
	class Shader
	{
	public:
		Shader(const std::string& vertex, const std::string& fragment);
		Shader(const Shader& other);
		~Shader();

		static void InitializeUniformBuffers();
		static void UpdateUniformBuffers();
		static Shader* LoadShader(const std::string& vsPath, const std::string& fsPath);
		static std::string PreprocessShader(const std::string source, const std::string includeIdentifier,
			const std::string includeSearchDir, std::set<std::string>& alreadyIncluded);

		inline const std::string& GetVertexShaderSource() const { return mVertexShader; }
		inline const std::string& GetFragmentShaderSource() const { return mFragmentShader; }

		void Bind();
		void Unbind();

		void SetUniformInt(const std::string& name, int val);
		void SetUniformFloat(const std::string& name, float val);
		void SetUniformFloat2(const std::string& name, float val1, float val2);
		void SetUniformFloat2(const std::string& name, const glm::vec2& val);
		void SetUniformFloat3(const std::string& name, float val1, float val2, float val3);
		void SetUniformFloat3(const std::string& name, const glm::vec3& val);
		void SetUniformFloat4(const std::string& name, float val1, float val2, float val3, float val4);
		void SetUniformFloat4(const std::string& name, const glm::vec4& val);
		void SetUniformMat3(const std::string& name, const glm::mat3& mat);
		void SetUniformMat4(const std::string& name, const glm::mat4& mat);

		int GetUniformLocation(const std::string& name);
		std::unordered_map<std::string, float> GetUniformFloats() const;
		std::unordered_map<std::string, int> GetUniformSamplers() const;

		static void UpdateShaderPointLighting(std::weak_ptr<Shader> shader, glm::vec3 objectPos);
		static void UpdateShaderDirectionalLighting(std::weak_ptr<Shader> shader);
	private:
		uint32_t mProgramId;
		std::string mVertexShader;
		std::string mFragmentShader;

		std::unordered_map<std::string, int> mUniformLocations;
	};
}