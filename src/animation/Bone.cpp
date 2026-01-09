#include "Bone.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cassert>

// Helper functions to convert Assimp types to GLM
static glm::vec3 assimpToGlmVec3(const aiVector3D &v) {
    return glm::vec3(v.x, v.y, v.z);
}

static glm::quat assimpToGlmQuat(const aiQuaternion &q) {
    return glm::quat(q.w, q.x, q.y, q.z);
}

Bone::Bone(const std::string &name, int id, const aiNodeAnim *channel)
    : m_Name(name), m_ID(id), m_LocalTransform(1.0f) {
    m_NumPositions = channel->mNumPositionKeys;

    for (int positionIndex = 0; positionIndex < m_NumPositions;
         ++positionIndex) {
        aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
        float timeStamp = channel->mPositionKeys[positionIndex].mTime;
        KeyPosition data;
        data.position = assimpToGlmVec3(aiPosition);
        data.timeStamp = timeStamp;
        m_Positions.push_back(data);
    }

    m_NumRotations = channel->mNumRotationKeys;
    for (int rotationIndex = 0; rotationIndex < m_NumRotations;
         ++rotationIndex) {
        aiQuaternion aiOrientation =
            channel->mRotationKeys[rotationIndex].mValue;
        float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
        KeyRotation data;
        data.orientation = assimpToGlmQuat(aiOrientation);
        data.timeStamp = timeStamp;
        m_Rotations.push_back(data);
    }

    m_NumScalings = channel->mNumScalingKeys;
    for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex) {
        aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
        float timeStamp = channel->mScalingKeys[keyIndex].mTime;
        KeyScale data;
        data.scale = assimpToGlmVec3(scale);
        data.timeStamp = timeStamp;
        m_Scales.push_back(data);
    }
}

void Bone::update(float animationTime) {
    glm::mat4 translation = interpolatePosition(animationTime);
    glm::mat4 rotation = interpolateRotation(animationTime);
    glm::mat4 scale = interpolateScaling(animationTime);
    m_LocalTransform = translation * rotation * scale;
}

int Bone::getPositionIndex(float animationTime) {
    for (int index = 0; index < m_NumPositions - 1; ++index) {
        if (animationTime < m_Positions[index + 1].timeStamp)
            return index;
    }
    assert(0);
    return 0;
}

int Bone::getRotationIndex(float animationTime) {
    for (int index = 0; index < m_NumRotations - 1; ++index) {
        if (animationTime < m_Rotations[index + 1].timeStamp)
            return index;
    }
    assert(0);
    return 0;
}

int Bone::getScaleIndex(float animationTime) {
    for (int index = 0; index < m_NumScalings - 1; ++index) {
        if (animationTime < m_Scales[index + 1].timeStamp)
            return index;
    }
    assert(0);
    return 0;
}

float Bone::getScaleFactor(float lastTimeStamp, float nextTimeStamp,
                           float animationTime) {
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

glm::mat4 Bone::interpolatePosition(float animationTime) {
    if (1 == m_NumPositions)
        return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

    int p0Index = getPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = getScaleFactor(m_Positions[p0Index].timeStamp,
                                       m_Positions[p1Index].timeStamp,
                                       animationTime);
    glm::vec3 finalPosition =
        glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position,
                 scaleFactor);
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::interpolateRotation(float animationTime) {
    if (1 == m_NumRotations) {
        auto rotation = glm::normalize(m_Rotations[0].orientation);
        return glm::toMat4(rotation);
    }

    int p0Index = getRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = getScaleFactor(m_Rotations[p0Index].timeStamp,
                                       m_Rotations[p1Index].timeStamp,
                                       animationTime);
    glm::quat finalRotation =
        glm::slerp(m_Rotations[p0Index].orientation,
                   m_Rotations[p1Index].orientation, scaleFactor);
    finalRotation = glm::normalize(finalRotation);
    return glm::toMat4(finalRotation);
}

glm::mat4 Bone::interpolateScaling(float animationTime) {
    if (1 == m_NumScalings)
        return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

    int p0Index = getScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor =
        getScaleFactor(m_Scales[p0Index].timeStamp, m_Scales[p1Index].timeStamp,
                       animationTime);
    glm::vec3 finalScale =
        glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale, scaleFactor);
    return glm::scale(glm::mat4(1.0f), finalScale);
}
