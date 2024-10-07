#include "graphics/material.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "log.h"
#include "based/core/profiler.h"

namespace based::graphics
{
	Material::Material(const std::shared_ptr<Shader>& shader, const std::string& name)
		: mMaterialName(name), mShader(shader)
	{
		BASED_ASSERT(mShader, "Attempting to instantiate a material with a nullptr shader");
	}

	Material::Material(const Material& other)
	{
		mShader = std::make_shared<graphics::Shader>(*other.mShader);
		mTextures = other.mTextures;

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
		assetLibrary.Load(name, std::make_shared<Material>(shader, name));
		return assetLibrary.Get(name);
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
}