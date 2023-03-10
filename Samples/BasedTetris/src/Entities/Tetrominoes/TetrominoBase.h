#pragma once

#include <based/core/basedtime.h>
#include <based/scene/entity.h>
#include "../PlayGrid.h"

enum TetrominoType
{
	LINE, L, REVERSEL, SQUARE, ZIGZAG, T, REVERSEZIGZAG
};

enum Direction
{
	UP, LEFT, RIGHT, DOWN
};

class TetrominoBase : public based::scene::Entity
{
private:
	float mWaitTime;
	float mFallInterval;
	bool mLocked = false;
	glm::vec3 mColor;
	glm::ivec2 mPosition;
	std::vector<glm::ivec2> mTiles;
	Direction mDirection;

	std::vector<glm::ivec2> mTilesUp;
	std::vector<glm::ivec2> mTilesLeft;
	std::vector<glm::ivec2> mTilesRight;
	std::vector<glm::ivec2> mTilesDown;

	PlayGrid* mGrid;
	inline static TetrominoBase* currentTetromino = nullptr;

	void DrawTile(int x, int y) const;
	static float Clamp(float min, float max, float value);
public:
	TetrominoBase(int x, int y, PlayGrid* grid, glm::vec3 color = {1.f, 0.f, 0.f}, float fallTime = 1.f);

	static TetrominoBase* SpawnTetromino(int x, int y, TetrominoType type, PlayGrid* grid);

	static TetrominoBase* GetCurrentTetromino() { return currentTetromino; }

	static TetrominoType GetRandomTetromino();

	glm::vec3 GetColor() const { return mColor; }
	glm::ivec2 GetPosition() const { return mPosition; }
	Direction GetDirection() const { return mDirection; }

	void DrawTetromino() const;
	void MoveOver(int dir = 1);
	void MoveDown();
	void DropTetromino();
	void Update(float deltaTime) override;
	void LockTetromino();

	bool ValidateNewPosition(glm::ivec2 dir = {0, 0});
	bool ValidateNewRotation(Direction dir);
	bool IsPartOfCurrentTetromino(int x, int y) const;

	void SetTiles(std::vector<glm::ivec2> tiles) { mTiles = tiles; }
	void SetRotation(std::vector<glm::ivec2> tiles, Direction dir);
	void SetPosition(int x, int y);

	virtual void RotateCW();
	virtual void RotateCCW();
};