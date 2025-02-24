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

		bool EditorSetMeshMaterial(std::shared_ptr<based::graphics::Mesh> mesh,
			std::shared_ptr<based::graphics::Material> newMat);
		bool EditorSetMeshMaterial_Undo(std::shared_ptr<based::graphics::Mesh> mesh,
			std::shared_ptr<based::graphics::Material> newMat);

		bool EditorSetMeshRendererMesh(std::shared_ptr<based::scene::Entity> entity,
			std::shared_ptr<based::graphics::Mesh> mesh);
		bool EditorSetMeshRendererMesh_Undo(std::shared_ptr<based::scene::Entity> entity,
			std::shared_ptr<based::graphics::Mesh> mesh);
	};
}
