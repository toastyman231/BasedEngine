#pragma once

#include <based/core/assetlibrary.h>

#include "based/scene/entity.h"
#include <based/graphics/sprite.h>
#include <based/graphics/defaultassetlibraries.h>
#include <based/input/keyboard.h>

class PlayGrid : public based::scene::Entity
{
public:
	PlayGrid(int x, int y);

	glm::ivec2 GetSize() const { return mSize; }

	void SetupTiles() const;
	void Initialize() override;
	void Update(float deltaTime) override;

	void DrawTile(int x, int y, glm::vec3 color) const;
	void MoveDown(int x, int y) const;

	bool TileFull(int x, int y) const;

private:
	float mTileSize;
	glm::ivec2 mSize;
	std::vector<based::graphics::Sprite*> mTiles;
	based::core::AssetLibrary<std::shared_ptr<based::graphics::Material>> mMaterials;

	static float Lerp(float a, float b, float t);
};
