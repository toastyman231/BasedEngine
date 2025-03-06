#pragma once
#include "panelbase.h"
#include "../external/imguizmo/ImGuizmo.h"
#include "based/graphics/texture.h"

namespace based::graphics
{
	class Texture;
}

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

	protected:
		std::shared_ptr<based::graphics::Camera> mViewCamera;
		std::shared_ptr<based::graphics::Framebuffer> mBackingBuffer;
		std::string mOutputBufferId = "SceneColor";
	};

	class EditorView : public GameView
	{
	public:
		EditorView(const std::shared_ptr<based::graphics::Camera>& camera, const std::string& title,
			const std::shared_ptr<based::graphics::Framebuffer>& buffer)
			: GameView(camera, title, buffer)
		{
		}

		void Render() override;
		void ProcessEvent(BasedEvent event) override;
		void Initialize() override;

	private:
		ImGuizmo::OPERATION mOperation = ImGuizmo::OPERATION::TRANSLATE;
		ImGuizmo::MODE mMode = ImGuizmo::MODE::WORLD;

		std::shared_ptr<based::graphics::Texture> mTranslateIcon;
		std::shared_ptr<based::graphics::Texture> mRotateIcon;
		std::shared_ptr<based::graphics::Texture> mScaleIcon;
		std::shared_ptr<based::graphics::Texture> mGlobalIcon;
		std::shared_ptr<based::graphics::Texture> mLocalIcon;
	};

	inline void EditorView::Initialize()
	{
		GameView::Initialize();

		mTranslateIcon = std::make_shared<based::graphics::Texture>("Assets/Icons/translate-icon.png");
		mRotateIcon = std::make_shared<based::graphics::Texture>("Assets/Icons/rotate-icon.png");;
		mScaleIcon = std::make_shared<based::graphics::Texture>("Assets/Icons/scale-icon.png");;
		mGlobalIcon = std::make_shared<based::graphics::Texture>("Assets/Icons/global-icon.png");;
		mLocalIcon = std::make_shared<based::graphics::Texture>("Assets/Icons/local-icon.png");;
	}
}
