#pragma once

#include "external/entt/entt.hpp"
#include "external/glm/glm.hpp"

#include <string>

#include "based/graphics/shader.h"

namespace based::ui
{
	class TextEntity
	{
	private:
		entt::entity mEntity;
		std::shared_ptr<graphics::Shader> mShader;
		glm::vec4 mColor;

		unsigned int VAO, VBO;
	public:
		TextEntity();
		TextEntity(const std::string& text, glm::vec3 pos);
		~TextEntity();

		static bool Initialize();
		static void Terminate();

		void SetText(const std::string& text) const;
		void SetColor(glm::vec4 col);
		void MoveText(glm::vec3 pos) const;
		void DeleteText();

		void RenderText(std::string text, float x, float y, glm::vec3 color, float scale);

		const glm::vec4 inline GetColor() const { return mColor; }
	};
}
