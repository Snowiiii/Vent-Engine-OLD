#pragma once

#include "../Vulkan_Mesh.hpp"

#include <string_view>
#include <vector>

#include <meshoptimizer.h>

class MeshFormatLoader
{
protected:
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
public:
    
    std::vector<Vertex> &getVertices() { return _vertices; }

    std::vector<uint32_t> &getIndices() { return _indices; }
};
