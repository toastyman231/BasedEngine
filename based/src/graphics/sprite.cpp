#include "pch.h"
#include "graphics/sprite.h"

#include "graphics/defaultassetlibraries.h"
#include "graphics/texture.h"

namespace based::graphics
{
	Sprite::Sprite(std::shared_ptr<VertexArray> va, std::shared_ptr<Material> mat)
		: Entity(), mVA(std::move(va)),
		mMaterial(std::move(mat)), mColor(1.f), mSize(1.f, 1.f), mPivot(0.f, 0.f), mSortOrder(0)
	{
		if (mMaterial->GetTextures()[0])
		{
			mSize = glm::vec2(mMaterial->GetTextures()[0]->GetWidth(), mMaterial->GetTextures()[0]->GetHeight());
		}

		mAlignment = ui::Middle;
		mMaterial->SetUniformValue("col", mColor);
		//AddComponent<scene::SpriteRenderer>(this);
	}

	Sprite::Sprite(glm::vec4 color)
		: Entity(), mColor(color), mSize(1.f), mPivot(0.f), mSortOrder(0)
	{
		mVA = DefaultLibraries::GetVALibrary().Get("Rect");

		const auto mat = std::make_shared<Material>(DefaultLibraries::GetShaderLibrary().Get("Rect"));
		mat->SetUniformValue("col", mColor);

		mAlignment = ui::Middle;
		mMaterial = mat;
		//AddComponent<scene::SpriteRenderer>(this);
	}

	Sprite::Sprite(std::shared_ptr<Material> mat)
		: Entity(), mMaterial(std::move(mat)), mColor(1.f), mSize(1.f), mPivot(0.f), mSortOrder(0)
	{
		if (mMaterial->GetTextures()[0])
		{
			mSize = glm::vec2(mMaterial->GetTextures()[0]->GetWidth(), mMaterial->GetTextures()[0]->GetHeight());
		}
		mMaterial->SetUniformValue("col", mColor);
		//AddComponent<scene::SpriteRenderer>(this);
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
		mMaterial = material;
	}

	void Sprite::SetColor(glm::vec4 color)
	{
		mColor = color;
		mMaterial->SetUniformValue("col", mColor);
	}

	void Sprite::SetTransform(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
	{
		Entity::SetTransform(pos, rot, scale);

		SetPivot(mAlignment);
	}

	void Sprite::SetPosition(glm::vec3 pos)
	{
		Entity::SetPosition(pos);

		SetPivot(mAlignment);
	}

	void Sprite::SetRotation(glm::vec3 rot)
	{
		Entity::SetRotation(rot);

		SetPivot(mAlignment);
	}

	void Sprite::SetScale(glm::vec3 scale)
	{
		Entity::SetScale(scale);

		SetPivot(mAlignment);
	}

	void Sprite::SetPivot(ui::Align alignment)
	{
		mAlignment = alignment;

		switch (alignment)
		{
		case ui::Align::TopLeft:
			mPivot = { (mSize.x * GetComponent<scene::Transform>().Scale.x) / 2.f,
			-(mSize.y * GetComponent<scene::Transform>().Scale.y) / 2.f };
			break;
		case ui::Align::TopMiddle:
			mPivot = { 0.f,-(mSize.y * GetComponent<scene::Transform>().Scale.y) / 2.f };
			break;
		case ui::Align::TopRight:
			mPivot = { -(mSize.x * GetComponent<scene::Transform>().Scale.x) / 2.f,
			-(mSize.y * GetComponent<scene::Transform>().Scale.y) / 2.f };
			break;
		case ui::Align::MiddleLeft:
			mPivot = { (mSize.x * GetComponent<scene::Transform>().Scale.x) / 2.f, 0.f };
			break;
		case ui::Align::Middle:
			mPivot = { 0, 0 };
			break;
		case ui::Align::MiddleRight:
			mPivot = { -(mSize.x * GetComponent<scene::Transform>().Scale.x) / 2.f, 0.f };
			break;
		case ui::Align::BottomLeft:
			mPivot = { (mSize.x * GetComponent<scene::Transform>().Scale.x) / 2.f,
			(mSize.y * GetComponent<scene::Transform>().Scale.y) / 2.f };
			break;
		case ui::Align::BottomMiddle:
			mPivot = {0.f, -(mSize.y * GetComponent<scene::Transform>().Scale.y) / 2.f };
			break;
		case ui::Align::BottomRight:
			mPivot = { -(mSize.x * GetComponent<scene::Transform>().Scale.x) / 2.f,
			(mSize.y * GetComponent<scene::Transform>().Scale.y) / 2.f };
			break;
		}
	}

	void Sprite::DrawSprite(Sprite* sprite)
	{
		PROFILE_FUNCTION();
		const std::shared_ptr<VertexArray> va = sprite->GetVA();
		const std::shared_ptr<Material> mat = sprite->GetMaterial();

		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
		auto model = glm::mat4(1.f);

		if (auto parent = sprite->Parent.lock())
		{
			position = sprite->GetComponent<scene::Transform>().LocalPosition + glm::vec3(sprite->mPivot, 0.f) + parent->GetTransform().Position;
			rotation = sprite->GetComponent<scene::Transform>().LocalRotation + parent->GetTransform().Rotation;
			scale = sprite->GetComponent<scene::Transform>().LocalScale * parent->GetTransform().Scale;
		} else
		{
			position = sprite->GetComponent<scene::Transform>().Position + glm::vec3(sprite->mPivot, 0.f);
			rotation = sprite->GetComponent<scene::Transform>().Rotation;
			scale = sprite->GetComponent<scene::Transform>().Scale;
		}

		model = glm::translate(model, position);
		// Rotations are passed as degrees and converted to radians here automatically
		model = glm::rotate(model, rotation.x * 0.0174533f, glm::vec3(1.f, 0.f, 0.f));
		model = glm::rotate(model, rotation.y * 0.0174533f, glm::vec3(0.f, 1.f, 0.f));
		model = glm::rotate(model, rotation.z * 0.0174533f, glm::vec3(0.f, 0.f, 1.f));
		model = glm::scale(model, scale);

		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, va, mat, model));
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
