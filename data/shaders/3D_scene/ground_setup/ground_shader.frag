#version 300 es
precision highp float;

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in mat3 TBN; // Matrice Tangente-Binormale-Normale

uniform sampler2D diffuseTexture;
uniform sampler2D normalMap;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    // Charger la texture diffuse
    vec3 albedo = texture(diffuseTexture, TexCoords).rgb;

    // Charger et convertir la normal map
    vec3 normal = texture(normalMap, TexCoords).rgb * 2.0 - 1.0;
    normal = normalize(TBN * normal); // Convertir vers l'espace monde

    // Lumière directionnelle simple
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 lighting = albedo * diff; // Diffuse shading

    FragColor = vec4(lighting, 1.0);
}
