#version 330
// Simplified PBR-ish lit shading for level solids (metallic / roughness uniforms).
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec4 lightColor;
uniform float lightIntensity;
uniform float metallic;
uniform float roughness;

out vec4 finalColor;

void main() {
    vec4 texel = texture(texture0, fragTexCoord);
    vec3 albedo = texel.rgb * colDiffuse.rgb * fragColor.rgb;
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(viewPos - fragPosition);
    vec3 L = normalize(lightPos - fragPosition);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float NdotV = max(dot(N, V), 0.0);

    // Dielectric F0 vs metal
    vec3 F0 = mix(vec3(0.04), albedo, clamp(metallic, 0.0, 1.0));
    float rough = clamp(roughness, 0.04, 1.0);

    // Specular (Blinn-Phong shininess from roughness)
    float shininess = mix(256.0, 4.0, rough);
    float spec = pow(NdotH, shininess) * (1.0 - rough * 0.85);

    vec3 ambient = 0.12 * albedo;
    vec3 diffuse = (1.0 - metallic) * albedo * NdotL * lightColor.rgb * lightIntensity;
    vec3 specular = F0 * spec * lightColor.rgb * lightIntensity;

    vec3 color = ambient + diffuse + specular;
    // slight energy conservation darken metals
    color = mix(color, color * 0.85 + specular * 0.5, metallic * 0.5);

    finalColor = vec4(color, texel.a * colDiffuse.a * fragColor.a);
}
