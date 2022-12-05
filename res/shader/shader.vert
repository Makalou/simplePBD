#version 460

layout(location=0) in vec3 inPosition;
layout(location=1) in vec4 inColor;
layout(location=2) in vec2 inTexCoord;
layout(location=3) in vec3 inNormal;

layout(location=0) out vec2 fragTexCoord;
layout(location=1) out vec3 fragPos;
layout(location=2) out vec3 fragNormal;
layout(location=3) out vec3 viewerpos;
layout(location=4) out vec4 light_sapce_pos;

layout(binding=0) uniform UniformBufferObject{
	vec3 viewPos;
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 light_sapce_matrix;
}ubo;

void main(){
	gl_Position = ubo.proj*ubo.view*ubo.model*vec4(inPosition,1.0);
	fragPos=vec3(ubo.model*vec4(inPosition,1.0));
	fragTexCoord = inTexCoord;
	fragNormal=mat3(transpose(inverse(ubo.model)))*inNormal;
	viewerpos=ubo.viewPos;
	light_sapce_pos = ubo.light_sapce_matrix*ubo.model*vec4(inPosition,1.0);
}