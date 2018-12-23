#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in float furLength;

out vec3 gPosition;
out vec2 gTexCoords;
out vec3 gNormal;
out float gFurLength;

void main()
{
	gPosition = position;
    gNormal = normal;
    gTexCoords = texCoords;
	gFurLength = furLength;
}