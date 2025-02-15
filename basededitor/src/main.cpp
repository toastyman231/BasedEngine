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
#include "external/imgui/imgui_internal.h"
#include "Panels/gameview.h"
#include "Panels/menubar.h"
#include "Player/editorplayer.h"

using namespace based;

class Editor : public App
{
public:
	editor::panels::GameView* mSceneView;
	editor::panels::GameView* mGameView;
	editor::panels::MenuBar* mMenuBar;

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
		
		editor::Statics::InitializeEditorStatics();
		std::string startupScenePath;

		if (std::filesystem::exists("Config/EditorConfig.yml"))
		{
			std::ifstream ifs("Config/EditorConfig.yml");
			std::stringstream stream;
			stream << ifs.rdbuf();

			YAML::Node data = YAML::Load(stream.str());
			if (data["StartupScene"]) startupScenePath = data["StartupScene"].as<std::string>();
		} else
		{
			BASED_WARN("No config file for editor!");
		}

		if (!startupScenePath.empty())
		{
			editor::Statics::LoadScene(startupScenePath);
		}
		else
		{
			BASED_WARN("Provided scene is invalid, falling back on default!");
			editor::Statics::LoadScene(ASSET_PATH("Scenes/Default3D.bscn"));
		}
		GetCurrentScene()->SetActiveCamera(editor::Statics::GetEditorCamera());

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
		mMenuBar = new editor::panels::MenuBar("Menu");

		mTestCube = based::scene::Entity::CreateEntity("Cube 2");//GetCurrentScene()->GetEntityStorage().Get("Cube");
		mTestCube->AddComponent<based::scene::MeshRenderer>(
			based::graphics::Mesh::LoadMeshFromFile(ASSET_PATH("Meshes/cube.obj"),
				GetCurrentScene()->GetMeshStorage()));
	}

	void Shutdown() override
	{
		App::Shutdown();
	}

	void Update(float deltaTime) override
	{
		App::Update(deltaTime);

		editor::EditorPlayerUpdateSystem(GetCurrentScene()->GetRegistry(), *mSceneView);
		mTestCube->SetPosition(mCubePos);
	}

	void Render() override
	{
		App::Render();
	}

	void ImguiRender() override
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::Begin("MainWindow", nullptr, flags);

		if (ImGui::DockBuilderGetNode(ImGui::GetID("MainDockspace")) == NULL)
		{
			ImGuiID dockspaceId = ImGui::GetID("MainDockspace");
			ImGui::DockBuilderRemoveNode(dockspaceId);
			ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_NoTabBar);
			ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetWindowSize());

			ImGuiID mainDockspaceId = dockspaceId;
			ImGuiID leftBarId = ImGui::DockBuilderSplitNode(dockspaceId,
				ImGuiDir_Left, 0.225f, nullptr, &mainDockspaceId);

			ImGui::DockBuilderDockWindow("Game View", mainDockspaceId);
			ImGui::DockBuilderDockWindow("Scene View", mainDockspaceId);
			ImGui::DockBuilderDockWindow("Test", leftBarId);
			ImGui::DockBuilderFinish(dockspaceId);
		}

		ImGui::DockSpace(ImGui::GetID("MainDockspace"), ImVec2(0.0f, 0.0f), 0);
		ImGui::End();

		mMenuBar->Render();
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
