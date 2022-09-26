#pragma once

#include "../Vulkan_Base.hpp"

#include <string_view>
#include <vector>

#include <meshoptimizer.h>

class MeshFormatLoader
{
private:
public:
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
    
    std::vector<Vertex> &getVertices() { return _vertices; }

    std::vector<uint32_t> &getIndices() { return _indices; }
};
