#include "based/pch.h"
#include "engineoperations.h"

#include "EditorComponents.h"
#include "external/history/History.h"
#include "editorstatics.h"
#include "based/graphics/mesh.h"

namespace editor
{
	bool EngineOperations::EditorSetEntityParent(std::shared_ptr<based::scene::Entity> parent,
		std::shared_ptr<based::scene::Entity> child)
	{
		HISTORY_PUSH(EditorSetEntityParent, parent, child);
		bool isSceneDirty = Statics::IsSceneDirty();
		HISTORY_SAVE(isSceneDirty);
		child->SetParent(parent);
		Statics::SetSceneDirty(true);

		return true;
	}

	bool EngineOperations::EditorSetEntityParent_Undo(std::shared_ptr<based::scene::Entity> parent,
		std::shared_ptr<based::scene::Entity> child)
	{
		HISTORY_POP();

		bool isSceneDirty;
		HISTORY_LOAD(isSceneDirty);
		child->SetParent(nullptr);
		Statics::SetSceneDirty(isSceneDirty);

		return true;
	}

	bool EngineOperations::EditorSetEntityName(std::shared_ptr<based::scene::Entity> entity, const std::string& name)
	{
		HISTORY_PUSH(EditorSetEntityName, entity, name);
		bool isSceneDirty = Statics::IsSceneDirty();
		HISTORY_SAVE(isSceneDirty);

		std::string previousName = entity->GetEntityName();
		HISTORY_SAVE(previousName);

		entity->SetEntityName(name);
		Statics::SetSceneDirty(true);

		return true;
	}

	bool EngineOperations::EditorSetEntityName_Undo(std::shared_ptr<based::scene::Entity> entity, const std::string& name)
	{
		HISTORY_POP();

		bool isSceneDirty;
		HISTORY_LOAD(isSceneDirty);

		std::string previousName;
		HISTORY_LOAD(previousName);
		entity->SetEntityName(previousName);
		Statics::SetSceneDirty(isSceneDirty);

		return true;
	}

	bool EngineOperations::EditorCreateEntity(std::string* outName)
	{
		HISTORY_PUSH(EditorCreateEntity, outName);
		bool isSceneDirty = Statics::IsSceneDirty();
		HISTORY_SAVE(isSceneDirty);

		std::string nameBase = "New Entity";
		auto scene = based::Engine::Instance().GetApp().GetCurrentScene();

		auto name = std::string(nameBase);
		int iterations = 0;
		while (scene->GetEntityStorage().Get(name))
		{
			name = nameBase.append(std::to_string(++iterations));
		}

		HISTORY_SAVE(name);
		auto entity = based::scene::Entity::CreateEntity(name);
		scene->GetEntityStorage().Load(name, entity);
		Statics::SetSceneDirty(true);

		*outName = name;

		return true;
	}

	bool EngineOperations::EditorCreateEntity_Undo(std::string* outName)
	{
		HISTORY_POP();

		std::string name;
		HISTORY_LOAD(name);

		bool isSceneDirty;
		HISTORY_LOAD(isSceneDirty);

		auto scene = based::Engine::Instance().GetApp().GetCurrentScene();
		auto entity = scene->GetEntityStorage().Get(name);

		based::scene::Entity::DestroyEntity(entity);

		Statics::SetSceneDirty(isSceneDirty);

		return true;
	}

	bool EngineOperations::EditorSetEntityActive(std::shared_ptr<based::scene::Entity> entity, bool active)
	{
		HISTORY_PUSH(EditorSetEntityActive, entity, active);
		bool isSceneDirty = Statics::IsSceneDirty();
		HISTORY_SAVE(isSceneDirty);

		entity->SetActive(active);

		Statics::SetSceneDirty(true);
		return true;
	}

	bool EngineOperations::EditorSetEntityActive_Undo(std::shared_ptr<based::scene::Entity> entity, bool active)
	{
		HISTORY_POP();

		bool isSceneDirty;
		HISTORY_LOAD(isSceneDirty);

		entity->SetActive(!active);

		Statics::SetSceneDirty(isSceneDirty);

		return true;
	}

	bool EngineOperations::EditorAddComponent(const entt::meta_type type, std::shared_ptr<based::scene::Entity> entity)
	{
		HISTORY_PUSH(EditorAddComponent, type, entity);

		bool isSceneDirty = Statics::IsSceneDirty();
		HISTORY_SAVE(isSceneDirty);

		using namespace entt::literals;

		// TODO: Evil, no good, very bad way of doing this
		if (auto func = type.func("AddComponent"_hs))
		{
			func.invoke({}, entity.get());

			Statics::SetSceneDirty(true);
		} else
		{
			BASED_ERROR("Add Component was invalid for type {}. Have you reflected it?", type.info().name());
		}

		return true;
	}

	bool EngineOperations::EditorAddComponent_Undo(const entt::meta_type type,
		std::shared_ptr<based::scene::Entity> entity)
	{
		HISTORY_POP();

		bool isSceneDirty;
		HISTORY_LOAD(isSceneDirty);

		using namespace entt::literals;

		if (auto func = type.func("RemoveComponent"_hs))
		{
			func.invoke({}, entity.get());

			Statics::SetSceneDirty(isSceneDirty);
		}
		else
		{
			BASED_ERROR("Remove Component was invalid for type {}. Have you reflected it?", type.info().name());
		}

		return true;
	}

	bool EngineOperations::EditorRemoveComponent(const entt::meta_type type, std::shared_ptr<based::scene::Entity> entity)
	{
		HISTORY_PUSH(EditorRemoveComponent, type, entity);

		bool isSceneDirty = Statics::IsSceneDirty();
		HISTORY_SAVE(isSceneDirty);

		using namespace entt::literals;

		if (auto func = type.func("RemoveComponent"_hs))
		{
			if (type.info().name().find("MeshRenderer") != std::string::npos)
			{
				auto oldMeshPtr = entity->GetComponent<based::scene::MeshRenderer>().mesh.lock();
				HISTORY_SAVE(oldMeshPtr);
			}

			func.invoke({}, entity.get());

			Statics::SetSceneDirty(true);
		}
		else
		{
			BASED_ERROR("Remove Component was invalid for type {}. Have you reflected it?", type.info().name());
		}
		return true;
	}

	bool EngineOperations::EditorRemoveComponent_Undo(const entt::meta_type type,
		std::shared_ptr<based::scene::Entity> entity)
	{
		HISTORY_POP();

		bool isSceneDirty;
		HISTORY_LOAD(isSceneDirty);

		using namespace entt::literals;

		if (auto func = type.func("AddComponent"_hs))
		{
			func.invoke({}, entity.get());

			if (type.info().name().find("MeshRenderer") != std::string::npos)
			{
				std::shared_ptr<based::graphics::Mesh> oldMeshPtr;
				HISTORY_LOAD(oldMeshPtr);

				auto& registry = based::Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
				registry.patch<based::scene::MeshRenderer>(entity->GetEntityHandle(),
					[oldMeshPtr](based::scene::MeshRenderer& mr)
					{
						mr.mesh = oldMeshPtr;
					});
			}

			Statics::SetSceneDirty(isSceneDirty);
		}
		else
		{
			BASED_ERROR("Add Component was invalid for type {}. Have you reflected it?", type.info().name());
		}

		return true;
	}

	bool EngineOperations::EditorSetMeshMaterial(std::shared_ptr<based::scene::Entity> entity,
	                                             std::shared_ptr<based::graphics::Material> newMat)
	{
		HISTORY_PUSH(EditorSetMeshMaterial, entity, newMat);

		if (!entity->HasComponent<based::scene::MeshRenderer>()) return false;

		auto mr = entity->GetComponent<based::scene::MeshRenderer>();
		auto mesh = mr.mesh.lock();
		auto material = mr.material.lock();

		if (!(mesh && material)) return false;

		bool isSceneDirty = Statics::IsSceneDirty();
		HISTORY_SAVE(isSceneDirty);

		auto oldMat = material;
		HISTORY_SAVE(oldMat);

		auto scene = based::Engine::Instance().GetApp().GetCurrentScene();
		scene->GetRegistry().patch<based::scene::MeshRenderer>(entity->GetEntityHandle(),
			[newMat](based::scene::MeshRenderer& m)
			{
				m.material = newMat;
			});

		Statics::SetSceneDirty(true);

		return true;
	}

	bool EngineOperations::EditorSetMeshMaterial_Undo(std::shared_ptr<based::scene::Entity> entity,
		std::shared_ptr<based::graphics::Material> newMat)
	{
		HISTORY_POP();

		if (!entity->HasComponent<based::scene::MeshRenderer>()) return false;

		auto mr = entity->GetComponent<based::scene::MeshRenderer>();
		auto mesh = mr.mesh.lock();
		auto material = mr.material.lock();

		if (!(mesh && material)) return false;

		bool isSceneDirty;
		HISTORY_LOAD(isSceneDirty);

		std::shared_ptr<based::graphics::Material> oldMat;
		HISTORY_LOAD(oldMat);

		auto scene = based::Engine::Instance().GetApp().GetCurrentScene();
		scene->GetRegistry().patch<based::scene::MeshRenderer>(entity->GetEntityHandle(),
			[oldMat](based::scene::MeshRenderer& m)
			{
				m.material = oldMat;
			});

		Statics::SetSceneDirty(isSceneDirty);

		return true;
	}

	bool EngineOperations::EditorSetMeshRendererMesh(std::shared_ptr<based::scene::Entity> entity,
		std::shared_ptr<based::graphics::Mesh> mesh)
	{
		HISTORY_PUSH(EditorSetMeshRendererMesh, entity, mesh);

		bool isSceneDirty = Statics::IsSceneDirty();
		HISTORY_SAVE(isSceneDirty);

		if (entity->HasComponent<based::scene::MeshRenderer>())
		{
			auto renderer = entity->GetComponent<based::scene::MeshRenderer>();

			auto oldMesh = renderer.mesh.lock();
			HISTORY_SAVE(oldMesh);

			auto scene = based::Engine::Instance().GetApp().GetCurrentScene();
			scene->GetRegistry().patch<based::scene::MeshRenderer>(entity->GetEntityHandle(), 
				[mesh](based::scene::MeshRenderer& mr){
					mr.mesh = mesh;
				});

			Statics::SetSceneDirty(true);
			return true;
		}

		return false;
	}

	bool EngineOperations::EditorSetMeshRendererMesh_Undo(std::shared_ptr<based::scene::Entity> entity,
		std::shared_ptr<based::graphics::Mesh> mesh)
	{
		HISTORY_POP();

		bool isSceneDirty;
		HISTORY_LOAD(isSceneDirty);

		std::shared_ptr<based::graphics::Mesh> oldMesh;
		HISTORY_LOAD(oldMesh);

		if (entity->HasComponent<based::scene::MeshRenderer>())
		{
			auto scene = based::Engine::Instance().GetApp().GetCurrentScene();
			scene->GetRegistry().patch<based::scene::MeshRenderer>(entity->GetEntityHandle(),
				[oldMesh](based::scene::MeshRenderer& mr) {
					mr.mesh = oldMesh;
				});

			Statics::SetSceneDirty(isSceneDirty);
			return true;
		}

		return false;
	}

	bool EngineOperations::EditorSetEntityTransform(std::shared_ptr<based::scene::Entity> entity,
	                                                based::scene::Transform transform, 
													based::scene::Transform savedTransform, bool local)
	{
		HISTORY_PUSH(EditorSetEntityTransform, entity, transform, savedTransform, local);

		bool isSceneDirty = Statics::IsSceneDirty();
		HISTORY_SAVE(isSceneDirty);

		if (!local)
			entity->SetTransform(transform.Position, transform.Rotation, transform.Scale);
		else
			entity->SetLocalTransform(transform.Position, transform.Rotation, transform.Scale);

		Statics::SetSceneDirty(true);

		return true;
	}

	bool EngineOperations::EditorSetEntityTransform_Undo(std::shared_ptr<based::scene::Entity> entity,
		based::scene::Transform transform,
		based::scene::Transform savedTransform, bool local)
	{
		HISTORY_POP();

		bool isSceneDirty;
		HISTORY_LOAD(isSceneDirty);

		if (!local)
			entity->SetTransform(savedTransform.Position, savedTransform.Rotation, savedTransform.Scale);
		else
			entity->SetLocalTransform(savedTransform.LocalPosition, savedTransform.LocalRotation, savedTransform.LocalScale);

		entity->AddComponent<MovedDueToUndo>();

		Statics::SetSceneDirty(isSceneDirty);

		return true;
	}

	bool EngineOperations::EditorSetPointLightData(std::shared_ptr<based::scene::Entity> entity,
		based::scene::PointLight oldLightData, based::scene::PointLight newLightData)
	{
		HISTORY_PUSH(EditorSetPointLightData, entity, oldLightData, newLightData);

		bool isSceneDirty = Statics::IsSceneDirty();
		HISTORY_SAVE(isSceneDirty);

		using namespace based;

		auto& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
		registry.patch<scene::PointLight>(entity->GetEntityHandle(),
			[newLightData](scene::PointLight& pl)
			{
				pl.intensity = newLightData.intensity;
				pl.linear = newLightData.linear;
				pl.quadratic = newLightData.quadratic;
				pl.constant = newLightData.constant;
				pl.color = newLightData.color;
			});

		Statics::SetSceneDirty(true);

		return true;
	}

	bool EngineOperations::EditorSetPointLightData_Undo(std::shared_ptr<based::scene::Entity> entity,
		based::scene::PointLight oldLightData, based::scene::PointLight newLightData)
	{
		HISTORY_POP();

		bool isSceneDirty;
		HISTORY_LOAD(isSceneDirty);

		using namespace based;

		auto& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
		registry.patch<scene::PointLight>(entity->GetEntityHandle(),
			[oldLightData](scene::PointLight& pl)
			{
				pl.intensity = oldLightData.intensity;
				pl.linear = oldLightData.linear;
				pl.quadratic = oldLightData.quadratic;
				pl.constant = oldLightData.constant;
				pl.color = oldLightData.color;
			});

		entity->AddComponent<LightChangedDueToUndo>();

		Statics::SetSceneDirty(isSceneDirty);

		return true;
	}

	bool EngineOperations::EditorSetDirectionalLightData(std::shared_ptr<based::scene::Entity> entity,
		based::scene::DirectionalLight oldLightData, based::scene::DirectionalLight newLightData)
	{
		HISTORY_PUSH(EditorSetDirectionalLightData, entity, oldLightData, newLightData);

		bool isSceneDirty = Statics::IsSceneDirty();
		HISTORY_SAVE(isSceneDirty);

		using namespace based;

		auto& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
		registry.patch<scene::DirectionalLight>(entity->GetEntityHandle(),
			[newLightData](scene::DirectionalLight& pl)
			{
				pl.intensity = newLightData.intensity;
				pl.color = newLightData.color;
			});

		Statics::SetSceneDirty(true);

		return true;
	}

	bool EngineOperations::EditorSetDirectionalLightData_Undo(std::shared_ptr<based::scene::Entity> entity,
		based::scene::DirectionalLight oldLightData, based::scene::DirectionalLight newLightData)
	{
		HISTORY_POP();

		bool isSceneDirty;
		HISTORY_LOAD(isSceneDirty);

		using namespace based;

		auto& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
		registry.patch<scene::DirectionalLight>(entity->GetEntityHandle(),
			[oldLightData](scene::DirectionalLight& pl)
			{
				pl.intensity = oldLightData.intensity;
				pl.color = oldLightData.color;
			});

		entity->AddComponent<DLightChangedDueToUndo>();

		Statics::SetSceneDirty(isSceneDirty);

		return true;
	}

	bool EngineOperations::EditorDuplicateEntity(std::shared_ptr<based::scene::Entity> entity)
	{
		HISTORY_PUSH(EditorDuplicateEntity, entity);

		bool isSceneDirty = Statics::IsSceneDirty();
		HISTORY_SAVE(isSceneDirty);

		auto scene = based::Engine::Instance().GetApp().GetCurrentScene();
		auto& registry = scene->GetRegistry();

		auto entityBacking = registry.create();
		auto newEntity = std::make_shared<based::scene::Entity>(entityBacking, registry);
		newEntity->SetEntityName(scene->GetEntityStorage().GetSafeName(entity->GetEntityName()));
		for (auto [id, storage] : registry.storage())
		{
			if (storage.contains(entity->GetEntityHandle()))
			{
				if (storage.type().name().find("based::scene::CameraComponent") != std::string_view::npos)
				{
					// Need to copy the camera manually because the normal copy constructor
					// would cause this camera component to point to the exact same camera.
					// But the camera copy constructor needs to work that way or the editor
					// and game cameras become the same camera at some point during startup.
					auto& comp = registry.emplace<based::scene::CameraComponent>(entityBacking);
					auto& otherComp = entity->GetComponent<based::scene::CameraComponent>();
					auto newCam = comp.camera.lock();
					if (auto oldCam = otherComp.camera.lock())
					{
						newCam->SetProjection(oldCam->GetProjectionType());
						newCam->SetFOV(oldCam->GetFOV());
						newCam->SetNear(oldCam->GetNear());
						newCam->SetFar(oldCam->GetFar());
						newCam->SetHeight(oldCam->GetHeight());
					}
				} else storage.push(entityBacking, storage.value(entity->GetEntityHandle()));
			}
		}
		newEntity->AddOrReplaceComponent<based::scene::EntityReference>(newEntity);
		scene->GetEntityStorage().Load(newEntity->GetEntityName(), newEntity);

		HISTORY_SAVE(newEntity);

		Statics::SetSceneDirty(true);

		return true;
	}

	bool EngineOperations::EditorDuplicateEntity_Undo(std::shared_ptr<based::scene::Entity> entity)
	{
		HISTORY_POP();

		bool isSceneDirty;
		HISTORY_LOAD(isSceneDirty);

		std::shared_ptr<based::scene::Entity> newEntity;
		HISTORY_LOAD(newEntity);

		based::scene::Entity::DestroyEntity(newEntity);

		Statics::SetSelectedEntities({});

		Statics::SetSceneDirty(isSceneDirty);

		return true;
	}

	bool EngineOperations::EditorSetCameraData(std::shared_ptr<based::scene::Entity> entity,
		based::graphics::CameraData oldData, based::graphics::CameraData newData)
	{
		HISTORY_PUSH(EditorSetCameraData, entity, oldData, newData);

		bool isSceneDirty = Statics::IsSceneDirty();
		HISTORY_SAVE(isSceneDirty);

		using namespace based;

		auto cam = entity->GetComponent<scene::CameraComponent>();
		auto scene = Engine::Instance().GetApp().GetCurrentScene();

		auto previousMainCam = scene->GetActiveCamera();
		HISTORY_SAVE(previousMainCam);

		if (auto camera = cam.camera.lock())
		{
			camera->SetProjection(newData.projection);
			camera->SetFOV(newData.fov);
			camera->SetNear(newData.nearPlane);
			camera->SetFar(newData.farPlane);
			camera->SetHeight(newData.height);

			if (newData.main && previousMainCam != camera)
			{
				scene->SetActiveCamera(camera);
			} else if (!newData.main && previousMainCam == camera)
			{
				scene->SetActiveCamera(nullptr);
			}
		}

		Statics::SetSceneDirty(true);

		return true;
	}

	bool EngineOperations::EditorSetCameraData_Undo(std::shared_ptr<based::scene::Entity> entity,
		based::graphics::CameraData oldData, based::graphics::CameraData newData)
	{
		HISTORY_POP();

		bool isSceneDirty;
		HISTORY_LOAD(isSceneDirty);

		using namespace based;

		std::shared_ptr<graphics::Camera> previousMainCam;
		HISTORY_LOAD(previousMainCam);

		auto cam = entity->GetComponent<scene::CameraComponent>();
		auto scene = Engine::Instance().GetApp().GetCurrentScene();

		if (auto camera = cam.camera.lock())
		{
			camera->SetProjection(oldData.projection);
			camera->SetFOV(oldData.fov);
			camera->SetNear(oldData.nearPlane);
			camera->SetFar(oldData.farPlane);
			camera->SetHeight(oldData.height);

			if (oldData.main && previousMainCam != scene->GetActiveCamera())
			{
				scene->SetActiveCamera(previousMainCam);
			}
			else if (!oldData.main && camera == scene->GetActiveCamera())
			{
				scene->SetActiveCamera(nullptr);
			}
		}

		entity->AddComponent<CamChangedDueToUndo>();

		Statics::SetSceneDirty(isSceneDirty);

		return true;
	}

	bool EngineOperations::EditorDeleteEntity(std::shared_ptr<based::scene::Entity> entity)
	{
		HISTORY_PUSH(EditorDeleteEntity, entity);

		using namespace based;

		bool isSceneDirty = Statics::IsSceneDirty();
		HISTORY_SAVE(isSceneDirty);

		entity->RemoveComponent<scene::EntityReference>();

		Statics::SetSceneDirty(true);
		Statics::SetSelectedEntities({});

		return true;
	}

	bool EngineOperations::EditorDeleteEntity_Undo(std::shared_ptr<based::scene::Entity> entity)
	{
		HISTORY_POP();

		bool isSceneDirty;
		HISTORY_LOAD(isSceneDirty);

		using namespace based;

		entity->AddComponent<scene::EntityReference>(entity);

		Statics::SetSceneDirty(isSceneDirty);

		return true;
	}
}
