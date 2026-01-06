#ifndef TAG_HPP
#define TAG_HPP

enum TagTypes {
  ACTIVE,
  MODEL,  // For loaded 3D models
};

// Tag component - simple enum-based tag for entity identification
struct Tag {
  TagTypes type;

  Tag() : type() {};
  Tag(TagTypes tagType) : type(tagType) {};

  bool operator==(const TagTypes &other) const { return type == other; }
};

#endif // TAG_HPP
