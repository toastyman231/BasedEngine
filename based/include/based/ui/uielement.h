#pragma once

#include "external/glm/glm.hpp"
#include "rect.h"
#include "based/graphics/material.h"
#include "based/graphics/vertex.h"

namespace based::ui {
	class UiElement {
	public:
		UiElement();
		UiElement(float x, float y, float width, float height);
		virtual ~UiElement() = default;

		virtual void OnCreate() = 0;
		virtual void OnShow() = 0;
		virtual void OnHide() = 0;
		virtual void OnDestroy() = 0;

		virtual void Update(float deltaTime) = 0;

		virtual void Draw();

		std::shared_ptr<graphics::Material> GetMaterial() { return mMaterial; }
		RectTransform* GetTransform() const { return mTransform; }
		Padding* GetPadding() const { return mTransform->Padding; }

		void SetPadding(Padding* padding) const { mTransform->Padding = padding; }
		void SetParent(UiElement* p) { parent = p; }

		static std::vector<UiElement*> GetAllUiElements() { return mUiElements; }

		glm::vec4 BackgroundColor = glm::vec4{ 1, 1, 1, 1 };
	private:
		UiElement* parent = nullptr;

		RectTransform* mTransform;
		std::shared_ptr<graphics::VertexArray> mVA;
		std::shared_ptr<graphics::Material> mMaterial;

		glm::vec2 GetRelativeScale() const;
		glm::vec2 GetAnchorOffset() const;
		glm::vec2 GetRelativePosition() const;

		inline static std::vector<UiElement*> mUiElements = {};
	};

	class Image final : public UiElement
	{
	public:
		Image(float x, float y, float width, float height) : UiElement(x, y, width, height) {}
		~Image() override = default;

		void OnCreate() override {}
		void OnShow() override {}
		void OnHide() override {}
		void OnDestroy() override {}
		void Update(float deltaTime) override {}

		void SetTexture(std::shared_ptr<graphics::Texture> tex);

	private:
		std::shared_ptr<graphics::Texture> mTexture;
	};
}
