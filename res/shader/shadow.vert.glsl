#version 460

layout(location=0) in vec3 inPosition;

layout(binding=0) uniform UniformBufferObject{
    vec3 viewPos;
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 light_space_matrix;
}ubo;

void main(){
    gl_Position = ubo.light_space_matrix*ubo.model*vec4(inPosition,1.0);
}
