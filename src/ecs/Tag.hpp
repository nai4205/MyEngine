#ifndef TAG_HPP
#define TAG_HPP

#include <unordered_set>

enum TagComponentTypes {
  ACTIVE,
  MODEL,
  OUTLINED,
};

struct TagComponent {
  std::unordered_set<TagComponentTypes> tags;

  TagComponent() = default;
  TagComponent(TagComponentTypes tagType) { tags.insert(tagType); }
  TagComponent(std::initializer_list<TagComponentTypes> tagList)
      : tags(tagList) {}

  void add(TagComponentTypes tag) { tags.insert(tag); }
  void remove(TagComponentTypes tag) { tags.erase(tag); }
  bool has(TagComponentTypes tag) const { return tags.find(tag) != tags.end(); }

  bool operator==(const TagComponentTypes &other) const { return has(other); }
};

#endif // TAG_HPP
