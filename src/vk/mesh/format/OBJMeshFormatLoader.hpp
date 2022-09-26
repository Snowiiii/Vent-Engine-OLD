#pragma once

#include "MeshFormatLoader.hpp"
#include <fast_obj.h>

class OBJMeshFormatLoader : public MeshFormatLoader
{
private:
public:
    OBJMeshFormatLoader(const std::string_view &path) {
        auto *mesh = fast_obj_read(path.data());
    if (!mesh)
    {
        // TODO: handle error
        return;
    }

    const auto index_count = mesh->index_count;
    std::vector<Vertex> vertices(index_count);

    for (auto i = 0; i < index_count; i++)
    {
        const auto &index = mesh->indices[i];

        const auto pos_idx = index.p * 3;
        const auto tex_idx = index.t * 2;
        const auto normal_idx = index.n * 3;

        auto &vertex = vertices[i];
        vertex.pos = glm::vec3(mesh->positions[pos_idx], mesh->positions[pos_idx + 1], mesh->positions[pos_idx + 2]);
        vertex.uv = glm::vec2(mesh->texcoords[tex_idx], mesh->texcoords[tex_idx + 1]);
        vertex.normal = glm::vec3(mesh->normals[normal_idx], mesh->normals[normal_idx + 1], mesh->normals[normal_idx + 2]);
    }

    std::vector<uint32_t> remap(index_count); // allocate temporary memory fovertices.size(), vertices.data(), rr,  the remap table
    size_t vertex_count = meshopt_generateVertexRemap(remap.data(), nullptr, index_count, vertices.data(), index_count, sizeof(Vertex));

    std::vector<Vertex> real_vertices(vertex_count);
    std::vector<uint32_t> indices(index_count);

    meshopt_remapVertexBuffer(real_vertices.data(), vertices.data(), index_count, sizeof(Vertex), remap.data());
    meshopt_remapIndexBuffer(indices.data(), nullptr, index_count, remap.data());

    meshopt_optimizeVertexCache(indices.data(), indices.data(), index_count, vertex_count);
    meshopt_optimizeOverdraw(indices.data(), indices.data(), index_count, &real_vertices[0].pos.x, vertex_count, sizeof(glm::vec3), 1.05f);
    meshopt_optimizeVertexFetch(real_vertices.data(), indices.data(), index_count, real_vertices.data(), vertex_count, sizeof(Vertex));

    _vertices = real_vertices;
    _indices = indices;

    fast_obj_destroy(mesh);
    }
};

