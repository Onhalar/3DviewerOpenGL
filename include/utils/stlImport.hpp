#ifndef STL_MODEL_LOADER_HPP
#define STL_MODEL_LOADER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include <config.hpp>
#include <FormatConsole.hpp>

// Assimp includes
// Make sure Assimp is properly installed and linked in your project
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

/**
 * @brief Structure to hold the extracted vertex data, normal data, and index data from an STL model.
 */
struct ModelData {
    std::vector<GLfloat> vertices; // Stores vertex positions (x, y, z)
    std::vector<GLfloat> normals;  // Stores vertex normals (nx, ny, nz)
    std::vector<GLuint> indices;   // Stores indices for indexed drawing
};

/**
 * @brief Loads an STL model from the specified file path and extracts its vertex positions, normals, and indices.
 *
 * This function uses the Assimp library to parse the STL file. It applies post-processing
 * steps to triangulate faces, generate smooth normals (if not present), and join identical vertices.
 * The `aiProcess_JoinIdenticalVertices` flag is particularly important for generating an indexed mesh,
 * which allows you to use an EBO for more efficient rendering.
 *
 * @param filePath The path to the STL file.
 * @return A pointer to a ModelData struct containing the extracted vertices, normals, and indices on success.
 * Returns nullptr if the file cannot be loaded or contains no valid mesh data.
 * The caller is responsible for deleting the returned ModelData pointer to free memory.
 */
ModelData* loadSTLData(const std::filesystem::path& filePath) {
    Assimp::Importer importer;
    // Post-processing flags:
    // aiProcess_Triangulate: Ensures all faces are triangles.
    // aiProcess_GenSmoothNormals: Generates smooth per-vertex normals if the model doesn't have them.
    // aiProcess_JoinIdenticalVertices: Joins duplicate vertices, allowing for indexed drawing (EBO).
    const aiScene* scene = importer.ReadFile(filePath.string(),
                                             aiProcess_Triangulate |
                                             aiProcess_GenSmoothNormals |
                                             aiProcess_JoinIdenticalVertices);

    // Check if the scene was loaded successfully
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::Failed to load STL file: " << importer.GetErrorString() << std::endl;
        return nullptr;
    }

    // For simplicity, we'll process only the first mesh found in the scene.
    // STL files typically contain a single mesh.
    if (scene->mNumMeshes == 0) {
        std::cerr << "ERROR::ASSIMP::No meshes found in the loaded STL file." << std::endl;
        return nullptr;
    }

    aiMesh* mesh = scene->mMeshes[0];

    // Basic checks for required data
    if (!mesh->HasPositions()) {
        std::cerr << "ERROR::ASSIMP::Mesh has no vertex positions." << std::endl;
        return nullptr;
    }
    if (!mesh->HasNormals()) {
        std::cerr << "ERROR::ASSIMP::Mesh has no normals. Ensure aiProcess_GenSmoothNormals is used." << std::endl;
        return nullptr;
    }
    // Check if the mesh has faces (and thus indices)
    if (!mesh->HasFaces()) {
        std::cerr << "ERROR::ASSIMP::Mesh has no faces (indices)." << std::endl;
        return nullptr;
    }


    ModelData* modelData = new ModelData();
    modelData->vertices.reserve(mesh->mNumVertices * 3);
    modelData->normals.reserve(mesh->mNumVertices * 3);
    modelData->indices.reserve(mesh->mNumFaces * 3); // Each face is a triangle, so 3 indices per face

    // Extract vertex positions and normals
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        // Positions (x, y, z)
        modelData->vertices.push_back(mesh->mVertices[i].x);
        modelData->vertices.push_back(mesh->mVertices[i].y);
        modelData->vertices.push_back(mesh->mVertices[i].z);

        // Normals (nx, ny, nz)
        modelData->normals.push_back(mesh->mNormals[i].x);
        modelData->normals.push_back(mesh->mNormals[i].y);
        modelData->normals.push_back(mesh->mNormals[i].z);
    }

    // Extract index data
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        // Ensure the face is a triangle (due to aiProcess_Triangulate)
        if (face.mNumIndices != 3) {
            std::cerr << "WARNING::ASSIMP::Face is not a triangle. Skipping face." << std::endl;
            continue;
        }
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            modelData->indices.push_back(face.mIndices[j]);
        }
    }

    if (debugMode) {
        std::cout << '\n' << formatProcess("Loaded") << " STL data from: '" << formatPath(getFileName(filePath)) << "'" << std::endl;
        std::cout << formatProcess("Extracted ") << mesh->mNumVertices << " " << formatRole("vertices") << " and "
                << modelData->indices.size() << formatRole(" indices") << std::endl;
    }
    return modelData;
}

#endif // STL_MODEL_LOADER_HPP
