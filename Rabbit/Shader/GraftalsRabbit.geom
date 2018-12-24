#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 9) out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 displacement;
uniform vec3 viewPos;
uniform float furLength;

in vec3 gPosition[];
in vec3 gNormal[];
in vec2 gTexCoords[];

out vec3 fNormal;
out vec3 fFragPosition;
out vec2 fTexCoords;

struct Vertex {
	vec3 normal;
	vec3 normal_;
	vec4 position;
	vec4 gPos;
	vec3 fragPosition;
	vec2 texCoords;
};

void emit(Vertex vertex) {
	gl_Position = vertex.gPos;
	fNormal = vertex.normal_;
	fFragPosition = vertex.fragPosition;
	fTexCoords = vertex.texCoords;
	EmitVertex();
}

#define setupVertex(v, idx) {\
	v.normal = gNormal[idx];\
	v.normal_ = mat3(transpose(inverse(model))) * v.normal;\
	v.position = vec4(gPosition[idx], 1.0f);\
	v.gPos = projection * view * model * v.position;\
	v.fragPosition = vec3(model * v.position);\
	v.texCoords = gTexCoords[idx];\
}

void main() {
	Vertex v1, v2, v3, v0;
	setupVertex(v1, 0);
	setupVertex(v2, 1);
	setupVertex(v3, 2);

	v0.normal = (v1.normal + v2.normal + v3.normal) / 3.0f;
	v0.normal_ = mat3(transpose(inverse(model))) * v0.normal;
	v0.position = vec4((gPosition[0] + gPosition[1] + gPosition[2]) / 3.0f + furLength * v0.normal + displacement, 1.0f);
	v0.gPos = projection * view * model * v0.position;
	v0.fragPosition = vec3(model * v0.position);
	v0.texCoords = (gTexCoords[0] + gTexCoords[1] + gTexCoords[2]) / 3.0f;

	vec3 eyeVec = normalize(viewPos - vec3(model * v0.position));
	float p = dot(v0.normal, eyeVec);
	if (p < -0.1)
		return;

	emit(v1);
	emit(v2);
	emit(v0);
	EndPrimitive();

	emit(v2);
	emit(v3);
	emit(v0);
	EndPrimitive();

	emit(v3);
	emit(v1);
	emit(v0);
	EndPrimitive();
}