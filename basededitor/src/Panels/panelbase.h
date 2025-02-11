#pragma once

namespace editor::panels
{
	class Panel
	{
	public:
		Panel(const std::string& title) : mPanelTitle(title) {}
		virtual ~Panel() = default;

		std::string GetPanelTitle() const { return mPanelTitle; }

		virtual void Initialize() = 0;
		virtual void Update(float deltaTime) = 0;
		virtual void Render() = 0;
		virtual void Shutdown() = 0;

	protected:
		std::string mPanelTitle = "Untitled Panel";
	};
}
