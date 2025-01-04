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

	std::string ShortenPath(const std::string& input)
	{
		std::size_t pos = input.find("BasedEngine/Assets") + std::string("BasedEngine/Assets").length() + 1;
		if (pos != std::string::npos) return input.substr(pos);
		return input;
	}

	void SceneSerializer::SerializeMaterial(YAML::Emitter& out, const std::shared_ptr<graphics::Material>& material)
	{
		out << YAML::BeginMap; // Material
		out << YAML::Key << "Material" << YAML::Value << material->mMaterialName;

		if (auto shader = material->GetShader().lock())
		{
			auto vSrc = ShortenPath(shader->GetVertexPath());
			out << YAML::Key << "VertexSource" << YAML::Value << vSrc;
			out << YAML::Key << "VertexIsEngineAsset" << YAML::Value << (vSrc != shader->GetVertexPath());
			auto fSrc = ShortenPath(shader->GetFragmentPath());
			out << YAML::Key << "FragmentSource" << YAML::Value << fSrc;
			out << YAML::Key << "FragmentIsEngineAsset" << YAML::Value << (fSrc != shader->GetFragmentPath());

			out << YAML::Key << "UniformInts"; // Ints
			out << YAML::BeginMap;

			for (const auto& uniform : material->GetUniformInts())
			{
				out << YAML::Key << uniform.first << YAML::Value << uniform.second;
			}
			out << YAML::EndMap; // Ints

			out << YAML::Key << "UniformFloats"; // Floats
			out << YAML::BeginMap;

			for (const auto& uniform : material->GetUniformFloats())
			{
				out << YAML::Key << uniform.first << YAML::Value << uniform.second;
			}
			out << YAML::EndMap; // Floats

			out << YAML::Key << "UniformFloat2s"; // Float2s
			out << YAML::BeginMap;

			for (const auto& uniform : material->GetUniformFloat2s())
			{
				out << YAML::Key << uniform.first << YAML::Value << uniform.second;
			}
			out << YAML::EndMap; // Float2s

			out << YAML::Key << "UniformFloat3s"; // Float3s
			out << YAML::BeginMap;

			for (const auto& uniform : material->GetUniformFloat3s())
			{
				out << YAML::Key << uniform.first << YAML::Value << uniform.second;
			}
			out << YAML::EndMap; // Float3s

			out << YAML::Key << "UniformFloat4s"; // Float4s
			out << YAML::BeginMap;

			for (const auto& uniform : material->GetUniformFloat4s())
			{
				out << YAML::Key << uniform.first << YAML::Value << uniform.second;
			}
			out << YAML::EndMap; // Float4s

			out << YAML::Key << "UniformMat3s"; // Mat3s
			out << YAML::BeginMap;

			for (const auto& uniform : material->GetUniformMat3s())
			{
				out << YAML::Key << uniform.first << YAML::Value << uniform.second;
			}
			out << YAML::EndMap; // Mat3s

			out << YAML::Key << "UniformMat4s"; // Mat4s
			out << YAML::BeginMap;

			for (const auto& uniform : material->GetUniformMat4s())
			{
				out << YAML::Key << uniform.first << YAML::Value << uniform.second;
			}
			out << YAML::EndMap; // Mat4s

			out << YAML::Key << "Textures" << YAML::BeginMap; // Textures
			for (const auto& texInfo : material->GetTextureOrder())
			{
				const auto& loc = texInfo.first;
				const auto& index = texInfo.second;

				if (auto& tex = material->GetTextures()[index])
				{
					out << YAML::Key << tex->GetName();
					out << YAML::BeginMap; // Texture
					out << YAML::Key << "Location" << YAML::Value << loc;
					out << YAML::Key << "ID" << YAML::Value << tex->GetUUID();
					out << YAML::Key << "Path" << YAML::Value << tex->GetPath();
					out << YAML::EndMap; // Texture
				}
			}
			out << YAML::EndMap; // Textures
		}

		out << YAML::EndMap; // Material
	}

	std::shared_ptr<graphics::Material> SceneSerializer::DeserializeMaterial(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Material"])
			return nullptr;

		std::string materialName = data["Material"].as<std::string>();
		BASED_TRACE("Deserializing material {}", materialName);

		auto vSrc = data["VertexSource"];
		if (!vSrc) return nullptr;
		auto fSrc = data["FragmentSource"];
		if (!fSrc) return nullptr;

		std::string vertexSource;
		std::string fragmentSource;

		if (data["VertexIsEngineAsset"]) vertexSource = ASSET_PATH(vSrc.as<std::string>());
		else vertexSource = vSrc.as<std::string>();
		if (data["FragmentIsEngineAsset"]) fragmentSource = ASSET_PATH(fSrc.as<std::string>());
		else fragmentSource = fSrc.as<std::string>();

		auto shader = LOAD_SHADER(vertexSource, fragmentSource);

		auto material = graphics::Material::CreateMaterial(
			shader, mScene->GetMaterialStorage(), materialName
		);

		if (auto uniformInts = data["UniformInts"])
		{
			for (const auto& uniform : uniformInts)
			{
				material->SetUniformValue(uniform.first.as<std::string>(), uniform.second.as<int>());
			}
		}

		if (auto uniformFloats = data["UniformFloats"])
		{
			for (const auto& uniform : uniformFloats)
			{
				material->SetUniformValue(uniform.first.as<std::string>(), uniform.second.as<float>());
			}
		}

		if (auto uniformFloat2s = data["UniformFloat2s"])
		{
			for (const auto& uniform : uniformFloat2s)
			{
				material->SetUniformValue(uniform.first.as<std::string>(), uniform.second.as<glm::vec2>());
			}
		}

		if (auto uniformFloat3s = data["UniformFloat3s"])
		{
			for (const auto& uniform : uniformFloat3s)
			{
				material->SetUniformValue(uniform.first.as<std::string>(), uniform.second.as<glm::vec3>());
			}
		}

		if (auto uniformFloat4s = data["UniformFloat4s"])
		{
			for (const auto& uniform : uniformFloat4s)
			{
				material->SetUniformValue(uniform.first.as<std::string>(), uniform.second.as<glm::vec4>());
			}
		}

		if (auto uniformMat3s = data["UniformMat3s"])
		{
			for (const auto& uniform : uniformMat3s)
			{
				material->SetUniformValue(uniform.first.as<std::string>(), uniform.second.as<glm::mat3>());
			}
		}

		if (auto uniformMat4s = data["UniformMat4s"])
		{
			for (const auto& uniform : uniformMat4s)
			{
				material->SetUniformValue(uniform.first.as<std::string>(), uniform.second.as<glm::mat4>());
			}
		}

		if (auto textures = data["Textures"])
		{
			for (const auto& texture : textures)
			{
				auto id = texture.second["ID"].as<uint64_t>();
				if (mLoadedTextures.find(id) != mLoadedTextures.end())
				{
					material->AddTexture(mLoadedTextures[id], texture.second["Location"].as<std::string>());
					continue;
				}

				auto tex = std::make_shared<graphics::Texture>(texture.second["Path"].as<std::string>(), true);
				tex->SetName(texture.first.as<std::string>());
				mLoadedTextures[id] = tex;
				mScene->GetTextureStorage().Load(tex->GetName(), tex);
				material->AddTexture(tex, texture.first.as<std::string>());
			}
		}

		mScene->GetMaterialStorage().Load(materialName, material);
		return material;
	}

	static void SerializeEntity(YAML::Emitter& out, const std::shared_ptr<Entity>& entity)
	{
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity";
		out << YAML::Value << entity->GetUUID();
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

		if (auto entities = data["Entities"])
		{
			for (auto entity : entities)
			{
				auto& nameComp = entity["Name"];
				std::string name = "New Entity";
				if (nameComp) name = nameComp.as<std::string>();

				uint64_t uuid = entity["Entity"].as<uint64_t>();

				auto deserializedEntity = Entity::CreateEntityWithUUID(name, uuid);

				if (auto& trans = entity["Transform"])
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

				if (auto& cameraComponent = entity["CameraComponent"])
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

				if (auto& enabled = entity["Enabled"]) deserializedEntity->SetActive(enabled.as<bool>());
				else deserializedEntity->SetActive(false);

				mScene->GetEntityStorage().Load(name, deserializedEntity);
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
