#pragma once

#include "based/log.h"

namespace based::core
{
	template <typename T>
	class SimpleAssetLibrary
	{
	public:
		SimpleAssetLibrary() = default;
		~SimpleAssetLibrary() = default;

		const std::unordered_map<std::string, T>& GetAll() const { return mAssets; }

		std::vector<std::string> GetKeys()
		{
			const auto vals = std::vector<std::string>();
			vals.reserve(mAssets.size());

			for (auto kv : mAssets)
			{
				vals.emplace_back(kv.first);
			}

			return vals;
		}

		void Load(const std::string& name, T asset, bool force = false)
		{
			std::string finalName = name;
			int count = 1;
			while (!force && Exists(finalName))
			{
				finalName = name + std::to_string(count);
				count++;
				//BASED_WARN("AssetLibrary::Load() - asset with same name: {}", finalName.c_str());
			}
			mAssets[finalName] = asset;
		}

		T Get(const std::string& name)
		{
			if (Exists(name))
			{
				return mAssets[name];
			}
			else
			{
				BASED_ERROR("AssetLibrary::Get() - Asset not found: {}", name.c_str());
				return NULL;
			}
		}

		void Delete(const std::string& name)
		{
			if (Exists(name))
			{
				auto it = mAssets.find(name);
				auto useCount = it->second.use_count();
				if (useCount > 1)
				{
					BASED_INFO("AssetLibrary::Delete() - remaining handles for asset {}: {}", name.c_str(), useCount);
				}
				mAssets.erase(it);
			}
		}

		void Clear()
		{
			for (auto [name, asset] : mAssets)
			{
				asset.reset();
			}

			mAssets.clear();
		}

		int Size() const
		{
			return static_cast<int>(mAssets.size());
		}

		bool Exists(const std::string& name)
		{
			return mAssets.find(name) != mAssets.end();
		}

		std::string GetSafeName(const std::string& name)
		{
			std::string finalName = name;
			int count = 1;
			while (Exists(finalName))
			{
				finalName = name + std::to_string(count);
				count++;
			}
			return finalName;
		}

	private:
		std::unordered_map<std::string, T> mAssets;
	};

	template <typename T>
	class AssetLibrary : public SimpleAssetLibrary<std::shared_ptr<T>>
	{
	public:
		AssetLibrary() : SimpleAssetLibrary<std::shared_ptr<T>>() {}
		~AssetLibrary() = default;
	};
}