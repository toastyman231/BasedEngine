#pragma once

#include "external/entt/entt.hpp"
#include "external/glm/glm.hpp"

#include <string>
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_ttf.h"

#include "based/graphics/shader.h"
#include "based/graphics/texture.h"

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
		std::shared_ptr<graphics::Shader> mShader;
		std::shared_ptr<graphics::Texture> mTexture;
		glm::vec4 mColor;
		SDL_Renderer* mRenderer;

		unsigned int VAO, VBO;
	public:
		TextEntity() = default;
		TextEntity(std::string text);
		TextEntity(const std::string& text, glm::vec3 pos);
		~TextEntity();

		static bool Initialize();
		static void Terminate();

		void SetText(const std::string& text) const;
		void SetColor(glm::vec4 col);
		void MoveText(glm::vec3 pos) const;
		void DeleteText();

		//void RenderText(std::string text, float x, float y, glm::vec3 color, float scale);
		void Draw_Font(const char* str, int x, int y, int width, int height, int size, SDL_Color color);

		const glm::vec4 inline GetColor() const { return mColor; }
	};
}
