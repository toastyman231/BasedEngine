#pragma once

#include "external/glm/glm.hpp"

#include <string>

#include "alignment.h"
#include "SDL2/SDL_ttf.h"

#include "based/core/assetlibrary.h"

#include "based/graphics/texture.h"
#include "based/graphics/vertex.h"
#include "based/scene/entity.h"

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
	class TextEntity : public scene::Entity
	{
	private:
		core::AssetLibrary<graphics::Material> mMaterialLibrary;
		core::AssetLibrary<graphics::VertexArray> mVALibrary;
		std::shared_ptr<graphics::Texture> mTexture;
		std::shared_ptr<graphics::VertexArray> mVA;
		std::string mText;
		std::string mFontPath;
		glm::vec2 mSize;
		Align mAlignment;
		TTF_Font* mFont;
		int mFontSize;
		SDL_Color mColor;

		bool mShouldRegenerate;
		bool mShouldRegenVA;

		SDL_Surface* ResizeToPowerOfTwo(SDL_Surface* surface);

		void RegenerateTexture();
		void RegenerateVertexArray();

		static void FlipSurface(SDL_Surface* surface);
	public:
		TextEntity() = default;
		TextEntity(std::string path,
			std::string text,
			int fontSize = 16,
			glm::vec3 pos = { 0,0,0 },
			SDL_Color color = { 0,0,0,255 });
		~TextEntity() override;

		void SetText(std::string text);
		void SetFont(std::string& path, int fontSize = 16);
		void SetSize(int size);
		void SetColor(SDL_Color col);
		void MoveText(glm::vec3 pos);
		void Shutdown() override;
		void MoveTexture(SDL_Surface* src, SDL_Surface* dest);
		void SetAlignment(Align alignment);

		static void DrawFont(TextEntity* textEntity);

		const SDL_Color inline GetColor() const { return mColor; }
	};
}