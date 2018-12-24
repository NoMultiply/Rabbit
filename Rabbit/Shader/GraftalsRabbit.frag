#version 330 core

#define USE_TEXTURE true

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
}; 

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 2

in vec3 fNormal;
in vec3 fFragPosition;
in vec2 fTexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform Material material;

// Function prototypes
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 result;
    vec3 viewDir = normalize(viewPos - fFragPosition);
    vec3 norm = normalize(fNormal);

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, fFragPosition, viewDir);

    color = vec4(result, 1.0f);
}


// Calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    vec3 ambient = light.ambient;
	if (USE_TEXTURE) 
		ambient *= vec3(texture(material.texture_diffuse1, fTexCoords));
    vec3 diffuse = light.diffuse * diff;
	if (USE_TEXTURE)
		diffuse *= vec3(texture(material.texture_diffuse1, fTexCoords));
    vec3 specular = light.specular * spec;
	if (USE_TEXTURE) 
		specular *= vec3(texture(material.texture_specular1, fTexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return ambient + diffuse + specular;
}