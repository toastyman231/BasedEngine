#include "pch.h"
#include "ui/textentity.h"

#include "app.h"
#include "engine.h"

#include "SDL2/SDL_ttf.h"
#include <utility>

#include "graphics/defaultassetlibraries.h"

#include "math/basedmath.h"

namespace based::ui
{
	TextEntity::TextEntity(std::string path, std::string text, int fontSize, glm::vec3 pos, BColor color, RenderSpace space)
		: Entity(Engine::Instance().GetApp().GetCurrentScene()->GetRegistry()),
		  mRenderSpace(space),
		  mFont(nullptr), mShouldRegenerate(false), mShouldRegenVA(false), mShouldRegenSpace(true)
	{
		SetFont(path, fontSize);
		SetText(std::move(text));
		MoveText(pos);
		SetColor(color);
		SetAlignment(Middle);
		SetRenderSpace(RenderSpace::Screen);

		auto shader = LOAD_SHADER(ASSET_PATH("Shaders/basic_lit.vert"), ASSET_PATH("Shaders/basic_unlit.vert"));
		mMaterial = std::make_shared<graphics::Material>(shader);
		mMaterial->mMaterialName = std::string("TextEntityMaterial-") + GetEntityName();
		mMaterial->AddTexture(mTexture, "material.diffuseMat.tex");
		mMaterial->SetUniformValue("material.diffuseMat.useSampler", 1);

		RegenerateTexture();

		RegenerateVertexArray();

		AddComponent<scene::TextRenderer>(this);
	}

	TextEntity::~TextEntity()
	{
		//TODO: Figure out why this causes an error when closing the game
		//if (mFont) TTF_CloseFont(static_cast<TTF_Font*>(mFont));
	}

	void TextEntity::SetText(std::string text)
	{
		mText = text;
		mShouldRegenerate = true;
		mShouldRegenVA = true;
	}

	void TextEntity::SetFont(std::string& path, int fontSize)
	{
		mFontPath = path;
		if (mFont) TTF_CloseFont(static_cast<TTF_Font*>(mFont));
		mFont = TTF_OpenFont(path.c_str(), fontSize);
		BASED_ASSERT(mFont, "Error loading font!");
		if (fontSize != mFontSize)
		{
			mFontSize = fontSize;
			mShouldRegenVA = true;
		}
		mShouldRegenerate = true;
	}

	void TextEntity::SetSize(int size)
	{
		if (size != mFontSize)
		{
			if (mFont) TTF_CloseFont(static_cast<TTF_Font*>(mFont));
			mFont = TTF_OpenFont(mFontPath.c_str(), size);
			BASED_ASSERT(mFont, "Error loading font!");
			mFontSize = size;
			mShouldRegenVA = true;
			mShouldRegenerate = true;
		}

	}

	void TextEntity::SetColor(BColor col)
	{
		mColor = col;
		mShouldRegenerate = true;
	}

	void TextEntity::SetRenderSpace(RenderSpace space)
	{
		mRenderSpace = space;
		mShouldRegenSpace = true;
	}

	void TextEntity::MoveText(glm::vec3 pos, bool overrideZ, float newZ)
	{
		auto newPos = Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()->ScreenToWorldPoint(pos);
		if (overrideZ) newPos.z = newZ;
		AddOrReplaceComponent<scene::Transform>(newPos);
	}

	void TextEntity::Shutdown()
	{
		Entity::Shutdown();
	}

	void TextEntity::SetAlignment(Align alignment)
	{
		mAlignment = alignment;
		mShouldRegenerate = true;
	}

	void TextEntity::SetIgnoreDepth(bool shouldIgnore)
	{
		mIgnoreDepth = shouldIgnore;
	}

	void TextEntity::DrawFont(TextEntity* textEntity)
	{
		if (!textEntity->IsActive() || Engine::Instance().GetRenderManager().GetCurrentPassName() == "ShadowDepthPass") return;
		if (textEntity->mShouldRegenSpace || textEntity->mRenderSpace == RenderSpace::World) 
			textEntity->RegenerateRenderSpace();

		scene::Transform& transform = textEntity->GetComponent<scene::Transform>();

		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PushCamera,
			Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()));

		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial,
			textEntity->mVA,
			textEntity->mMaterial,
			transform.GetGlobalMatrix(),
			textEntity->mIgnoreDepth ? GL_ALWAYS : GL_LEQUAL));

		if (textEntity->mShouldRegenerate) textEntity->RegenerateTexture();
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

		theSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			math::NextPowerOfTwo(surface->w), math::NextPowerOfTwo(surface->h), 32, rmask, gmask, bmask, amask);
		SDL_FillRect(theSurface, NULL, SDL_MapRGBA(theSurface->format, 0, 0, 0, 0));
		convertedSurface = SDL_ConvertSurface(surface, theSurface->format, SDL_SWSURFACE);

		MoveTexture(convertedSurface, theSurface);
		FlipSurface(theSurface);

		SDL_FreeSurface(convertedSurface);
		return theSurface;
	}

	void TextEntity::RegenerateTexture()
	{
		uint32_t texture;
		SDL_Surface* surface = TTF_RenderUTF8_Blended(static_cast<TTF_Font*>(mFont), 
			mText.c_str(), {mColor.r, mColor.g, mColor.b, mColor.a});
		surface = ResizeToPowerOfTwo(surface);

		mSize = { surface->w, surface->h };

		mTexture = std::make_shared<graphics::Texture>(surface, texture);
		mTexture->SetTextureFilter(graphics::TextureFilter::Nearest);

		//mMaterial->RemoveTexture("material.diffuseMat.tex");
		mMaterial->AddTexture(mTexture, "material.diffuseMat.tex");

		SDL_FreeSurface(surface);
		mShouldRegenerate = false;
		if (mShouldRegenVA) RegenerateVertexArray();
	}

	void TextEntity::RegenerateVertexArray()
	{
		mVA = std::make_shared<graphics::VertexArray>();
		float objectWidth = mSize.x / Engine::Instance().GetWindow().GetSize().x;
		float objectHeight = mSize.y / Engine::Instance().GetWindow().GetSize().y;

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
	}

	void TextEntity::RegenerateRenderSpace()
	{
		switch (mRenderSpace)
		{
		case RenderSpace::Screen:
			/*mMaterial->SetUniformValue("proj", glm::mat4(1.f));
			mMaterial->SetUniformValue("view", glm::mat4(1.f));*/
			mMaterial->SetUniformValue("useOverrideMatrices", 1);
			break;
		case RenderSpace::World:
			mMaterial->SetUniformValue("useOverrideMatrices", 0);
			/*mMaterial->SetUniformValue("proj", graphics::Shader::GetShaderGlobals().proj);
			mMaterial->SetUniformValue("view", graphics::Shader::GetShaderGlobals().view);*/
			break;
		}
		mShouldRegenSpace = false;
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

				//BASED_TRACE("SRC H: {}, DEST H: {}, YOFF: {}", src->h, dest->h, yOffset);

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
					srcPixels = (Uint32*)src->pixels + (y - yOffset) * src->w; // Subtract yOffset to get row index back in bound of original surf
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