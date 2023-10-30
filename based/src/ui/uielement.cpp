#include <utility>

#include "based/ui/uielement.h"

#include "engine.h"
#include "external/glm/gtx/transform.hpp"
#include "graphics/defaultassetlibraries.h"

namespace based::ui
{
	UiElement::UiElement() : UiElement(0, 0, 100, 100)
	{
	}

	UiElement::UiElement(float x, float y, float width, float height)
	{
		mTransform = new RectTransform(x, y, width, height);
		mVA = graphics::DefaultLibraries::GetVALibrary().Get("TexturedRect");
		mMaterial = graphics::DefaultLibraries::GetMaterialLibrary().Get("UI");
		mMaterial->GetShader()->SetUniformInt("textureSample", 0);

		mUiElements.emplace_back(this);
	}

	void UiElement::Draw()
	{
		auto model = glm::mat4(1.f);

		const float scaleX = mTransform->GetSize().x - (GetPadding()->right * 2.f);
		const float scaleY = mTransform->GetSize().y - (GetPadding()->bottom * 2.f);
		const float anchorX = mTransform->anchorPoint.x * 1920.f; // TODO: Make relative to parent (or 1920 or whatever if no parent)
		const float anchorY = mTransform->anchorPoint.y * 1080.f; // Same as above
		const float xPos = (mTransform->GetCenter().x - (1920.f / 2.f)) + GetPadding()->left - (mTransform->width * mTransform->alignment.x) + anchorX;
		const float yPos = (mTransform->GetCenter().y + (1080.f / 2.f - mTransform->height)) + GetPadding()->top + (mTransform->height * mTransform->alignment.y) - anchorY;

		model = glm::translate(model, glm::vec3{ xPos, yPos, 0.f } * 0.00185416f);
		model = glm::scale(model, glm::vec3{ scaleX, scaleY, 0.f } * 0.00185416f);

		mMaterial->GetShader()->SetUniformFloat4("bgColor", BackgroundColor);
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, mVA, mMaterial, model));
	}

	void Image::SetTexture(std::shared_ptr<graphics::Texture> tex)
	{
		const int useTexture = (tex != nullptr) ? 1 : 0;
		mTexture = std::move(tex);
		GetMaterial()->GetShader()->SetUniformInt("textureSample", useTexture);
		GetMaterial()->SetTexture(mTexture);
	}
}
