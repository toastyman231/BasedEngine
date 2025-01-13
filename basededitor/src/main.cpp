#include "based/pch.h"

#include "based/main.h"
#include "based/engine.h"
#include "based/log.h"
#include "based/core/assetlibrary.h"

#include "based/graphics/camera.h"
#include "based/graphics/framebuffer.h"
#include "based/graphics/material.h"
#include "based/graphics/shader.h"
#include "based/graphics/texture.h"
#include "based/graphics/vertex.h"

#include "based/input/joystick.h"
#include "based/input/keyboard.h"
#include "based/input/mouse.h"

#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"
#include "external/glm/gtc/type_ptr.hpp"
#include "external/imgui/imgui.h"

using namespace based;

class Editor : public App
{
public:
	core::WindowProperties GetWindowProperties() override
	{
		core::WindowProperties props;
		props.title = "BasedEditor";
		props.w = 1280;
		props.h = 720;
		props.imguiProps.IsDockingEnabled = true;
		return props;
	}

	void Initialize() override
	{
		App::Initialize();
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

App* CreateApp()
{
	return new Editor();
}
