#version 330
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;
uniform int lightEnabled;
uniform int lightType;
uniform vec3 lightPos;
uniform vec3 lightTarget;
uniform vec4 lightColor;
uniform float lightIntensity;
out vec4 finalColor;
void main() {
    vec4 texel = texture(texture0, fragTexCoord);
    vec3 normal = normalize(fragNormal);
    vec3 lightDir;
    if (lightType == 1) {
        lightDir = normalize(-lightTarget);
    } else {
        lightDir = normalize(lightPos - fragPosition);
    }
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 ambient = 0.15 * lightColor.rgb;
    vec3 diffuse = diff * lightColor.rgb * lightIntensity;
    if (lightEnabled == 0) {
        diffuse = vec3(1.0);
        ambient = vec3(0.0);
    }
    vec3 result = (ambient + diffuse) * texel.rgb * colDiffuse.rgb * fragColor.rgb;
    finalColor = vec4(result, texel.a * colDiffuse.a * fragColor.a);
}
