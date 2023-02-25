#pragma once

#include "external/entt/entt.hpp"
#include "external/glm/glm.hpp"

#include <string>
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_ttf.h"

#include "based/core/assetlibrary.h"

#include "based/graphics/shader.h"
#include "based/graphics/texture.h"
#include "based/graphics/vertex.h"

namespace based
{
	namespace graphics
	{
		class Material;
		class Texture;
	}
}

namespace based::ui
{
	class TextEntity
	{
	private:
		entt::entity mEntity;
		core::AssetLibrary<graphics::Material> mMaterialLibrary;
		std::shared_ptr<graphics::Texture> mTexture;
		std::shared_ptr<graphics::VertexArray> mVA;
		std::string mText;
		glm::vec2 mSize;
		TTF_Font* mFont;
		int mFontSize;
		SDL_Color mColor;

		bool setupComplete;
		bool mShouldRegenerate;

		SDL_Surface* ResizeToPowerOfTwo(SDL_Surface* surface);

		void RegenerateTexture();

		static void FlipSurface(SDL_Surface* surface);
	public:
		TextEntity() = default;
		TextEntity(std::string path, 
			std::string text, 
			int fontSize = 16, 
			glm::vec3 pos = {0,0,0},
			SDL_Color color = { 0,0,0,255 });
		~TextEntity();

		void SetText(std::string text);
		void SetFont(std::string& path, int fontSize = 16);
		void SetColor(SDL_Color col);
		void MoveText(glm::vec3 pos) const;
		void DeleteText();
		void MoveTexture(SDL_Surface* src, SDL_Surface* dest);

		void DrawFont();

		const SDL_Color inline GetColor() const { return mColor; }
	};
}
