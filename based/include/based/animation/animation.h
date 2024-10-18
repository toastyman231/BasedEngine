#pragma once

#include "bone.h"
#include "based/graphics/model.h"

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
        Animation(const std::string& animationPath, const std::shared_ptr<graphics::Model>& model, const std::string& animationName);
        ~Animation() = default;

        Bone* FindBone(const std::string& name);

        inline float GetTicksPerSecond() const { return static_cast<float>(m_TicksPerSecond); }

        inline float GetDuration() const { return m_Duration; }

        inline const AssimpNodeData& GetRootNode() { return m_RootNode; }

        inline const std::map<std::string, graphics::BoneInfo>& GetBoneIDMap()
        {
            return m_BoneInfoMap;
        }

        inline bool IsPlaying() const { return m_IsPlaying; }
        inline bool IsLooping() const { return m_IsLooping; }

        inline void SetPlaying(bool playing) { m_IsPlaying = playing; }
        inline void SetLooping(bool looping) { m_IsLooping = looping; }

        inline void SetPlaybackSpeed(float speed) { m_Speed = speed; }
        inline float GetPlaybackSpeed() const { return m_Speed; }
    private:
        bool m_IsPlaying = false;
        bool m_IsLooping = false;
        float m_Duration;
        float m_Speed = 1.f;
        int m_TicksPerSecond;
        std::vector<Bone> m_Bones;
        AssimpNodeData m_RootNode;
        std::map<std::string, graphics::BoneInfo> m_BoneInfoMap;

        void ReadMissingBones(const aiAnimation* animation, graphics::Model& model);
        void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);
        aiAnimation* GetAnimationByName(const aiScene* scene, const std::string& name);
    };
}
