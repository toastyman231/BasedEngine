#pragma once

#include "based/core/uuid.h"
#include "based/core/serializer.h"
#include "based/scene/scene.h"
#include "based/core/assetlibrary.h"
#include "based/core/yamlformatter.h"

namespace based::animation
{
	struct TransitionRules;
	class AnimationStateMachine;
	class Animation;
}

namespace based::scene
{
	struct ScriptComponent;

	class SceneSerializer : public core::Serializer
	{
	public:
		SceneSerializer(const std::shared_ptr<Scene>& scene);

		void SerializeMaterial(YAML::Emitter& out, const std::shared_ptr<graphics::Material>& material);
		std::shared_ptr<graphics::Material> DeserializeMaterial(const std::string& filepath);

		void SerializeAnimation(YAML::Emitter& out, const std::shared_ptr<animation::Animation>& animation);
		std::shared_ptr<animation::Animation> DeserializeAnimation(const std::string& filepath);

		void SerializeStateMachine(YAML::Emitter& out, 
			const std::shared_ptr<animation::AnimationStateMachine> stateMachine);

		void Serialize(const std::string& filepath) override;
		void SerializeRuntime(const std::string& filepath) override;

		bool Deserialize(const std::string& filepath) override;
		bool DeserializeRuntime(const std::string& filepath) override;

		void SetProjectDirectory(const std::string& dir) { mProjectDirectory = dir; }

		template <typename T>
		static void SerializeScriptComponent(YAML::Emitter* out, scene::ScriptComponent* component)
		{
			BASED_TRACE("Serializing scriptable component");
			auto compCast = (T*)component;
			core::YAMLFormatter::Serialize(*out, entt::meta_any{ *compCast });
		}

		template <typename T>
		static void AddMetaComponent(scene::Entity* entity, scene::Scene* scene, entt::meta_any* component)
		{
			BASED_TRACE("Adding reflected meta component");
			entity->AddComponent<T>();

			scene->GetRegistry().patch<T>(entity->GetEntityHandle(),
				[component](auto& c) { c = component->cast<T>(); });
		}
	private:
		void SerializeEntity(YAML::Emitter& out, const std::shared_ptr<Entity>& entity);
		bool CreateDirectoryIfNotExists(const std::string& filepath);
		int CountFilesInDir(const std::string& filepath);
		void DeserializeEntity(YAML::detail::iterator_value entity);
		std::shared_ptr<graphics::Material> SearchForMaterialByUUID(core::UUID id);
		std::shared_ptr<graphics::Mesh> SearchForMeshByUUID(core::UUID id);
		std::shared_ptr<graphics::Model> SearchForModelByUUID(core::UUID id);

		struct TransitionStorageData
		{
			int source;
			int destination;
			animation::TransitionRules* rules;
			bool autoReset;

			TransitionStorageData(int s, int d, animation::TransitionRules* r, bool a)
				: source(s), destination(d), rules(r), autoReset(a) {}
		};

		std::string mProjectDirectory;
		std::shared_ptr<Scene> mScene;

		std::unordered_map<core::UUID, core::UUID> parentMap;
		std::unordered_map<core::UUID, std::shared_ptr<Entity>> mLoadedEntities;
		std::unordered_map<core::UUID, std::shared_ptr<graphics::Material>> mLoadedMaterials;
		std::unordered_map<core::UUID, std::shared_ptr<graphics::Mesh>> mLoadedMeshes;
		std::unordered_map<core::UUID, std::shared_ptr<graphics::Model>> mLoadedModels;
		std::unordered_map<core::UUID, std::shared_ptr<graphics::Texture>> mLoadedTextures;
		std::unordered_map<core::UUID, std::shared_ptr<animation::Animation>> mLoadedAnimations;

		std::vector<TransitionStorageData> mStoredTransitions;
	};
}
