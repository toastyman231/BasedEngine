#include "based/pch.h"
#include "engineoperations.h"

#include "editorstatics.h"
#include "based/graphics/mesh.h"
#include "external/history/History.h"

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

		return true;
	}

	bool EngineOperations::EditorSetMeshMaterial(std::shared_ptr<based::graphics::Mesh> mesh,
		std::shared_ptr<based::graphics::Material> newMat)
	{
		HISTORY_PUSH(EditorSetMeshMaterial, mesh, newMat);

		bool isSceneDirty = Statics::IsSceneDirty();
		HISTORY_SAVE(isSceneDirty);

		auto oldMat = mesh->material;
		HISTORY_SAVE(oldMat);

		mesh->material = newMat;

		return true;
	}

	bool EngineOperations::EditorSetMeshMaterial_Undo(std::shared_ptr<based::graphics::Mesh> mesh,
		std::shared_ptr<based::graphics::Material> newMat)
	{
		HISTORY_POP();

		bool isSceneDirty;
		HISTORY_LOAD(isSceneDirty);

		std::shared_ptr<based::graphics::Material> oldMat;
		HISTORY_LOAD(oldMat);

		mesh->material = oldMat;

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

			return true;
		}

		return false;
	}
}
