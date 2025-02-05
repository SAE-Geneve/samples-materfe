#version 300 es
precision highp float;

layout(location = 0) in vec3 aPos;       // Position du sommet
layout(location = 1) in vec3 aNormal;    // Normale du sommet
layout(location = 2) in vec2 aTexCoord;  // Coordonnées UV
layout(location = 3) in vec3 aTangent;   // Tangente du sommet

out vec2 TexCoords;
out vec3 FragPos;
out mat3 TBN;        // Matrice de transformation pour normal mapping

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Transformation des coordonnées
    FragPos = vec3(model * vec4(aPos, 1.0));
    TexCoords = aTexCoord;

    // Calcul de la normale en espace monde
    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
    vec3 B = cross(N, T); // Binormale

    // Matrice TBN pour normal mapping
    TBN = mat3(T, B, N);

    // Projection finale
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
