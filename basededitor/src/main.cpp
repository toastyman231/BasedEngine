#include <iostream>
#include "based/log.h"
#include "based/engine.h"
#include "based/main.h"

#include "based/graphics/mesh.h"
#include "based/graphics/shader.h"
#include "based/input/mouse.h"
#include "based/input/keyboard.h"
#include "based/input/joystick.h"

#include "external/imgui/imgui.h"

using namespace based;

class Editor : public based::App
{
private:
    std::shared_ptr<graphics::Mesh> mMesh;
    std::shared_ptr<graphics::Shader> mShader;

    float xKeyOffset = 0.f;
    float yKeyOffset = 0.f;
    float keySpeed = 0.0001f;
public:
    core::WindowProperties GetWindowProperties()
    {
        core::WindowProperties props;
        props.title = "BasedEditor";
        props.w = 800;
        props.h = 600;
        props.imguiProps.IsDockingEnabled = true;
        return props;
    }

    void Initialize() override
    {
        // Test mesh
        float vertices[]
        {
             0.5,  0.5f, 0.f,
             0.5, -0.5f, 0.f,
            -0.5, -0.5f, 0.f,
            -0.5,  0.5f, 0.f,
        };
        uint32_t elements[]
        {
            0, 3, 1,
            1, 3, 2
        };
        mMesh = std::make_shared<graphics::Mesh>(&vertices[0], 4, 3, &elements[0], 6);

        // Test shader
        const char* vertexShader = R"(
                    #version 410 core
                    layout (location = 0) in vec3 position;
                    out vec3 vpos;
                    uniform vec2 offset = vec2(0.5);
                    void main()
                    {
                        vpos = position + vec3(offset, 0);
                        gl_Position = vec4(position, 1.0);
                    }
                )";
        const char* fragmentShader = R"(
                    #version 410 core
                    out vec4 outColor;
                    in vec3 vpos;

                    uniform vec3 color = vec3(0.0);
                    uniform float blue = 0.5f;
                    void main()
                    {
                        outColor = vec4(vpos.xy, blue, 1.0);
                    }
                )";
        mShader = std::make_shared<graphics::Shader>(vertexShader, fragmentShader);
        mShader->SetUniformFloat3("color", 1, 0, 0);
    }

    void Shutdown() override
    {
        
    }

    void Update() override
    {
        int windowWidth = 0;
        int windowHeight = 0;
        Engine::Instance().GetWindow().GetSize(windowWidth, windowHeight);

        float xNorm = (float)input::Mouse::X() / (float)windowWidth;
        float yNorm = (float)(windowHeight - input::Mouse::Y()) / (float)windowHeight;

        if (input::Keyboard::Key(BASED_INPUT_KEY_LEFT)) xKeyOffset -= keySpeed;
        if (input::Keyboard::Key(BASED_INPUT_KEY_RIGHT)) xKeyOffset += keySpeed;
        if (input::Keyboard::Key(BASED_INPUT_KEY_UP)) yKeyOffset += keySpeed;
        if (input::Keyboard::Key(BASED_INPUT_KEY_DOWN)) yKeyOffset -= keySpeed;

        if (input::Keyboard::KeyDown(BASED_INPUT_KEY_LEFT)) xKeyOffset -= keySpeed * 100;
        if (input::Keyboard::KeyDown(BASED_INPUT_KEY_RIGHT)) xKeyOffset += keySpeed * 100;

        if (input::Joystick::IsJoystickAvailable(0))
        {
            if (input::Joystick::GetButton(0, input::Joystick::Button::DPAD_Left)) xKeyOffset -= keySpeed;
            if (input::Joystick::GetButton(0, input::Joystick::Button::DPAD_Right)) xKeyOffset += keySpeed;
            if (input::Joystick::GetButton(0, input::Joystick::Button::DPAD_Up)) yKeyOffset += keySpeed;
            if (input::Joystick::GetButton(0, input::Joystick::Button::DPAD_Down)) yKeyOffset -= keySpeed;

            float blue = input::Joystick::GetAxis(0, input::Joystick::Axis::LeftTrigger);
            mShader->SetUniformFloat("blue", blue);
            // TODO: controller doesnt work
        }

        mShader->SetUniformFloat2("offset", xNorm + xKeyOffset, yNorm + yKeyOffset);
    }

    void Render() override
    {
        auto rc = std::make_unique < graphics::rendercommands::RenderMesh>(mMesh, mShader);
        Engine::Instance().GetRenderManager().Submit(std::move(rc));
        Engine::Instance().GetRenderManager().Flush();
    }

    void ImguiRender() override
    {
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        if (ImGui::Begin("RectPos X"))
        {
            ImGui::DragFloat("Rect Pos X", &xKeyOffset, 0.01f);
        }
        ImGui::End();

        if (ImGui::Begin("RectPos Y"))
        {
            ImGui::DragFloat("Rect Pos Y", &yKeyOffset, 0.01f);
        }

        ImGui::End();
    }
};

based::App* CreateApp()
{
    return new Editor();
}