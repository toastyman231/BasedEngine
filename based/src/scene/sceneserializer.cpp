#include "pch.h"
#include "based/scene/sceneserializer.h"

#include "animation/animator.h"
#include "graphics/mesh.h"
#include "graphics/model.h"
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
		std::size_t pos = input.find("BasedEngine/Assets");
		if (pos != std::string::npos) return input.substr(pos + std::string("BasedEngine/Assets").length() + 1);
		return input;
	}

	void SceneSerializer::SerializeMaterial(YAML::Emitter& out, const std::shared_ptr<graphics::Material>& material)
	{
		out << YAML::BeginMap; // Material
		out << YAML::Key << "Material" << YAML::Value << material->mMaterialName;
		out << YAML::Key << "ID" << YAML::Value << material->GetUUID();

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
			int index = 0;
			for (const auto& tex : material->GetTextures())
			{
				out << YAML::Key << tex->GetName();
				out << YAML::BeginMap; // Texture
				out << YAML::Key << "Location" << YAML::Value << material->GetTextureLocationByIndex(index);
				out << YAML::Key << "ID" << YAML::Value << tex->GetUUID();
				out << YAML::Key << "Path" << YAML::Value << tex->GetPath();
				out << YAML::EndMap; // Texture

				index++;
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

		if (data["VertexIsEngineAsset"].as<bool>()) vertexSource = ASSET_PATH(vSrc.as<std::string>());
		else vertexSource = vSrc.as<std::string>();
		if (data["FragmentIsEngineAsset"].as<bool>()) fragmentSource = ASSET_PATH(fSrc.as<std::string>());
		else fragmentSource = fSrc.as<std::string>();

		auto shader = LOAD_SHADER(vertexSource, fragmentSource);

		auto id = data["ID"] ? data["ID"].as<uint64_t>() : core::UUID();
		auto material = std::make_shared<graphics::Material>(shader, id, materialName);
		mScene->GetMaterialStorage().Load(materialName, material, true);

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

		mScene->GetMaterialStorage().Load(materialName, material, true);
		return material;
	}

	void SceneSerializer::SerializeAnimation(YAML::Emitter& out, const std::shared_ptr<animation::Animation>& animation)
	{
		out << YAML::BeginMap << YAML::Key << "Animation" << YAML::BeginMap; // Animation

		out << YAML::Key << "ID" << YAML::Value << animation->GetUUID();
		out << YAML::Key << "Path" << YAML::Value << animation->GetAnimationSource();
		out << YAML::Key << "Skeleton" << YAML::Value << animation->GetSkeleton()->GetUUID();

		if (animation->GetAnimationIndex() >= 0)
		{
			out << YAML::Key << "Index" << YAML::Value << animation->GetAnimationIndex();
		} else
		{
			out << YAML::Key << "Name" << YAML::Value << animation->GetAnimationName();
		}

		out << YAML::Key << "PlaybackSpeed" << YAML::Value << animation->GetPlaybackSpeed();
		out << YAML::Key << "Looping" << YAML::Value << animation->IsLooping();

		out << YAML::EndMap << YAML::EndMap; // Animation
	}

	std::shared_ptr<animation::Animation> SceneSerializer::DeserializeAnimation(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Animation"])
		{
			BASED_ERROR("Could not load animation at {}", filepath);
			return nullptr;
		}

		auto& animation = data["Animation"];

		auto animId = animation["ID"].as<uint64_t>();
		auto animSource = animation["Path"].as<std::string>();
		auto skeletonId = animation["Skeleton"].as<uint64_t>();

		bool useIndex = false;
		int index;
		std::string animName;

		if (auto& i = animation["Index"])
		{
			useIndex = true;
			index = i.as<int>();
		} else if (auto& n = animation["Name"])
		{
			useIndex = false;
			animName = n.as<std::string>();
		} else
		{
			BASED_ERROR("Could not find index or name of animation to load from {}!", filepath);
		}

		std::shared_ptr<graphics::Model> model;
		if (auto& m = mLoadedModels[skeletonId]) model = m;
		else
		{
			for (auto& pair : mScene->GetModelStorage().GetAll())
			{
				if (pair.second->GetUUID() == skeletonId)
				{
					model = pair.second;
					break;
				}
			}

			if (model == nullptr)
			{
				BASED_ERROR("Could not find model with ID {} when loading animation at {}!", skeletonId, filepath);
				return nullptr;
			}
		}

		std::shared_ptr<animation::Animation> anim;
		if (useIndex)
		{
			anim = std::make_shared<animation::Animation>(animSource, model, index, animId);
		} else
		{
			anim = std::make_shared<animation::Animation>(animSource, model, animId, animName);
		}

		if (auto& speed = animation["PlaybackSpeed"])
		{
			anim->SetPlaybackSpeed(speed.as<float>());
		}

		if (auto& looping = animation["Looping"])
		{
			anim->SetLooping(looping.as<bool>());
		}

		return anim;
	}

	void SceneSerializer::SerializeEntity(YAML::Emitter& out, const std::shared_ptr<Entity>& entity)
	{
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity";
		out << YAML::Value << entity->GetUUID();
		out << YAML::Key << "Name" << YAML::Value << entity->GetEntityName();
		if (auto parent = entity->Parent.lock()) 
			out << YAML::Key << "Parent" << YAML::Value << parent->GetUUID();

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

		if (entity->HasComponent<MeshRenderer>())
		{
			auto meshRenderer = entity->GetComponent<MeshRenderer>();
			auto mesh = meshRenderer.mesh;

			if (auto m = mesh.lock())
			{
				out << YAML::Key << "MeshRenderer" << YAML::BeginMap; // MeshRenderer

				if (!m->GetMeshSource().empty())
				{
					out << YAML::Key << "Mesh" << YAML::BeginMap; // Mesh
					out << YAML::Key << "ID" << YAML::Value << m->GetUUID();
					auto shortPath = ShortenPath(m->GetMeshSource());
					out << YAML::Key << "Path" << YAML::Value << shortPath;
					out << YAML::Key << "IsEngineAsset" << YAML::Value << (m->GetMeshSource() != shortPath);
					out << YAML::EndMap; // Mesh
				} else
				{
					out << YAML::Key << "Mesh" << YAML::BeginMap; // Mesh
					out << YAML::Key << "ID" << YAML::Value << m->GetUUID();

					CreateDirectoryIfNotExists("Assets/GENERATED/Meshes");

					std::string pathname = std::string("Assets/GENERATED/Meshes/")
						.append("Gen_Mesh_")
						.append(std::to_string(CountFilesInDir("Assets/GENERATED/Meshes")))
						.append(".bin");

					std::ofstream ofs(pathname, std::ios::binary);

					size_t size = m->vertices.size();
					ofs.write(reinterpret_cast<const char*>(&size), sizeof(size_t)); // Write vertex size
					ofs.write(reinterpret_cast<const char*>(m->vertices.data()), 
						size * sizeof(graphics::Vertex)); // Write vertices
					size = m->indices.size();
					ofs.write(reinterpret_cast<const char*>(&size), sizeof(size_t)); // Write index size
					ofs.write(reinterpret_cast<const char*>(m->indices.data()),
						size * sizeof(unsigned int)); // Write indices

					out << YAML::Key << "Path" << YAML::Value << pathname;
					out << YAML::Key << "IsBinary" << YAML::Value << true;
					out << YAML::EndMap; // Mesh
				}

				out << YAML::Key << "Material" << YAML::BeginMap; // Material

				std::string shortPath;
				if (!m->material->IsFileMaterial() || m->material->GetMaterialSource().empty())
				{
					SceneSerializer matSerializer(mScene);
					YAML::Emitter matOut;
					matSerializer.SerializeMaterial(matOut, m->material);

					CreateDirectoryIfNotExists("Assets/Materials");

					shortPath = std::string("Assets/Materials/")
						.append(m->material->mMaterialName)
						.append(".bmat");
					std::ofstream fout(shortPath);
					fout << matOut.c_str();
				}
				else shortPath = m->material->GetMaterialSource();
				out << YAML::Key << "ID" << YAML::Value << m->material->GetUUID();
				auto shortenedPath = ShortenPath(shortPath);
				out << YAML::Key << "Path" << YAML::Value << shortenedPath;
				out << YAML::Key << "IsEngineAsset" << YAML::Value << (shortenedPath != shortPath);
				out << YAML::EndMap; // Material

				if (auto instanceMesh =
					std::dynamic_pointer_cast<graphics::InstancedMesh>(m))
				{
					// Write instance data as binary for faster speeds
					CreateDirectoryIfNotExists("Assets/GENERATED/Transforms");

					std::string pathname = std::string("Assets/GENERATED/Transforms/")
						.append("transforms_")
						.append(std::to_string(CountFilesInDir("Assets/GENERATED/Transforms")))
						.append(".bin");
					auto size = instanceMesh->GetInstanceTransforms().size();

					std::ofstream ofs(pathname, std::ios::binary);
					ofs.write(reinterpret_cast<const char*>(
						instanceMesh->GetInstanceTransforms().data()),
						size * sizeof(Transform));

					out << YAML::Key << "Instances" << YAML::BeginMap; // Instances
					out << YAML::Key << "Path" << YAML::Value << pathname;
					out << YAML::Key << "Size" << YAML::Value << (int)size;
					out << YAML::EndMap; // Instances
				}

				out << YAML::EndMap; // MeshRenderer
			}
		}

		if (entity->HasComponent<ModelRenderer>())
		{
			auto& modelRenderer = entity->GetComponent<ModelRenderer>();
			auto& model = modelRenderer.model;

			if (auto& modelPtr = model.lock())
			{
				out << YAML::Key << "ModelRenderer" << YAML::BeginMap; // ModelRenderer

				if (!modelPtr->GetModelSource().empty())
				{
					out << YAML::Key << "Model" << YAML::BeginMap; // Model
					out << YAML::Key << "Name" << YAML::Value << modelPtr->GetModelName();
					out << YAML::Key << "ID" << YAML::Value << modelPtr->GetUUID();
					auto shortPath = ShortenPath(modelPtr->GetModelSource());
					out << YAML::Key << "Path" << YAML::Value << shortPath;
					out << YAML::Key << "IsEngineAsset" << YAML::Value << (modelPtr->GetModelSource() != shortPath);
					out << YAML::EndMap; // Model
				}

				out << YAML::Key << "Materials" << YAML::BeginSeq; // Materials

				for (auto& material : modelPtr->GetMaterials())
				{
					out << YAML::BeginMap << YAML::Key << "Material" << YAML::BeginMap; // Material

					std::string shortPath;
					if (!material->IsFileMaterial() || material->GetMaterialSource().empty())
					{
						SceneSerializer matSerializer(mScene);
						YAML::Emitter matOut;
						matSerializer.SerializeMaterial(matOut, material);

						CreateDirectoryIfNotExists("Assets/Materials");

						shortPath = std::string("Assets/Materials/")
							.append(material->mMaterialName)
							.append(".bmat");
						std::ofstream fout(shortPath);
						fout << matOut.c_str();
					}
					else shortPath = material->GetMaterialSource();
					out << YAML::Key << "ID" << YAML::Value << material->GetUUID();
					auto shortenedPath = ShortenPath(shortPath);
					out << YAML::Key << "Path" << YAML::Value << shortenedPath;
					out << YAML::Key << "IsEngineAsset" << YAML::Value << (shortenedPath != shortPath);
					out << YAML::EndMap << YAML::EndMap; // Material
				}

				out << YAML::EndSeq; // Materials

				out << YAML::EndMap; // ModelRenderer
			}
		}

		if (entity->HasComponent<AnimatorComponent>())
		{
			auto& animatorComponent = entity->GetComponent<AnimatorComponent>();
			auto& animator = animatorComponent.animator;

			if (auto animatorPtr = animator.lock())
			{
				//animatorPtr->
			}
		}

		if (entity->HasComponent<PointLight>())
		{
			auto& pointLight = entity->GetComponent<PointLight>();

			out << YAML::Key << "PointLight" << YAML::BeginMap; // Point Light

			out << YAML::Key << "Color" << YAML::Value << pointLight.color;
			out << YAML::Key << "Constant" << YAML::Value << pointLight.constant;
			out << YAML::Key << "Intensity" << YAML::Value << pointLight.intensity;
			out << YAML::Key << "Linear" << YAML::Value << pointLight.linear;
			out << YAML::Key << "Quadratic" << YAML::Value << pointLight.quadratic;

			out << YAML::EndMap; // Point Light
		}

		if (entity->HasComponent<DirectionalLight>())
		{
			auto& pointLight = entity->GetComponent<DirectionalLight>();

			out << YAML::Key << "DirectionalLight" << YAML::BeginMap; // Directional Light

			out << YAML::Key << "Color" << YAML::Value << pointLight.color;
			out << YAML::Key << "Direction" << YAML::Value << pointLight.direction;

			out << YAML::EndMap; // Directional Light
		}

		out << YAML::Key << "Enabled" << YAML::Value << entity->IsActive(); // Enabled

		out << YAML::EndMap; // Entity
	}

	bool SceneSerializer::CreateDirectoryIfNotExists(const std::string& filepath)
	{
		if (std::filesystem::is_directory(filepath)) return false;
		else
		{
			std::filesystem::create_directories(filepath);
			return true;
		}
	}

	int SceneSerializer::CountFilesInDir(const std::string& filepath)
	{
		auto dirIter = std::filesystem::directory_iterator(filepath);

		return static_cast<int>(std::count_if(
			begin(dirIter),
			end(dirIter),
			[](auto& entry) { return entry.is_regular_file(); }
		));
	}

	void SceneSerializer::DeserializeEntity(YAML::detail::iterator_value entity)
	{
		auto& nameComp = entity["Name"];
		std::string name = "New Entity";
		if (nameComp) name = nameComp.as<std::string>();

		uint64_t uuid = entity["Entity"].as<uint64_t>();

		auto deserializedEntity = Entity::CreateEntityWithUUID(name, uuid);

		if (auto& parent = entity["Parent"])
		{
			parentMap[deserializedEntity->GetUUID()] = parent.as<uint64_t>();
		}

		if (auto& trans = entity["Transform"])
		{
			deserializedEntity->SetTransform(
				trans["Position"].as<glm::vec3>(),
				trans["Rotation"].as<glm::vec3>(),
				trans["Scale"].as<glm::vec3>());
		}

		if (auto& cameraComponent = entity["CameraComponent"])
		{
			auto newCam = std::make_shared<graphics::Camera>();
			deserializedEntity->AddComponent<CameraComponent>(newCam);
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

				if (cameraComponent["Main"].as<bool>()) mScene->SetActiveCamera(cam);
			}
		}

		if (auto& meshRenderer = entity["MeshRenderer"])
		{
			auto& materialId = meshRenderer["Material"]["ID"];
			std::shared_ptr<graphics::Material> material;
			if (materialId)
			{
				auto id = materialId.as<uint64_t>();
				if (mLoadedMaterials.find(id) != mLoadedMaterials.end())
					material = mLoadedMaterials[id];
				else
				{
					material = graphics::Material::LoadMaterialFromFile(
						meshRenderer["Material"]["Path"].as<std::string>(),
						mScene->GetMaterialStorage());
					mLoadedMaterials[material->GetUUID()] = material;
				}
			}
			else
			{
				material = graphics::Material::CreateMaterial(
					LOAD_SHADER(ASSET_PATH("Shaders/basic_lit.vert"), ASSET_PATH("Shaders/basic_lit.frag")),
					mScene->GetMaterialStorage()
				);
			}

			auto meshId = meshRenderer["Mesh"]["ID"].as<uint64_t>();
			std::shared_ptr<graphics::Mesh> mesh;
			if (mLoadedMeshes.find(meshId) != mLoadedMeshes.end())
			{
				mesh = mLoadedMeshes[meshId];
			}
			else
			{
				if (auto& isBinary = meshRenderer["Mesh"]["IsBinary"])
				{
					std::vector<graphics::Vertex> vertices;
					std::vector<unsigned int> indices;

					std::ifstream ifs(meshRenderer["Mesh"]["Path"].as<std::string>(), std::ios::binary);
					size_t size;
					ifs.read(reinterpret_cast<char*>(&size), sizeof(size_t)); // Read vertex size
					vertices.resize(size);
					ifs.read(reinterpret_cast<char*>(vertices.data()), size * sizeof(graphics::Vertex)); // Read vertices
					ifs.read(reinterpret_cast<char*>(&size), sizeof(size_t)); // Read index size
					indices.resize(size);
					ifs.read(reinterpret_cast<char*>(indices.data()), size * sizeof(unsigned int)); // Read vertices

					mesh = graphics::Mesh::CreateMesh(vertices, indices, {}, 
						mScene->GetMeshStorage(), meshId, name);
				} else
				{
					auto path = meshRenderer["Mesh"]["IsEngineAsset"].as<bool>() ?
						ASSET_PATH(meshRenderer["Mesh"]["Path"].as<std::string>()) :
						meshRenderer["Mesh"]["Path"].as<std::string>();
					mesh = graphics::Mesh::LoadMeshWithUUID(path,
						mScene->GetMeshStorage(), meshId);
				}

				mLoadedMeshes[meshId] = mesh;
			}
			mesh->material = material;

			std::shared_ptr<graphics::InstancedMesh> instanceMesh;
			if (auto& instances = meshRenderer["Instances"])
			{
				instanceMesh = graphics::Mesh::CreateInstancedMesh(mesh, mScene->GetMeshStorage());
				instanceMesh->material = material;
				std::ifstream ifs(instances["Path"].as<std::string>(), std::ios::binary);
				auto size = instances["Size"].as<int>();
				std::vector<Transform> transforms(size);
				ifs.read(reinterpret_cast<char*>(transforms.data()), size * sizeof(Transform));
				instanceMesh->AddInstances(transforms);
			}

			if (instanceMesh) deserializedEntity->AddComponent<MeshRenderer>(instanceMesh);
			else deserializedEntity->AddComponent<MeshRenderer>(mesh);
		}

		if (auto& modelRenderer = entity["ModelRenderer"])
		{
			std::vector<std::shared_ptr<graphics::Material>> modelMaterials;
			if (auto& materials = modelRenderer["Materials"])
			{
				for (auto& m : materials)
				{
					auto& material = m["Material"];
					auto& materialId = material["ID"];
					std::shared_ptr<graphics::Material> mat;
					if (materialId)
					{
						auto id = materialId.as<uint64_t>();
						if (mLoadedMaterials.find(id) != mLoadedMaterials.end())
							mat = mLoadedMaterials[id];
						else
						{
							mat = graphics::Material::LoadMaterialFromFile(
								material["Path"].as<std::string>(),
								mScene->GetMaterialStorage());
							mLoadedMaterials[mat->GetUUID()] = mat;
						}
					}
					else
					{
						mat = graphics::Material::CreateMaterial(
							LOAD_SHADER(ASSET_PATH("Shaders/basic_lit.vert"), ASSET_PATH("Shaders/basic_lit.frag")),
							mScene->GetMaterialStorage()
						);
					}

					// Set default bone matrices, in case there's no Animator component to do it
					for (int i = 0; i < 100; ++i)
						mat->SetUniformValue("finalBonesMatrices[" + std::to_string(i) + "]", glm::mat4(1.f));

					modelMaterials.push_back(mat);
				}
			}

			auto modelId = modelRenderer["Model"]["ID"].as<uint64_t>();
			std::shared_ptr<graphics::Model> model;
			if (mLoadedMeshes.find(modelId) != mLoadedMeshes.end())
			{
				model = mLoadedModels[modelId];
			} else
			{
				auto path = modelRenderer["Model"]["IsEngineAsset"].as<bool>() ?
					ASSET_PATH(modelRenderer["Model"]["Path"].as<std::string>()) :
					modelRenderer["Model"]["Path"].as<std::string>();
				model = graphics::Model::CreateModelWithUUID(path, 
					mScene->GetModelStorage(), modelRenderer["Model"]["Name"].as<std::string>(), 
					modelId);
				mLoadedModels[modelId] = model;
			}

			model->SetMaterials(modelMaterials);

			deserializedEntity->AddComponent<ModelRenderer>(model);
		}

		if (auto& pointLight = entity["PointLight"])
		{
			glm::vec3 color = pointLight["Color"].as<glm::vec3>();
			float constant = pointLight["Constant"].as<float>();
			float intensity = pointLight["Intensity"].as<float>();
			float linear = pointLight["Linear"].as<float>();
			float quadratic = pointLight["Quadratic"].as<float>();

			deserializedEntity->AddComponent<PointLight>(constant, linear, quadratic, intensity, color);
		}

		if (auto& dirLight = entity["DirectionalLight"])
		{
			glm::vec3 color = dirLight["Color"].as<glm::vec3>();
			glm::vec3 direction = dirLight["Direction"].as<glm::vec3>();

			deserializedEntity->AddComponent<DirectionalLight>(direction, color);
		}

		if (auto& enabled = entity["Enabled"]) deserializedEntity->SetActive(enabled.as<bool>());
		else deserializedEntity->SetActive(false);

		mScene->GetEntityStorage().Load(name, deserializedEntity);
		mLoadedEntities[deserializedEntity->GetUUID()] = deserializedEntity;
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

		CreateDirectoryIfNotExists(filepath.substr(0, filepath.find_last_of("/")));

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
				DeserializeEntity(entity);
			}

			for (const auto& parentPair : parentMap)
			{
				mLoadedEntities[parentPair.first]->SetParent(mLoadedEntities[parentPair.second]);
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
