#include "based/pch.h"
#include "engineoperations.h"

#include "editorstatics.h"
#include "external/history/History.h"

namespace editor
{
	bool EngineOperations::EditorSetEntityParent(std::shared_ptr<based::scene::Entity> parent,
		std::shared_ptr<based::scene::Entity> child)
	{
		HISTORY_PUSH(EditorSetEntityParent, parent, child);
		bool isSceneDirty = Statics::IsSceneDirty();
		HISTORY_SAVE(isSceneDirty);
		BASED_TRACE("SAVE {}", isSceneDirty);
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
}
