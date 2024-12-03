#version 300 es
precision highp float;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 normal;
in vec2 texCoord;
in vec3 FragPos;

layout (location = 0) out vec4 outColor;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform sampler2D ourTexture1;
uniform Material material;
uniform Light light;

void main()
{
    //vec4 tex1Color = texture(ourTexture1, texCoord);
    //outColor = tex1Color;

    // diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient  = light.ambient * vec3(texture(ourTexture1, texCoord));
    vec3 diffuse  = light.diffuse * (diff * vec3(texture(ourTexture1, texCoord)));
    vec3 specular = light.specular * (spec * material.specular);

    vec3 result = ambient + diffuse + specular;
    outColor = vec4(result, 1.0);
}