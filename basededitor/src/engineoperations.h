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
	};
}
