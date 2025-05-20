#pragma once

namespace based::core
{
	// Based on Unreal Engine Gameplay Tags

	struct Tag
	{
		// Constructors
		Tag(const std::string& tag);
		Tag(const Tag& other);
		~Tag();

		// Operators
		Tag& operator=(const Tag& other)
		{
			if (this != &other)
			{
				mTag = other.mTag;
			}
			return *this;
		}
		bool operator==(const Tag& other) const
		{
			if (!this->mSubTag && other.mSubTag || this->mSubTag && !other.mSubTag) return false;

			bool subtagMatches;
			if (this->mSubTag && other.mSubTag) subtagMatches = *this->mSubTag == *other.mSubTag;
			else subtagMatches = true;

			return this->mTag == other.mTag && subtagMatches;
		}

		// Functions
		bool MatchesTag(const Tag* other, bool exact = false) const;
		bool MatchesAny(const std::vector<Tag*>& tags, bool exact = false) const;
		bool MatchesAll(const std::vector<Tag*>& tags, bool exact = false) const;

		bool Empty() const { return mTag == ""; }
	private:
		std::string mTag;
		Tag* mSubTag = nullptr;
	};

	struct TagContainer
	{
		TagContainer() = default;
		TagContainer(const std::vector<Tag>& tags)
		{
			AddTags(tags);
		}

		bool AddTag(const Tag& tag);
		bool AddTags(const std::vector<Tag>& tags);

		bool RemoveTag(const Tag& tag);

		// Returns true if at least one tag was successfully removed
		bool RemoveTags(const std::vector<Tag>& tags);

		bool HasTag(const Tag& tag, bool exact = false) const;
		bool HasAllTags(const std::vector<Tag>& tags, bool exact = false) const;
		bool HasAnyTag(const std::vector<Tag>& tags, bool exact = false) const;

		std::vector<Tag> GetTags() const { return mTags; }

		bool Empty() const;
		size_t Size() const;

	private:
		std::vector<Tag> mTags;
	};

	struct TagComponent
	{
		TagComponent() = default;
		TagComponent(const TagContainer& inTags)
		{
			tags = inTags;
		}

		TagContainer tags;
	};
}