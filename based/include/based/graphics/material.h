#pragma once

#include "based/core/uuid.h"
#include "based/core/assetlibrary.h"

#define DEFAULT_MAT_LIB based::graphics::DefaultLibraries::GetMaterialLibrary()

namespace based::graphics
{
	class Shader;
	class Texture;
	class Material
	{
	public:
		Material(const std::shared_ptr<Shader>& shader, const std::string& name = "New Material");
		Material(const std::shared_ptr<Shader>& shader, core::UUID uuid, const std::string& name = "New Material");
		Material(const Material& other);
		~Material();

		static std::shared_ptr<Material> CreateMaterial(const std::shared_ptr<Shader>& shader,
			core::AssetLibrary<Material>& assetLibrary, const std::string& name = "New Material");
		static std::shared_ptr<Material> LoadMaterialFromFile(const std::string& filepath,
			core::AssetLibrary<Material>& assetLibrary);

		inline std::weak_ptr<Shader> GetShader() const { return mShader; }
		inline std::vector<std::shared_ptr<Texture>>& GetTextures() { return mTextures; }
		inline core::UUID GetUUID() const { return mUUID; }

		void SetShader(std::shared_ptr<Shader> shader);
		void AddTexture(std::shared_ptr<Texture> texture, std::string location = "");
		void RemoveTexture(std::string location);
		void UpdateShaderUniforms() const;

		std::string GetTextureLocationByIndex(int index) const;

		std::unordered_map<std::string, int> GetTextureOrder() const { return mTextureOrder; }

		bool IsFileMaterial() const { return !mMaterialSource.empty(); }
		std::string GetMaterialSource() const { return mMaterialSource; }

		std::string mMaterialName;

#define GETUNIFORMVALUE(mapName, defaultReturn) \
	const auto& it = mapName.find(name);\
	if (it != (mapName).end())\
	{\
		return it->second;\
	}\
	return defaultReturn;

		template<typename T>
		inline T GetUniformValue(const std::string& name) const
		{
			if constexpr (std::is_same<T, int>()) { GETUNIFORMVALUE(mUniformInts, 0) }
			else if constexpr (std::is_same<T, float>()) { GETUNIFORMVALUE(mUniformFloats, 0) }
			else if constexpr (std::is_same<T, glm::vec2>()) { GETUNIFORMVALUE(mUniformFloat2s, glm::vec2(0.f)) }
			else if constexpr (std::is_same<T, glm::vec3>()) { GETUNIFORMVALUE(mUniformFloat3s, glm::vec3(0.f)) }
			else if constexpr (std::is_same<T, glm::vec4>()) { GETUNIFORMVALUE(mUniformFloat4s, glm::vec4(0.f)) }
			else if constexpr (std::is_same<T, glm::mat3>()) { GETUNIFORMVALUE(mUniformMat3s, glm::mat3(1.f)) }
			else if constexpr (std::is_same<T, glm::mat4>()) { GETUNIFORMVALUE(mUniformMat4s, glm::mat4(1.f)) }
			else
			{
				static_assert(false, "Unsupported data type in Material::GetUniformValue()");
			}
		}

		template<typename T>
		inline T GetUniformValue(const std::string& name, const T defaultVal) const
		{
			if constexpr (std::is_same<T, int>()) { GETUNIFORMVALUE(mUniformInts, defaultVal) }
			else if constexpr (std::is_same<T, float>()) { GETUNIFORMVALUE(mUniformFloats, defaultVal) }
			else if constexpr (std::is_same<T, glm::vec2>()) { GETUNIFORMVALUE(mUniformFloat2s, defaultVal) }
			else if constexpr (std::is_same<T, glm::vec3>()) { GETUNIFORMVALUE(mUniformFloat3s, defaultVal) }
			else if constexpr (std::is_same<T, glm::vec4>()) { GETUNIFORMVALUE(mUniformFloat4s, defaultVal) }
			else if constexpr (std::is_same<T, glm::mat3>()) { GETUNIFORMVALUE(mUniformMat3s, defaultVal) }
			else if constexpr (std::is_same<T, glm::mat4>()) { GETUNIFORMVALUE(mUniformMat4s, defaultVal) }
			else
			{
				static_assert(false, "Unsupported data type in Material::GetUniformValue()");
			}
		}
#undef GETUNIFORMVALUE

		template<typename T>
		inline void SetUniformValue(const std::string& name, const T& val)
		{
			if constexpr (std::is_same<T, int>()) { mUniformInts[name] = val; }
			else if constexpr (std::is_same<T, float>()) { mUniformFloats[name] = val; }
			else if constexpr (std::is_same<T, glm::vec2>()) { mUniformFloat2s[name] = val; }
			else if constexpr (std::is_same<T, glm::vec3>()) { mUniformFloat3s[name] = val; }
			else if constexpr (std::is_same<T, glm::vec4>()) { mUniformFloat4s[name] = val; }
			else if constexpr (std::is_same<T, glm::mat3>()) { mUniformMat3s[name] = val; }
			else if constexpr (std::is_same<T, glm::mat4>()) { mUniformMat4s[name] = val; }
			else
			{
				static_assert(false, "Unsupported data type in Material::SetUniformValue()");
			}
		}

		std::unordered_map<std::string, int> GetUniformInts() const { return mUniformInts; }
		std::unordered_map<std::string, float> GetUniformFloats() const { return mUniformFloats; }
		std::unordered_map<std::string, glm::vec2> GetUniformFloat2s() const { return mUniformFloat2s; }
		std::unordered_map<std::string, glm::vec3> GetUniformFloat3s() const { return mUniformFloat3s; }
		std::unordered_map<std::string, glm::vec4> GetUniformFloat4s() const { return mUniformFloat4s; }
		std::unordered_map<std::string, glm::mat3> GetUniformMat3s() const { return mUniformMat3s; }
		std::unordered_map<std::string, glm::mat4> GetUniformMat4s() const { return mUniformMat4s; }
	private:
		core::UUID mUUID;
		std::shared_ptr<Shader> mShader;
		std::vector<std::shared_ptr<Texture>> mTextures;
		std::unordered_map<std::string, int> mTextureOrder;

		std::string mMaterialSource;

		// Data
		std::unordered_map<std::string, int> mUniformInts;
		std::unordered_map<std::string, float> mUniformFloats;
		std::unordered_map<std::string, glm::vec2> mUniformFloat2s;
		std::unordered_map<std::string, glm::vec3> mUniformFloat3s;
		std::unordered_map<std::string, glm::vec4> mUniformFloat4s;
		std::unordered_map<std::string, glm::mat3> mUniformMat3s;
		std::unordered_map<std::string, glm::mat4> mUniformMat4s;

		friend class SceneSerializer;
	};
}