#pragma once
#include "material.h"

class shader
{
public:
	void loadFrom();
public:
	void bindUniform(float val);
	void bindUniform(VkImageView val);
	void bindUniform(size_t idx, glm::vec2 val);
	void bindUniform(size_t idx, glm::vec3 val);
	void bindUniform(size_t idx, glm::vec4 val);
	void bindUniform(size_t idx, glm::mat3 val);
	void bindUniform(size_t idx, glm::mat4 val);
};
