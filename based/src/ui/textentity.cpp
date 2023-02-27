#include "ui/textentity.h"

#include "app.h"
#include "engine.h"

#include "SDL2/SDL_ttf.h"

#include "external/glm/ext/matrix_clip_space.hpp"
#include "external/glm/gtc/type_ptr.hpp"

#include "graphics/defaultassetlibraries.h"
#include "graphics/helpers.h"

#include "scene/components.h"

namespace based::ui
{
	TextEntity::TextEntity(std::string path, std::string text, int fontSize, glm::vec3 pos, SDL_Color color)
		: setupComplete(false), mShouldRegenerate(false)
	{
		SetFont(path, fontSize);
		SetText(text);
		MoveText(pos);
		SetColor(color);
		SetAlignment(Middle);

		RegenerateTexture();

		mVA = std::make_shared<graphics::VertexArray>();
		float objectWidth = mSize.x / Engine::Instance().GetWindow().GetSize().x;
		float objectHeight = mSize.y / Engine::Instance().GetWindow().GetSize().y;
		// TODO: May need to recreate VA when regenerating texture (if it changes size for instance)

		{
			BASED_CREATE_VERTEX_BUFFER(vb, float);
			vb->PushVertex({ objectWidth, objectHeight, 0.f });
			vb->PushVertex({ objectWidth, -objectHeight, 0.f });
			vb->PushVertex({ -objectWidth, -objectHeight, 0.f });
			vb->PushVertex({ -objectWidth, objectHeight, 0.f });
			vb->SetLayout({ 3 });
			mVA->PushBuffer(std::move(vb));
		}
		{
			BASED_CREATE_VERTEX_BUFFER(vb, short);
			vb->PushVertex({ 1, 1 });
			vb->PushVertex({ 1, 0 });
			vb->PushVertex({ 0, 0 });
			vb->PushVertex({ 0, 1 });
			vb->SetLayout({ 2 });
			mVA->PushBuffer(std::move(vb));
		}

		mVA->SetElements({ 0, 3, 1, 1, 3, 2 });
		mVA->Upload();

		auto shader = graphics::DefaultLibraries::GetShaderLibrary().Get("TexturedRect");
		auto mat = std::make_shared<graphics::Material>(shader, mTexture);
		mMaterialLibrary.Load("Material", mat);
		//TODO: bring back entity functionality
		//mEntity = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry().create();
		//Engine::Instance().GetApp().GetCurrentScene()->GetRegistry().emplace<scene::Transform>(mEntity, glm::vec3(0.f));
		//Engine::Instance().GetApp().GetCurrentScene()->GetRegistry().emplace<scene::TextRenderer>(mEntity, mEntity);
		setupComplete = true;
	}

	TextEntity::~TextEntity()
	{
		TTF_CloseFont(mFont);
		//glDeleteTextures(1, &texture);
	}

	void TextEntity::SetText(std::string text)
	{
		mText = text;
		mShouldRegenerate = true;
		//if (setupComplete) RegenerateTexture();
		// TODO: regenerate texture
	}

	void TextEntity::SetFont(std::string& path, int fontSize)
	{
		mFont = TTF_OpenFont(path.c_str(), fontSize);
		BASED_ASSERT(mFont, "Error loading font!");
		mFontSize = fontSize;
		mShouldRegenerate = true;
		//if (setupComplete) RegenerateTexture();
		// TODO: regenerate texture
	}

	void TextEntity::SetColor(SDL_Color col)
	{
		mColor = col;
		mShouldRegenerate = true;
		//if (setupComplete) RegenerateTexture();
		// TODO: regenerate texture
	}

	void TextEntity::MoveText(glm::vec3 pos)
	{
		// TODO: Add this
		mPosition = Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()->ScreenToWorldPoint(pos);
		//glm::vec3 oldRot = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry().get<scene::Transform>(mEntity).Rotation;
		//glm::vec3 oldScale = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry().get<scene::Transform>(mEntity).Scale;
		//Engine::Instance().GetApp().GetCurrentScene()->GetRegistry()
		//	.replace<scene::Transform>(mEntity, pos, oldRot, oldScale);
	}

	void TextEntity::SetAlignment(Align alignment)
	{
		mAlignment = alignment;
		mShouldRegenerate = true;
		//if (setupComplete) RegenerateTexture();
	}

	void TextEntity::DeleteText()
	{
		// TODO: Is this good?
		delete(this);
	}

	void TextEntity::DrawFont()
	{

		// TODO: Don't regenerate texture/VA if the text hasn't changed at all

		Engine::Instance().GetRenderManager().Submit(
			BASED_SUBMIT_RC(PushCamera, Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()));
		auto model = glm::mat4(1.f);
		model = glm::translate(model, mPosition);
		//model = glm::translate(model, glm::vec3(static_cast<float>(x), static_cast<float>(y), 0.f));
		//model = glm::scale(model, glm::vec3(mSize.x / mFontSize, mSize.y / mFontSize, 0.f));
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, 
			/*graphics::DefaultLibraries::GetVALibrary().Get("TextVA"),*/mVA, 
			/*graphics::DefaultLibraries::GetMaterialLibrary().Get("TextMat")*/mMaterialLibrary.Get("Material"),
			model));
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PopCamera));

		if (mShouldRegenerate) RegenerateTexture();
	}

	/*
	* Taken from https://github.com/gpcz/OpenGL-SDL-Code-Warehouse
	*/
	SDL_Surface* TextEntity::ResizeToPowerOfTwo(SDL_Surface* surface)
	{
		SDL_Surface* theSurface, * convertedSurface;
		Uint32 rmask, gmask, bmask, amask;

		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0xff000000;

		theSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, graphics::NextPowerOfTwo(surface->w), graphics::NextPowerOfTwo(surface->h), 32, rmask, gmask, bmask, amask);
		SDL_FillRect(theSurface, NULL, SDL_MapRGBA(theSurface->format, 0, 0, 0, 0));
		convertedSurface = SDL_ConvertSurface(surface, theSurface->format, SDL_SWSURFACE);

		MoveTexture(convertedSurface, theSurface);
		FlipSurface(theSurface);

		//SDL_FreeSurface(theSurface);
		SDL_FreeSurface(convertedSurface);
		return theSurface;
	}

	void TextEntity::RegenerateTexture()
	{
		GLuint texture;
		SDL_Surface* surface = TTF_RenderUTF8_Blended(mFont, mText.c_str(), mColor);
		surface = ResizeToPowerOfTwo(surface);

		mSize = { surface->w, surface->h };

		mTexture = std::make_shared<graphics::Texture>(surface, texture);
		mTexture->SetTextureFilter(graphics::TextureFilter::Nearest);
		auto shader = graphics::DefaultLibraries::GetShaderLibrary().Get("TexturedRect");
		mMaterialLibrary.Load("Material", std::make_shared<graphics::Material>(shader, mTexture));

		SDL_FreeSurface(surface);
		mShouldRegenerate = false;
		//glDeleteTextures(1, &texture);
	}

	/*
	* Solution by https://stackoverflow.com/users/13272497/vvanpelt
	*/
	void TextEntity::FlipSurface(SDL_Surface* surface)
	{
		SDL_LockSurface(surface);

		int pitch = surface->pitch; // row size
		char* temp = new char[pitch]; // intermediate buffer
		char* pixels = (char*)surface->pixels;

		for (int i = 0; i < surface->h / 2; ++i)
		{
			// get pointers to the two rows to swap
			char* row1 = pixels + i * pitch;
			char* row2 = pixels + (surface->h - i - 1) * pitch;

			// swap rows
			memcpy(temp, row1, pitch);
			memcpy(row1, row2, pitch);
			memcpy(row2, temp, pitch);
		}

		delete[] temp;

		SDL_UnlockSurface(surface);
	}

	/*
	* Taken from https://github.com/gpcz/OpenGL-SDL-Code-Warehouse
	* Modified to add alignment offset
	*/
	void TextEntity::MoveTexture(SDL_Surface* src, SDL_Surface* dest)
	{
		Sint32 x, y;
		Uint32* srcPixels, * destPixels;

		if (src && dest)
		{
			if (dest->w >= src->w && dest->h >= src->h)
			{
				// You need to lock surfaces before handling their raw pixels.
				SDL_LockSurface(dest);
				SDL_LockSurface(src);

				// Start adding pixels at the proper row to align the text vertically
				int yOffset = 0; // Align to the top by default
				if (mAlignment == 3 || mAlignment == 4 || mAlignment == 5) // Align in the middle
					yOffset = (dest->h - src->h) / 2;
				else if (mAlignment == 6 || mAlignment == 7 || mAlignment == 8) // Align to the bottom
					yOffset = (dest->h - src->h);

				BASED_TRACE("SRC H: {}, DEST H: {}, YOFF: {}", src->h, dest->h, yOffset);

				// Skip any rows before the offset, and only add pixels for as many rows as the original surface has
				for (y = yOffset; y < yOffset + src->h; y++) 
				{
					// Offset the start of each row by however much is needed to align the text properly
					int xOffset = 0; // Align to the left by default
					if (mAlignment == 1 || mAlignment == 4 || mAlignment == 7) // Align in the middle
						xOffset = (dest->w - src->w) / 2;
					else if (mAlignment == 2 || mAlignment == 5 || mAlignment == 8) // Align to the right
						xOffset = (dest->w - src->w);

					// The source's pixels are easy: a row
					// start is pixels+y*src->w.
					srcPixels = (Uint32*)src->pixels + (y-yOffset) * src->w; // Subtract yOffset to get row index back in bound of original surf
					// Destination's pixel rowstarts are dest->pixels + y*dest->w.
					destPixels = (Uint32*)dest->pixels + y * dest->w + xOffset;
					for (x = 0; x < src->w; x++)
					{
						*destPixels = *srcPixels;
						destPixels++;
						srcPixels++;
					}
				}
				// We've done what we need to do.  Time to clean up.
				SDL_UnlockSurface(src);
				SDL_UnlockSurface(dest);
			}
		}
	}
}
