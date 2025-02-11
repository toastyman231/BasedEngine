#include "based/pch.h"

#include "based/main.h"
#include "based/engine.h"
#include "based/log.h"
#include "based/graphics/camera.h"

#include "external/glm/gtc/type_ptr.hpp"

#include "editorstatics.h"
#include "based/core/basedtime.h"
#include "based/graphics/mesh.h"
#include "based/input/mouse.h"
#include "based/scene/entity.h"
#include "Panels/gameview.h"

using namespace based;

class Editor : public App
{
public:
	editor::panels::GameView* mSceneView;
	editor::panels::GameView* mGameView;

	std::shared_ptr<scene::Entity> mTestCube;
	glm::vec3 mCubePos;

	core::WindowProperties GetWindowProperties() override
	{
		core::WindowProperties props;
		props.title = "Based Editor";
		props.w = 1280;
		props.h = 720;
		props.imguiProps.IsDockingEnabled = true;
		return props;
	}

	void Initialize() override
	{
		App::Initialize();
		Engine::Instance().GetWindow().SetShouldRenderToScreen(false);
		input::Mouse::SetCursorVisible(true);
		input::Mouse::SetCursorMode(input::CursorMode::Free);
		core::Time::SetTimeScale(0);

		editor::Statics::LoadScene(ASSET_PATH("Scenes/Default3D.bscn"));
		editor::Statics::InitializeEditorStatics();

		auto editorSceneBuffer = std::make_shared<graphics::Framebuffer>(1280, 720);
		editorSceneBuffer->SetClearColor(glm::vec4(GetWindowProperties().clearColor, 1.0));

		auto gameCamera = GetCurrentScene()->GetCameraStorage().Get("MainCamera");

		Engine::Instance().GetRenderManager().RemovePass(2);

		auto editorMainPass = new graphics::CustomRenderPass(
			"MainRenderPass", Engine::Instance().GetWindow().GetFramebuffer(), 
			nullptr, gameCamera);
		editorMainPass->SetOutputName("SceneColor");
		Engine::Instance().GetRenderManager().InjectPass(editorMainPass,
			(int)graphics::PassInjectionPoint::BeforeMainColor);

		auto editorRenderPass = new graphics::CustomRenderPass(
			"EditorRenderPass", editorSceneBuffer, nullptr, 
			editor::Statics::GetEditorCamera());
		editorRenderPass->SetOutputName("EditorColor");
		Engine::Instance().GetRenderManager().InjectPass(editorRenderPass, 
			(int)graphics::PassInjectionPoint::BeforeMainColor);

		mSceneView = new editor::panels::GameView(
			gameCamera, "Scene View", editorSceneBuffer);
		mGameView = new editor::panels::GameView(
			editor::Statics::GetEditorCamera(), "Game View", Engine::Instance().GetWindow().GetFramebuffer());

		mTestCube = GetCurrentScene()->GetEntityStorage().Get("Cube");
	}

	void Shutdown() override
	{
		App::Shutdown();
	}

	void Update(float deltaTime) override
	{
		App::Update(deltaTime);

		mTestCube->SetPosition(mCubePos);
	}

	void Render() override
	{
		App::Render();
	}

	void ImguiRender() override
	{
		mGameView->Render();
		mSceneView->Render();

		if (ImGui::Begin("Test"))
		{
			ImGui::DragFloat3("Cube Pos", glm::value_ptr(mCubePos), 0.01f);
		}
		ImGui::End();
	}
};

App* CreateApp()
{
	return new Editor();
}
