#version 300 es
precision highp float;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 Bright;

//all in
in vec3 FragPos;
in vec2 TexCoords;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

//all uniforms
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

void main()
{
    // obtain normal from normal map in range [0,1]
    vec3 normal = texture(normalMap, TexCoords).rgb;
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space

    // get diffuse color
    vec3 color = texture(diffuseMap, TexCoords).rgb;
    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse
    vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;

    // Apply gamma correction
    float gamma = 2.2;
    vec3 finalColor = ambient + diffuse + specular;
    finalColor = pow(finalColor, vec3(1.0 / gamma));

    FragColor = vec4(finalColor, 1.0);
    Bright = vec4(0.0);
}
