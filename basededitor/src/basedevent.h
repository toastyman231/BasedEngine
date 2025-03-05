#pragma once

namespace editor
{
	enum class BasedEventType
	{
		BASED_EVENT_DELETE,
		BASED_EVENT_DUPLICATE,
		BASED_EVENT_TRANSLATE,
		BASED_EVENT_ROTATE,
		BASED_EVENT_SCALE
	};

	struct BasedEvent
	{
		BasedEventType EventType;
	};
}
