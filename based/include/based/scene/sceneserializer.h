#pragma once

#include "based/core/uuid.h"
#include "based/core/serializer.h"
#include "based/scene/scene.h"
#include "based/core/assetlibrary.h"

namespace based::scene
{
	class SceneSerializer : public core::Serializer
	{
	public:
		SceneSerializer(const std::shared_ptr<Scene>& scene);

		static void SerializeMaterial(YAML::Emitter& out, const std::shared_ptr<graphics::Material>& material);
		std::shared_ptr<graphics::Material> DeserializeMaterial(const std::string& filepath);

		void Serialize(const std::string& filepath) override;
		void SerializeRuntime(const std::string& filepath) override;

		bool Deserialize(const std::string& filepath) override;
		bool DeserializeRuntime(const std::string& filepath) override;
	private:
		std::shared_ptr<Scene> mScene;

		std::unordered_map<core::UUID, std::shared_ptr<Entity>> mLoadedEntities;
		std::unordered_map<core::UUID, std::shared_ptr<graphics::Texture>> mLoadedTextures;
	};
}