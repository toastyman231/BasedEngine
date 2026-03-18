#pragma once
#include "editorstatics.h"
#include "based/scene/entity.h"
#include "external/history/History.h"

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
		
		template <typename T>
		bool EditorAddComponent(std::shared_ptr<based::scene::Entity> entity)
		{
			HISTORY_PUSH_T(EditorAddComponent, T, entity);

			bool isSceneDirty = Statics::IsSceneDirty();
			HISTORY_SAVE(isSceneDirty);

			entity->AddComponent<T>();

			Statics::SetSceneDirty(true);

			return true;
		}

		template <typename T>
		bool EditorAddComponent_Undo(std::shared_ptr<based::scene::Entity> entity)
		{
			HISTORY_POP();

			bool isSceneDirty;
			HISTORY_LOAD(isSceneDirty);

			entity->RemoveComponent<T>();

			Statics::SetSceneDirty(isSceneDirty);

			return true;
		}

		template <typename T>
		bool EditorRemoveComponent(std::shared_ptr<based::scene::Entity> entity)
		{
			HISTORY_PUSH_T(EditorRemoveComponent, T, entity);

			bool isSceneDirty = Statics::IsSceneDirty();
			HISTORY_SAVE(isSceneDirty);

			if constexpr (entt::type_hash<T>() == entt::type_hash<based::scene::MeshRenderer>())
			{
				auto oldMeshPtr = entity->GetComponent<based::scene::MeshRenderer>().mesh.lock();
				HISTORY_SAVE(oldMeshPtr);
			}

			entity->RemoveComponent<T>();

			Statics::SetSceneDirty(true);

			return true;
		}

		template <typename T>
		bool EditorRemoveComponent_Undo(std::shared_ptr<based::scene::Entity> entity)
		{
			HISTORY_POP();

			bool isSceneDirty;
			HISTORY_LOAD(isSceneDirty);

			entity->AddComponent<T>();

			if constexpr (std::is_same_v<T, based::scene::MeshRenderer>)
			{
				std::shared_ptr<based::graphics::Mesh> oldMeshPtr;
				HISTORY_LOAD(oldMeshPtr);

				auto& component = entity->GetComponent<T>();
				component.mesh = oldMeshPtr;
			}

			Statics::SetSceneDirty(isSceneDirty);

			return true;
		}

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

		bool EditorSetCameraData(std::shared_ptr<based::scene::Entity> entity,
			based::graphics::CameraData oldData, based::graphics::CameraData newData);
		bool EditorSetCameraData_Undo(std::shared_ptr<based::scene::Entity> entity,
			based::graphics::CameraData oldData, based::graphics::CameraData newData);

		bool EditorDeleteEntity(std::shared_ptr<based::scene::Entity> entity);
		bool EditorDeleteEntity_Undo(std::shared_ptr<based::scene::Entity> entity);

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

extern editor::EngineOperations g_EngineOperations;