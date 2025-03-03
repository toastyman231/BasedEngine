#pragma once

namespace editor
{
	enum class BasedEventType
	{
		BASED_EVENT_DELETE,
		BASED_EVENT_DUPLICATE
	};

	struct BasedEvent
	{
		BasedEventType EventType;
	};
}
