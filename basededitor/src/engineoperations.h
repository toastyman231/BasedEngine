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
	};
}
