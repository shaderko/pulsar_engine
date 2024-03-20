#version 410 core

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

    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform Light light;

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

void main()
{
    float distance = length(light.position - FragPos);
    
    // Calculate attenuation based on distance
    float attenuation = 1.0 / (distance / 1000);

    // Ambient lighting
    vec3 ambient = light.ambient * material.ambient;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    // Specular lighting
    // vec3 viewDir = normalize(-FragPos);
    // vec3 reflectDir = reflect(-lightDir, norm);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // vec3 specular = light.specular * (spec * material.specular);

    // Combine results
    vec3 result = (ambient + diffuse) * attenuation;
    FragColor = vec4(result, 1.0);
    // FragColor = vec4(vec3(1, 1, 1), 1.0);
}
