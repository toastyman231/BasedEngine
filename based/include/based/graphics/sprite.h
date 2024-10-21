#pragma once

#include "based/ui/alignment.h"
#include "based/scene/components.h"

namespace based::graphics
{
	class Shader;
	class Texture;
	class Material;
	class VertexArray;

	enum class SpriteShape : uint8_t
	{
		Square
	};

	/*class Sprite : public scene::ScriptableBehavior
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
		Sprite(const std::shared_ptr<scene::Entity>& owner, glm::vec4 color);
		Sprite(const std::shared_ptr<scene::Entity>& owner, std::shared_ptr<Material> mat);
		Sprite(const std::shared_ptr<scene::Entity>& owner, SpriteShape shape, std::shared_ptr<Material> mat);
		~Sprite() override = default;

		int GetSortOrder() const { return mSortOrder; }
		void SetSortOrder(int order) { mSortOrder = order; }

		void SetSprite(std::shared_ptr<Texture> texture);
		void SetShader(std::shared_ptr<Shader> shader) const;
		void SetMaterial(std::shared_ptr<Material> material);
		void SetColor(glm::vec4 color);

		void SetPivot(ui::Align alignment);
		glm::vec2 GetPivot() const { return mPivot; }
		glm::vec4 GetColor() const { return mColor; }

		std::shared_ptr<VertexArray> GetVA() { return mVA; }
		std::shared_ptr<Material> GetMaterial() { return mMaterial; }

		void Draw();
		void Initialize() override {}
		void Update(float deltaTime) override { BASED_TRACE("Updating sprite!"); }
		void Shutdown() override { BASED_TRACE("Shutting down sprite!"); }
	};*/
}
