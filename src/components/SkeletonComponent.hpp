#pragma once

// Marker component to identify entities that use animated shaders
struct SkeletonComponent {
    bool isAnimated = true; // Flag for shader selection
    int boneCount = 0;

    SkeletonComponent() = default;
    SkeletonComponent(int count) : boneCount(count) {}
};
