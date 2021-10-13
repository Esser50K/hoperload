#include "ChunkMesh.h"

#include "Voxels.h"
#include <iostream>

namespace
{
    using v3 = glm::ivec3;
    using v2 = glm::vec2;
    const std::array<glm::vec2, 4> greedyTexCoords = {
        v2{1.0f, 1.0f},
        v2{1.0f, 0.0f},
        v2{0.0f, 0.0f},
        v2{0.0f, 1.0f},
    };
} // namespace

#include "Voxels.h"
#include <iostream>
namespace
{
    using v3 = glm::ivec3;
    using v2 = glm::vec2;
    using Face = VoxelMeshFace;
    // clang-format off
    const Face FRONT_FACE   = {{v3{1, 1, 1}, {0, 1, 1}, {0, 0, 1}, {1, 0, 1}}, 0.8f};
    const Face LEFT_FACE    = {{v3{0, 1, 1}, {0, 1, 0}, {0, 0, 0}, {0, 0, 1}}, 0.6f};
    const Face BACK_FACE    = {{v3{0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}}, 0.8f};
    const Face RIGHT_FACE   = {{v3{1, 1, 0}, {1, 1, 1}, {1, 0, 1}, {1, 0, 0}}, 0.6f};
    const Face TOP_FACE     = {{v3{1, 1, 0}, {0, 1, 0}, {0, 1, 1}, {1, 1, 1}}, 1.0f};
    const Face BOTTOM_FACE  = {{v3{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}}, 0.4f};
    // clang-format on

    const std::array<glm::vec2, 4> textureCoords = {v2{1.0f, 0.0f}, v2{0.0f, 0.0f},
                                                    v2{0.0f, 1.0f}, v2{1.0f, 1.0f}};

    bool shouldMakeFace(VoxelID thisId, VoxelID compareId)
    {
        VoxelID air = 0;

        // auto& thisVoxel = getVoxelType((VoxelType)thisId);
        auto& compareVoxel = getVoxelType((VoxelType)compareId);

        if (compareId == air)
        {
            return true;
        }
        else if (thisId != compareId && compareVoxel.isTransparent)
        {
            return true;
        }
        return false;
    }
} // namespace

void ChunkMesh::addVoxelFace(const VoxelMeshFace& face, const ChunkPosition& chunkPos,
                             const VoxelPosition& blockPos, GLfloat textureId,
                             float lightValue)
{
    glm::ivec3 chunkPos3d = {chunkPos.x, chunkPos.y, 0};
    for (int i = 0; i < 4; i++)
    {
        VoxelVertex v;
        v.position = face.vertexPositions[i] + chunkPos3d * CHUNK_SIZE + blockPos;
        v.textureCoord = glm::vec3(textureCoords[i].x, textureCoords[i].y, textureId);
        v.light = face.light * lightValue / 15;
        vertices.push_back(v);
    }
    indices.push_back(indicesCount);
    indices.push_back(indicesCount + 1);
    indices.push_back(indicesCount + 2);
    indices.push_back(indicesCount + 2);
    indices.push_back(indicesCount + 3);
    indices.push_back(indicesCount);
    indicesCount += 4;
}

ChunkMesh createGreedyChunkMesh(const Chunk& chunk)
{
    ChunkMesh mesh;
    auto p = chunk.position();
    mesh.chunkPosY = chunk.position().y * CHUNK_SIZE;
    mesh.chunkPos = chunk.position();

    for (int y = 0; y < CHUNK_SIZE; y++)
    {
        for (int x = 0; x < CHUNK_SIZE; x++)
        {
            for (int z = 0; z < CHUNK_DEPTH; z++)
            {
                // If it is "not air"
                VoxelPosition voxelPosition(x, y, z);
                auto voxel = chunk.qGetVoxel(voxelPosition);
                auto light = chunk.getSunlight(voxelPosition);
                auto& voxData = getVoxelType((VoxelType)chunk.qGetVoxel({x, y, z}));

                if (voxel != AIR)
                {

                    // Left voxel face
                    if (shouldMakeFace(voxel, chunk.getVoxel({x - 1, y, z})))
                    {
                        mesh.addVoxelFace(LEFT_FACE, p, voxelPosition,
                                          voxData.textureSide, light);
                    }

                    // Right chunk face
                    if (shouldMakeFace(voxel, chunk.getVoxel({x + 1, y, z})))
                    {
                        mesh.addVoxelFace(RIGHT_FACE, p, voxelPosition,
                                          voxData.textureSide, light);
                    }

                    // Front chunk face
                    if (shouldMakeFace(voxel, chunk.getVoxel({x, y, z + 1})))
                    {
                        mesh.addVoxelFace(FRONT_FACE, p, voxelPosition,
                                          voxData.textureSide, light);
                    }
                    // Bottom chunk face
                    if (shouldMakeFace(voxel, chunk.getVoxel({x, y - 1, z})))
                    {
                        mesh.addVoxelFace(BOTTOM_FACE, p, voxelPosition,
                                          voxData.textureBottom, light);
                    }

                    // Top chunk face
                    if (shouldMakeFace(voxel, chunk.getVoxel({x, y + 1, z})))
                    {
                        mesh.addVoxelFace(TOP_FACE, p, voxelPosition, voxData.textureTop,
                                          light);
                    }
                }
            }
        }
    }
    return mesh;
}

/*
// Ported from https://eddieabbondanz.io/post/voxel/greedy-mesh/
// Consider a 3D vector as an array rather than 3 seperate components
// and this becomes a lot more clear
// eg If you have array with glm::vec3 = {11, 29, 23};, X is 1, Y is 2, and Z is
3
// OR thinking in arrays, [0] = 11, [1] = 29, and [2] = 23
// By working out which component of the vector needs to be sweeped through, the
// algorithm is able to be used for multiple directions without changing
ChunkMesh createGreedyChunkMeshv2(const Chunk& chunk)
{
    ChunkMesh mesh;
    auto p = chunk.position();
    mesh.chunkPosY = chunk.position().y * CHUNK_SIZE;
    mesh.chunkPos = chunk.position();

    // For each slice, a mask is created to determine if a block face has
already been
    // visited
    std::array<bool, CHUNK_AREA> mask;
    auto maskIdx = [](int x, int y) { return y * CHUNK_SIZE + x; };

    // Goes through all 4 axis eg X, -X, Y
    for (int faceAxis = 0; faceAxis < 4; faceAxis++) {
        // For -X, -Y this is true, else is false
        bool isBackFace = faceAxis > 2;

        // X, Y component of the vector. This is the "slice" component of the
        // chunk This says which layer the
        int sliceDir = faceAxis % 2;

        // These are the other two components of the vector complemening
"direction"
        // that are used to sweep across the individual planes/slices
        // Eg if the "slice direction" is 0 (or X),
        // then these will be 1 and 2, (or Y and Z) etc
        int sweepDirA = (sliceDir + 1) % 2;
        int sweepDirB = (sliceDir + 2) % 2;

        // The "working" location within the chunk
        VoxelPosition start{0};
        VoxelPosition end{0};

        // iterate the chunk layers
        for (start[sliceDir] = 0; start[sliceDir] < CHUNK_SIZE;
start[sliceDir]++) {

            // Reset the mask for each layer that is visted
            mask = {false};

            // Iterate the voxels of this layer
            for (start[sweepDirA] = 0; start[sweepDirA] < CHUNK_SIZE;
                 start[sweepDirA]++) {
                for (start[sweepDirB] = 0; start[sweepDirB] < CHUNK_SIZE;
                     start[sweepDirB]++) {

                    // Get the voxel at this location in the chunk
                    uint16_t thisVoxel = chunk.getVoxel(start);

                    // Skip this voxel in the working direction if its working
face is
                    // not visible or has already been merged
                    if (mask.at(maskIdx(start[sweepDirA], start[sweepDirB])) ||
                        !isVoxelSolid(thisVoxel) ||
                        !chunk.isFaceVisible(start, sliceDir, isBackFace))
                        continue;

                    // At this point a SINGLE voxel face has been found.
                    // Adajacent voxel faces are then combined into this one by:
                    //  1. Calculate the width of this voxel section
                    //  2. Calculate the height of this voxel section
                    //  3. Create 4 verticies, and add them to the chunk mesh

                    VoxelPosition currPos = start;
                    glm::ivec3 quadSize{0};

                    // Work out how "wide" the section is by going throuugh a
single
                    // row of voxels and looking at the next voxel along to see
if it
                    // is the same type of voxel and has a visible face

                    /////////////////////////////////////////////////////
                    /////////////////////////////////////////////////////
                    ////
                    ////  vvvv   NEED TO WORK OUT HOW THIS WORKS  vvvv
                    ////
                    /////////////////////////////////////////////////////
                    /////////////////////////////////////////////////////
                    for (currPos = start, currPos[sweepDirB]++;
                         currPos[sweepDirB] < CHUNK_SIZE &&
                         chunk.compareStep(start, currPos, sliceDir, isBackFace)
&& !mask.at(maskIdx(currPos[sweepDirA], currPos[sweepDirB]));
                         currPos[sweepDirB]++) {
                    }
                    quadSize[sweepDirB] = currPos[sweepDirB] - start[sweepDirB];

                    // Figure out the height, then save it
                    for (currPos = start, currPos[sweepDirA]++;
                         currPos[sweepDirA] < CHUNK_SIZE &&
                         chunk.compareStep(start, currPos, sliceDir, isBackFace)
&& !mask.at(maskIdx(currPos[sweepDirA], currPos[sweepDirB]));
                         currPos[sweepDirA]++) {
                        for (currPos[sweepDirB] = start[sweepDirB];
                             currPos[sweepDirB] < CHUNK_SIZE &&
                             chunk.compareStep(start, currPos, sliceDir,
isBackFace)
&& !mask.at(maskIdx(currPos[sweepDirA], currPos[sweepDirB]));
currPos[sweepDirB]++) {
                        }

                        // If we didn't reach the end then its not a good add.
                        if (currPos[sweepDirB] - start[sweepDirB] <
quadSize[sweepDirB]) { break;
                        }
                        else {
                            currPos[sweepDirB] = start[sweepDirB];
                        }
                    }
                    quadSize[sweepDirA] = currPos[sweepDirA] - start[sweepDirA];
                    /////////////////////////////////////////////////////
                    /////////////////////////////////////////////////////
                    ////
                    //// ^^^^   NEED TO WORK OUT HOW THAT WORKS  ^^^^^
                    ////
                    /////////////////////////////////////////////////////
                    /////////////////////////////////////////////////////

                    // Calculate the "width" and "height"
                    glm::ivec3 width{0};
                    glm::ivec3 height{0};
                    glm::ivec3 offset = glm::ivec3{p.x, p.y, 0} * CHUNK_SIZE +
                                        glm::ivec3{start.x, start.y, 0};

                    // The components of the width and height are the sweep
planes width[sweepDirA] = quadSize[sweepDirA]; height[sweepDirB] =
quadSize[sweepDirB];

                    // The "offset" is the layer within this chunk the voxel is
                    offset[sliceDir] += isBackFace ? 0 : 1;

                    // Calculate the positon
                    VoxelVertex verticies[4];
                    verticies[0].position = offset;
                    verticies[1].position = offset + width;
                    verticies[2].position = offset + width + height;
                    verticies[3].position = offset + height;

                    // Calulate the texture coords and add to the mesh
                    auto quadWidth = quadSize[sweepDirB];
                    auto quadHeight = quadSize[sweepDirA];
                    GLuint texture = getVoxelTexture(thisVoxel, sliceDir,
isBackFace); glm::vec3 normal{0}; normal[sliceDir] = isBackFace ? -1 : 1; for
(int i = 0; i < 4; i++) { auto s = greedyTexCoords[i].s * quadWidth; auto t =
greedyTexCoords[i].t * quadHeight; verticies[i].textureCoord = {s, t, texture};

                        verticies[i].normal = normal;

                        mesh.vertices.push_back(verticies[i]);
                    }
                    if (isBackFace) {
                        mesh.indices.push_back(mesh.indicesCount + 2);
                        mesh.indices.push_back(mesh.indicesCount + 1);
                        mesh.indices.push_back(mesh.indicesCount);

                        mesh.indices.push_back(mesh.indicesCount);
                        mesh.indices.push_back(mesh.indicesCount + 3);
                        mesh.indices.push_back(mesh.indicesCount + 2);
                    }
                    else {
                        mesh.indices.push_back(mesh.indicesCount);
                        mesh.indices.push_back(mesh.indicesCount + 1);
                        mesh.indices.push_back(mesh.indicesCount + 2);
                        mesh.indices.push_back(mesh.indicesCount + 2);
                        mesh.indices.push_back(mesh.indicesCount + 3);
                        mesh.indices.push_back(mesh.indicesCount);
                    }
                    mesh.indicesCount += 4;

                    // Finally, add the face to the mask so they aren't repeated
                    for (int y = 0; y < quadSize[sweepDirA]; y++) {
                        for (int x = 0; x < quadSize[sweepDirB]; x++) {
                            mask.at(maskIdx(start[sweepDirA] + y,
start[sweepDirB] + x)) = true;
                        }
                    }
                }
            }
        }
    }
    return mesh;
}
*/