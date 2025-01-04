#pragma once

#include <string>

#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
	return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat3& m)
{
	out << YAML::BeginSeq;

	out << YAML::Flow;
	out << YAML::BeginSeq << m[0].x << m[0].y << m[0].z << YAML::EndSeq;

	out << YAML::Flow;
	out << YAML::BeginSeq << m[1].x << m[1].y << m[1].z << YAML::EndSeq;

	out << YAML::Flow;
	out << YAML::BeginSeq << m[2].x << m[2].y << m[2].z << YAML::EndSeq;

	out << YAML::EndSeq;

	return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat4& m)
{
	out << YAML::BeginSeq;

	out << YAML::Flow;
	out << YAML::BeginSeq << m[0].x << m[0].y << m[0].z << m[0].w << YAML::EndSeq;

	out << YAML::Flow;
	out << YAML::BeginSeq << m[1].x << m[1].y << m[1].z << m[1].w << YAML::EndSeq;

	out << YAML::Flow;
	out << YAML::BeginSeq << m[2].x << m[2].y << m[2].z << m[2].w << YAML::EndSeq;

	out << YAML::Flow;
	out << YAML::BeginSeq << m[3].x << m[3].y << m[3].z << m[3].w << YAML::EndSeq;

	out << YAML::EndSeq;

	return out;
}

namespace based::core {
	class Serializer {
	public:
		virtual ~Serializer() = default;

		virtual void Serialize(const std::string& filepath) = 0;
		virtual void SerializeRuntime(const std::string& filepath) = 0;

		virtual bool Deserialize(const std::string& filepath) = 0;
		virtual bool DeserializeRuntime(const std::string& filepath) = 0;
	};
}

namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::mat3>
	{
		static Node encode(const glm::mat3& rhs)
		{
			Node node;
			node.push_back(rhs[0].x);
			node.push_back(rhs[0].y);
			node.push_back(rhs[0].z);

			node.push_back(rhs[1].x);
			node.push_back(rhs[1].y);
			node.push_back(rhs[1].z);

			node.push_back(rhs[2].x);
			node.push_back(rhs[2].y);
			node.push_back(rhs[2].z);
			return node;
		}

		static bool decode(const Node& node, glm::mat3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs[0].x = node[0].as<float>();
			rhs[0].y = node[1].as<float>();
			rhs[0].z = node[2].as<float>();

			rhs[1].x = node[3].as<float>();
			rhs[1].y = node[4].as<float>();
			rhs[1].z = node[5].as<float>();

			rhs[2].x = node[6].as<float>();
			rhs[2].y = node[7].as<float>();
			rhs[2].z = node[8].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::mat4>
	{
		static Node encode(const glm::mat4& rhs)
		{
			Node node;
			node.push_back(rhs[0].x);
			node.push_back(rhs[0].y);
			node.push_back(rhs[0].z);
			node.push_back(rhs[0].w);

			node.push_back(rhs[1].x);
			node.push_back(rhs[1].y);
			node.push_back(rhs[1].z);
			node.push_back(rhs[1].w);

			node.push_back(rhs[2].x);
			node.push_back(rhs[2].y);
			node.push_back(rhs[2].z);
			node.push_back(rhs[2].w);

			return node;
		}

		static bool decode(const Node& node, glm::mat4& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs[0].x = node[0].as<float>();
			rhs[0].y = node[1].as<float>();
			rhs[0].z = node[2].as<float>();
			rhs[0].w = node[3].as<float>();

			rhs[1].x = node[4].as<float>();
			rhs[1].y = node[5].as<float>();
			rhs[1].z = node[6].as<float>();
			rhs[1].w = node[7].as<float>();

			rhs[2].x = node[8].as<float>();
			rhs[2].y = node[9].as<float>();
			rhs[2].z = node[10].as<float>();
			rhs[2].w = node[11].as<float>();
			return true;
		}
	};
}