#version 450 core

layout(location = 0) in vec2 i_Position;
layout(location = 0) out vec4 o_Color;

layout(binding = 0, input_attachment_index = 0) uniform subpassInput u_GBuffers[3];
layout(binding = 1) uniform samplerCube skyBox;
uint mode = 0; // 0, 1, 2,

void main() {
    vec3 envColor = texture(skyBox, vec3(i_Position, 1.0)).rgb;
	float depth = subpassLoad(u_GBuffers[0]).a;
	if(depth == 1.0f) {
        o_Color = vec4(envColor, 1.0);
    } else {
		o_Color = subpassLoad(u_GBuffers[1]);
	    o_Color.w = 1.0;
	}
}