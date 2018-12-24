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

struct SpotLight {

	vec3 position;
	vec3 direction;
	float cutoff;
	float outCutoff;

	float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
};

#define NR_POINT_LIGHTS 2

in vec3 fragPosition;
in vec3 Normal;
in vec2 TexCoords;
in float fragLayer;

out vec4 color;

uniform float metallic;
uniform float roughness;
uniform float ao;

uniform sampler2D fur;
uniform vec3 viewPos;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;

// Function prototypes
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);


const float PI = 3.14159265359;

void main() 
{
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - fragPosition);

	vec3 albedo = pow(texture(material.texture_diffuse1, TexCoords).rgb, vec3(2.2));

	vec3 Lo = vec3(0.0f);
	for(int i = 0; i < NR_POINT_LIGHTS; i++) 
	{
		vec3 lightDir = normalize(pointLights[i].position - fragPosition);
		vec3 halfway = normalize(lightDir + viewDir);

		float distance = length(pointLights[i].position - fragPosition);
		float attenuation = 1.0f / (pointLights[i].constant + pointLights[i].linear * distance + pointLights[i].quadratic * (distance * distance)); 
		vec3 radiance = pointLights[i].diffuse * attenuation; 

		vec3 F0 = vec3(0.04); 
		F0 = mix(F0, albedo, metallic);
		//vec3 F = fresnelSchlick(max(dot(halfway, viewDir), 0.0), F0);
		vec3 F = fresnelSchlick(max(dot(norm, viewDir), 0.0), F0);

		float NDF = DistributionGGX(norm, halfway, roughness);       
		float G = GeometrySmith(norm, viewDir, lightDir, roughness);

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(norm, viewDir), 0.0) * max(dot(norm, lightDir), 0.0);
		vec3 specular = numerator / max(denominator, 0.001);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
  
		kD *= 1.0 - metallic;

		float NdotL = max(dot(norm, lightDir), 0.0);
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	float theta = dot(normalize(spotLight.position - fragPosition), normalize(-spotLight.direction));
	float epsilon = spotLight.cutoff - spotLight.outCutoff;
	float intensity = clamp((theta - spotLight.outCutoff) / epsilon, 0, 1.0);

	vec3 lightDir = normalize(spotLight.position - fragPosition);
	vec3 halfway = normalize(lightDir + viewDir);

	float distance = length(spotLight.position - fragPosition);
	float attenuation = 1.0f / (spotLight.constant + spotLight.linear * distance + spotLight.quadratic * (distance * distance)); 
	vec3 radiance = spotLight.diffuse * attenuation; 

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);
	//vec3 F = fresnelSchlick(max(dot(halfway, viewDir), 0.0), F0);
	vec3 F = fresnelSchlick(max(dot(norm, viewDir), 0.0), F0);

	float NDF = DistributionGGX(norm, halfway, roughness);       
	float G = GeometrySmith(norm, viewDir, lightDir, roughness);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(norm, viewDir), 0.0) * max(dot(norm, lightDir), 0.0);
	vec3 specular = numerator / max(denominator, 0.001);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
  
	kD *= 1.0 - metallic;

	float NdotL = max(dot(norm, lightDir), 0.0);
	Lo += (kD * albedo / PI + specular) * radiance * NdotL * intensity;

	vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 result = ambient + Lo;
	
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0/2.2));  


	float fakeShadow = mix(0.4, 1.0, fragLayer);
  
	vec4 furData = texture(fur, TexCoords);
	vec4 furColor = vec4(result, 1.0f) * fakeShadow;
  
	float visibility = (fragLayer > furData.r) ? 0.0 : furData.a;
	furColor.a = (fragLayer == 0.0) ? 1.0 : visibility;

    color = furColor;
	//color = vec4(result, 1.0);
}


void main1()
{    
    vec3 result;
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 norm = normalize(Normal);
    
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, fragPosition, viewDir);

    float fakeShadow = mix(0.4, 1.0, fragLayer);
  
	vec4 furData = texture(fur, TexCoords);
	vec4 furColor = vec4(result, 1.0f) * fakeShadow;
  
	float visibility = (fragLayer > furData.r) ? 0.0 : furData.a;
	furColor.a = (fragLayer == 0.0) ? 1.0 : visibility;

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
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return ambient + diffuse + specular;
}


vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}