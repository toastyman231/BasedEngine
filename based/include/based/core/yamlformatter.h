#pragma once
#include "yaml-cpp/emitter.h"

#define SERIALIZE_SIMPLE_TYPE(T)                                    \
if (type.id() == entt::type_hash<T>()) {                            \
	object.allow_cast<T>();                                         \
	out << YAML::Key << "Value" << YAML::Value << object.cast<T>(); \
	out << YAML::Key << "Type" << YAML::Value << type.id();         \
} else

#define DESERIALIZE_SIMPLE_TYPE(T)                     \
if (memberTypeId == entt::type_hash<T>()) {            \
	auto value = memberInfo.second["Value"].as<T>();   \
	member.set(object, value);                         \
} else

namespace based::core::YAMLFormatter
{
	inline void SerializeMetaType(YAML::Emitter& out, entt::meta_type type, entt::meta_any object)
	{
		if (type.is_class() && type.id() != entt::type_hash<std::string>())
		{
			for (auto baseType : type.base())
			{
				SerializeMetaType(out, baseType.second, object);
			}

			for (auto& [id, member] : type.data())
			{
				auto field = member.get(object);
				out << YAML::Key << id << YAML::BeginMap;
				if (member.type().is_class() && member.type().id() != entt::type_hash<std::string>())
				{
					out << YAML::Key << "ChildObject" << YAML::BeginMap;
					out << YAML::Key << "ObjectType" << YAML::Value << member.type().id();
					out << YAML::Key << "Data" << YAML::BeginMap;
				}
				SerializeMetaType(out, member.type(), field);
				if (member.type().is_class() && member.type().id() != entt::type_hash<std::string>())
					out << YAML::EndMap << YAML::EndMap;
				out << YAML::EndMap;
			}

			return;
		}

		if (type.is_enum())
		{
			object.allow_cast<int>();
			out << YAML::Key << "Value" << YAML::Value << object.cast<int>();
			out << YAML::Key << "Type" << YAML::Value << type.id();
			return;
		}

		SERIALIZE_SIMPLE_TYPE(std::string)
		SERIALIZE_SIMPLE_TYPE(int)
		SERIALIZE_SIMPLE_TYPE(unsigned int)
		SERIALIZE_SIMPLE_TYPE(float)
		SERIALIZE_SIMPLE_TYPE(char)
		SERIALIZE_SIMPLE_TYPE(unsigned char)
		SERIALIZE_SIMPLE_TYPE(short)
		SERIALIZE_SIMPLE_TYPE(unsigned short)
		SERIALIZE_SIMPLE_TYPE(double)
		SERIALIZE_SIMPLE_TYPE(bool)
		SERIALIZE_SIMPLE_TYPE(long)
		SERIALIZE_SIMPLE_TYPE(unsigned long)
		SERIALIZE_SIMPLE_TYPE(long long)
		SERIALIZE_SIMPLE_TYPE(unsigned long long)
		{
			BASED_WARN("Type {} cannot be serialized!", type.id());
		}
	}

	template <typename T>
	bool Serialize(YAML::Emitter& out, T object)
	{
		auto type = entt::resolve(entt::type_hash<T>());
		if (!type)
		{
			BASED_ERROR("Type could not be resolved, did you reflect it properly?");
			return false;
		}

		auto metaObject = entt::meta_any{ object };

		if (type.is_class() && type.id() != entt::type_hash<std::string>())
		{
			out << YAML::Key << "ObjectType" << YAML::Value << type.id();

			out << YAML::Key << "Data" << YAML::BeginMap; // Data

			SerializeMetaType(out, type, metaObject);

			out << YAML::EndMap; // Data
		}
		else SerializeMetaType(out, type, metaObject);

		return true;
	}

	inline bool Serialize(YAML::Emitter& out, entt::meta_any object)
	{
		auto type = object.type();
		if (!type)
		{
			BASED_ERROR("Type could not be resolved, did you reflect it properly?");
			return false;
		}

		if (type.is_class() && type.id() != entt::type_hash<std::string>())
		{
			out << YAML::Key << "ObjectType" << YAML::Value << type.id();

			out << YAML::Key << "Data" << YAML::BeginMap; // Data

			SerializeMetaType(out, type, object);

			out << YAML::EndMap; // Data
		}
		else SerializeMetaType(out, type, object);

		return true;
	}

	inline void DeserializeMetaType(YAML::Node data, entt::meta_type type, entt::meta_any& object)
	{
		auto membersData = data["Data"];
		for (auto memberInfo : membersData)
		{
			auto memberId = memberInfo.first.as<unsigned>();

			if (auto childNode = memberInfo.second["ChildObject"])
			{
				auto childTypeId = childNode["ObjectType"].as<unsigned>();
				auto childType = entt::resolve(childTypeId);
				BASED_ASSERT(childType, "Child object type could not be deserialized, did you reflect it?");
				auto child = childType.construct();
				DeserializeMetaType(childNode, childType, child);
				object.set(memberId, child);
				continue;
			}

			auto memberTypeId = memberInfo.second["Type"].as<unsigned>();
			auto memberType = entt::resolve(memberTypeId);

			BASED_ASSERT(memberType, "Member type could not be deserialized, did you reflect it?");

			auto member = type.data(memberId);

			if (member.type().is_enum())
			{
				auto enumValue = memberInfo.second["Value"].as<int>();
				auto metaEnum = entt::meta_any{ enumValue };
				member.set(object, metaEnum);
				continue;
			}

			DESERIALIZE_SIMPLE_TYPE(std::string)
			DESERIALIZE_SIMPLE_TYPE(int)
			DESERIALIZE_SIMPLE_TYPE(unsigned int)
			DESERIALIZE_SIMPLE_TYPE(float)
			DESERIALIZE_SIMPLE_TYPE(char)
			DESERIALIZE_SIMPLE_TYPE(unsigned char)
			DESERIALIZE_SIMPLE_TYPE(short)
			DESERIALIZE_SIMPLE_TYPE(unsigned short)
			DESERIALIZE_SIMPLE_TYPE(double)
			DESERIALIZE_SIMPLE_TYPE(bool)
			DESERIALIZE_SIMPLE_TYPE(long)
			DESERIALIZE_SIMPLE_TYPE(unsigned long)
			DESERIALIZE_SIMPLE_TYPE(long long)
			DESERIALIZE_SIMPLE_TYPE(unsigned long long)
			{
				BASED_WARN("Type {} cannot be deserialized!", type.id());
			}
		}
	}

	template <typename T>
	T Deserialize(YAML::Node data)
	{
		auto typeId = data["ObjectType"].as<unsigned>();

		auto type = entt::resolve(typeId);

		BASED_ASSERT(type, "Type could not be deserialized, did you reflect it?");

		auto object = type.construct();

		DeserializeMetaType(data, type, object);

		object.allow_cast<T>();
		return object.cast<T>();
	}

	inline entt::meta_any Deserialize(YAML::Node data)
	{
		auto typeId = data["ObjectType"].as<unsigned>();

		auto type = entt::resolve(typeId);

		BASED_ASSERT(type, "Type could not be deserialized, did you reflect it?");

		auto object = type.construct();

		DeserializeMetaType(data, type, object);

		return object;
	}
}
