#include "based/log.h"
#include "based/main.h"

#include "based/scene/components.h"
#include "based/scene/entity.h"
#include "based/ui/textentity.h"
#include "Entities/PlayGrid.h"
#include "Entities/Tetrominoes/TetrominoBase.h"

using namespace based;

class BasedApp : public based::App
{
private:
	PlayGrid* playGrid = nullptr;
	TetrominoBase* currentTetromino = nullptr;
	based::ui::TextEntity* scoreText = nullptr;
public:
	core::WindowProperties GetWindowProperties() override
	{
		core::WindowProperties props;
		props.w = 1280;
		props.h = 720;
		props.imguiProps.IsDockingEnabled = true;
		props.clearColor = { 0, 0, 0 };

		return props;
	}

	void Initialize() override
	{
		App::Initialize();

		playGrid = new PlayGrid(10, 16);
		scoreText = new based::ui::TextEntity("Assets/fonts/Arimo-Bold.ttf", "Score: 0", 48,
			{ 150.f, based::Engine::Instance().GetWindow().GetSize().y / 2, 0.f }, {255, 255, 255, 255});
		currentTetromino = TetrominoBase::SpawnTetromino(5, 0, LINE, playGrid);
	}

	void Shutdown() override
	{
		App::Shutdown();
	}

	void Update(float deltaTime) override
	{
		App::Update(deltaTime);
		currentTetromino = TetrominoBase::GetCurrentTetromino();
		scoreText->SetText("Score: " + std::to_string(playGrid->GetScore()));

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_D))
		{
			currentTetromino->MoveOver();
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_A))
		{
			currentTetromino->MoveOver(-1);
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_RIGHT))
		{
			currentTetromino->RotateCW();
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_LEFT))
		{
			currentTetromino->RotateCCW();
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_SPACE))
		{
			currentTetromino->DropTetromino();
		}
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