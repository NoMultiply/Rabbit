#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in float layer;

out vec2 TexCoords;
out float fragLayer;
out vec3 fragPosition;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 displacement;
uniform vec3 rabbitPostion;

void main()
{

	vec4 newPos;
    vec3 pos = vec3(model * vec4(position, 1.0f));
    if(length(pos - rabbitPostion) < 0.8f){
        float dis = length(pos - rabbitPostion);
        vec3 force = (3.0f - dis) * normalize(pos - rabbitPostion);
        newPos = vec4(position + force, 1.0f);
    }
    else{
        vec3 layerDisplacement = pow(layer, 3.0) * displacement;
        newPos = vec4(position + layerDisplacement, 1.0f);
    }
    
    fragPosition = vec3(model * newPos);
    gl_Position = projection * view * model * newPos;
    // gl_Position = projection * view * model * vec4(position, 1.0f);
    // fragPosition = vec3(model * vec4(position, 1.0f));
    Normal = mat3(transpose(inverse(model))) * normal;
    TexCoords = texCoords;
	fragLayer = layer;
}