#pragma once
#include "based/scene/entity.h"

namespace editor
{
	class EngineOperations
	{
	public:
		bool EditorSetEntityParent(std::shared_ptr<based::scene::Entity> parent,
			std::shared_ptr<based::scene::Entity> child);
		bool EditorSetEntityParent_Undo(std::shared_ptr<based::scene::Entity> parent,
			std::shared_ptr<based::scene::Entity> child);

		bool EditorSetEntityName(std::shared_ptr<based::scene::Entity> entity, const std::string& name);
		bool EditorSetEntityName_Undo(std::shared_ptr<based::scene::Entity> entity, const std::string& name);

		bool EditorCreateEntity(std::string* outName);
		bool EditorCreateEntity_Undo(std::string* outName);

		bool EditorSetEntityActive(std::shared_ptr<based::scene::Entity> entity, bool active);
		bool EditorSetEntityActive_Undo(std::shared_ptr<based::scene::Entity> entity, bool active);

		bool EditorAddComponent(const entt::meta_type type, std::shared_ptr<based::scene::Entity> entity);
		bool EditorAddComponent_Undo(const entt::meta_type type, std::shared_ptr<based::scene::Entity> entity);

		bool EditorRemoveComponent(const entt::meta_type type, std::shared_ptr<based::scene::Entity> entity);
		bool EditorRemoveComponent_Undo(const entt::meta_type type, std::shared_ptr<based::scene::Entity> entity);

		bool EditorSetMeshMaterial(std::shared_ptr<based::scene::Entity> entity,
			std::shared_ptr<based::graphics::Material> newMat);
		bool EditorSetMeshMaterial_Undo(std::shared_ptr<based::scene::Entity> entity,
			std::shared_ptr<based::graphics::Material> newMat);

		bool EditorSetMeshRendererMesh(std::shared_ptr<based::scene::Entity> entity,
			std::shared_ptr<based::graphics::Mesh> mesh);
		bool EditorSetMeshRendererMesh_Undo(std::shared_ptr<based::scene::Entity> entity,
			std::shared_ptr<based::graphics::Mesh> mesh);

		bool EditorSetEntityTransform(std::shared_ptr<based::scene::Entity> entity,
			based::scene::Transform transform,
			based::scene::Transform savedTransform, bool local);
		bool EditorSetEntityTransform_Undo(std::shared_ptr<based::scene::Entity> entity,
			based::scene::Transform transform,
			based::scene::Transform savedTransform, bool local);

		bool EditorSetPointLightData(std::shared_ptr<based::scene::Entity> entity,
			based::scene::PointLight oldLightData, based::scene::PointLight newLightData);
		bool EditorSetPointLightData_Undo(std::shared_ptr<based::scene::Entity> entity,
			based::scene::PointLight oldLightData, based::scene::PointLight newLightData);

		bool EditorSetDirectionalLightData(std::shared_ptr<based::scene::Entity> entity,
			based::scene::DirectionalLight oldLightData, based::scene::DirectionalLight newLightData);
		bool EditorSetDirectionalLightData_Undo(std::shared_ptr<based::scene::Entity> entity,
			based::scene::DirectionalLight oldLightData, based::scene::DirectionalLight newLightData);

		bool EditorDuplicateEntity(std::shared_ptr<based::scene::Entity> entity);
		bool EditorDuplicateEntity_Undo(std::shared_ptr<based::scene::Entity> entity);

		template <typename Type, typename ...Args>
		static void AddComponent(based::scene::Entity* entity, Args &&... args)
		{
			entity->AddComponent<Type>(args...);
		}
		template <typename Type>
		static void RemoveComponent(based::scene::Entity* entity)
		{
			entity->RemoveComponent<Type>();
		}
	};
}
