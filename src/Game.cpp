#include "Game.h"

#include "Chunks/ChunkMesh.h"
#include "Chunks/ChunkTerrainGen.h"
#include "Chunks/Voxels.h"
#include "Utility.h"

Game::Game()
{
    m_sceneShader.loadFromFile("SceneVertex.glsl", "SceneFragment.glsl");
    m_terrain.bufferMesh(createTerrainMesh(256, 256, true));
    m_lightCube.bufferMesh(createCubeMesh({10.5f, 10.5f, 10.5f}));
    m_texture.loadFromFile("OpenGLLogo.png", 8);

    m_voxelShader.loadFromFile("TerrainVertex.glsl", "TerrainFragment.glsl");

    float aspect = (float)WIDTH / (float)HEIGHT;
    m_projectionMatrix = createProjectionMatrix(aspect, 90.0f);

    m_lightTransform = {{0, 15, 0}, {0, 0, 0}};

    m_chunkTextures.create(16, 16);
    initVoxelSystem(m_chunkTextures);

    int worldHeight = 4;
    int worldWidth = 4;
    m_cameraTransform = {{50, worldHeight * CHUNK_SIZE, 50}, {0, 270, 0}};

    for (int cx = 0; cx < 2; cx++)
    {
        for (int cy = 0; cy < 4; cy++)
        {
            Chunk& c = m_chunkMap.addChunk({cx, cy});
            createChunkTerrain(c, cx, cy, worldWidth, worldHeight, {});

            ChunkMesh mesh = createGreedyChunkMesh(c);
            VertexArray chunkVertexArray;
            chunkVertexArray.bufferMesh(mesh);
            int verts = mesh.vertices.size();
            int faces = verts / 4;
            std::cout << "Verts " << verts << "    faces     " << faces << std::endl;
            m_chunkRenderables.emplace_back(mesh.chunkPos, chunkVertexArray.getRendable(),
                                            verts, faces);
            m_chunkVertexArrays.push_back(std::move(chunkVertexArray));
        }
    }
}

void Game::onInput(const Keyboard& keyboard, const sf::Window& window, bool isMouseActive)
{
    Transform& camera = m_cameraTransform;

    float PLAYER_SPEED = 0.5f;
    if (keyboard.isKeyDown(sf::Keyboard::LControl))
    {
        PLAYER_SPEED = 5.0f;
    }
    if (keyboard.isKeyDown(sf::Keyboard::W))
    {
        camera.position += forwardsVector(camera.rotation) * PLAYER_SPEED;
    }
    else if (keyboard.isKeyDown(sf::Keyboard::S))
    {
        camera.position += backwardsVector(camera.rotation) * PLAYER_SPEED;
    }
    if (keyboard.isKeyDown(sf::Keyboard::A))
    {
        camera.position += leftVector(camera.rotation) * PLAYER_SPEED;
    }
    else if (keyboard.isKeyDown(sf::Keyboard::D))
    {
        camera.position += rightVector(camera.rotation) * PLAYER_SPEED;
    }

    if (!isMouseActive)
    {
        return;
    }
    static auto lastMousePosition = sf::Mouse::getPosition(window);
    auto change = sf::Mouse::getPosition(window) - lastMousePosition;
    camera.rotation.x -= static_cast<float>(change.y * 0.5);
    camera.rotation.y += static_cast<float>(change.x * 0.5);
    sf::Mouse::setPosition({(int)window.getSize().x / 2, (int)window.getSize().y / 2},
                           window);
    lastMousePosition.x = (int)window.getSize().x / 2;
    lastMousePosition.y = (int)window.getSize().y / 2;

    camera.rotation.x = glm::clamp(camera.rotation.x, -89.9f, 89.9f);
    camera.rotation.y = (int)camera.rotation.y % 360;
}

void Game::onUpdate()
{
    m_lightTransform.position.y += std::sin(m_timer.getElapsedTime().asSeconds());
}

void Game::onRender()
{
    setClearColour(COLOUR_SKY_BLUE);

    auto viewMatrix = createViewMartix(m_cameraTransform, {0, 1, 0});
    auto projectionViewMatrix = m_projectionMatrix * viewMatrix;
    m_frustum.update(projectionViewMatrix);

    Framebuffer::unbind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderScene(projectionViewMatrix);
}

void Game::onGUI() { guiDebugScreen(m_cameraTransform); }

void Game::renderScene(const glm::mat4& projectionViewMatrix)
{
    // Normal stuff
    glDisable(GL_CULL_FACE);
    m_sceneShader.bind();
    m_sceneShader.set("projectionViewMatrix", projectionViewMatrix);
    m_sceneShader.set("isLight", false);
    m_sceneShader.set("lightColour", glm::vec3{1.0, 1.0, 1.0});
    m_sceneShader.set("eyePosition", m_cameraTransform.position);

    m_texture.bind(0);

    glm::mat4 terrainModel{1.0f};
    terrainModel = glm::translate(terrainModel, {0, 0, 0});
    m_sceneShader.set("modelMatrix", terrainModel);
    m_sceneShader.set("lightPosition", m_lightTransform.position);
    m_terrain.getRendable().drawElements();

    glEnable(GL_CULL_FACE);
    auto lightModel = createModelMatrix(m_lightTransform);
    m_sceneShader.set("modelMatrix", lightModel);
    m_sceneShader.set("isLight", true);
    m_lightCube.getRendable().drawElements();

    // Chunks
    m_voxelShader.bind();
    m_voxelShader.set("projectionViewMatrix", projectionViewMatrix);
    //   m_voxelShader.set("lightColour", glm::vec3{1.0, 1.0, 1.0});
    //   m_voxelShader.set("lightDirection", m_lightTransform.rotation);
    //    m_voxelShader.set("eyePosition", m_cameraTransform.position);
    m_chunkTextures.bind();

    glm::mat4 voxelModel{1.0f};
    voxelModel = glm::translate(voxelModel, {0, 0, 0});
    m_voxelShader.set("modelMatrix", voxelModel);

    for (auto& chunk : m_chunkRenderables)
    {
        // if (m_frustum.chunkIsInFrustum(chunk.position)) {
        chunk.renderable.drawElements();
        //    if (count) {
        //        m_stats.chunksDrawn++;
        //        m_stats.verticiesDrawn += chunk.numVerts;
        //        m_stats.blockFacesDrawn += chunk.numFaces;
        //    }
        //}
    }
}
