#include "pch.h"
#include "based/scene/sceneserializer.h"

#include "scene/entity.h"
#include "scene/components.h"

namespace based::scene
{
	SceneSerializer::SceneSerializer(const std::shared_ptr<Scene>& scene)
		: mScene(scene)
	{
	}

	static void SerializeEntity(YAML::Emitter& out, const std::shared_ptr<Entity>& entity)
	{
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity";
		out << YAML::Value << "12837192831273"; // TODO: Entity ID goes here
		out << YAML::Key << "Name" << YAML::Value << entity->GetEntityName();

		if (entity->HasComponent<Transform>())
		{
			out << YAML::Key << "Transform"; // Transform
			out << YAML::BeginMap;

			auto trans = entity->GetComponent<Transform>();
			out << YAML::Key << "Position" << YAML::Value << trans.Position;
			out << YAML::Key << "Rotation" << YAML::Value << trans.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << trans.Scale;
			out << YAML::Key << "LocalPosition" << YAML::Value << trans.LocalPosition;
			out << YAML::Key << "LocalRotation" << YAML::Value << trans.LocalRotation;
			out << YAML::Key << "LocalScale" << YAML::Value << trans.LocalScale;

			out << YAML::EndMap; // Transform
		}

		if (entity->HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent"; // CameraComponent
			out << YAML::BeginMap;

			auto cameraComponent = entity->GetComponent<CameraComponent>();
			auto& camera = cameraComponent.camera;

			if (auto cam = camera.lock())
			{
				out << YAML::Key << "Camera" << YAML::Value;
				out << YAML::BeginMap; // Camera
				out << YAML::Key << "ProjectionType" << YAML::Value << (int)cam->GetProjectionType();
				out << YAML::Key << "PerspectiveFOV" << YAML::Value << cam->GetFOV();
				out << YAML::Key << "PerspectiveNear" << YAML::Value << cam->GetNear();
				out << YAML::Key << "PerspectiveFar" << YAML::Value << cam->GetFar();
				out << YAML::Key << "OrthographicSize" << YAML::Value << cam->GetHeight();
				out << YAML::Key << "AspectRatio" << YAML::Value << cam->GetAspectRatio();
				out << YAML::EndMap; // Camera

				out << YAML::Key << "Main" << YAML::Value << cam->main;
			}

			out << YAML::EndMap; // CameraComponent
		}

		out << YAML::Key << "Enabled" << YAML::Value << entity->IsActive(); // Enabled

		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene";
		out << YAML::Value << "Unnamed Scene";
		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		for (auto ent: mScene->GetRegistry().view<EntityReference>())
		{
			auto entity = mScene->GetRegistry().get<EntityReference>(ent).entity;
			if (auto myEntity = entity.lock()) SerializeEntity(out, myEntity);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		BASED_ASSERT(false, "Not implemented!");
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		BASED_TRACE("Deserializing scene {}", sceneName);

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				auto nameComp = entity["Name"];
				std::string name = "New Entity";
				if (nameComp) name = nameComp.as<std::string>();
				auto deserializedEntity = Entity::CreateEntity<Entity>(
					name, glm::vec3(0), 
					glm::vec3(0), glm::vec3(1)
				);

				auto trans = entity["Transform"];
				if (trans)
				{
					deserializedEntity->SetTransform(
						trans["Position"].as<glm::vec3>(),
						trans["Rotation"].as<glm::vec3>(),
						trans["Scale"].as<glm::vec3>());
					deserializedEntity->SetLocalTransform(
						trans["LocalPosition"].as<glm::vec3>(),
						trans["LocalRotation"].as<glm::vec3>(),
						trans["LocalScale"].as<glm::vec3>());
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					deserializedEntity->AddComponent<CameraComponent>(std::make_shared<graphics::Camera>());
					auto& cc = deserializedEntity->GetComponent<CameraComponent>();

					auto& cameraProps = cameraComponent["Camera"];
					if (auto cam = cc.camera.lock())
					{
						cam->SetProjection((graphics::Projection)cameraProps["ProjectionType"].as<int>());

						cam->SetFOV(cameraProps["PerspectiveFOV"].as<float>());
						cam->SetNear(cameraProps["PerspectiveNear"].as<float>());
						cam->SetFar(cameraProps["PerspectiveFar"].as<float>());

						cam->SetHeight(cameraProps["OrthographicSize"].as<float>());
						cam->SetAspectRatio(cameraProps["AspectRatio"].as<float>());

						if (cameraProps["Main"].as<bool>()) mScene->SetActiveCamera(cam);
					}
				}


				GetEntityStorage().Load(name, deserializedEntity);
			}
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		BASED_ASSERT(false, "Not implemented!");
		return false;
	}
}
