#version 330 core

#define LAYERS 10
// LAYERS * 2 - 1

layout (points) in;
layout (line_strip, max_vertices = 19) out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 displacement;
uniform vec3 viewPos;
uniform int lodLevel;

in vec3 gPosition[];
in vec3 gNormal[];
in vec2 gTexCoords[];
in float gFurLength[];

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

#define emitV1WithBias(l) {\
	v1.position = vec4(gPosition[0] + (l), 1.0f);\
	v1.gPos = projection * view * model * v1.position;\
	v1.fragPosition = vec3(model * v1.position);\
	emit(v1);\
}

void main() {
	vec3 eyeVec = normalize(viewPos - vec3(model * vec4(gPosition[0], 1.0f)));
	float p = dot(gNormal[0], eyeVec);
	if ((lodLevel == 1 && -0.1f < p && p < 0.2f) || (lodLevel == 2 && 0.4f < p && p < 0.6f)) {
		vec3 dir = normalize(cross(eyeVec, gNormal[0]));
		Vertex v1;
		v1.normal = gNormal[0];
		v1.normal_ = mat3(transpose(inverse(model))) * v1.normal;
		v1.texCoords = gTexCoords[0];

		vec3 width = -dir * gFurLength[0] / 2;
		vec3 height = v1.normal * gFurLength[0];
		for (int i = 0; i < LAYERS; ++i) {
			float layer = i;
			layer /= (LAYERS - 1);
			vec3 bias = width + layer * height - pow(layer, 2) * 2 * width;
			emitV1WithBias(bias);
		}
		for (int i = LAYERS - 2; i >= 0; --i) {
			float layer = i;
			layer /= (LAYERS - 1);
			vec3 bias = layer * height - pow(layer, 2) * width;
			emitV1WithBias(bias);
		}
		EndPrimitive();
	}
}