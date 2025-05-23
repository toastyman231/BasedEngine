#include "pch.h"
#include "graphics/material.h"

#include "app.h"
#include "engine.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "scene/sceneserializer.h"

namespace based::graphics
{
	Material::Material(const std::shared_ptr<Shader>& shader, const std::string& name)
		: mMaterialName(name), mShader(shader), mUUID(core::UUID())
	{
		BASED_ASSERT(mShader, "Attempting to instantiate a material with a nullptr shader");
	}

	Material::Material(const std::shared_ptr<Shader>& shader, core::UUID uuid, const std::string& name)
		: Material(shader, name)
	{
		mUUID = uuid;
	}

	Material::Material(const Material& other)
	{
		mShader = std::make_shared<graphics::Shader>(*other.mShader);
		mTextures = other.mTextures;
		mUUID = other.GetUUID();

		// Data
		mUniformInts = other.mUniformInts;
		mUniformFloats = other.mUniformFloats;
		mUniformFloat2s = other.mUniformFloat2s;
		mUniformFloat3s = other.mUniformFloat3s;
		mUniformFloat4s = other.mUniformFloat4s;
		mUniformMat3s = other.mUniformMat3s;
		mUniformMat4s = other.mUniformMat4s;
		mTextureOrder = other.mTextureOrder;
	}

	Material::~Material()
	{
		BASED_TRACE("Deleting {}", mMaterialName);
		BASED_TRACE("Ref count to shader: {}", mShader.use_count());
	}

	std::shared_ptr<Material> Material::CreateMaterial(const std::shared_ptr<Shader>& shader,
		core::AssetLibrary<Material>& assetLibrary, const std::string& name)
	{
		auto asset = std::make_shared<Material>(shader, name);
		assetLibrary.Load(name, asset);
		return assetLibrary.Get(name);
	}

	std::shared_ptr<Material> Material::LoadMaterialFromFile(const std::string& filepath,
		core::AssetLibrary<Material>& assetLibrary, const std::string& filePrefix, const std::string& saveAs)
	{
		PROFILE_FUNCTION();
		scene::SceneSerializer serializer(Engine::Instance().GetApp().GetCurrentScene());
		auto material = serializer.DeserializeMaterial(filePrefix + filepath);
		BASED_ASSERT(material != nullptr, "Material is null!");
		material->mMaterialSource = filepath;
		return material;
	}

	std::shared_ptr<Material> Material::LoadMaterialWithUUID(const std::string& filepath, 
		core::UUID id, const std::string& filePrefix, bool absolute)
	{
		scene::SceneSerializer serializer(Engine::Instance().GetApp().GetCurrentScene());
		if (absolute) serializer.SetProjectDirectory(filePrefix);
		auto material = serializer.DeserializeMaterial(absolute ? filepath : filePrefix + filepath);
		BASED_ASSERT(material != nullptr, "Material is null!");
		material->mUUID = id;
		material->mMaterialSource = filepath;
		return material;
	}

	std::shared_ptr<Material> Material::LoadMaterialFromFileAbsolute(const std::string& filepath,
	                                                                 const std::string& filePrefix, const std::string& saveAs)
	{
		scene::SceneSerializer serializer(Engine::Instance().GetApp().GetCurrentScene());
		serializer.SetProjectDirectory(filePrefix);
		auto material = serializer.DeserializeMaterial(filepath);
		BASED_ASSERT(material != nullptr, "Material is null!");
		material->mMaterialSource = filepath;
		return material;
	}

	std::shared_ptr<Material> Material::LoadFileMaterialWithoutSaving(const std::string& filepath)
	{
		scene::SceneSerializer serializer(Engine::Instance().GetApp().GetCurrentScene());
		auto material = serializer.DeserializeMaterial(filepath);
		BASED_ASSERT(material != nullptr, "Material is null!");
		material->mMaterialSource = filepath;
		return material;
	}

	void Material::SetShader(std::shared_ptr<Shader> shader)
	{
		BASED_ASSERT(shader, "Attempting to set a nullptr shader");
		if (shader)
		{
			mShader = shader;
		}
	}

	void Material::AddTexture(std::shared_ptr<Texture> texture, std::string location)
	{
		mTextures.emplace_back(texture);
		if (!location.empty())
		{
			const int index = static_cast<int>(mTextures.size() - 1);
			SetUniformValue(location, index);
			mTextureOrder[location] = index;
		}
	}

	void Material::RemoveTexture(std::string location)
	{
		if (mTextureOrder.find(location) == mTextureOrder.end()) return;

		const int index = mTextureOrder[location];
		mTextures.erase(mTextures.begin() + index);
		mTextureOrder.erase(location);
		SetUniformValue(location, 0);
	}

	void Material::UpdateShaderUniforms() const
	{
		PROFILE_FUNCTION();
		if (mShader)
		{
			for (const auto& it : mUniformInts)
			{
				mShader->SetUniformInt(it.first, it.second);
			}
			for (const auto& it : mUniformFloats)
			{
				mShader->SetUniformFloat(it.first, it.second);
			}
			for (const auto& it : mUniformFloat2s)
			{
				mShader->SetUniformFloat2(it.first, it.second);
			}
			for (const auto& it : mUniformFloat3s)
			{
				mShader->SetUniformFloat3(it.first, it.second);
			}
			for (const auto& it : mUniformFloat4s)
			{
				mShader->SetUniformFloat4(it.first, it.second);
			}
			for (const auto& it : mUniformMat3s)
			{
				mShader->SetUniformMat3(it.first, it.second);
			}
			for (const auto& it : mUniformMat4s)
			{
				mShader->SetUniformMat4(it.first, it.second);
			}
		}
	}

	std::string Material::GetTextureLocationByIndex(int index) const
	{
		if (index < 0 || index > mTextures.size()) return "NONE";

		for (auto& it : mTextureOrder)
			if (it.second == index)
				return it.first;

		return "NONE";
	}
}
