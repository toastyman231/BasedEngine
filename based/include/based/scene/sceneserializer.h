#pragma once

#include "based/core/serializer.h"
#include "based/scene/scene.h"
#include "based/core/assetlibrary.h"

namespace based::scene
{
	class SceneSerializer : public core::Serializer
	{
	public:
		SceneSerializer(const std::shared_ptr<Scene>& scene);

		void Serialize(const std::string& filepath) override;
		void SerializeRuntime(const std::string& filepath) override;

		bool Deserialize(const std::string& filepath) override;
		bool DeserializeRuntime(const std::string& filepath) override;

		core::AssetLibrary<Entity>& GetEntityStorage() { return mEntityStorage; }
	private:
		std::shared_ptr<Scene> mScene;
		core::AssetLibrary<Entity> mEntityStorage;
	};
}