#pragma once
#include <string>
#include <vector>

#include "bone.h"
#include "external/glm/fwd.hpp"
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
        Animation(const std::string& animationPath, graphics::Model* model);
        ~Animation() {}

        Bone* FindBone(const std::string& name);

        inline float GetTicksPerSecond() { return static_cast<float>(m_TicksPerSecond); }

        inline float GetDuration() { return m_Duration; }

        inline const AssimpNodeData& GetRootNode() { return m_RootNode; }

        inline const std::map<std::string, graphics::BoneInfo>& GetBoneIDMap()
        {
            return m_BoneInfoMap;
        }

        inline bool IsPlaying() const { return m_IsPlaying; }
        inline bool IsLooping() const { return m_IsLooping; }

        inline void SetPlaying(bool playing) { m_IsPlaying = playing; }
        inline void SetLooping(bool looping) { m_IsLooping = looping; }
    private:
        bool m_IsPlaying = false;
        bool m_IsLooping = false;
        float m_Duration;
        int m_TicksPerSecond;
        std::vector<Bone> m_Bones;
        AssimpNodeData m_RootNode;
        std::map<std::string, graphics::BoneInfo> m_BoneInfoMap;

        void ReadMissingBones(const aiAnimation* animation, graphics::Model& model);
        void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);
    };
}
