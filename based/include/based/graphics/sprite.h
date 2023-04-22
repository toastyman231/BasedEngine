#pragma once

#include "based/scene/entity.h"

#include "based/ui/alignment.h"

namespace based::graphics
{
	class Sprite : public scene::Entity
	{
	private:
		std::shared_ptr<VertexArray> mVA;
		std::shared_ptr<Material> mMaterial;
		glm::vec4 mColor;
		glm::vec2 mSize;
		glm::vec2 mPivot;
		ui::Align mAlignment;
		int mSortOrder;

		void RegenerateVA();
	public:
		Sprite(glm::vec4 color);
		Sprite(std::shared_ptr<Material> mat);
		Sprite(std::shared_ptr<VertexArray> va, std::shared_ptr<Material> mat);

		int GetSortOrder() const { return mSortOrder; }
		void SetSortOrder(int order) { mSortOrder = order; }

		void SetSprite(std::shared_ptr<Texture> texture);
		void SetShader(std::shared_ptr<Shader> shader) const;
		void SetMaterial(std::shared_ptr<Material> material);
		void SetColor(glm::vec4 color);

		void SetTransform(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale) override;
		void SetPosition(glm::vec3 pos) override;
		void SetRotation(glm::vec3 rot) override;
		void SetScale(glm::vec3 scale) override;

		void SetPivot(ui::Align alignment);
		glm::vec2 GetPivot() const { return mPivot; }
		glm::vec4 GetColor() const { return mColor; }

		std::shared_ptr<VertexArray> GetVA() { return mVA; }
		std::shared_ptr<Material> GetMaterial() { return mMaterial; }

		static void DrawSprite(Sprite* sprite);
	};
}
