//
// Created by Mat on 11/27/2024.
//

#ifndef SAMPLES_OPENGL_TEXTURE_LOADER_H
#define SAMPLES_OPENGL_TEXTURE_LOADER_H
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "load3D/mesh.h"

#include <vector>
#include <string>
#include <string_view>
#include <iostream>


namespace TextureManager {
    unsigned int Load(const char* path);
    unsigned int loadCubemap(std::vector<std::string> faces);
}

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);

class Model
{
public:
    // model data
    std::vector<Texture> textures_loaded_;	// stores all the textures loaded so far,
    // optimization to make sure textures aren't loaded more than once.
    std::vector<Mesh>    meshes_;
    std::string directory_;
    bool gammaCorrection;

    // constructor, expects a filepath to a 3D model.
    explicit Model(std::string const &path, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw(GLuint &shader)
    {
        for(auto & mesh : meshes_)
            mesh.Draw(shader);
    }

private:
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(std::string const &path);

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void ProcessNode(aiNode *node, const aiScene *scene);

    Mesh ProcessMesh(aiMesh *mesh, const aiScene *scene);

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string& typeName);
};
#endif //SAMPLES_OPENGL_TEXTURE_LOADER_H
