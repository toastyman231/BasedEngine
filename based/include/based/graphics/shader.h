#pragma once

#define LOAD_SHADER(vs, fs) std::shared_ptr<based::graphics::Shader>(based::graphics::Shader::LoadShader(vs, fs))
#define LOAD_COMPUTE_SHADER(source) std::shared_ptr<based::graphics::ComputeShader>(based::graphics::ComputeShader::LoadShader(source))

namespace based::graphics
{
	class Texture;

	struct ShaderGlobals
	{
		glm::mat4 proj;
		glm::mat4 view;
		glm::vec4 eyePos;
		glm::vec4 eyeForward;
		float time;
		int32_t renderMode;
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

		static ShaderGlobals GetShaderGlobals();
	private:
		uint32_t mProgramId;
		std::string mVertexShader;
		std::string mFragmentShader;

		std::unordered_map<std::string, int> mUniformLocations;
	};

	class ComputeShader
	{
	public:
		ComputeShader(const std::string& source);
		~ComputeShader();

		static ComputeShader* LoadShader(const std::string& source);

		inline const std::string& GetShaderSource() const { return mShaderSource; }

		void Bind() const;
		void Unbind();

		void AddTexture(std::shared_ptr<Texture> texture, std::string location = "");
		void RemoveTexture(std::string location);

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

		void SetWorkGroupSize(glm::vec<3, glm::uint> size) { mWorkGroups = size; }
		glm::vec<3, glm::uint> GetWorkGroupSize() const { return mWorkGroups; }

		std::vector<std::shared_ptr<Texture>> GetTextures() const { return mTextures; }
	private:
		uint32_t mProgramId;
		std::string mShaderSource;
		glm::vec<3, glm::uint> mWorkGroups;

		std::unordered_map<std::string, int> mUniformLocations;
		std::vector<std::shared_ptr<Texture>> mTextures;
		std::unordered_map<std::string, int> mTextureOrder;
	};
}