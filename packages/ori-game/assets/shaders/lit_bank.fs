#version 330
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform int light0Enabled; uniform int light0Type; uniform vec3 light0Pos; uniform vec3 light0Target; uniform vec4 light0Color; uniform float light0Intensity;
uniform int light1Enabled; uniform int light1Type; uniform vec3 light1Pos; uniform vec3 light1Target; uniform vec4 light1Color; uniform float light1Intensity;
uniform int light2Enabled; uniform int light2Type; uniform vec3 light2Pos; uniform vec3 light2Target; uniform vec4 light2Color; uniform float light2Intensity;
uniform int light3Enabled; uniform int light3Type; uniform vec3 light3Pos; uniform vec3 light3Target; uniform vec4 light3Color; uniform float light3Intensity;
out vec4 finalColor;

vec3 contrib(int en, int typ, vec3 pos, vec3 tgt, vec4 col, float inten, vec3 n, vec3 p) {
    if (en == 0) return vec3(0.0);
    vec3 ld = (typ == 1) ? normalize(-tgt) : normalize(pos - p);
    float d = max(dot(n, ld), 0.0);
    return d * col.rgb * inten;
}

void main() {
    vec4 texel = texture(texture0, fragTexCoord);
    vec3 n = normalize(fragNormal);
    vec3 amb = vec3(0.12);
    vec3 sum = amb;
    sum += contrib(light0Enabled, light0Type, light0Pos, light0Target, light0Color, light0Intensity, n, fragPosition);
    sum += contrib(light1Enabled, light1Type, light1Pos, light1Target, light1Color, light1Intensity, n, fragPosition);
    sum += contrib(light2Enabled, light2Type, light2Pos, light2Target, light2Color, light2Intensity, n, fragPosition);
    sum += contrib(light3Enabled, light3Type, light3Pos, light3Target, light3Color, light3Intensity, n, fragPosition);
    vec3 rgb = sum * texel.rgb * colDiffuse.rgb * fragColor.rgb;
    finalColor = vec4(rgb, texel.a * colDiffuse.a * fragColor.a);
}
