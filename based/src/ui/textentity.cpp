#include "ui/textentity.h"

#include "app.h"
#include "engine.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include "SDL2/SDL_ttf.h"

#include "external/glm/ext/matrix_clip_space.hpp"
#include "external/glm/gtc/type_ptr.hpp"

#include "graphics/defaultassetlibraries.h"
#include "graphics/helpers.h"

#include "scene/components.h"

namespace based::ui
{
	struct Character
	{
		unsigned int TextureID;  // ID handle of the glyph texture
		glm::ivec2   Size;       // Size of glyph
		glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
		long Advance;    // Offset to advance to next glyph
	};

	std::map<GLchar, Character> Characters;

	TextEntity::TextEntity(std::string dummyVariable) : VAO(0), VBO(0)
	{
		//TODO: bring back entity functionality
		//mEntity = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry().create();
		//Engine::Instance().GetApp().GetCurrentScene()->GetRegistry().emplace<scene::Transform>(mEntity, glm::vec3(0.f));
		//Engine::Instance().GetApp().GetCurrentScene()->GetRegistry().emplace<scene::TextRenderer>(mEntity, mEntity);
		mRenderer = SDL_CreateRenderer(Engine::Instance().GetWindow().GetSDLWindow(), -1, 0);

		/*Draw_Font("This is a test!", Engine::Instance().GetWindow().GetSize().x / 2, 
			Engine::Instance().GetWindow().GetSize().y / 2, 74, 32, 32, { 255, 255, 255 });*/

		//mColor = glm::vec4(1.f);
		//auto lib = graphics::DefaultLibraries::GetShaderLibrary();
		//mShader = lib.Get("Text");

		//glEnable(GL_CULL_FACE); BASED_CHECK_GL_ERROR;
		//glEnable(GL_BLEND); BASED_CHECK_GL_ERROR;
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); BASED_CHECK_GL_ERROR;
		////glDisable(GL_DEPTH_TEST); BASED_CHECK_GL_ERROR;

		////mShader = graphics::DefaultLibraries::GetShaderLibrary().Get("Text");
		//glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(Engine::Instance().GetWindow().GetSize().x), 0.0f,
		//	static_cast<float>(Engine::Instance().GetWindow().GetSize().y));
		//mShader->Bind();
		//glUniformMatrix4fv(mShader->GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
		//glUniform1i(mShader->GetUniformLocation("tex"), 0);

		//FT_Library ft;
		//if (FT_Init_FreeType(&ft))
		//{
		//	BASED_ERROR("ERROR::FREETYPE: Could not init FreeType Library");
		//	return;
		//}

		//FT_Face face;
		////std::string font_name = std::filesystem::path("res/fonts/arial.ttf");
		//// TODO: Generate font path automatically
		//// TODO: consider switching to msdfgl or freetype-gl
		//if (FT_New_Face(ft, "C:/Users/jmorg/Documents/Repos/BasedEngine/bin/Debug/Sandbox/res/fonts/arial.ttf", 0, &face))
		//{
		//	BASED_ERROR("ERROR::FREETYPE: Failed to load font");
		//	return;
		//}

		//FT_Set_Pixel_Sizes(face, 0, 48);

		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1); BASED_CHECK_GL_ERROR; // disable byte-alignment restriction 

		//for (unsigned char c = 0; c < 128; c++)
		//{
		//	// load character glyph
		//	if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		//	{
		//		BASED_ERROR("ERROR::FREETYTPE: Failed to load Glyph");
		//		continue;
		//	}

		//	// generate texture
		//	unsigned int texture;
		//	glGenTextures(1, &texture); BASED_CHECK_GL_ERROR;
		//	glBindTexture(GL_TEXTURE_2D, texture); BASED_CHECK_GL_ERROR;
		//	glTexImage2D(
		//		GL_TEXTURE_2D,
		//		0,
		//		GL_RED,
		//		face->glyph->bitmap.width,
		//		face->glyph->bitmap.rows,
		//		0,
		//		GL_RED,
		//		GL_UNSIGNED_BYTE,
		//		face->glyph->bitmap.buffer
		//	); BASED_CHECK_GL_ERROR;
		//	// set texture options
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); BASED_CHECK_GL_ERROR;
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); BASED_CHECK_GL_ERROR;
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); BASED_CHECK_GL_ERROR;
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); BASED_CHECK_GL_ERROR;
		//	// now store character for later use
		//	Character character = {
		//		texture,
		//		glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
		//		glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
		//		face->glyph->advance.x
		//	};
		//	Characters.insert(std::pair<char, Character>(c, character));
		//}

		//FT_Done_Face(face);
		//FT_Done_FreeType(ft);

		////BASED_TRACE("VAO: " + VAO);
		//glGenVertexArrays(1, &VAO); BASED_CHECK_GL_ERROR;
		//BASED_TRACE("VAO: {}", VAO);
		//glGenBuffers(1, &VBO); BASED_CHECK_GL_ERROR;
		//glBindVertexArray(VAO); BASED_CHECK_GL_ERROR;
		//glBindBuffer(GL_ARRAY_BUFFER, VBO); BASED_CHECK_GL_ERROR;
		//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW); BASED_CHECK_GL_ERROR;
		//glEnableVertexAttribArray(0); BASED_CHECK_GL_ERROR;
		//glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0); BASED_CHECK_GL_ERROR;
		//glBindBuffer(GL_ARRAY_BUFFER, 0); BASED_CHECK_GL_ERROR;
		//glBindVertexArray(0); BASED_CHECK_GL_ERROR;

		//BASED_TRACE("Text entity setup complete");
		//RenderText(0.f, 0.f, glm::vec3(1.f), 1.f);
	}

	TextEntity::TextEntity(const std::string& text, glm::vec3 pos)
	{
		//mEntity = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry().create();
		//Engine::Instance().GetApp().GetCurrentScene()->GetRegistry().emplace<scene::Transform>(mEntity, pos);
		//Engine::Instance().GetApp().GetCurrentScene()->GetRegistry().emplace<scene::TextRenderer>(mEntity, mEntity);

		mColor = glm::vec4(1.f);
	}

	TextEntity::~TextEntity()
	{

	}

	bool TextEntity::Initialize()
	{
		return true;

	}

	void TextEntity::Terminate()
	{

	}

	void TextEntity::SetText(const std::string& text) const
	{

	}

	void TextEntity::SetColor(glm::vec4 col)
	{
		mColor = col;
	}

	void TextEntity::MoveText(glm::vec3 pos) const
	{
		//glm::vec3 oldRot = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry().get<scene::Transform>(mEntity).Rotation;
		//glm::vec3 oldScale = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry().get<scene::Transform>(mEntity).Scale;
		//Engine::Instance().GetApp().GetCurrentScene()->GetRegistry()
		//	.replace<scene::Transform>(mEntity, pos, oldRot, oldScale);
	}

	void TextEntity::DeleteText()
	{

	}

	void TextEntity::RenderText(std::string text, float x, float y, glm::vec3 color, float scale)
	{
		// activate corresponding render state
		//auto shader = graphics::DefaultLibraries::GetShaderLibrary().Get("Text");
		mShader->Bind();
		//BASED_TRACE(mShader->GetFragmentShaderSource());
		glUniform3f(mShader->GetUniformLocation("fontColor"), color.x, color.y, color.z); BASED_CHECK_GL_ERROR;
		glActiveTexture(GL_TEXTURE0); BASED_CHECK_GL_ERROR;
		//BASED_TRACE("VAO: {}", VAO);
		glBindVertexArray(VAO); BASED_CHECK_GL_ERROR;

		// iterate through all characters
		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++)
		{
			Character ch = Characters[*c];

			float xpos = x + ch.Bearing.x * scale;
			float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

			float w = ch.Size.x * scale;
			float h = ch.Size.y * scale;
			// update VBO for each character
			float vertices[6][4] = {
				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos,     ypos,       0.0f, 1.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },

				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },
				{ xpos + w, ypos + h,   1.0f, 0.0f }
			};
			// render glyph texture over quad
			glBindTexture(GL_TEXTURE_2D, ch.TextureID); BASED_CHECK_GL_ERROR;
			// update content of VBO memory
			glBindBuffer(GL_ARRAY_BUFFER, VBO); BASED_CHECK_GL_ERROR;
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); BASED_CHECK_GL_ERROR;
			glBindBuffer(GL_ARRAY_BUFFER, 0); BASED_CHECK_GL_ERROR;
			// render quad
			glDrawArrays(GL_TRIANGLES, 0, 6); BASED_CHECK_GL_ERROR;
			// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
		}
		glBindVertexArray(0); BASED_CHECK_GL_ERROR;
		glBindTexture(GL_TEXTURE_2D, 0); BASED_CHECK_GL_ERROR;
	}

	void TextEntity::Draw_Font(const char* str, int x, int y, int width, int height, int size,
		SDL_Color color)
	{
		TTF_Font* font = TTF_OpenFont("C:\\Users\\jmorg\\Documents\\Repos\\BasedEngine\\bin\\Debug\\Sandbox\\res\\fonts\\arial.ttf", size);

		GLuint texture;
		SDL_Surface* surface = TTF_RenderText_Blended(font, str, color);

		//glEnable(GL_TEXTURE_2D); BASED_CHECK_GL_ERROR;
		glGenTextures(1, &texture); BASED_CHECK_GL_ERROR;
		glBindTexture(GL_TEXTURE_2D, texture); BASED_CHECK_GL_ERROR;

		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); BASED_CHECK_GL_ERROR;
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); BASED_CHECK_GL_ERROR;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
		BASED_CHECK_GL_ERROR;

		mTexture = std::make_shared<graphics::Texture>(texture, surface->h, surface->w);
		/*std::shared_ptr<graphics::Material> charMat = std::make_shared<graphics::Material>(
			graphics::DefaultLibraries::GetShaderLibrary().Get("TexturedRect"),
			charTex
			);*/
		std::shared_ptr<graphics::Shader> shader = graphics::DefaultLibraries::GetShaderLibrary().Get("TexturedRect");

		const std::shared_ptr<graphics::VertexArray> va = graphics::DefaultLibraries::GetVALibrary().Get("TexturedRect");
		//auto model = glm::mat4(1.f);
		//model = glm::translate(model, glm::vec3{x, y, 0});
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PushCamera, Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()));
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayTextured, va, mTexture, shader));//BASED_SUBMIT_RC(RenderVertexArrayMaterial, va, charMat, model));
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PopCamera));

		//glBegin(GL_QUADS); BASED_CHECK_GL_ERROR;
		//{
		//	glTexCoord2d(0, 1); BASED_CHECK_GL_ERROR; glVertex3f(0, 0, 0); BASED_CHECK_GL_ERROR;
		//	glTexCoord2d(1, 1); BASED_CHECK_GL_ERROR; glVertex3f(static_cast<float>(0 + surface->w), 0, 0); BASED_CHECK_GL_ERROR;
		//	glTexCoord2d(1, 0); BASED_CHECK_GL_ERROR; glVertex3f(static_cast<float>(0 + surface->w), static_cast<float>(0 + surface->h), 0); BASED_CHECK_GL_ERROR;
		//	glTexCoord2d(0, 0); BASED_CHECK_GL_ERROR; glVertex3f(0, static_cast<float>(0 + surface->h), 0); BASED_CHECK_GL_ERROR;
		//}
		//glEnd(); BASED_CHECK_GL_ERROR;
		//glDisable(GL_TEXTURE_2D); BASED_CHECK_GL_ERROR;

		TTF_CloseFont(font);
		SDL_FreeSurface(surface);
		glDeleteTextures(1, &texture);
		BASED_TRACE("Finished render!");
		/*TTF_Font* font = TTF_OpenFont("res/fonts/arial.ttf", size);

		SDL_Surface* message_surf = TTF_RenderText_Blended(font, str, color);
		SDL_Texture* Message = SDL_CreateTextureFromSurface(mRenderer, message_surf);
		SDL_Rect Message_rect = { x, y, width, height };
		SDL_RenderCopy(mRenderer, Message, NULL, &Message_rect);
		SDL_RenderPresent(mRenderer);

		SDL_DestroyTexture(Message);
		SDL_FreeSurface(message_surf);
		TTF_CloseFont(font);*/
	}
}
