#pragma once

#include "based/scene/entity.h"

namespace based::graphics
{
	class Sprite : public scene::Entity
	{
	private:
		std::shared_ptr<VertexArray> mVA;
		std::shared_ptr<Material> mMaterial;
		glm::vec2 mSize;
		int mSortOrder;

		void RegenerateVA();
	public:
		Sprite(std::shared_ptr<VertexArray> va, std::shared_ptr<Material> mat);

		int GetSortOrder() const { return mSortOrder; }
		void SetSortOrder(int order) { mSortOrder = order; }

		void SetSprite(std::shared_ptr<Texture> texture);
		void SetShader(std::shared_ptr<Shader> shader) const;

		std::shared_ptr<VertexArray> GetVA() { return mVA; }
		std::shared_ptr<Material> GetMaterial() { return mMaterial; }

		static void DrawSprite(Sprite* sprite);
	};
}
