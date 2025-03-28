#include "pch.h"
#include <based/animation/animation.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace based::animation
{
	Animation::Animation(const std::string& animationPath, const std::shared_ptr<graphics::Model>& model, int index)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
		assert(scene && scene->mRootNode);
		const auto animation = scene->mAnimations[index];
		m_Duration = static_cast<float>(animation->mDuration);
		m_TicksPerSecond = static_cast<int>(animation->mTicksPerSecond);
		m_Skeleton = model;
		m_AnimationIndex = index;
		m_UUID = core::UUID();
		m_AnimationSource = animationPath;
		m_AnimationName = animation->mName.C_Str();//FixAnimationName(std::string(animation->mName.C_Str()));
		ReadHierarchyData(m_RootNode, scene->mRootNode);
		ReadMissingBones(animation, *model);
	}

	Animation::Animation(const std::string& animationPath, const std::shared_ptr<graphics::Model>& model,
		core::UUID uuid, int index)
		: Animation(animationPath, model, index)
	{
		m_UUID = uuid;
	}

	Animation::Animation(const std::string& animationPath, const std::shared_ptr<graphics::Model>& model,
		core::UUID uuid, const std::string& animationName)
		: Animation(animationPath, model, animationName)
	{
		m_UUID = uuid;
	}

	std::shared_ptr<Animation> Animation::LoadAnimationFromFile(const std::string& filepath,
		core::AssetLibrary<Animation>& assetLibrary, const std::string& filePrefix)
	{
		scene::SceneSerializer serializer(Engine::Instance().GetApp().GetCurrentScene());
		auto anim = serializer.DeserializeAnimation(filePrefix + filepath);
		anim->m_IsFileAnimation = true;
		anim->m_FileSource = filepath;
		assetLibrary.Load(anim->GetAnimationName(), anim);

		return anim;
	}

	std::shared_ptr<Animation> Animation::LoadAnimationFromFileAbsolute(const std::string& filepath,
		core::AssetLibrary<Animation>& assetLibrary, const std::string& filePrefix)
	{
		scene::SceneSerializer serializer(Engine::Instance().GetApp().GetCurrentScene());
		serializer.SetProjectDirectory(filePrefix);
		auto anim = serializer.DeserializeAnimation(filepath);
		anim->m_IsFileAnimation = true;
		anim->m_FileSource = filepath;
		assetLibrary.Load(anim->GetAnimationName(), anim);

		return anim;
	}

	Animation::Animation(const std::string& animationPath, const std::shared_ptr<graphics::Model>& model, 
	                     const std::string& animationName)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
		assert(scene && scene->mRootNode);
		const auto animation = GetAnimationByName(scene, animationName);
		BASED_ASSERT(animation, "Could not find animation");
		if (animation)
		{
			m_Duration = static_cast<float>(animation->mDuration);
			m_TicksPerSecond = static_cast<int>(animation->mTicksPerSecond);
			m_Skeleton = model;
			m_AnimationName = animation->mName.C_Str();//FixAnimationName(std::string(animationName));
			m_UUID = core::UUID();
			m_AnimationSource = animationPath;
			ReadHierarchyData(m_RootNode, scene->mRootNode);
			ReadMissingBones(animation, *model);
		}
	}

	Bone* Animation::FindBone(const std::string& name)
	{
		auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
			[&](const Bone& Bone)
			{
				return Bone.GetBoneName() == name;
			}
		);
		if (iter == m_Bones.end()) return nullptr;
		else return &(*iter);
	}

	void Animation::ReadMissingBones(const aiAnimation* animation, graphics::Model& model)
	{
		int size = animation->mNumChannels;

		auto& boneInfoMap = model.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
		int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

		//reading channels(bones engaged in an animation and their keyframes)
		for (int i = 0; i < size; i++)
		{
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;

			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				boneInfoMap[boneName].id = boneCount;
				boneCount++;
			}
			m_Bones.emplace_back(channel->mNodeName.data,
			                     boneInfoMap[channel->mNodeName.data].id, channel);
		}

		m_BoneInfoMap = boneInfoMap;
	}

	void Animation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
	{
		assert(src);

		dest.name = src->mName.data;
		dest.transformation = graphics::AssimpGLMHelpers::ConvertMatrixToGLMFormat(&src->mTransformation);
		dest.childrenCount = src->mNumChildren;

		for (int i = 0; i < static_cast<int>(src->mNumChildren); i++)
		{
			AssimpNodeData newData;
			ReadHierarchyData(newData, src->mChildren[i]);
			dest.children.push_back(newData);
		}
	}

	aiAnimation* Animation::GetAnimationByName(const aiScene* scene, const std::string& name)
	{
		for (int i = 0; i < static_cast<int>(scene->mNumAnimations); ++i)
		{
			aiAnimation* animation = scene->mAnimations[i];
			if (std::string(animation->mName.C_Str()) == std::string(name))
			{
				return animation;
			}
		}
		BASED_WARN("Could not find animation {}", name);
		return nullptr;
	}

	std::string Animation::FixAnimationName(std::string& name)
	{
		size_t pos = 0;
		while ((pos = name.find("|", pos)) != std::string::npos)
		{
			name.replace(pos, strlen("|"), "-");
			pos += strlen("-");
		}

		return name;
	}
}
