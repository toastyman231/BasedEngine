#include "based/pch.h"

#include "based/main.h"
#include "based/engine.h"
#include "based/log.h"
#include "based/graphics/camera.h"

#include "external/glm/gtc/type_ptr.hpp"

#include "editorstatics.h"
#include "Panels/scenehierarchy.h"
#include "based/core/basedtime.h"
#include "based/graphics/mesh.h"
#include "based/input/mouse.h"
#include "based/scene/entity.h"
#include "external/imgui/imgui_internal.h"
#include "external/imguizmo/ImGuizmo.h"
#include "external/tfd/tinyfiledialogs.h"
#include "Panels/detailspanel.h"
#include "Panels/filebrowser.h"
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
	editor::panels::SceneHierarchy* mSceneHierarchy;
	editor::panels::DetailsPanel* mDetailsPanel;
	editor::panels::FileBrowser* mFileBrowser;

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

		graphics::Material::LoadMaterialFromFile(
			ASSET_PATH("Materials/Lit.bmat"), 
			GetCurrentScene()->GetMaterialStorage());

		mMenuBar = new editor::panels::MenuBar("Menu");
		mSceneHierarchy = new editor::panels::SceneHierarchy("Hierarchy");
		mFileBrowser = new editor::panels::FileBrowser("File Browser");
		mFileBrowser->Initialize();

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

		auto editorSceneBuffer = std::make_shared<graphics::Framebuffer>(1280, 720);
		editorSceneBuffer->SetClearColor(glm::vec4(GetWindowProperties().clearColor, 1.0));

		auto gameCamera = GetCurrentScene()->GetCameraStorage().Get("MainCamera");

		Engine::Instance().GetRenderManager().RemovePass(2);

		auto editorMainPass = new graphics::CustomRenderPass(
			"MainColorPass", Engine::Instance().GetWindow().GetFramebuffer(), 
			nullptr);
		editorMainPass->AddOutputName("SceneColor");
		Engine::Instance().GetRenderManager().InjectPass(editorMainPass,
			(int)graphics::PassInjectionPoint::BeforeMainColor);

		auto editorRenderPass = new graphics::CustomRenderPass(
			"EditorRenderPass", editorSceneBuffer, nullptr, 
			editor::Statics::GetEditorCamera());
		editorRenderPass->AddOutputName("EditorColor");
		Engine::Instance().GetRenderManager().InjectPass(editorRenderPass, 
			(int)graphics::PassInjectionPoint::BeforeMainColor);

		mSceneView = new editor::panels::EditorView(
			editor::Statics::GetEditorCamera(), "Scene View", editorSceneBuffer);
		mSceneView->Initialize();
		mGameView = new editor::panels::GameView(
			gameCamera, "Game View", Engine::Instance().GetWindow().GetFramebuffer());
		mDetailsPanel = new editor::panels::DetailsPanel("Details");
	}

	void Shutdown() override
	{
		App::Shutdown();
	}

	void Update(float deltaTime) override
	{
		App::Update(deltaTime);

		editor::EditorPlayerUpdateSystem(GetCurrentScene()->GetRegistry(), *mSceneView);

		if (input::Keyboard::Key(BASED_INPUT_KEY_LCTRL) 
			&& input::Keyboard::KeyDown(BASED_INPUT_KEY_Z))
		{
			editor::Statics::GetHistory().Undo();
		}

		if (input::Keyboard::Key(BASED_INPUT_KEY_LCTRL)
			&& input::Keyboard::KeyDown(BASED_INPUT_KEY_Y))
		{
			editor::Statics::GetHistory().Redo();
		}

		if (input::Keyboard::Key(BASED_INPUT_KEY_LCTRL)
			&& input::Keyboard::KeyDown(BASED_INPUT_KEY_S))
		{
			editor::Statics::SaveScene(editor::Statics::GetCurrentSceneSaveLocation());
		}

		if (input::Keyboard::Key(BASED_INPUT_KEY_LCTRL) 
			&& input::Keyboard::KeyDown(BASED_INPUT_KEY_D))
		{
			mSceneView->ProcessEvent({ editor::BasedEventType::BASED_EVENT_DUPLICATE });
			mGameView->ProcessEvent({ editor::BasedEventType::BASED_EVENT_DUPLICATE });
			mMenuBar->ProcessEvent({ editor::BasedEventType::BASED_EVENT_DUPLICATE });
			mSceneHierarchy->ProcessEvent({ editor::BasedEventType::BASED_EVENT_DUPLICATE });
			mDetailsPanel->ProcessEvent({ editor::BasedEventType::BASED_EVENT_DUPLICATE });
			mFileBrowser->ProcessEvent({ editor::BasedEventType::BASED_EVENT_DUPLICATE });
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_DELETE))
		{
			mSceneView->ProcessEvent({ editor::BasedEventType::BASED_EVENT_DELETE });
			mGameView->ProcessEvent({ editor::BasedEventType::BASED_EVENT_DELETE });
			mMenuBar->ProcessEvent({ editor::BasedEventType::BASED_EVENT_DELETE });
			mSceneHierarchy->ProcessEvent({ editor::BasedEventType::BASED_EVENT_DELETE });
			mDetailsPanel->ProcessEvent({ editor::BasedEventType::BASED_EVENT_DELETE });
			mFileBrowser->ProcessEvent({ editor::BasedEventType::BASED_EVENT_DELETE });
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_W))
		{
			mSceneView->ProcessEvent({ editor::BasedEventType::BASED_EVENT_TRANSLATE });
			mGameView->ProcessEvent({ editor::BasedEventType::BASED_EVENT_TRANSLATE });
			mMenuBar->ProcessEvent({ editor::BasedEventType::BASED_EVENT_TRANSLATE });
			mSceneHierarchy->ProcessEvent({ editor::BasedEventType::BASED_EVENT_TRANSLATE });
			mDetailsPanel->ProcessEvent({ editor::BasedEventType::BASED_EVENT_TRANSLATE });
			mFileBrowser->ProcessEvent({ editor::BasedEventType::BASED_EVENT_TRANSLATE });
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_E))
		{
			mSceneView->ProcessEvent({ editor::BasedEventType::BASED_EVENT_ROTATE });
			mGameView->ProcessEvent({ editor::BasedEventType::BASED_EVENT_ROTATE });
			mMenuBar->ProcessEvent({ editor::BasedEventType::BASED_EVENT_ROTATE });
			mSceneHierarchy->ProcessEvent({ editor::BasedEventType::BASED_EVENT_ROTATE });
			mDetailsPanel->ProcessEvent({ editor::BasedEventType::BASED_EVENT_ROTATE });
			mFileBrowser->ProcessEvent({ editor::BasedEventType::BASED_EVENT_ROTATE });
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_R))
		{
			mSceneView->ProcessEvent({ editor::BasedEventType::BASED_EVENT_SCALE });
			mGameView->ProcessEvent({ editor::BasedEventType::BASED_EVENT_SCALE });
			mMenuBar->ProcessEvent({ editor::BasedEventType::BASED_EVENT_SCALE });
			mSceneHierarchy->ProcessEvent({ editor::BasedEventType::BASED_EVENT_SCALE });
			mDetailsPanel->ProcessEvent({ editor::BasedEventType::BASED_EVENT_SCALE });
			mFileBrowser->ProcessEvent({ editor::BasedEventType::BASED_EVENT_SCALE });
		}
	}

	void Render() override
	{
		App::Render();
	}

	void ImguiRender() override
	{
		ImGuizmo::BeginFrame();

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
			ImGuiID rightBarId = ImGui::DockBuilderSplitNode(dockspaceId,
				ImGuiDir_Right, 0.25f, nullptr, &mainDockspaceId);
			ImGuiID bottomBarId = ImGui::DockBuilderSplitNode(mainDockspaceId,
				ImGuiDir_Down, 0.35f, nullptr, &mainDockspaceId);
			ImGuiID leftBarId = ImGui::DockBuilderSplitNode(mainDockspaceId,
				ImGuiDir_Left, 0.25f, nullptr, &mainDockspaceId);

			ImGui::DockBuilderDockWindow("Details", rightBarId);
			ImGui::DockBuilderDockWindow("File Browser", bottomBarId);
			ImGui::DockBuilderDockWindow("Hierarchy", leftBarId);
			ImGui::DockBuilderDockWindow("Game View", mainDockspaceId);
			ImGui::DockBuilderDockWindow("Scene View", mainDockspaceId);
			ImGui::DockBuilderFinish(dockspaceId);
		}

		ImGui::DockSpace(ImGui::GetID("MainDockspace"), ImVec2(0.0f, 0.0f), 0);
		ImGui::End();

		mMenuBar->Render();
		mSceneView->Render();
		mGameView->Render();
		mSceneHierarchy->Render();
		mFileBrowser->Render();
		mDetailsPanel->Render();
	}

	bool ValidateShutdown() override
	{
		if (editor::Statics::IsSceneDirty())
		{
			auto shouldSave = tinyfd_messageBox(
				"Save Current Scene?",
				"You have unsaved changes, would you like to save the current scene?",
				"yesnocancel",
				"question",
				1
			);

			if (shouldSave == 0)
			{
				return false;
			}

			if (shouldSave == 1)
			{
				auto saveResult = editor::Statics::SaveScene();
				if (!saveResult)
				{
					BASED_WARN("Scene did not save properly, aborting create new scene!");
				}
			}
		}

		return true;
	}

	void HandleFileDrop(const std::string& path) override
	{
		if (mFileBrowser->IsFileViewerHovered())
		{
			if (mFileBrowser->HandleFileDrop(path))
				BASED_TRACE("Accepted drop from {}", path);
		} /*else
		{
			Engine::Instance().GetWindow().SetCursor("unavailable");
		}*/
	}
};

App* CreateApp()
{
	return new Editor();
}
