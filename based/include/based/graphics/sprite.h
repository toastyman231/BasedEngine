#pragma once

#include "based/scene/entity.h"

namespace based::graphics
{
	enum Align
	{
		TopLeft, TopMiddle, TopRight,
		MiddleLeft, Middle, MiddleRight,
		BottomLeft, BottomMiddle, BottomRight
	};

	class Sprite : public scene::Entity
	{
	private:
		std::shared_ptr<VertexArray> mVA;
		std::shared_ptr<Material> mMaterial;
		glm::vec4 mColor;
		glm::vec2 mSize;
		glm::vec2 mPivot;
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

		void SetPivot(Align alignment);
		glm::vec2 GetPivot() const { return mPivot; }
		glm::vec4 GetColor() const { return mColor; }

		std::shared_ptr<VertexArray> GetVA() { return mVA; }
		std::shared_ptr<Material> GetMaterial() { return mMaterial; }

		static void DrawSprite(Sprite* sprite);
	};
}
