#include "utility.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <unordered_map>

std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file:"+filename+" !");
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

void loadModel(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, const std::string path)
{
	tinyobj::attrib_t attrib;//holders all of the positions, normals texture coordinates
	std::vector<tinyobj::shape_t>shapes;//contains all of the separate objects and their faces
	std::vector<tinyobj::material_t> materials;//
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, uint32_t>uniqueVertices = {};
	vertices.clear();
	indices.clear();

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex = {};
			vertex.pos = { attrib.vertices[3 * index.vertex_index + 0],attrib.vertices[3 * index.vertex_index + 1],attrib.vertices[3 * index.vertex_index + 2] };
            if(!attrib.texcoords.empty())
			    vertex.texCoord = { attrib.texcoords[2 * index.texcoord_index + 0],1.0f-attrib.texcoords[2 * index.texcoord_index + 1] };
            if(!attrib.normals.empty())
			    vertex.normal = { attrib.normals[3 * index.normal_index + 0],attrib.normals[3 * index.normal_index + 1],attrib.normals[3*index.normal_index+2] };
			vertex.color = { 1.0f,1.0f,1.0f,1.0f};

			if (uniqueVertices.find(vertex)==uniqueVertices.cend()) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}
			indices.push_back(uniqueVertices[vertex]);
		}
	}
}
