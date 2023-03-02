#pragma once
#include "based/scene/entity.h"

class TestEntity : public based::scene::Entity
{
public:
	TestEntity() : Entity() { TestEntity::OnStart(); }
	~TestEntity() override { Entity::~Entity(); }

	void OnEnable() override
	{
		BASED_TRACE("Enabled test entity!");
	}

	void OnDisable() override
	{
		BASED_TRACE("Disabled test entity!");
	}

	void OnStart() override
	{
		BASED_TRACE("I just started!");
	}

	void Initialize() override
	{
		BASED_TRACE("I am initializing!");
	}

	void Update() override
	{
		//BASED_TRACE("I am updating!")
	}

	void OnDestroy() override
	{
		BASED_TRACE("I just got destroyed!");
	}

	void Shutdown() override
	{
		BASED_TRACE("I am shutting down!");
	}
};
