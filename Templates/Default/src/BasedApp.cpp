#include "based/log.h"
#include "based/main.h"

#include <memory>

#include "based/graphics/defaultassetlibraries.h"

#include "based/core/assetlibrary.h"
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

		auto texture = std::make_shared<graphics::Texture>("Assets/icon.png");
		auto material = graphics::Material::CreateMaterial(
			graphics::DefaultLibraries::GetShaderLibrary().Get("TexturedRect"),
			DEFAULT_MAT_LIB, "Icon");
		material->AddTexture(texture, "tex");
		material->SetUniformValue("col", glm::vec4{ 1.f, 1.f, 1.f, 1.f });
		auto mesh = graphics::Mesh::CreateMesh(
			graphics::DefaultLibraries::GetVALibrary().Get("TexturedRect"),
			material, DEFAULT_MESH_LIB, "IconMesh");
		iconEntity = scene::Entity::CreateEntity<scene::Entity>("Icon");
		iconEntity->AddComponent<scene::MeshRenderer>(mesh);

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
};

based::App* CreateApp()
{
	return new BasedApp();
}