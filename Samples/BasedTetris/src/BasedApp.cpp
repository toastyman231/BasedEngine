#include "based/log.h"
#include "based/main.h"

#include "based/scene/components.h"
#include "based/scene/entity.h"
#include "based/scene/audio.h"
#include "based/ui/textentity.h"
#include "Entities/PlayGrid.h"
#include "Entities/Tetrominoes/TetrominoBase.h"

using namespace based;

class BasedApp : public based::App
{
private:
	std::shared_ptr<PlayGrid> playGrid = nullptr;
	TetrominoBase* currentTetromino = nullptr;
	ui::TextEntity* scoreText = nullptr;
	ui::TextEntity* gameOverText = nullptr;
	ui::TextEntity* pausedText = nullptr;
	scene::Audio* tetrisTheme = nullptr;

	bool firstTime = true;
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
		// TODO: Add way to hide console in release config
		// TODO: Figure out how to stop errors on game shutdown

		playGrid = scene::Entity::CreateEntity<PlayGrid>("PlayGrid",
			glm::vec3(0.f), glm::vec3(0.f), glm::vec3(1.f),
			10, 16);
		pausedText = new ui::TextEntity("Assets/fonts/Arimo-Bold.ttf", "Paused", 48,
			{ 150.f, Engine::Instance().GetWindow().GetSize().y / 2 - 150.f, 0.f }, { 255, 255, 255, 255 });
		scoreText = new ui::TextEntity("Assets/fonts/Arimo-Bold.ttf", "Score: 0", 48,
			{ 150.f, Engine::Instance().GetWindow().GetSize().y / 2, 0.f }, {255, 255, 255, 255});
		gameOverText = new ui::TextEntity("Assets/fonts/Arimo-Bold.ttf", "Game Over!", 48,
			{150.f, Engine::Instance().GetWindow().GetSize().y / 2 + 150.f, 0.f}, { 255, 255, 255, 255 });
		gameOverText->SetActive(false);
		pausedText->SetActive(false);
		playGrid->SetScoreText(scoreText);
		currentTetromino = TetrominoBase::SpawnTetromino(5, 0, LINE, playGrid);

		tetrisTheme = new scene::Audio(std::string("Assets/sounds/tetris.mp3"), 0.5f, true);
		tetrisTheme->Play();
	}

	void Shutdown() override
	{
		App::Shutdown();
	}

	void Update(float deltaTime) override
	{
		App::Update(deltaTime);

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_ESCAPE))
		{
			playGrid->TogglePaused();
			pausedText->SetActive(playGrid->Paused());
		}

		if (playGrid->Paused()) return;

		if (playGrid->GameOver())
		{
			if (!firstTime) 
			{
				if (input::Keyboard::KeyDown(BASED_INPUT_KEY_R))
				{
					LoadScene(persistentScene);
				}
				return;
			}
			tetrisTheme->Stop();
			scene::Audio::PlayAudio(std::string("Assets/sounds/gameover.mp3"));
			gameOverText->SetActive(true);
			firstTime = false;
			return;
		}

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

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_P))
		{
			if (tetrisTheme->IsPlaying())
			{
				tetrisTheme->Pause();
			}
			else tetrisTheme->Play();
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