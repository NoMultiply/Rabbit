#version 330 core
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
in float fFragLayer;

out vec4 color;

uniform sampler2D fur;
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

	float fakeShadow = mix(0.4, 1.0, fFragLayer);

	vec4 furData = texture(fur, fTexCoords);
	vec4 furColor = vec4(result, 1.0f) * fakeShadow;
  
	float visibility = (fFragLayer > furData.r) ? 0.0 : furData.a;
	furColor.a = (fFragLayer == 0.0) ? 1.0 : visibility;

    color = furColor;
    // color = vec4(result, 1.0f);
    // color = vec4(1.0f);
}


// Calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // Combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, fTexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, fTexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, fTexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return ambient + diffuse + specular;
}