#pragma once
#include "panelbase.h"

namespace based::graphics
{
	class Framebuffer;
}

namespace based::graphics
{
	class Camera;
}

namespace editor::panels
{
	class GameView : public Panel
	{
	public:
		GameView(
			const std::shared_ptr<based::graphics::Camera>& camera,
			const std::string& title = "Game View",
			const std::shared_ptr<based::graphics::Framebuffer>& buffer = nullptr)
			: Panel(title), mViewCamera(camera), mBackingBuffer(buffer)
		{}
		~GameView() override = default;

		std::shared_ptr<based::graphics::Framebuffer> GetBackingBuffer() const { return mBackingBuffer; }

		void Initialize() override {}
		void Update(float deltaTime) override {}
		void Shutdown() override {}
		void Render() override;

	private:
		std::shared_ptr<based::graphics::Camera> mViewCamera;
		std::shared_ptr<based::graphics::Framebuffer> mBackingBuffer;
		std::string mOutputBufferId = "SceneColor";
	};
}
