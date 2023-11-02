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
		mMaterial = std::make_shared<graphics::Material>(*graphics::DefaultLibraries::GetMaterialLibrary().Get("UI"));
		mMaterial->GetShader()->SetUniformInt("textureSample", 0);

		mUiElements.emplace_back(this);
	}

	void UiElement::Draw()
	{
		auto model = glm::mat4(1.f);

		const glm::vec2 parentTransform = (parent != nullptr) ?
			                                  parent->GetRelativePosition() : glm::vec2{ 0.f, 0.f };

		const glm::vec2 scale = GetRelativeScale();
		const glm::vec2 anchor = GetAnchorOffset();
		const glm::vec2 pos = GetRelativePosition();

		model = glm::translate(model, glm::vec3{ pos.x + anchor.x + parentTransform.x, pos.y - anchor.y + parentTransform.y, 0.f } * 0.00185416f);
		model = glm::scale(model, glm::vec3{ scale.x, scale.y, 0.f } * 0.00185416f);

		mMaterial->GetShader()->SetUniformFloat4("bgColor", BackgroundColor);
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, mVA, mMaterial, model));
	}

	void UiElement::SetParent(UiElement* p)
	{
		if (p == nullptr && parent)
		{
			parent->RemoveChild(this);
			return;
		}

		parent = p;
		parent->mChildren.emplace_back(this);
	}

	bool UiElement::RemoveChild(UiElement* child)
	{
		int i = 0;
		for (const auto childUi : mChildren)
		{
			if (childUi == child)
			{
				mChildren.erase(mChildren.begin() + i);
				childUi->parent = nullptr;
				return true;
			}
			i++;
		}

		return false;
	}

	void UiElement::ShowElement(UiElement* elementToShow)
	{
		if (!elementToShow || elementToShow->IsShowing()) return;

		elementToShow->mIsShowing = true;
		elementToShow->OnShow();
	}

	void UiElement::HideElement(UiElement* elementToHide)
	{
		if (!elementToHide || !elementToHide->IsShowing()) return;

		elementToHide->mIsShowing = false;
		elementToHide->OnHide();
	}

	glm::vec2 UiElement::GetRelativeScale() const
	{
		const float scaleX = mTransform->GetSize().x - (GetPadding()->right * 2.f);
		const float scaleY = mTransform->GetSize().y - (GetPadding()->bottom * 2.f);
		return {scaleX, scaleY};
	}

	glm::vec2 UiElement::GetAnchorOffset() const
	{
		const float maxX = (parent != nullptr) ? parent->GetRelativeScale().x : 1920.f;
		const float maxY = (parent != nullptr) ? parent->GetRelativeScale().y : 1080.f;
		const float anchorX = mTransform->anchorPoint.x * maxX;
		const float anchorY = mTransform->anchorPoint.y * maxY;
		return {anchorX, anchorY};
	}

	glm::vec2 UiElement::GetRelativePosition() const
	{
		const float maxX = (parent != nullptr) ? parent->GetRelativeScale().x : 1920.f;
		const float maxY = (parent != nullptr) ? parent->GetRelativeScale().y : 1080.f;
		const float xPos = (mTransform->GetCenter().x - (maxX / 2.f)) + GetPadding()->left - (mTransform->width * mTransform->alignment.x);
		const float yPos = (mTransform->GetCenter().y + (maxY / 2.f - mTransform->height)) + GetPadding()->top + (mTransform->height * mTransform->alignment.y);
		return {xPos, yPos};
	}

	void Image::SetTexture(std::shared_ptr<graphics::Texture> tex)
	{
		const int useTexture = (tex != nullptr) ? 1 : 0;
		mTexture = std::move(tex);
		GetMaterial()->GetShader()->SetUniformInt("textureSample", useTexture);
		GetMaterial()->SetTexture(mTexture);
	}
}
