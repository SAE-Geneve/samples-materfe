//
// Created by Mat on 11/27/2024.
//

#ifndef SAMPLES_OPENGL_TEXTURE_LOADER_H
#define SAMPLES_OPENGL_TEXTURE_LOADER_H
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "load3D/mesh.h"

#include <vector>
#include <string>
#include <string_view>


namespace TextureManager {
    unsigned int Load(const char* path);
    unsigned int loadCubemap(std::vector<std::string> faces);
};

class Model
{
public:
    explicit Model(std::string& path)
    {
        loadModel(path);
    }
    Model() = default;

    void Draw(GLuint &shader);
private:
    // model data
    std::vector<Mesh> meshes{};
    std::string directory{};
    std::vector<Texture> textures_loaded{};




    void loadModel(std::string& path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                              const char* typeName);
};


#endif //SAMPLES_OPENGL_TEXTURE_LOADER_H
