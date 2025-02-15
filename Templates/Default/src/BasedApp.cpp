#include "based/pch.h"
#include "based/log.h"
#include "based/main.h"
#include "based/graphics/mesh.h"
#include "based/scene/components.h"
#include "based/input/mouse.h"
#include "based/scene/entity.h"

using namespace based;

class BasedApp : public based::App
{
private:
	std::shared_ptr<scene::Entity> iconEntity;
public:
	core::WindowProperties GetWindowProperties() override
	{
		core::WindowProperties props;
		props.w = 1280;
		props.h = 720;
		props.imguiProps.IsDockingEnabled = true;

		return props;
	}

	based::GameSettings GetGameSettings() override
	{
		based::GameSettings settings;
		settings.gameMemory = 1;

		return settings;
	}

	void Initialize() override
	{
		App::Initialize();
		Engine::Instance().GetWindow().SetShouldRenderToScreen(true);
		input::Mouse::SetCursorVisible(!Engine::Instance().GetWindow().GetShouldRenderToScreen());
		input::Mouse::SetCursorMode(Engine::Instance().GetWindow().GetShouldRenderToScreen() ?
			input::CursorMode::Confined : input::CursorMode::Free);
		auto mainCamera = std::make_shared<graphics::Camera>();
		GetCurrentScene()->SetActiveCamera(mainCamera);

		auto material = graphics::Material::LoadMaterialFromFile(
			ASSET_PATH("Materials/Unlit.bmat"),
			GetCurrentScene()->GetMaterialStorage());
		auto mesh = graphics::Mesh::CreateMesh(
			graphics::DefaultLibraries::GetVALibrary().Get("TexturedRect"),
			material, DEFAULT_MESH_LIB, "IconMesh");
		iconEntity = scene::Entity::CreateEntity<scene::Entity>("Icon");
		iconEntity->AddComponent<scene::MeshRenderer>(mesh);
		iconEntity->SetScale({ 500, 500, 1 });

		GetCurrentScene()->GetActiveCamera()->SetPosition({ 0.f, 0.f, 10.f });
	}

	void Shutdown() override
	{
		App::Shutdown();
	}

	void Update(float deltaTime) override
	{
		App::Update(deltaTime);
	}

	void Render() override
	{
		App::Render();
	}

	void ImguiRender() override
	{
	}
};

based::App* CreateApp()
{
	return new BasedApp();
}