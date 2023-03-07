#include "based/log.h"
#include "based/main.h"

#include "based/scene/components.h"
#include "based/scene/entity.h"
#include "Entities/PlayGrid.h"
#include "Entities/Tetrominoes/TetrominoBase.h"

using namespace based;

class BasedApp : public based::App
{
private:
	PlayGrid* playGrid = nullptr;
	TetrominoBase* currentTetromino = nullptr;
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