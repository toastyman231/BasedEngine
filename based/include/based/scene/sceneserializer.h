#pragma once

#include "based/core/uuid.h"
#include "based/core/serializer.h"
#include "based/scene/scene.h"
#include "based/core/assetlibrary.h"

namespace based::animation
{
	class Animation;
}

namespace based::scene
{
	class SceneSerializer : public core::Serializer
	{
	public:
		SceneSerializer(const std::shared_ptr<Scene>& scene);

		void SerializeMaterial(YAML::Emitter& out, const std::shared_ptr<graphics::Material>& material);
		std::shared_ptr<graphics::Material> DeserializeMaterial(const std::string& filepath);

		void SerializeAnimation(YAML::Emitter& out, const std::shared_ptr<animation::Animation>& animation);
		std::shared_ptr<animation::Animation> DeserializeAnimation(const std::string& filepath);

		void Serialize(const std::string& filepath) override;
		void SerializeRuntime(const std::string& filepath) override;

		bool Deserialize(const std::string& filepath) override;
		bool DeserializeRuntime(const std::string& filepath) override;
	private:
		void SerializeEntity(YAML::Emitter& out, const std::shared_ptr<Entity>& entity);
		bool CreateDirectoryIfNotExists(const std::string& filepath);
		int CountFilesInDir(const std::string& filepath);
		void DeserializeEntity(YAML::detail::iterator_value entity);

		std::shared_ptr<Scene> mScene;

		std::unordered_map<core::UUID, core::UUID> parentMap;
		std::unordered_map<core::UUID, std::shared_ptr<Entity>> mLoadedEntities;
		std::unordered_map<core::UUID, std::shared_ptr<graphics::Material>> mLoadedMaterials;
		std::unordered_map<core::UUID, std::shared_ptr<graphics::Mesh>> mLoadedMeshes;
		std::unordered_map<core::UUID, std::shared_ptr<graphics::Model>> mLoadedModels;
		std::unordered_map<core::UUID, std::shared_ptr<graphics::Texture>> mLoadedTextures;
		std::unordered_map<core::UUID, std::shared_ptr<animation::Animation>> mLoadedAnimations;
	};
}
