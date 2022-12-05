//
// Created by 王泽远 on 2022/7/1.
//

#ifndef PHY_SIM_MESH_H
#define PHY_SIM_MESH_H

#include <vector>
#include "vertex.h"

struct vertexPositionView{
public:
    void operator=(const glm::vec3 & p){
        for(auto & p_pos : p_positions)
            *p_pos = p;
    }
    glm::vec3& operator*(){
        return *p_positions[0];
    }
    void reg(glm::vec3* p){
        p_positions.push_back(p);
    }

    bool operator == (const glm::vec3 v){
        return v == *p_positions[0];
    }

    void operator +=(const glm::vec3& v){
        for(auto & p_pos : p_positions)
            *p_pos += v;
    }

    void operator -=(const glm::vec3& v){
        for(auto & p_pos : p_positions)
            *p_pos -= v;
    }

    const glm::vec3 * operator->() const {
        return p_positions[0];
    }

private:
    std::vector<glm::vec3*> p_positions;
};

struct Mesh{
public:
    void recalculateNormals();
    VkBuffer vertexBuffer;
    VkBuffer indexBuffer;

    std::vector<uint32_t> indies = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4
    };

    std::vector<Vertex> vertices = {};
    std::vector<vertexPositionView> position_views = {};
};
#endif //PHY_SIM_MESH_H
