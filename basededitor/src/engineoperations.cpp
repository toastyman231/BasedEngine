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
}
