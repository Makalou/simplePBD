#pragma once

#include "shader.h"

#include "vulkan/vulkan.h"

#include "glm/glm.hpp"
#include <vector>

enum RenderingMode
{
	OPAQUE
};

struct shaderProp
{
private:
	std::vector<float> m_floats;
	std::vector<VkImageView> m_texture2Ds;

	std::vector<glm::vec2> m_vec2s;
	std::vector<glm::vec3> m_vec3s;
	std::vector<glm::vec4> m_vec4s;

	std::vector<glm::mat3> m_mat3s;
	std::vector<glm::mat4> m_mat4s;
};

class material
{
private:
	RenderingMode m_renderingMode = OPAQUE;
	shader m_shader;
private:
	shaderProp m_shaderprop;
};