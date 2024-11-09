#include "based/pch.h"

#include <memory>
#include <external/glm/vec3.hpp>
#include "based/main.h"

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
	std::shared_ptr<ui::TextEntity> scoreText = nullptr;
	std::shared_ptr<ui::TextEntity> gameOverText = nullptr;
	std::shared_ptr<ui::TextEntity> pausedText = nullptr;
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
		GetCurrentScene()->GetActiveCamera()->SetProjection(graphics::Projection::ORTHOGRAPHIC);
		GetCurrentScene()->GetActiveCamera()->SetPosition(glm::vec3(0.f, 0.f, 10.f));

		playGrid = scene::Entity::CreateEntity<PlayGrid>("PlayGrid",
			glm::vec3(0.f), glm::vec3(0.f), glm::vec3(1.f),
			10, 16);
		pausedText = scene::Entity::CreateEntity<ui::TextEntity>("PauseText", 
			{ 0.f, 0.f, 0.f }, {0.f, 0.f, 0.f},
			{1.f, 1.f, 1.f}, "Assets/fonts/Arimo-Bold.ttf", "Paused", 48);
		pausedText->SetRenderSpace(ui::RenderSpace::World);
		pausedText->SetColor({ 255, 255, 255, 255 });
		pausedText->MoveText({ 150.f, Engine::Instance().GetWindow().GetSize().y / 2 - 150.f, 0.f }, true);
		scoreText = scene::Entity::CreateEntity<ui::TextEntity>("ScoreText",
			{ 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f },
			{ 1.f, 1.f, 1.f }, "Assets/fonts/Arimo-Bold.ttf", "Score: 0", 48);
		scoreText->SetRenderSpace(based::ui::RenderSpace::World);
		scoreText->SetColor({ 255, 255, 255 ,255 });
		scoreText->MoveText({ 150.f, Engine::Instance().GetWindow().GetSize().y / 2, 0.f }, true);
		gameOverText = scene::Entity::CreateEntity<ui::TextEntity>("GameOverText",
			{ 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f },
			{ 1.f, 1.f, 1.f }, "Assets/fonts/Arimo-Bold.ttf", "Game Over!", 48);
		gameOverText->SetRenderSpace(ui::RenderSpace::World);
		gameOverText->SetColor({ 255, 255, 255, 255 });
		gameOverText->MoveText({ 150.f, Engine::Instance().GetWindow().GetSize().y / 2 + 150.f, 0.f }, true);
		gameOverText->SetActive(false);
		pausedText->SetActive(false);
		playGrid->SetScoreText(scoreText.get());
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
		currentTetromino->Update(deltaTime);

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