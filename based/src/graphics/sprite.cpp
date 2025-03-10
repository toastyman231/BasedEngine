#include "pch.h"
#include "graphics/sprite.h"

#include "engine.h"
#include "graphics/defaultassetlibraries.h"
#include "graphics/texture.h"
#include "scene/entity.h"

#include <utility>

namespace based::graphics
{
	Sprite::Sprite(SpriteShape shape, std::shared_ptr<Material> mat)
		: Entity(), mMaterial(std::move(mat)), mColor(1.f), mSize(1.f, 1.f), mPivot(0.f, 0.f),
		mSortOrder(0)
	{
		switch (shape)
		{
		case SpriteShape::Square:
			mVA = graphics::DefaultLibraries::GetVALibrary().Get("TexturedRect");
			break;
		}
		if (!mMaterial->GetTextures().empty())
		{
			mSize = glm::vec2(mMaterial->GetTextures()[0]->GetWidth(), mMaterial->GetTextures()[0]->GetHeight());
		}
		mAlignment = ui::Middle;
		mMaterial->SetUniformValue("col", mColor);
	}

	Sprite::Sprite(glm::vec4 color)
		: mColor(color), mSize(1.f), mPivot(0.f), mSortOrder(0)
	{
		mVA = DefaultLibraries::GetVALibrary().Get("Rect");

		const auto mat = std::make_shared<Material>(DefaultLibraries::GetShaderLibrary().Get("Rect"));
		mat->SetUniformValue("col", mColor);

		mAlignment = ui::Middle;
		mMaterial = mat;
	}

	Sprite::Sprite(std::shared_ptr<Material> mat)
		: mMaterial(std::move(mat)), mColor(1.f), mSize(1.f), mPivot(0.f), mAlignment(ui::Middle),
		mSortOrder(0)
	{
		if (mMaterial->GetTextures()[0])
		{
			mSize = glm::vec2(mMaterial->GetTextures()[0]->GetWidth(), mMaterial->GetTextures()[0]->GetHeight());
		}
		mMaterial->SetUniformValue("col", mColor);
	}

	void Sprite::SetSprite(std::shared_ptr<Texture> texture)
	{
		mMaterial->AddTexture(std::move(texture));
		mSize = glm::vec2(mMaterial->GetTextures()[0]->GetWidth(), mMaterial->GetTextures()[0]->GetHeight());
		RegenerateVA();
	}

	void Sprite::SetShader(std::shared_ptr<Shader> shader) const
	{
		mMaterial->SetShader(std::move(shader));
	}

	void Sprite::SetMaterial(std::shared_ptr<Material> material)
	{
		mMaterial = std::move(material);
	}

	void Sprite::SetColor(glm::vec4 color)
	{
		mColor = color;
		mMaterial->SetUniformValue("material.diffuseMat.color", mColor);
	}

	void Sprite::SetPivot(ui::Align alignment)
	{
		auto ent = this;//mOwner.lock();
		if (!ent) return;

		mAlignment = alignment;

		switch (alignment)
		{
		case ui::Align::TopLeft:
			mPivot = { (mSize.x * ent->GetComponent<scene::Transform>().Scale().x) / 2.f,
			-(mSize.y * ent->GetComponent<scene::Transform>().Scale().y) / 2.f };
			break;
		case ui::Align::TopMiddle:
			mPivot = { 0.f,-(mSize.y * ent->GetComponent<scene::Transform>().Scale().y) / 2.f };
			break;
		case ui::Align::TopRight:
			mPivot = { -(mSize.x * ent->GetComponent<scene::Transform>().Scale().x) / 2.f,
			-(mSize.y * ent->GetComponent<scene::Transform>().Scale().y) / 2.f };
			break;
		case ui::Align::MiddleLeft:
			mPivot = { (mSize.x * ent->GetComponent<scene::Transform>().Scale().x) / 2.f, 0.f };
			break;
		case ui::Align::Middle:
			mPivot = { 0, 0 };
			break;
		case ui::Align::MiddleRight:
			mPivot = { -(mSize.x * ent->GetComponent<scene::Transform>().Scale().x) / 2.f, 0.f };
			break;
		case ui::Align::BottomLeft:
			mPivot = { (mSize.x * ent->GetComponent<scene::Transform>().Scale().x) / 2.f,
			(mSize.y * ent->GetComponent<scene::Transform>().Scale().y) / 2.f };
			break;
		case ui::Align::BottomMiddle:
			mPivot = {0.f, -(mSize.y * ent->GetComponent<scene::Transform>().Scale().y) / 2.f };
			break;
		case ui::Align::BottomRight:
			mPivot = { -(mSize.x * ent->GetComponent<scene::Transform>().Scale().x) / 2.f,
			(mSize.y * ent->GetComponent<scene::Transform>().Scale().y) / 2.f };
			break;
		}
	}

	void Sprite::Draw()
	{
		PROFILE_FUNCTION();
		auto ent = this;//mOwner.lock();
		if (!ent)
		{
			BASED_WARN("Trying to draw sprite with expired owner!");
			return;
		}

		auto& transform = ent->GetTransform();

		const std::shared_ptr<VertexArray> va = GetVA();
		const std::shared_ptr<Material> mat = GetMaterial();

		/*const glm::vec3 pos = transform.Position;
		const glm::vec3 rot = transform.Rotation;
		const glm::vec3 Scale() = transform.Scale();
		const glm::vec3 localPos = transform.LocalPosition;
		const glm::vec3 localRot = transform.LocalRotation;
		auto model = glm::mat4(1.f);

		model = glm::translate(model, pos - localPos);

		// Rotations are passed as degrees and converted to radians here automatically
		model = glm::rotate(model, glm::radians(-(rot.y - localRot.y)), glm::vec3(0.f, 1.f, 0.f));
		model = glm::rotate(model, glm::radians(-(rot.x - localRot.x)), glm::vec3(1.f, 0.f, 0.f));
		model = glm::rotate(model, glm::radians(-(rot.z - localRot.z)), glm::vec3(0.f, 0.f, 1.f));

		model = glm::translate(model, localPos);

		// Rotations are passed as degrees and converted to radians here automatically
		model = glm::rotate(model, glm::radians(localRot.y), glm::vec3(0.f, 1.f, 0.f));
		model = glm::rotate(model, glm::radians(localRot.x), glm::vec3(1.f, 0.f, 0.f));
		model = glm::rotate(model, glm::radians(localRot.z), glm::vec3(0.f, 0.f, 1.f));

		model = glm::Scale()(model, Scale());*/

		Engine::Instance().GetRenderManager().Submit(
			BASED_SUBMIT_RC(RenderVertexArrayMaterial, va, mat, transform.GetGlobalMatrix()));
	}

	void Sprite::RegenerateVA()
	{
		PROFILE_FUNCTION();
		mVA = std::make_shared<VertexArray>();
		float objectWidth = mSize.x / Engine::Instance().GetWindow().GetSize().x;
		float objectHeight = mSize.y / Engine::Instance().GetWindow().GetSize().y;

		{
			BASED_CREATE_VERTEX_BUFFER(vb, float);
			vb->PushVertex({ objectWidth, objectHeight, 0.f });
			vb->PushVertex({ objectWidth, -objectHeight, 0.f });
			vb->PushVertex({ -objectWidth, -objectHeight, 0.f });
			vb->PushVertex({ -objectWidth, objectHeight, 0.f });
			vb->SetLayout({ 3 });
			mVA->PushBuffer(std::move(vb));
		}
		{
			BASED_CREATE_VERTEX_BUFFER(vb, short);
			vb->PushVertex({ 1, 1 });
			vb->PushVertex({ 1, 0 });
			vb->PushVertex({ 0, 0 });
			vb->PushVertex({ 0, 1 });
			vb->SetLayout({ 2 });
			mVA->PushBuffer(std::move(vb));
		}

		mVA->SetElements({ 0, 3, 1, 1, 3, 2 });
		mVA->Upload();
	}
}
