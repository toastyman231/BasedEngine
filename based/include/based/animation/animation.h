#pragma once

#include "bone.h"
#include "based/graphics/model.h"

struct aiAnimation;

namespace based::animation
{
    struct AssimpNodeData
    {
        glm::mat4 transformation;
        std::string name;
        int childrenCount;
        std::vector<AssimpNodeData> children;
    };

    class Animation
    {
    public:
        Animation() = default;
        Animation(const std::string& animationPath, const std::shared_ptr<graphics::Model>& model, int index = 0);
        Animation(const std::string& animationPath, const std::shared_ptr<graphics::Model>& model, 
            core::UUID uuid, int index = 0);
        Animation(const std::string& animationPath, const std::shared_ptr<graphics::Model>& model, const std::string& animationName);
        Animation(const std::string& animationPath, const std::shared_ptr<graphics::Model>& model, 
            core::UUID uuid, const std::string& animationName);
        ~Animation() = default;

        static std::shared_ptr<Animation> LoadAnimationFromFile(const std::string& filepath,
            core::AssetLibrary<Animation>& assetLibrary);

        Bone* FindBone(const std::string& name);

        inline float GetTicksPerSecond() const { return static_cast<float>(m_TicksPerSecond); }

        inline float GetDuration() const { return m_Duration; }

        inline int GetAnimationIndex() const { return m_AnimationIndex; }

        inline core::UUID GetUUID() const { return m_UUID; }

        std::string GetAnimationSource() const { return m_AnimationSource; }

        std::string GetAnimationFileSource() const { return m_FileSource; }

        std::string GetAnimationName() const { return m_AnimationName; }
        std::string GetSafeAnimationName() { return FixAnimationName(std::string(m_AnimationName)); }

        std::shared_ptr<graphics::Model> GetSkeleton() const { return m_Skeleton.lock(); }

        inline const AssimpNodeData& GetRootNode() { return m_RootNode; }

        inline const std::map<std::string, graphics::BoneInfo>& GetBoneIDMap()
        {
            return m_BoneInfoMap;
        }

        inline bool IsFileAnimation() const { return m_IsFileAnimation; }

        inline bool IsPlaying() const { return m_IsPlaying; }
        inline bool IsLooping() const { return m_IsLooping; }

        inline void SetPlaying(bool playing) { m_IsPlaying = playing; }
        inline void SetLooping(bool looping) { m_IsLooping = looping; }

        inline void SetPlaybackSpeed(float speed) { m_Speed = speed; }
        inline float GetPlaybackSpeed() const { return m_Speed; }

        inline void SetFileAnimation(const std::string& filepath)
        {
            m_IsFileAnimation = true;
            m_FileSource = filepath;
        }
    private:
        core::UUID m_UUID;
        bool m_IsPlaying = false;
        bool m_IsLooping = false;
        bool m_IsFileAnimation = false;
        float m_Duration;
        float m_Speed = 1.f;
        int m_TicksPerSecond;
        int m_AnimationIndex = -1;
        std::vector<Bone> m_Bones;
        AssimpNodeData m_RootNode;
        std::string m_AnimationSource;
        std::string m_FileSource;
        std::string m_AnimationName;
        std::map<std::string, graphics::BoneInfo> m_BoneInfoMap;
        std::weak_ptr<graphics::Model> m_Skeleton;

        void ReadMissingBones(const aiAnimation* animation, graphics::Model& model);
        void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);
        aiAnimation* GetAnimationByName(const aiScene* scene, const std::string& name);
        std::string FixAnimationName(std::string& name);
    };
}
