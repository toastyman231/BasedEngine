#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "../log.h"

namespace based::core
{
	template <typename T>
	class AssetLibrary
	{
	public:
		AssetLibrary() = default;
		~AssetLibrary() = default;

		const std::unordered_map<std::string, std::shared_ptr<T>>& GetAll() const { return mAssets; }
		const std::vector<std::string>* GetAllFlat()
		{
			auto* vals = new std::vector<std::string>();
			vals->reserve(mAssets.size());

			for (auto kv : mAssets)
			{
				vals->push_back(kv.first);
			}

			return vals;
		}

		void Load(const std::string& name, std::shared_ptr<T> asset)
		{
			if (Exists(name))
			{
				BASED_WARN("AssetLibrary::Load() - overwriting asset with same name: {}", name.c_str());
			}
			mAssets[name] = asset;
		}

		std::shared_ptr<T> Get(const std::string& name)
		{
			if (Exists(name))
			{
				return mAssets[name];
			}
			else
			{
				BASED_ERROR("AssetLibrary::Get() - Asset not found: {}", name.c_str());
				return nullptr;
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
			mAssets.clear();
		}

		bool Exists(const std::string& name)
		{
			return mAssets.find(name) != mAssets.end();
		}
	private:
		std::unordered_map<std::string, std::shared_ptr<T>> mAssets;
	};
}