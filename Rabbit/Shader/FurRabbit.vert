#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout(location = 3) in float layer;

out vec2 TexCoords;
out float fragLayer;
out vec3 fragPosition;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 displacement;

void main()
{
	vec3 layerDisplacement = pow(layer, 3.0) * displacement;
	vec4 newPos = vec4(position + layerDisplacement, 1.0f);
    gl_Position = projection * view * model * newPos;
    fragPosition = vec3(model * newPos);
    Normal = mat3(transpose(inverse(model))) * normal;
    TexCoords = texCoords;
	fragLayer = layer;
}