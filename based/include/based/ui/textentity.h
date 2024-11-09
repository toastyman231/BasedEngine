#pragma once

#include "alignment.h"
#include "uidefines.h"

#include "based/graphics/texture.h"
#include "based/graphics/vertex.h"
#include "based/scene/entity.h"

struct BColor
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

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
		std::shared_ptr<graphics::Material> mMaterial;
		std::shared_ptr<graphics::VertexArray> mVA;
		std::shared_ptr<graphics::Texture> mTexture;
		std::string mText;
		std::string mFontPath;
		glm::vec2 mSize;
		RenderSpace mRenderSpace;
		Align mAlignment;
		void* mFont;
		int mFontSize;
		BColor mColor;

		bool mShouldRegenerate;
		bool mShouldRegenVA;
		bool mShouldRegenSpace = true;

		SDL_Surface* ResizeToPowerOfTwo(SDL_Surface* surface);

		void RegenerateTexture();
		void RegenerateVertexArray();
		void RegenerateRenderSpace();

		static void FlipSurface(SDL_Surface* surface);
	public:
		TextEntity() = default;
		TextEntity(std::string path,
			std::string text,
			int fontSize = 16,
			glm::vec3 pos = { 0,0,0 },
			BColor color = { 0,0,0,255 },
			RenderSpace space = RenderSpace::Screen);
		~TextEntity() override;

		void SetText(std::string text);
		void SetFont(std::string& path, int fontSize = 16);
		void SetSize(int size);
		void SetColor(BColor col);
		void SetRenderSpace(RenderSpace space);
		void MoveText(glm::vec3 pos, bool overrideZ = false, float newZ = 0.f);
		void Shutdown() override;
		void MoveTexture(SDL_Surface* src, SDL_Surface* dest);
		void SetAlignment(Align alignment);

		static void DrawFont(TextEntity* textEntity);

		const BColor inline GetColor() const { return mColor; }
	};
}