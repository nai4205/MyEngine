#ifndef TAG_HPP
#define TAG_HPP

enum TagComponentTypes {
  ACTIVE,
  MODEL,  // For loaded 3D models
};

// TagComponent component - simple enum-based tag for entity identification
struct TagComponent {
  TagComponentTypes type;

  TagComponent() : type() {};
  TagComponent(TagComponentTypes tagType) : type(tagType) {};

  bool operator==(const TagComponentTypes &other) const { return type == other; }
};

#endif // TAG_HPP
