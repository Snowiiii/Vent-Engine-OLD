#pragma once

#include "../Vulkan_Mesh.hpp"

#include <string>
#include <vector>

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include <meshoptimizer.h>

class MeshFormatLoader
{
protected:
    struct Material
    {
        /* Material name */
        char *name;

        /* Parameters */
        float Ka[3]; /* Ambient */
        float Kd[3]; /* Diffuse */
        float Ks[3]; /* Specular */
        float Ke[3]; /* Emission */
        float Kt[3]; /* Transmittance */
        float Ns;    /* Shininess */
        float Ni;    /* Index of refraction */
        float Tf[3]; /* Transmission filter */
        float d;     /* Disolve (alpha) */
        int illum;   /* Illumination model */

        /* Texture maps */
        char *map_Ka;
        char *map_Kd;
        char *map_Ks;
        char *map_Ke;
        char *map_Kt;
        char *map_Ns;
        char *map_Ni;
        char *map_d;
        char *map_bump;
    };

    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;

    std::vector<Material> _Materials;

public:
    Assimp::Importer importer;

    bool load(const std::string &path)
    {
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_ValidateDataStructure);

        if (!scene)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to Load Model: %s, Error: %s", path.data(), importer.GetErrorString());
            return false;
        }

        const auto mesh_count = scene->mNumMeshes;
        const aiVector3D Zero3D(0, 0, 0);

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        uint32_t NumVertices = 0;
        uint32_t NumIndicies = 0;

        for (auto i = 0; i < mesh_count; i++)
        {
            const auto *mesh = scene->mMeshes[i];

            NumVertices += mesh->mNumVertices;

            for (u_int32_t o = 0; o < mesh->mNumFaces; o++)
            {
                const aiFace &face = mesh->mFaces[o];
                NumIndicies += face.mNumIndices;
            }
        }

        vertices.reserve(NumVertices);
        indices.reserve(NumIndicies);

        for (auto i = 0; i < mesh_count; i++)
        {
            const auto *mesh = scene->mMeshes[i];

            for (uint32_t x = 0; x < mesh->mNumVertices; x++)
            {
                const aiVector3D *pos = &(mesh->mVertices[x]);
                const aiVector3D *normal = &(mesh->mNormals[x]);
                const aiVector3D *uv = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][x]) : &Zero3D;

                Vertex vertex;
                vertex.pos = glm::vec3(pos->x, pos->y, pos->z);
                vertex.uv = glm::vec2(uv->x, uv->y);
                vertex.normal = glm::vec3(normal->x, normal->y, normal->z);

                vertices.push_back(vertex);
            }

            for (u_int32_t o = 0; o < mesh->mNumFaces; o++)
            {
                const aiFace &face = mesh->mFaces[o];
                if (face.mNumIndices != 3)  break;

                indices.push_back(face.mIndices[0]);
                indices.push_back(face.mIndices[1]);
                indices.push_back(face.mIndices[2]);
            }
        }

        // std::vector<uint32_t> remap(mesh_count); // allocate temporary memory fovertices.size(), vertices.data(), rr,  the remap table
        // size_t vertex_count = meshopt_generateVertexRemap(remap.data(), nullptr, mesh_count, vertices.data(), mesh_count, sizeof(Vertex));

        // std::vector<Vertex> real_vertices(vertex_count);
        // std::vector<uint32_t> indices(mesh_count);

        // meshopt_remapVertexBuffer(real_vertices.data(), vertices.data(), mesh_count, sizeof(Vertex), remap.data());
        // meshopt_remapIndexBuffer(indices.data(), nullptr, mesh_count, remap.data());

        // meshopt_optimizeVertexCache(indices.data(), indices.data(), mesh_count, vertex_count);
        // meshopt_optimizeOverdraw(indices.data(), indices.data(), mesh_count, &real_vertices[0].pos.x, vertex_count, sizeof(glm::vec3), 1.05f);
        // meshopt_optimizeVertexFetch(real_vertices.data(), indices.data(), mesh_count, real_vertices.data(), vertex_count, sizeof(Vertex));
        _vertices = vertices;
        _indices = indices;

        return true;

        // We're done. Everything will be cleaned up by the importer destructor
    }

    std::vector<Vertex> &getVertices() { return _vertices; }

    std::vector<uint32_t> &getIndices() { return _indices; }
};
