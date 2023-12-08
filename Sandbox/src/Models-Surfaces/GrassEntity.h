#pragma once
#include <utility>

#include "based/graphics/mesh.h"
#include "based/scene/entity.h"

class GrassEntity: public based::scene::Entity
{
public:
	GrassEntity(based::graphics::Mesh* mesh, std::shared_ptr<based::graphics::Material> mat) : Entity()
	{
		mMesh = mesh;
		mMaterial = std::move(mat);
	}

	void Initialize() override
	{
		mMesh->material = mMaterial;
		mMesh->material->SetUniformValue("randomLean", based::math::RandomRange(0.f, 0.6f));
		AddComponent<based::scene::MeshRenderer>(mMesh);
	}

	void Update(float deltaTime) override
	{
		
	}

	void Shutdown() override
	{
		
	}

	void UpdateShaders(glm::vec3 lightColor, glm::vec3 lightPosition, bool useLight, float ambientStrength)
	{
		mMesh->material->SetUniformValue("lightColor", lightColor);

		mMesh->material->SetUniformValue("lightPos", lightPosition);

		mMesh->material->SetUniformValue("ambientStrength", ambientStrength);

		if (!useLight)
		{
			mMesh->material->SetUniformValue("ambientStrength", 1.f);
		}
	}

private:
	based::graphics::Mesh* mMesh;
	std::shared_ptr<based::graphics::Material> mMaterial;
};