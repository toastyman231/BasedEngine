#include "pch.h"
#include "core/gameplaytags.h"

namespace based::core
{
	Tag::Tag(const std::string& tag)
	{
		auto iter = tag.find_first_of('.');

		if (iter == std::string::npos)
		{
			mTag = tag;
			mSubTag = nullptr;
		}
		else
		{
			auto first = tag.substr(0, iter);
			auto second = tag.substr(iter + 1);

			mTag = first;
			mSubTag = new Tag(second);
		}
	}

	Tag::Tag(const Tag& other)
	{
		mTag = other.mTag;
		if (other.mSubTag)
			mSubTag = new Tag(*other.mSubTag);
	}

	Tag::~Tag()
	{
		delete mSubTag;
	}

	bool Tag::MatchesTag(const Tag* other, bool exact) const
	{
		if (!other) return false;
		if (exact) return this == other;

		bool subtagMatches;
		if ((mSubTag && !other->mSubTag) || (!mSubTag && !other->mSubTag)) subtagMatches = true;
		else if (!mSubTag && other->mSubTag) subtagMatches = false;
		else subtagMatches = mSubTag->MatchesTag(other->mSubTag, exact);

		return mTag == other->mTag && subtagMatches;
	}

	bool Tag::MatchesAny(const std::vector<Tag*>& tags, bool exact) const
	{
		for (const auto& tag : tags)
		{
			if (MatchesTag(tag, exact)) return true;
		}

		return false;
	}

	bool Tag::MatchesAll(const std::vector<Tag*>& tags, bool exact) const
	{
		for (const auto& tag : tags)
		{
			if (!MatchesTag(tag, exact)) return false;
		}

		return true;
	}

	bool TagContainer::AddTag(const Tag& tag)
	{
		if (HasTag(tag, true) || tag.Empty()) return false;

		mTags.emplace_back(tag);

		return true;
	}

	bool TagContainer::AddTags(const std::vector<Tag>& tags)
	{
		bool result = false;
		for (const auto& tag : tags)
		{
			if (AddTag(tag)) result = true;
		}

		return result;
	}

	bool TagContainer::RemoveTag(const Tag& tag)
	{
		if (!HasTag(tag, true)) return false;

		mTags.erase(std::find(mTags.begin(), mTags.end(), tag));

		return true;
	}

	bool TagContainer::RemoveTags(const std::vector<Tag>& tags)
	{
		bool result = false;
		for (const auto& tag : tags)
		{
			if (RemoveTag(tag)) result = true;
		}

		return result;
	}

	bool TagContainer::HasTag(const Tag& tag, bool exact) const
	{
		if (tag.Empty()) return false;

		if (exact) return std::find(mTags.begin(), mTags.end(), tag) != mTags.end();

		for (const auto& t : mTags)
		{
			if (t.MatchesTag(&tag, exact)) return true;
		}

		return false;
	}

	bool TagContainer::HasAllTags(const std::vector<Tag>& tags, bool exact) const
	{
		if (tags.empty()) return true;

		for (const auto& t : tags)
		{
			if (!HasTag(t, exact)) return false;
		}

		return true;
	}

	bool TagContainer::HasAnyTag(const std::vector<Tag>& tags, bool exact) const
	{
		if (tags.empty()) return false;

		for (const auto& t : tags)
		{
			if (HasTag(t, exact)) return true;
		}

		return false;
	}

	bool TagContainer::Empty() const
	{
		return mTags.empty();
	}

	size_t TagContainer::Size() const
	{
		return mTags.size();
	}
}
