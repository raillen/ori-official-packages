#version 330

// GPU skinning (raylib models_gpu_skinning). Max bones matches typical game rigs.
#define MAX_BONE_NUM 128

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;
in vec4 vertexBoneIds;
in vec4 vertexBoneWeights;

uniform mat4 mvp;
uniform mat4 boneMatrices[MAX_BONE_NUM];

out vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
    int boneIndex0 = int(vertexBoneIds.x);
    int boneIndex1 = int(vertexBoneIds.y);
    int boneIndex2 = int(vertexBoneIds.z);
    int boneIndex3 = int(vertexBoneIds.w);

    vec4 skinnedPosition =
        vertexBoneWeights.x * (boneMatrices[boneIndex0] * vec4(vertexPosition, 1.0)) +
        vertexBoneWeights.y * (boneMatrices[boneIndex1] * vec4(vertexPosition, 1.0)) +
        vertexBoneWeights.z * (boneMatrices[boneIndex2] * vec4(vertexPosition, 1.0)) +
        vertexBoneWeights.w * (boneMatrices[boneIndex3] * vec4(vertexPosition, 1.0));

    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    gl_Position = mvp * skinnedPosition;
}
