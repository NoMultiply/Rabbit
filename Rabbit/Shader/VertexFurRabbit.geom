#version 330 core

#define FUR_LAYERS 10

layout (triangles) in;
layout (line_strip, max_vertices = 30) out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 displacement;
uniform float furLength;

in vec3 gNormal[];
in vec2 gTexCoords[];

out vec3 fNormal;
out vec3 fFragPosition;
out vec2 fTexCoords;
out float fFragLayer;

vec3 GetNormal() {
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}

void main() {
	for (int j = 0; j < 3; ++j) {
		// fNormal = mat3(transpose(inverse(model))) * gNormal[j];
		fNormal = gNormal[j];
		fTexCoords = gTexCoords[j];
		for (int i = 0; i < FUR_LAYERS; ++i) {
			float layer = i;
			layer /= (FUR_LAYERS - 1);
			float layerFurLength = furLength * layer;
			vec3 layerDisplacement = pow(layer, 3.0) * displacement;
			vec4 newPos = vec4(vec3(gl_in[j].gl_Position) + gNormal[j] * layerFurLength + layerDisplacement, 1.0f);
			gl_Position = projection * view * model * newPos;
			fFragPosition = vec3(model * newPos);
			fFragLayer = layer;
			EmitVertex();
		}
		EndPrimitive();
	}
}