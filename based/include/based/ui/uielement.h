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
		void SetParent(UiElement* p);
		bool RemoveChild(UiElement* child);
		bool IsShowing() const { return mIsShowing; }
		unsigned long long GetChildCount() const { return mChildren.size(); }
		std::vector<UiElement*> GetAllChildren() const { return mChildren; }

		static void ShowElement(UiElement* elementToShow);
		static void HideElement(UiElement* elementToHide);

		static std::vector<UiElement*> GetAllUiElements() { return mUiElements; }

		template<typename Type, typename... Args>
		static inline Type* CreateUiElement(float x, float y, float width, float height, Args &&... args)
		{
			Type* newElement = new Type(x, y, width, height, args...);

			newElement->OnCreate();

			return newElement;
		}

		glm::vec4 BackgroundColor = glm::vec4{ 1, 1, 1, 1 };
	private:
		UiElement* parent = nullptr;
		std::vector<UiElement*> mChildren;

		RectTransform* mTransform;
		std::shared_ptr<graphics::VertexArray> mVA;
		std::shared_ptr<graphics::Material> mMaterial;

		bool mIsShowing = false;

		glm::vec2 GetRelativeScale() const;
		glm::vec2 GetAnchorOffset() const;
		glm::vec2 GetRelativePosition() const;

		inline static std::vector<UiElement*> mUiElements = {};
	};

	class Image final : public UiElement
	{
	public:
		Image(float x, float y, float width, float height) : UiElement(x, y, width, height) {}
		Image(const Image& other) = default;
		~Image() override = default;

		void SetTexture(std::shared_ptr<graphics::Texture> tex);
		void OnCreate() override {}
		void OnShow() override {}
		void OnHide() override {}
		void OnDestroy() override {}
		void Update(float deltaTime) override {}

	private:
		std::shared_ptr<graphics::Texture> mTexture;
	};
}
