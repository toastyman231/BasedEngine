#include <utility>

#include "graphics/sprite.h"

#include "external/glm/gtx/transform.hpp"
#include "graphics/texture.h"

namespace based::graphics
{
	Sprite::Sprite(std::shared_ptr<VertexArray> va, std::shared_ptr<Material> mat)
		: Entity(), mVA(std::move(va)), mMaterial(std::move(mat)), mSize(0.f, 0.f)
	{
		if (mMaterial->GetTexture())
		{
			mSize = glm::vec2(mMaterial->GetTexture()->GetWidth(), mMaterial->GetTexture()->GetHeight());
		}
		AddComponent<scene::SpriteRenderer>(this);
	}

	void Sprite::SetSprite(std::shared_ptr<Texture> texture)
	{
		mMaterial->SetTexture(std::move(texture));
		mSize = glm::vec2(mMaterial->GetTexture()->GetWidth(), mMaterial->GetTexture()->GetHeight());
		RegenerateVA();
	}

	void Sprite::SetShader(std::shared_ptr<Shader> shader) const
	{
		mMaterial->SetShader(std::move(shader));
	}

	void Sprite::DrawSprite(Sprite* sprite)
	{
		const std::shared_ptr<VertexArray> va = sprite->GetVA();
		const std::shared_ptr<Material> mat = sprite->GetMaterial();
		const glm::vec3 rotation = sprite->GetComponent<scene::Transform>().Rotation;
		auto model = glm::mat4(1.f);
		model = glm::translate(model, sprite->GetComponent<scene::Transform>().Position);
		// Rotations are passed as degrees and converted to radians here automatically
		model = glm::rotate(model, rotation.x * 0.0174533f, glm::vec3(1.f, 0.f, 0.f));
		model = glm::rotate(model, rotation.y * 0.0174533f, glm::vec3(0.f, 1.f, 0.f));
		model = glm::rotate(model, rotation.z * 0.0174533f, glm::vec3(0.f, 0.f, 1.f));
		model = glm::scale(model, sprite->GetComponent<scene::Transform>().Scale);
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, va, mat, model));
	}

	void Sprite::RegenerateVA()
	{
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
