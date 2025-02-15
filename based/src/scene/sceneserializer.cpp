#include "pch.h"
#include "based/scene/sceneserializer.h"

#include "yamlformatter.h"
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
		{
			BASED_ERROR("No valid material data found at {}", filepath);
			return nullptr;
		}

		std::string materialName = data["Material"].as<std::string>();
		BASED_TRACE("Deserializing material {}", materialName);

		auto vSrc = data["VertexSource"];
		if (!vSrc)
		{
			BASED_ERROR("No valid material vertex source found!");
			return nullptr;
		}
		auto fSrc = data["FragmentSource"];
		if (!fSrc)
		{
			BASED_ERROR("No valid material fragment source found!");
			return nullptr;
		}

		std::string vertexSource;
		std::string fragmentSource;

		if (data["VertexIsEngineAsset"].as<bool>()) vertexSource = ASSET_PATH(vSrc.as<std::string>());
		else vertexSource = mProjectDirectory + vSrc.as<std::string>();
		if (data["FragmentIsEngineAsset"].as<bool>()) fragmentSource = ASSET_PATH(fSrc.as<std::string>());
		else fragmentSource = mProjectDirectory + fSrc.as<std::string>();

		auto shader = LOAD_SHADER(vertexSource, fragmentSource);

		auto id = data["ID"] ? data["ID"].as<uint64_t>() : core::UUID();
		auto material = std::make_shared<graphics::Material>(shader, id, materialName);
		if (mScene) mScene->GetMaterialStorage().Load(materialName, material, true);

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

				auto path = texture.second["Path"].as<std::string>();
				if (texture.second["IsEngineAsset"] && texture.second["IsEngineAsset"].as<bool>())
					path = ASSET_PATH(path);
				else
					path = mProjectDirectory + path;
				auto tex = std::make_shared<graphics::Texture>(path, true);
				tex->SetName(texture.first.as<std::string>());
				mLoadedTextures[id] = tex;
				if (mScene) mScene->GetTextureStorage().Load(tex->GetName(), tex);
				material->AddTexture(tex, texture.first.as<std::string>());
			}
		}

		if (mScene) mScene->GetMaterialStorage().Load(materialName, material, true);
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
		auto animSource = mProjectDirectory + animation["Path"].as<std::string>();
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
			anim = std::make_shared<animation::Animation>(animSource, model, animId, index);
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

	void SceneSerializer::SerializeStateMachine(YAML::Emitter& out,
		const std::shared_ptr<animation::AnimationStateMachine> stateMachine)
	{
		out << YAML::Key << "StateMachine" << YAML::BeginMap; // StateMachine

		out << YAML::Key << "FloatParams" << YAML::BeginMap; // Floats
		for (auto& floatParam : stateMachine->GetFloatParams())
		{
			out << YAML::Key << floatParam.first << YAML::Value << floatParam.second;
		}
		out << YAML::EndMap; // Floats

		out << YAML::Key << "IntParams" << YAML::BeginMap; // Ints
		for (auto& intParam : stateMachine->GetIntParams())
		{
			out << YAML::Key << intParam.first << YAML::Value << intParam.second;
		}
		out << YAML::EndMap; // Ints

		out << YAML::Key << "BoolParams" << YAML::BeginMap; // Bools
		for (auto& boolParam : stateMachine->GetBoolParams())
		{
			out << YAML::Key << boolParam.first << YAML::Value << boolParam.second;
		}
		out << YAML::EndMap; // Bools

		out << YAML::Key << "StringParams" << YAML::BeginMap; // Strings
		for (auto& stringParam : stateMachine->GetStringParams())
		{
			out << YAML::Key << stringParam.first << YAML::Value << stringParam.second;
		}
		out << YAML::EndMap; // Strings

		out << YAML::Key << "States" << YAML::BeginSeq; // States

		for (auto& state : stateMachine->GetStates())
		{
			out << YAML::BeginMap << YAML::Key << "State" << YAML::BeginMap; // State
			out << YAML::Key << "Name" << YAML::Value << state->GetStateName();

			auto& currentAnimation = state->GetStateAnimationClip();
			std::string path;
			if (!currentAnimation->IsFileAnimation())
			{
				SceneSerializer serializer(mScene);
				YAML::Emitter animOut;
				serializer.SerializeAnimation(animOut, currentAnimation);

				CreateDirectoryIfNotExists("Assets/GENERATED/Animations");

				path = std::string("Assets/GENERATED/Animations/")
					.append(currentAnimation->GetSafeAnimationName())
					.append(".banim");
				std::ofstream ofs(path);
				ofs << animOut.c_str();

				currentAnimation->SetFileAnimation(path);
			}
			else path = currentAnimation->GetAnimationFileSource();

			out << YAML::Key << "Animation" << YAML::BeginMap; // Animation
			out << YAML::Key << "ID" << currentAnimation->GetUUID();
			out << YAML::Key << "Path" << path;
			out << YAML::EndMap; // Animation

			auto stateIndex = [stateMachine](const std::shared_ptr<animation::AnimationState>& state) -> int
				{
					if (!state) return -1;
					auto& states = stateMachine->GetStates();
					auto it = std::find(states.cbegin(),
						states.cend(), state);
					if (it != states.cend())
					{
						return static_cast<int>(std::distance(states.cbegin(), it));
					}
					return -1;
				};

			out << YAML::Key << "Transitions" << YAML::BeginSeq; // Transitions
			for (auto& transition : state->GetTransitions())
			{
				out << YAML::BeginMap << YAML::Key << "Transition" << YAML::BeginMap; // Transition
				out << YAML::Key << "Source" << YAML::Value << stateIndex(transition->GetSourceState().lock());
				out << YAML::Key << "Destination" << YAML::Value << stateIndex(transition->GetDestinationState().lock());

				out << YAML::Key << "TransitionRules" << YAML::BeginMap; // TransitionRules

				auto& transitionRules = transition->GetTransitionRules();

				out << YAML::Key << "FloatParams" << YAML::BeginMap; // Floats
				for (auto& rule : transitionRules.floatRules)
				{
					out << YAML::Key << "FloatRule" << YAML::BeginMap; // Rule
					out << YAML::Key << "Name" << YAML::Value << rule.name;
					out << YAML::Key << "Value" << YAML::Value << rule.val;
					out << YAML::Key << "Default" << YAML::Value << rule.defaultVal;
					out << YAML::EndMap; // Rule
				}
				out << YAML::EndMap; // Floats

				out << YAML::Key << "IntParams" << YAML::BeginMap; // Ints
				for (auto& rule : transitionRules.intRules)
				{
					out << YAML::Key << "IntRule" << YAML::BeginMap; // Rule
					out << YAML::Key << "Name" << YAML::Value << rule.name;
					out << YAML::Key << "Value" << YAML::Value << rule.val;
					out << YAML::Key << "Default" << YAML::Value << rule.defaultVal;
					out << YAML::EndMap; // Rule
				}
				out << YAML::EndMap; // Ints

				out << YAML::Key << "BoolParams" << YAML::BeginSeq; // Bools
				for (auto& rule : transitionRules.boolRules)
				{
					out << YAML::BeginMap << YAML::Key << "BoolRule" << YAML::BeginMap; // Rule
					out << YAML::Key << "Name" << YAML::Value << rule.name;
					out << YAML::Key << "Value" << YAML::Value << rule.val;
					out << YAML::Key << "Default" << YAML::Value << rule.defaultVal;
					out << YAML::EndMap << YAML::EndMap; // Rule
				}
				out << YAML::EndSeq; // Bools

				out << YAML::Key << "StringParams" << YAML::BeginMap; // Strings
				for (auto& rule : transitionRules.stringRules)
				{
					out << YAML::Key << "StringRule" << YAML::BeginMap; // Rule
					out << YAML::Key << "Name" << YAML::Value << rule.name;
					out << YAML::Key << "Value" << YAML::Value << rule.val;
					out << YAML::Key << "Default" << YAML::Value << rule.defaultVal;
					out << YAML::EndMap; // Rule
				}
				out << YAML::EndMap; // Strings

				out << YAML::EndMap; // TransitionRules

				out << YAML::Key << "AutoReset" << YAML::Value << transition->ShouldAutoTransition();

				out << YAML::EndMap << YAML::EndMap;
			}
			out << YAML::EndSeq; // Transitions

			out << YAML::Key << "IsDefault" << YAML::Value << (state == stateMachine->GetDefaultState());

			out << YAML::EndMap << YAML::EndMap; // State
		}

		out << YAML::EndSeq; // States

		out << YAML::EndMap; // StateMachine
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

					CreateDirectoryIfNotExists("Assets/GENERATED/Materials");

					shortPath = std::string("Assets/GENERATED/Materials/")
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

						CreateDirectoryIfNotExists("Assets/GENERATED/Materials");

						shortPath = std::string("Assets/GENERATED/Materials/")
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
				out << YAML::Key << "AnimatorComponent" << YAML::BeginMap; // AnimatorComponent

				auto currentAnimation = animatorPtr->GetCurrentAnimation();

				if (currentAnimation)
				{
					std::string path;
					if (!currentAnimation->IsFileAnimation())
					{
						SceneSerializer serializer(mScene);
						YAML::Emitter animOut;
						serializer.SerializeAnimation(animOut, currentAnimation);

						CreateDirectoryIfNotExists("Assets/GENERATED/Animations");

						path = std::string("Assets/GENERATED/Animations/")
							.append(currentAnimation->GetSafeAnimationName())
							.append(".banim");
						std::ofstream ofs(path);
						ofs << animOut.c_str();

						currentAnimation->SetFileAnimation(path);
					}
					else path = currentAnimation->GetAnimationFileSource();

					out << YAML::Key << "Animation" << YAML::BeginMap; // Animation

					out << YAML::Key << "ID" << YAML::Value << currentAnimation->GetUUID();
					out << YAML::Key << "Path" << YAML::Value << path;

					out << YAML::EndMap; // Animation

					if (auto& stateMachine = animatorPtr->GetStateMachine())
					{
						SerializeStateMachine(out, stateMachine);
					}

					out << YAML::Key << "TimeMode" << YAML::Value << (int)animatorPtr->GetTimeMode();
				}

				out << YAML::EndMap; // AnimatorComponent;
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
			auto& directionalLight = entity->GetComponent<DirectionalLight>();

			out << YAML::Key << "DirectionalLight" << YAML::BeginMap; // Directional Light

			out << YAML::Key << "Color" << YAML::Value << directionalLight.color;
			out << YAML::Key << "Direction" << YAML::Value << directionalLight.direction;
			out << YAML::Key << "Intensity" << YAML::Value << directionalLight.intensity;

			out << YAML::EndMap; // Directional Light
		}

		for (auto&& curr : mScene->GetRegistry().storage())
		{
			using namespace entt::literals;

			if (auto& storage = curr.second; storage.contains(entity->GetEntityHandle()))
			{
				auto type = entt::resolve(storage.type().hash());
				if (!type)
				{
					//BASED_WARN("Cannot serialize component with type: {}", storage.type().hash());
					continue;
				}

				for (auto baseType : type.base())
				{
					if (baseType.second.id() == entt::type_hash<ScriptComponent>())
					{
						auto typeName = std::string(type.info().name());
						typeName = typeName.substr(typeName.find(" ") + 1);
						out << YAML::Key << typeName << YAML::BeginMap;
						auto component = static_cast<ScriptComponent*>(storage.get(entity->GetEntityHandle()));

						type.func("SerializeScriptComponent"_hs).invoke(*component, &out, component);

						out << YAML::EndMap;
						break;
					}
				}
			}
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
			if (cameraComponent["Main"] && cameraComponent["Main"].as<bool>())
			{
				mScene->GetCameraStorage().Load("MainCamera", newCam);
			} else
			{
				mScene->GetCameraStorage().Load("Camera", newCam);
			}

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
					auto materialPath = meshRenderer["Material"]["Path"].as<std::string>();
					auto truePath = materialPath;
					if (meshRenderer["Material"]["IsEngineAsset"]
						&& meshRenderer["Material"]["IsEngineAsset"].as<bool>())
						truePath = ASSET_PATH(materialPath);
					else truePath = mProjectDirectory + materialPath;
					material = DeserializeMaterial(truePath);
					BASED_ASSERT(material != nullptr, "Material is null!");
					material->SetMaterialSource(materialPath);
						/*graphics::Material::LoadMaterialFromFile(
						materialPath,
						mScene->GetMaterialStorage(),
						mProjectDirectory);*/
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

					std::ifstream ifs(mProjectDirectory + meshRenderer["Mesh"]["Path"].as<std::string>(), std::ios::binary);
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
						mProjectDirectory + meshRenderer["Mesh"]["Path"].as<std::string>();
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
				std::ifstream ifs(mProjectDirectory + instances["Path"].as<std::string>(), std::ios::binary);
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
						auto materialPath = material["Path"].as<std::string>();

						if (mLoadedMaterials.find(id) != mLoadedMaterials.end())
							mat = mLoadedMaterials[id];
						else
						{
							auto truePath = materialPath;
							if (material["IsEngineAsset"]
								&& material["IsEngineAsset"].as<bool>())
								truePath = ASSET_PATH(materialPath);
							else truePath = mProjectDirectory + materialPath;
							mat = DeserializeMaterial(truePath);
							BASED_ASSERT(mat != nullptr, "Material is null!");
							mat->SetMaterialSource(materialPath);
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
					mProjectDirectory + modelRenderer["Model"]["Path"].as<std::string>();
				model = graphics::Model::CreateModelWithUUID(path, 
					mScene->GetModelStorage(), modelRenderer["Model"]["Name"].as<std::string>(), 
					modelId);
				mLoadedModels[modelId] = model;
			}

			model->SetMaterials(modelMaterials);

			deserializedEntity->AddComponent<ModelRenderer>(model);
		}

		if (auto& animatorComponent = entity["AnimatorComponent"])
		{
			if (auto& animation = animatorComponent["Animation"])
			{
				std::shared_ptr<animation::Animation> anim;
				core::UUID animId = animation["ID"].as<uint64_t>();
				auto path = animation["Path"].as<std::string>();

				if (mLoadedAnimations.find(animId) != mLoadedAnimations.end())
					anim = mLoadedAnimations[animId];
				else
				{
					anim = DeserializeAnimation(mProjectDirectory + path);
					anim->SetFileAnimation(path);
					mScene->GetAnimationStorage().Load(anim->GetAnimationName(), anim);
					/*animation::Animation::LoadAnimationFromFile(mProjectDirectory + path,
						mScene->GetAnimationStorage());*/
					BASED_ASSERT(anim, "Loaded animation is not valid!");
					mLoadedAnimations[animId] = anim;
				}

				auto animator = std::make_shared<animation::Animator>(anim);
				mScene->GetAnimatorStorage().Load("Animator", animator);

				if (auto& timeMode = animatorComponent["TimeMode"])
					animator->SetTimeMode((animation::TimeMode)timeMode.as<int>());

				if (auto& sm = animatorComponent["StateMachine"])
				{
					auto stateMachine = std::make_shared<animation::AnimationStateMachine>(animator);

					if (auto& fp = sm["FloatParams"])
					{
						for (auto& floatParam : fp)
						{
							stateMachine->SetFloat(floatParam.first.as<std::string>(), 
								floatParam.second.as<float>());
						}
					}

					if (auto& ip = sm["IntParams"])
					{
						for (auto& intParam : ip)
						{
							stateMachine->SetInteger(intParam.first.as<std::string>(),
								intParam.second.as<int>());
						}
					}

					if (auto& bp = sm["BoolParams"])
					{
						for (auto& boolParam : bp)
						{
							stateMachine->SetBool(boolParam.first.as<std::string>(),
								boolParam.second.as<bool>());
						}
					}

					if (auto& sp = sm["StringParams"])
					{
						for (auto& stringParam : sp)
						{
							stateMachine->SetString(stringParam.first.as<std::string>(),
								stringParam.second.as<std::string>());
						}
					}

					if (auto& states = sm["States"])
					{
						for (auto& stateIter : states)
						{
							auto& state = stateIter["State"];
							std::string stateName = state["Name"].as<std::string>();

							auto& animNode = state["Animation"];
							std::shared_ptr<animation::Animation> anim;
							core::UUID animId = animNode["ID"].as<uint64_t>();
							auto path = animNode["Path"].as<std::string>();

							if (mLoadedAnimations.find(animId) != mLoadedAnimations.end())
								anim = mLoadedAnimations[animId];
							else
							{
								anim = DeserializeAnimation(mProjectDirectory + path);
								anim->SetFileAnimation(path);
								mScene->GetAnimationStorage().Load(anim->GetAnimationName(), anim);
								/*animation::Animation::LoadAnimationFromFile(mProjectDirectory + path,
									mScene->GetAnimationStorage());*/
								BASED_ASSERT(anim, "Loaded animation is not valid!");
								mLoadedAnimations[animId] = anim;
							}

							auto animState = std::make_shared<animation::AnimationState>(anim, stateName);
							auto isDefault = state["IsDefault"].as<bool>();
							stateMachine->AddState(animState, isDefault);

							if (auto& transitions = state["Transitions"])
							{
								for (auto& transitionIter : transitions)
								{
									auto& transition = transitionIter["Transition"];
									auto source = transition["Source"].as<int>();
									auto destination = transition["Destination"].as<int>();
									
									auto rules = new animation::TransitionRules();

									auto& transitionRules = transition["TransitionRules"];
									if (auto& fp = transitionRules["FloatParams"])
									{
										for (auto& floatParamIter : fp)
										{
											auto& floatParam = floatParamIter["FloatRule"];
											rules->floatRules.emplace_back(
												animation::TransitionRule<float>
												{
												floatParam["Name"].as<std::string>(),
													floatParam["Value"].as<float>(),
													floatParam["Default"].as<float>()
												}
											);
										}
									}

									if (auto& ip = transitionRules["IntParams"])
									{
										for (auto& intParamIter : ip)
										{
											auto& intParam = intParamIter["IntRule"];
											rules->intRules.emplace_back(
												animation::TransitionRule<int>
											{
												intParam["Name"].as<std::string>(),
													intParam["Value"].as<int>(),
													intParam["Default"].as<int>()
											}
											);
										}
									}

									if (auto& bp = transitionRules["BoolParams"])
									{
										for (auto& boolParamIter : bp)
										{
											auto& boolParam = boolParamIter["BoolRule"];
											rules->boolRules.emplace_back(
												animation::TransitionRule<bool>
											{
												boolParam["Name"].as<std::string>(),
													boolParam["Value"].as<bool>(),
													boolParam["Default"].as<bool>()
											}
											);
										}
									}

									if (auto& sp = transitionRules["StringParams"])
									{
										for (auto& stringParamIter : sp)
										{
											auto& stringParam = stringParamIter["StringRule"];
											rules->stringRules.emplace_back(
												animation::TransitionRule<std::string>
											{
												stringParam["Name"].as<std::string>(),
													stringParam["Value"].as<std::string>(),
													stringParam["Default"].as<std::string>()
											}
											);
										}
									}

									mStoredTransitions.emplace_back(source, destination, rules,
										transition["AutoReset"].as<bool>());
								}
							}
						}

						for (auto& transitionData : mStoredTransitions)
						{
							auto animTransition = std::make_shared<animation::AnimationTransition>(
								stateMachine->GetStates()[transitionData.source], 
								stateMachine->GetStates()[transitionData.destination], 
								animator, stateMachine, *transitionData.rules,
								transitionData.autoReset
							);
							delete transitionData.rules;
							transitionData.rules = nullptr;
							stateMachine->GetStates()[transitionData.source]->AddTransition(animTransition);
						}

						animator->SetStateMachine(stateMachine);
					}
				}

				deserializedEntity->AddComponent<AnimatorComponent>(animator);
			}
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
			float intensity = 1.f;
			if (auto intensityData = dirLight["Intensity"])
			{
				intensity = intensityData.as<float>();
			}

			deserializedEntity->AddComponent<DirectionalLight>(direction, color, intensity);
		}

		for (auto& component : entity)
		{
			if (!component.second.IsMap()) continue;

			auto componentNode = component.second;

			if (!componentNode["ObjectType"]) continue;

			auto comp = core::YAMLFormatter::Deserialize(componentNode);

			using namespace entt::literals;
			auto compTypeId = componentNode["ObjectType"].as<unsigned>();
			auto compType = entt::resolve(compTypeId);
			BASED_ASSERT(compType, "Invalid component type!");

			auto compFunc = compType.func("AddMetaComponent"_hs);
			BASED_ASSERT(compFunc, "Invalid function!");

			compFunc.invoke({}, 
				deserializedEntity.get(), mScene.get(), &comp);
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
