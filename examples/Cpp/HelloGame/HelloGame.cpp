/*
 * HelloGame.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <LLGL/Utils/ForRange.h>
#include "FileUtils.h"
#include <algorithm>
#include <limits.h>


// Enables cheats by allowing page up/down to select next or previous level
#define ENABLE_CHEATS 0


class Example_HelloGame : public ExampleBase
{

    static constexpr float  levelTransitionSpeed    = 0.5f; // in seconds
    static constexpr float  levelDoneSpeed          = 1.0f; // in seconds
    static constexpr float  playerMoveSpeed         = 0.25f; // in seconds
    static constexpr float  playerFallAcceleration  = 2.0f; // in units per seconds
    static constexpr float  warpEffectDuration      = 1.0f; // in seconds
    static constexpr int    warpEffectBounces       = 3;
    static constexpr float  warpEffectScale         = 2.0f;
    static constexpr float  wallPosY                = 2.0f;
    static constexpr int    inputStackSize          = 4;
    static constexpr float  playerColor[3]          = { 0.6f, 0.7f, 1.0f };
    static constexpr float  treeColorGradient[2][3] = { { 0.1f, 0.5f, 0.1f }, { 0.2f, 0.8f, 0.3f } };
    static constexpr float  treeAnimSpeed           = 8.0f; // in seconds
    static constexpr float  treeAnimRadius          = 0.2f;
    static constexpr int    shadowMapSize           = 512;

    LLGL::PipelineLayout*   scenePSOLayout[2]       = {};
    LLGL::PipelineState*    scenePSO[2]             = {};
    ShaderPipeline          sceneShaders;

    LLGL::PipelineLayout*   groundPSOLayout         = nullptr;
    LLGL::PipelineState*    groundPSO               = nullptr;
    ShaderPipeline          groundShaders;

    LLGL::Buffer*           cbufferScene            = nullptr;
    LLGL::Buffer*           vertexBuffer            = nullptr;
    LLGL::Buffer*           instanceBuffer          = nullptr;
    std::uint32_t           instanceBufferCapacity  = 0; // Number of instances the instance buffer can hold

    LLGL::Texture*          groundColorMap          = nullptr;
    LLGL::Sampler*          groundColorMapSampler   = nullptr;

    LLGL::Texture*          shadowMap               = nullptr;
    LLGL::Sampler*          shadowMapSampler        = nullptr;
    LLGL::RenderTarget*     shadowMapTarget         = nullptr;

    LLGL::VertexFormat      vertexFormat;

    TriangleMesh            mdlPlayer;
    TriangleMesh            mdlBlock;
    TriangleMesh            mdlTree;
    TriangleMesh            mdlGround;

    struct alignas(16) Scene
    {
        Gs::Matrix4f    vpMatrix;
        Gs::Matrix4f    vpShadowMatrix;
        Gs::Vector3f    lightDir        = Gs::Vector3f(-0.25f, -0.7f, 1.25f).Normalized();
        float           shininess       = 90.0f;                                            // Blinn-phong specular power factor
        Gs::Vector3f    viewPos;                                                        // World-space camera position
        float           shadowSizeInv   = 0.0f;
        Gs::Vector3f    warpCenter;
        float           warpIntensity   = 0.0f;
        Gs::Vector3f    bendDir;
        float           ambientItensity = 0.6f;
        float           groundTint[3]   = { 0.8f, 1.0f, 0.6f };
        float           groundScale     = 2.0f;
    }
    scene;

    struct alignas(4) Uniforms
    {
        float           worldOffset[3]  = { 0.0f, 0.0f, 0.0f };
        float           bendIntensity   = 0.0f;
        std::uint32_t   firstInstance   = 0;
    }
    uniforms;

    struct Vertex
    {
        float           position[3];
        float           normal[3];
        float           texCoord[2];
    };

    struct Instance
    {
        float           wMatrix[3][4];
        float           color[4];
    };

    // Decor for trees in the background
    struct Decor
    {
        int             gridPos[2]      = {};
        std::uint32_t   instanceIndex   = 0;
    };

    struct Tile
    {
        Tile() :
            instanceIndex { ~0u },
            isActive      { 0   }
        {
        }

        std::uint32_t   instanceIndex; // Index into 'meshInstances'
        std::uint32_t   isActive : 1;

        bool IsValid() const
        {
            return (instanceIndex != ~0u);
        }

        bool IsActivated() const
        {
            return (isActive != 0);
        }
    };

    struct TileRow
    {
        std::vector<Tile> tiles;
    };

    struct TileGrid
    {
        std::vector<TileRow>    rows;
        int                     gridSize[2] = {}; // Bounding box in grid coordinates

        void Resize(int width, int height)
        {
            if (width > gridSize[0] || height > gridSize[1])
            {
                gridSize[0] = std::max<int>(gridSize[0], width  + 1);
                gridSize[1] = std::max<int>(gridSize[1], height + 1);

                rows.resize(static_cast<std::size_t>(gridSize[1]));
                for (TileRow& row : rows)
                    row.tiles.resize(static_cast<std::size_t>(gridSize[0]));
            }
        }

        void Put(int x, int y, const Tile* tile)
        {
            Resize(x + 1, y + 1);
            if (tile != nullptr)
                *Get(x, y) = *tile;
        }

        Tile* Get(int x, int y)
        {
            return (x >= 0 && x < gridSize[0] && y >= 0 && y < gridSize[1] ? &(rows[y].tiles[x]) : nullptr);
        }

        const Tile* Get(int x, int y) const
        {
            return (x >= 0 && x < gridSize[0] && y >= 0 && y < gridSize[1] ? &(rows[y].tiles[x]) : nullptr);
        }

        // Counts all valid tile in this grid
        std::uint32_t CountTiles() const
        {
            std::uint32_t n = 0;
            for (const TileRow& row : rows)
            {
                for (const Tile& tile : row.tiles)
                {
                    if (tile.IsValid())
                        ++n;
                }
            }
            return n;
        };
    };

    struct Player;

    struct InstanceRange
    {
        std::uint32_t begin = ~0u;
        std::uint32_t end   = 0u;

        std::uint32_t Count() const
        {
            return (begin < end ? end - begin : 0);
        }

        void Invalidate()
        {
            begin   = ~0u;
            end     = 0u;
        }

        void Insert(std::uint32_t newBegin, std::uint32_t newEnd)
        {
            begin   = std::min<std::uint32_t>(begin, newBegin);
            end     = std::max<std::uint32_t>(end, newEnd);
        }

        void Insert(std::uint32_t index)
        {
            Insert(index, index + 1);
        }
    };

    struct Level
    {
        std::string             name;
        LLGL::ColorRGBub        wallColors[2];
        TileGrid                floor;
        TileGrid                walls;
        std::vector<Decor>      trees;
        std::vector<Instance>   meshInstances;
        InstanceRange           meshInstanceDirtyRange;
        InstanceRange           tileInstanceRange;
        InstanceRange           treeInstanceRange;
        int                     gridSize[2]                 = {}; // Bounding box in grid coordinates
        int                     playerStart[2]              = {};
        float                   viewDistance                = 0.0f;
        int                     activatedTiles              = 0;
        int                     maxTilesToActivate          = 0;

        bool IsWall(int x, int y) const
        {
            const Tile* tile = walls.Get(x, y);
            return (tile != nullptr && tile->IsValid());
        }

        bool IsFloor(int x, int y) const
        {
            const Tile* tile = floor.Get(x, y);
            return (tile != nullptr && tile->IsValid());
        }

        bool IsTileBlocked(int x, int y) const
        {
            if (IsWall(x, y))
                return true;
            const Tile* tile = floor.Get(x, y);
            return (tile != nullptr && tile->IsActivated());
        }

        bool IsTileHole(int x, int y) const
        {
            const Tile* tile = floor.Get(x, y);
            return (tile == nullptr || !tile->IsValid());
        }

        // Returns true if all tiles have been activated.
        bool IsCompleted() const
        {
            return (activatedTiles == maxTilesToActivate);
        }

        // Activates the specified tile and returns true if this was the last tile to activate.
        bool ActivateTile(int x, int y, const float (&color)[3])
        {
            Tile* tile = floor.Get(x, y);
            if (tile != nullptr && tile->IsValid())
            {
                if (tile->isActive == 0)
                {
                    tile->isActive = 1;
                    Instance& instance = meshInstances[tile->instanceIndex];
                    {
                        instance.color[0] = color[0];
                        instance.color[1] = color[1];
                        instance.color[2] = color[2];
                    }
                    meshInstanceDirtyRange.Insert(tile->instanceIndex);
                    ++activatedTiles;
                    return IsCompleted();
                }
            }
            return false;
        }

        void PutPlayer(Player& player)
        {
            player.Put(playerStart);
            ActivateTile(playerStart[0], playerStart[1], playerColor);
        }

        void ResetTiles()
        {
            activatedTiles      = 0;
            maxTilesToActivate  = 0;

            for_range(row, gridSize[1])
            {
                for_range(col, gridSize[0])
                {
                    Tile* floorTile = floor.Get(col, row);
                    if (floorTile != nullptr && floorTile->IsValid())
                    {
                        Instance& floorInstance = meshInstances[floorTile->instanceIndex];

                        const Tile* wallTile = walls.Get(col, row);
                        if (wallTile != nullptr && wallTile->IsValid())
                        {
                            // Copy floor color from wall if it's underneath
                            const Instance& wallInstance = meshInstances[wallTile->instanceIndex];
                            floorInstance.color[0] = wallInstance.color[0];
                            floorInstance.color[1] = wallInstance.color[1];
                            floorInstance.color[2] = wallInstance.color[2];
                        }
                        else
                        {
                            // Reset floor color to default
                            floorInstance.color[0] = 1.0f;
                            floorInstance.color[1] = 1.0f;
                            floorInstance.color[2] = 1.0f;
                            ++maxTilesToActivate;
                        }

                        // Reset tile
                        floorTile->isActive = 0;
                    }
                }
            }
        }
    };

    struct Camera
    {
        float           viewDistance = 0.0f;
        Gs::Vector2f    levelCenterPos;

        void FocusOnLevel(const Level& level)
        {
            viewDistance = level.viewDistance;
            levelCenterPos.x = static_cast<float>(level.gridSize[0]) - 1.0f;
            levelCenterPos.y = static_cast<float>(level.gridSize[1]) - 1.0f;
        }

        void TransitionBetweenLevels(const Level& levelA, const Level& levelB, float transition)
        {
            Camera camA, camB;
            camA.FocusOnLevel(levelA);
            camB.FocusOnLevel(levelB);
            viewDistance    = Gs::Lerp(camA.viewDistance, camB.viewDistance, transition);
            levelCenterPos  = Gs::Lerp(camA.levelCenterPos, camB.levelCenterPos, transition);
        }
    }
    camera;

    struct Player
    {
        Instance    instance                    = {};
        int         gridPos[2]                  = {};
        int         moveDirStack                = 0;
        int         moveDir[inputStackSize][2]  = {};
        float       moveTransition              = 0.0f;
        bool        isFalling                   = false;
        float       fallDepth                   = 0.0f;
        float       fallVelocity                = 0.0f;

        void Move(int moveX, int moveZ)
        {
            if (moveDirStack < inputStackSize)
            {
                for_subrange_reverse(i, 1, moveDirStack + 1)
                {
                    moveDir[i][0] = moveDir[i - 1][0];
                    moveDir[i][1] = moveDir[i - 1][1];
                }
                moveDir[0][0] = moveX;
                moveDir[0][1] = moveZ;
                ++moveDirStack;
            }
        }

        void Put(const int (&pos)[2])
        {
            // Reset all player states and place onto grid position
            gridPos[0]      = pos[0];
            gridPos[1]      = pos[1];
            moveDirStack    = 0;
            moveTransition  = 0.0f;
            isFalling       = false;
            fallDepth       = 0.0f;
            fallVelocity    = 0.0f;
        }
    }
    player;

    std::vector<Level>  levels;
    int                 currentLevelIndex   = -1;
    Level*              currentLevel        = nullptr;
    Level*              nextLevel           = nullptr;
    float               levelTransition     = 0.0f; // Transitioning state between two levesl - in the range [0, 1]
    float               levelDistance       = 0.0f; // Distance between two levels (to transition between them)
    std::uint32_t       levelInstanceOffset = 0;
    float               levelDoneTransition = 0.0f; // Transition starting when the level is completed

    struct Gradient
    {
        Gs::Vector3f    points[2];
        LLGL::ColorRGBf colors[2];

        LLGL::ColorRGBf operator() (const Gs::Vector3f& p) const
        {
            const Gs::Vector3f closestPoint = ClosestPointOnLineSegment(points[0], points[1], p);
            const float closestPointDist = Gs::Distance(points[0], closestPoint);
            const float segmentLength = Gs::Distance(points[0], points[1]);
            const float interpolation = closestPointDist / segmentLength;
            return Gs::Lerp(colors[0], colors[1], interpolation);
        }
    };

    struct Effects
    {
        bool    warpEnabled     = false;
        float   warpTime        = 0.0f;
        float   treeBendTime    = 0.0f;

        void StartWarp()
        {
            warpEnabled = true;
            warpTime    = 0.0f;
        }
    }
    effects;

public:

    Example_HelloGame() :
        ExampleBase { "LLGL Example: HelloGame" }
    {
        // Create all graphics objects
        auto vertexFormat = CreateResources();
        CreateShaders(vertexFormat);
        CreatePipelines();
        LoadLevels();
        SelectLevel(0);
    }

private:

    LLGL::VertexFormat CreateResources()
    {
        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.attributes =
        {
            LLGL::VertexAttribute{ "position", LLGL::Format::RGB32Float, /*location:*/ 0, offsetof(Vertex, position), sizeof(Vertex) },
            LLGL::VertexAttribute{ "normal",   LLGL::Format::RGB32Float, /*location:*/ 1, offsetof(Vertex, normal  ), sizeof(Vertex) },
            LLGL::VertexAttribute{ "texCoord", LLGL::Format::RG32Float,  /*location:*/ 2, offsetof(Vertex, texCoord), sizeof(Vertex) },
        };

        // Load 3D models
        std::vector<TexturedVertex> vertices;
        mdlPlayer   = LoadObjModel(vertices, "HelloGame_Player.obj");
        mdlBlock    = LoadObjModel(vertices, "HelloGame_Block.obj");
        mdlTree     = LoadObjModel(vertices, "HelloGame_Tree.obj");
        mdlGround   = LoadObjModel(vertices, "HelloGame_Ground.obj");

        // Create vertex, index, and constant buffer
        vertexBuffer    = CreateVertexBuffer(vertices, vertexFormat);
        cbufferScene    = CreateConstantBuffer(scene);

        // Load texture and sampler
        groundColorMap = LoadTexture("Grass.jpg");
        groundColorMapSampler = renderer->CreateSampler({});

        // Create shadow map
        LLGL::TextureDescriptor shadowMapDesc;
        {
            shadowMapDesc.debugName     = "ShadowMap";
            shadowMapDesc.type          = LLGL::TextureType::Texture2D;
            shadowMapDesc.bindFlags     = LLGL::BindFlags::DepthStencilAttachment | LLGL::BindFlags::Sampled;
            shadowMapDesc.format        = LLGL::Format::D32Float;
            shadowMapDesc.extent.width  = static_cast<std::uint32_t>(shadowMapSize);
            shadowMapDesc.extent.height = static_cast<std::uint32_t>(shadowMapSize);
            shadowMapDesc.extent.depth  = 1;
            shadowMapDesc.mipLevels     = 1;
        }
        shadowMap = renderer->CreateTexture(shadowMapDesc);

        LLGL::RenderTargetDescriptor shadowTargetDesc;
        {
            shadowTargetDesc.debugName              = "ShadowMapTarget";
            shadowTargetDesc.resolution.width       = shadowMapDesc.extent.width;
            shadowTargetDesc.resolution.height      = shadowMapDesc.extent.height;
            shadowTargetDesc.depthStencilAttachment = shadowMap;
        }
        shadowMapTarget = renderer->CreateRenderTarget(shadowTargetDesc);

        LLGL::SamplerDescriptor shadowSamplerDesc;
        {
            // Clamp-to-border sampler address mode requires GLES 3.2, so use standard clamp mode in case hardware only supports GLES 3.0
            if (renderer->GetRendererID() == LLGL::RendererID::OpenGLES ||
                renderer->GetRendererID() == LLGL::RendererID::WebGL)
            {
                shadowSamplerDesc.addressModeU      = LLGL::SamplerAddressMode::Clamp;
                shadowSamplerDesc.addressModeV      = LLGL::SamplerAddressMode::Clamp;
                shadowSamplerDesc.addressModeW      = LLGL::SamplerAddressMode::Clamp;
            }
            else
            {
                shadowSamplerDesc.addressModeU      = LLGL::SamplerAddressMode::Border;
                shadowSamplerDesc.addressModeV      = LLGL::SamplerAddressMode::Border;
                shadowSamplerDesc.addressModeW      = LLGL::SamplerAddressMode::Border;
                shadowSamplerDesc.borderColor[0]    = 1.0f;
                shadowSamplerDesc.borderColor[1]    = 1.0f;
                shadowSamplerDesc.borderColor[2]    = 1.0f;
                shadowSamplerDesc.borderColor[3]    = 1.0f;
            }
            shadowSamplerDesc.compareEnabled    = true;
            shadowSamplerDesc.mipMapEnabled     = false;
        }
        shadowMapSampler = renderer->CreateSampler(shadowSamplerDesc);

        // Pass inverse size of shadow map to shader for PCF shadow mapping
        scene.shadowSizeInv = 1.0f / static_cast<float>(shadowMapSize);

        return vertexFormat;
    }

    void CreateInstanceBuffer(std::uint32_t numInstances)
    {
        // Check if the buffer must be resized
        if (numInstances <= instanceBufferCapacity)
            return;

        // Release previous buffers
        if (instanceBuffer != nullptr)
            renderer->Release(*instanceBuffer);

        // Create instance buffer from mesh instance data
        LLGL::BufferDescriptor instanceBufferDesc;
        {
            instanceBufferDesc.debugName    = "InstanceBuffer";
            instanceBufferDesc.size         = sizeof(Instance) * numInstances;
            instanceBufferDesc.stride       = sizeof(Instance);
            instanceBufferDesc.bindFlags    = LLGL::BindFlags::Sampled;
        }
        instanceBuffer = renderer->CreateBuffer(instanceBufferDesc);
        instanceBufferCapacity = numInstances;
    }

    void CreateShaders(const LLGL::VertexFormat& vertexFormat)
    {
        if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            sceneShaders.vs  = LoadShader({ LLGL::ShaderType::Vertex,   "HelloGame.hlsl", "VSInstance", "vs_5_0" }, { vertexFormat });
            sceneShaders.ps  = LoadShader({ LLGL::ShaderType::Fragment, "HelloGame.hlsl", "PSInstance", "ps_5_0" });

            groundShaders.vs = LoadShader({ LLGL::ShaderType::Vertex,   "HelloGame.hlsl", "VSGround",   "vs_5_0" }, { vertexFormat });
            groundShaders.ps = LoadShader({ LLGL::ShaderType::Fragment, "HelloGame.hlsl", "PSGround",   "ps_5_0" });
        }
#if 0
        else if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            sceneShaders.vs = LoadShader({ LLGL::ShaderType::Vertex,     "HelloGame.vert" }, { vertexFormat });
            sceneShaders.ps = LoadShader({ LLGL::ShaderType::Fragment,   "HelloGame.frag" });
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            sceneShaders.vs = LoadShader({ LLGL::ShaderType::Vertex,     "HelloGame.450core.vert.spv" }, { vertexFormat });
            sceneShaders.ps = LoadShader({ LLGL::ShaderType::Fragment,   "HelloGame.450core.frag.spv" });
        }
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            sceneShaders.vs = LoadShader({ LLGL::ShaderType::Vertex,     "HelloGame.metal", "VSInstance", "vs_5_0" }, { vertexFormat });
            sceneShaders.ps = LoadShader({ LLGL::ShaderType::Fragment,   "HelloGame.metal", "PSInstance", "ps_5_0" });
        }
#endif
        else
        {
            throw std::runtime_error("No shaders provided for this backend");
        }
    }

    void CreatePipelines()
    {
        // Create PSO for instanced meshes
        scenePSOLayout[0] = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(Scene@1):vert:frag,"
                "buffer(instances@2):vert,"
                "texture(shadowMap@3):frag,"
                "sampler(shadowMapSampler@3):frag,"
                "float3(worldOffset),"
                "float(bendIntensity),"
                "uint(firstInstance),"
            )
        );

        LLGL::GraphicsPipelineDescriptor scenePSODesc;
        {
            scenePSODesc.debugName                      = "InstancedMesh.PSO";
            scenePSODesc.pipelineLayout                 = scenePSOLayout[0];
            scenePSODesc.vertexShader                   = sceneShaders.vs;
            scenePSODesc.fragmentShader                 = sceneShaders.ps;
            scenePSODesc.renderPass                     = swapChain->GetRenderPass();
            scenePSODesc.depth.testEnabled              = true;
            scenePSODesc.depth.writeEnabled             = true;
            scenePSODesc.rasterizer.cullMode            = LLGL::CullMode::Back;
            scenePSODesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
        }
        scenePSO[0] = renderer->CreatePipelineState(scenePSODesc);
        ReportPSOErrors(scenePSO[0]);

        // Create PSO for shadow-mapping but without a fragment shader
        scenePSOLayout[1] = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(Scene@1):vert,"
                "buffer(instances@2):vert,"
                "float3(worldOffset),"
                "float(bendIntensity),"
                "uint(firstInstance),"
            )
        );

        {
            scenePSODesc.debugName                              = "InstancedMesh.Shadow.PSO";
            scenePSODesc.pipelineLayout                         = scenePSOLayout[1];
            scenePSODesc.fragmentShader                         = nullptr;
            scenePSODesc.rasterizer.depthBias.constantFactor    = 4.0f;
            scenePSODesc.rasterizer.depthBias.slopeFactor       = 1.5f;
            scenePSODesc.rasterizer.cullMode                    = LLGL::CullMode::Front;
            scenePSODesc.blend.targets[0].colorMask             = 0x0;
        }
        scenePSO[1] = renderer->CreatePipelineState(scenePSODesc);
        ReportPSOErrors(scenePSO[1]);

        // Create PSO for background
        groundPSOLayout = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(Scene@1):vert,"
                "texture(colorMap@0):frag,"
                "sampler(colorMapSampler@0):frag,"
                "texture(shadowMap@3):frag,"
                "sampler(shadowMapSampler@3):frag,"
            )
        );

        LLGL::GraphicsPipelineDescriptor groundPSODesc;
        {
            groundPSODesc.debugName                     = "Ground.PSO";
            groundPSODesc.pipelineLayout                = groundPSOLayout;
            groundPSODesc.vertexShader                  = groundShaders.vs;
            groundPSODesc.fragmentShader                = groundShaders.ps;
            groundPSODesc.renderPass                    = swapChain->GetRenderPass();
            groundPSODesc.depth.testEnabled             = true;
            groundPSODesc.depth.writeEnabled            = true;
            groundPSODesc.rasterizer.cullMode           = LLGL::CullMode::Back;
            groundPSODesc.rasterizer.multiSampleEnabled = (GetSampleCount() > 1);
        }
        groundPSO = renderer->CreatePipelineState(groundPSODesc);
        ReportPSOErrors(groundPSO);
    }

    static Gs::Vector3f GridPosToWorldPos(const int (&gridPos)[2], float posY)
    {
        const float posX = static_cast<float>(gridPos[0])*2.0f;
        const float posZ = static_cast<float>(gridPos[1])*2.0f;
        return Gs::Vector3f{ posX, posY, posZ };
    }

    static void RotateAroundPivot(Gs::AffineMatrix4f& outMatrix, const Gs::Vector3f& pivot, const Gs::Vector3f& axis, float angle)
    {
        Gs::Matrix3f rotation;
        Gs::RotateFree(rotation, axis, angle);
        const Gs::Vector3f offset = rotation * pivot;

        Gs::Translate(outMatrix, pivot - offset);
        Gs::RotateFree(outMatrix, axis, angle);
    }

    void SetPlayerTransform(Gs::AffineMatrix4f& outMatrix, const int (&gridPosA)[2], int moveX, int moveZ, float posY, float transition)
    {
        outMatrix.LoadIdentity();

        const Gs::Vector3f posA = GridPosToWorldPos(gridPosA, posY);
        Gs::Translate(outMatrix, posA);

        if (transition > 0.0f)
        {
            const float angle = Gs::SmoothStep(transition) * Gs::pi * 0.5f;
            if (moveX < 0)
            {
                // Move left
                RotateAroundPivot(outMatrix, Gs::Vector3f{ -1,-1, 0 }, Gs::Vector3f{ 0,0,1 }, -angle);
            }
            else if (moveX > 0)
            {
                // Move right
                RotateAroundPivot(outMatrix, Gs::Vector3f{ +1,-1, 0 }, Gs::Vector3f{ 0,0,1 }, +angle);
            }
            else if (moveZ < 0)
            {
                // Move forwards
                RotateAroundPivot(outMatrix, Gs::Vector3f{  0,-1,-1 }, Gs::Vector3f{ 1,0,0 }, +angle);
            }
            else if (moveZ > 0)
            {
                // Move backwards
                RotateAroundPivot(outMatrix, Gs::Vector3f{  0,-1,+1 }, Gs::Vector3f{ 1,0,0 }, -angle);
            }
        }
    }

    void SetInstanceAttribs(Instance& instance, const int (&gridPos)[2], float posY, const Gradient* gradient = nullptr)
    {
        Gs::AffineMatrix4f& wMatrix = *reinterpret_cast<Gs::AffineMatrix4f*>(instance.wMatrix);
        wMatrix.LoadIdentity();

        const Gs::Vector3f pos = GridPosToWorldPos(gridPos, posY);
        Gs::Translate(wMatrix, pos);

        LLGL::ColorRGBf color;
        if (gradient != nullptr)
            color = (*gradient)(pos);

        instance.color[0] = color.r;
        instance.color[1] = color.g;
        instance.color[2] = color.b;
        instance.color[3] = 1.0f;
    }

    void GenerateTileInstances(TileGrid& grid, std::vector<Instance>& meshInstances, std::uint32_t& instanceCounter, float posY, const Gradient* gradient)
    {
        int gridPos[2];

        for (gridPos[1] = 0; gridPos[1] < grid.gridSize[1]; ++gridPos[1])
        {
            TileRow& row = grid.rows[gridPos[1]];
            for (gridPos[0] = 0; gridPos[0] < grid.gridSize[0]; ++gridPos[0])
            {
                Tile& tile = row.tiles[gridPos[0]];
                if (tile.IsValid())
                {
                    tile.instanceIndex = instanceCounter++;
                    SetInstanceAttribs(meshInstances[tile.instanceIndex], gridPos, posY, gradient);
                }
            }
        }
    }

    void GenerateDecorInstances(std::vector<Decor>& decors, std::vector<Instance>& meshInstances, std::uint32_t& instanceCounter, const Gradient* gradient)
    {
        for (Decor& decor : decors)
        {
            decor.instanceIndex = instanceCounter++;
            SetInstanceAttribs(meshInstances[decor.instanceIndex], decor.gridPos, 0.0f, gradient);
        }
    }

    void FinalizeLevel(Level& level)
    {
        // Build instance data from tiles
        level.meshInstances.resize(level.floor.CountTiles() + level.walls.CountTiles() + level.trees.size());

        // Colorize wall tiles with gradient
        Gradient gradient;
        gradient.colors[0] = level.wallColors[0].Cast<float>();
        gradient.colors[1] = level.wallColors[1].Cast<float>();
        gradient.points[0] = Gs::Vector3f{ 0.0f, wallPosY, 0.0f };
        gradient.points[1] = Gs::Vector3f{ static_cast<float>(level.gridSize[0])*2.0f, wallPosY, static_cast<float>(level.gridSize[1])*2.0f };

        std::uint32_t instanceCounter = 0;
        GenerateTileInstances(level.floor, level.meshInstances, instanceCounter, 0.0f, nullptr);
        GenerateTileInstances(level.walls, level.meshInstances, instanceCounter, wallPosY, &gradient);
        level.tileInstanceRange.Insert(0, instanceCounter);

        // Colorize tree decors with gradient
        gradient.colors[0] = LLGL::ColorRGBf{ treeColorGradient[0][0], treeColorGradient[0][1], treeColorGradient[0][2] };
        gradient.colors[1] = LLGL::ColorRGBf{ treeColorGradient[1][0], treeColorGradient[1][1], treeColorGradient[1][2] };
        gradient.points[0] = Gs::Vector3f{ 0.0f, wallPosY, 0.0f };
        gradient.points[1] = Gs::Vector3f{ static_cast<float>(level.gridSize[0])*2.0f, wallPosY, static_cast<float>(level.gridSize[1])*2.0f };

        std::uint32_t treeInstanceStart = instanceCounter;
        GenerateDecorInstances(level.trees, level.meshInstances, instanceCounter, &gradient);
        level.treeInstanceRange.Insert(treeInstanceStart, instanceCounter);
    }

    void LoadLevels()
    {
        // Load level files
        const std::vector<std::string> levelsFileLines = ReadTextLines("HelloGame.levels.txt");

        std::string name;
        std::string wallGradient;
        std::vector<std::string> currentGrid;

        auto HexToInt = [](char c) -> int
        {
            if (c >= '0' && c <= '9')
                return (c - '0');
            else if (c >= 'a' && c <= 'f')
                return 0xA + (c - 'a');
            else if (c >= 'A' && c <= 'F')
                return 0xA + (c - 'A');
            else
                return -1;
        };

        auto ParseColorRGB = [&HexToInt](const char*& s) -> LLGL::ColorRGBub
        {
            std::uint32_t color = 0;
            char c;

            // Skip characters until first hex digit is found
            while ((c = *s) != '\0' && HexToInt(c) == -1)
                ++s;

            while ((c = *s) != '\0' && HexToInt(c) != -1)
            {
                color <<= 4;
                color |= HexToInt(c);
                ++s;
            }

            return LLGL::ColorRGBub
            {
                ((color >> 16) & 0xFF),
                ((color >>  8) & 0xFF),
                ((color      ) & 0xFF)
            };
        };

        auto FlushLevelConstruct = [&]() -> void
        {
            if (currentGrid.empty())
                return;

            // Construct a new level
            Level newLevel;

            newLevel.name = (name.empty() ? "Unnamed" : name);

            if (!wallGradient.empty())
            {
                const char* wallGradientStr = wallGradient.c_str();
                newLevel.wallColors[0] = ParseColorRGB(wallGradientStr);
                newLevel.wallColors[1] = ParseColorRGB(wallGradientStr);
            }
            else
            {
                newLevel.wallColors[0] = {};
                newLevel.wallColors[1] = {};
            }

            // Determine longest row
            int gridOffset[2] = { INT_MAX, INT_MAX };
            newLevel.gridSize[0] = 0;
            newLevel.gridSize[1] = 0;

            for_range(i, currentGrid.size())
            {
                const std::string& row = currentGrid[i];
                const std::size_t rowStart = row.find_first_of(".#@");
                if (rowStart != std::string::npos)
                {
                    gridOffset[0] = std::min<int>(gridOffset[0], static_cast<int>(rowStart));
                    gridOffset[1] = std::min<int>(gridOffset[1], static_cast<int>(i));
                    const std::size_t rowEnd = row.find_last_of(".#@");
                    newLevel.gridSize[0] = std::max<int>(newLevel.gridSize[0], static_cast<int>(rowEnd - rowStart) + 1);
                    newLevel.gridSize[1]++;
                }
            }

            newLevel.viewDistance = static_cast<float>(std::max<int>(newLevel.gridSize[0], newLevel.gridSize[1])) * 2.7f;

            // Build grid of tiles row by row by interpreting characters from the level text file
            Tile initialTile;
            initialTile.instanceIndex = 0;

            int gridPosY = newLevel.gridSize[1] + gridOffset[1];
            for (const std::string& row : currentGrid)
            {
                --gridPosY;
                int gridPosX = -gridOffset[0];

                for (char c : row)
                {
                    if (c == '#')
                    {
                        // Add floor and wall tile
                        newLevel.floor.Put(gridPosX, gridPosY, &initialTile);
                        newLevel.walls.Put(gridPosX, gridPosY, &initialTile);
                    }
                    else if (c == '.')
                    {
                        // Add floor tile only
                        newLevel.floor.Put(gridPosX, gridPosY, &initialTile);
                    }
                    else if (c == '@')
                    {
                        // Add floor tile and position player
                        newLevel.floor.Put(gridPosX, gridPosY, &initialTile);
                        newLevel.playerStart[0] = gridPosX;
                        newLevel.playerStart[1] = gridPosY;
                    }
                    else if (c == '$')
                    {
                        // Add floor tile only
                        Decor newDecor;
                        newDecor.gridPos[0] = gridPosX;
                        newDecor.gridPos[1] = gridPosY;
                        newLevel.trees.push_back(newDecor);
                    }
                    ++gridPosX;
                }
            }

            // Finalize level by generating mesh instances for all tiles
            FinalizeLevel(newLevel);

            // Flush new level
            levels.push_back(newLevel);
            currentGrid.clear();
            name.clear();
        };

        for (const std::string& line : levelsFileLines)
        {
            if (line.empty())
                FlushLevelConstruct();
            else if (line.compare(0, 6, "LEVEL:") == 0)
                name = line.substr(line.find_first_not_of(" \t", 6));
            else if (line.compare(0, 6, "WALLS:") == 0)
                wallGradient = line.substr(line.find_first_not_of(" \t", 6));
            else
                currentGrid.push_back(line);
        }

        // Flush even ramining constructs
        FlushLevelConstruct();
    }

    void SelectLevel(int index)
    {
        const int numLevels = static_cast<int>(levels.size());
        if (numLevels == 0)
        {
            // No levels loaded yet
            return;
        }
        if (nextLevel != nullptr)
        {
            // Still transitioning into another level
            return;
        }

        // Wrap level index around the range from both ends
        if (index < 0)
            index = (numLevels + (index % numLevels)) % numLevels;
        if (index >= numLevels)
            index = index % numLevels;

        if (currentLevelIndex == index)
        {
            // Level unchanged
            return;
        }

        // Update instance buffer
        const std::uint64_t playerBufferSize = sizeof(Instance);
        if (currentLevel != nullptr)
        {
            // Select next level to transition to
            nextLevel = &levels[index];
            levelDistance = static_cast<float>(currentLevel->gridSize[0] + nextLevel->gridSize[0]) * 1.5f;

            // Position player
            nextLevel->ResetTiles();
            nextLevel->PutPlayer(player);

            // Update instance buffer from current and next level instance data plus one instance for the player model
            CreateInstanceBuffer(1 + static_cast<std::uint32_t>(currentLevel->meshInstances.size() + nextLevel->meshInstances.size()));

            const std::uint64_t instanceBufferSize0 = sizeof(Instance) * currentLevel->meshInstances.size();
            const std::uint64_t instanceBufferSize1 = sizeof(Instance) * nextLevel   ->meshInstances.size();

            renderer->WriteBuffer(*instanceBuffer, playerBufferSize                      , currentLevel->meshInstances.data(), instanceBufferSize0);
            renderer->WriteBuffer(*instanceBuffer, playerBufferSize + instanceBufferSize0, nextLevel   ->meshInstances.data(), instanceBufferSize1);
        }
        else
        {
            // Select first level
            currentLevel = &levels[index];
            levelDistance = 0.0f;

            // Position player
            currentLevel->ResetTiles();
            currentLevel->PutPlayer(player);

            // Update instance buffer from current level instance data plus one instance for the player model
            CreateInstanceBuffer(1 + static_cast<std::uint32_t>(currentLevel->meshInstances.size()));
            renderer->WriteBuffer(*instanceBuffer, playerBufferSize, currentLevel->meshInstances.data(), sizeof(Instance) * currentLevel->meshInstances.size());
        }

        // Store index to current level to conveneintly selecting next and previous levels
        currentLevelIndex = index;
        levelInstanceOffset = 0;
    }

    void SetShadowMapView()
    {
        // Generate shadow map projection for light direction
        Gs::Vector3f lightDir = scene.lightDir;
        Gs::Vector3f upVector = (lightDir.y < 0.999f ? Gs::Vector3f{ 0, 1, 0 } : Gs::Vector3f{ 0, 0, -1 });
        Gs::Vector3f tangentU = Gs::Cross(upVector, lightDir).Normalized();
        Gs::Vector3f tangentV = Gs::Cross(lightDir, tangentU);

        Gs::Matrix4f lightOrientation;

        lightOrientation(0, 0) = tangentU.x;
        lightOrientation(0, 1) = tangentU.y;
        lightOrientation(0, 2) = tangentU.z;

        lightOrientation(1, 0) = tangentV.x;
        lightOrientation(1, 1) = tangentV.y;
        lightOrientation(1, 2) = tangentV.z;

        lightOrientation(2, 0) = lightDir.x;
        lightOrientation(2, 1) = lightDir.y;
        lightOrientation(2, 2) = lightDir.z;

        lightOrientation.MakeInverse();

        // Update view transformation from light perspective
        scene.vpMatrix.LoadIdentity();
        Gs::Translate(scene.vpMatrix, { camera.levelCenterPos.x, 0.0f, camera.levelCenterPos.y });
        scene.vpMatrix *= lightOrientation;
        Gs::Translate(scene.vpMatrix, { 0, 0, -50.0f });

        const float shadowMapScale = camera.viewDistance*1.5f;
        Gs::Matrix4f lightProjection = OrthogonalProjection(shadowMapScale, shadowMapScale, 0.1f, 100.0f);

        scene.vpMatrix.MakeInverse();
        scene.vpMatrix = lightProjection * scene.vpMatrix;

        // Update shadow map matrix for texture space
        scene.vpShadowMatrix = scene.vpMatrix;
    }

    void SetCameraView()
    {
        // Update view transformation from camera perspective
        scene.vpMatrix.LoadIdentity();
        Gs::Translate(scene.vpMatrix, { camera.levelCenterPos.x, 0, camera.levelCenterPos.y });
        Gs::RotateFree(scene.vpMatrix, { 1, 0, 0 }, Gs::Deg2Rad(-65.0f));
        Gs::Translate(scene.vpMatrix, { 0, 0, -camera.viewDistance });
        scene.viewPos = Gs::TransformVector(scene.vpMatrix, Gs::Vector3f{ 0, 0, 0 });
        scene.vpMatrix.MakeInverse();
        scene.vpMatrix = projection * scene.vpMatrix;
    }

    void UpdateScene(float dt)
    {
        // Update user input, but not while transitioning
        if (nextLevel == nullptr)
        {
            if (player.moveDirStack < inputStackSize)
            {
                if (input.KeyDownRepeated(LLGL::Key::Left))
                    player.Move(-1, 0);
                else if (input.KeyDownRepeated(LLGL::Key::Right))
                    player.Move(+1, 0);
                else if (input.KeyDownRepeated(LLGL::Key::Up))
                    player.Move(0, +1);
                else if (input.KeyDownRepeated(LLGL::Key::Down))
                    player.Move(0, -1);
            }
            #if ENABLE_CHEATS
            if (input.KeyDown(LLGL::Key::PageUp))
                SelectLevel(currentLevelIndex + 1);
            else if (input.KeyDown(LLGL::Key::PageDown))
                SelectLevel(currentLevelIndex - 1);
            #endif
        }

        // Get information from current level
        if (currentLevel != nullptr)
        {
            camera.FocusOnLevel(*currentLevel);

            // Transition to next level if one is selected
            if (nextLevel != nullptr)
            {
                // Transition from current to next level
                levelTransition += dt / levelTransitionSpeed;
                camera.TransitionBetweenLevels(*currentLevel, *nextLevel, levelTransition);

                // Finish transition interpolation reached the end of [0..1] interval
                if (levelTransition >= 1.0f)
                {
                    levelTransition     = 0.0f;
                    levelInstanceOffset = static_cast<std::uint32_t>(currentLevel->meshInstances.size());
                    currentLevel        = nextLevel;
                    nextLevel           = nullptr;

                    camera.FocusOnLevel(*currentLevel);
                }
            }
            else if (currentLevel->IsCompleted())
            {
                // Wait until next level is selected
                levelDoneTransition += dt / levelDoneSpeed;
                if (levelDoneTransition >= 1.0f)
                {
                    SelectLevel(currentLevelIndex + 1);
                    levelDoneTransition = 0.0f;
                }
            }
        }

        // Update player transformation
        Gs::AffineMatrix4f& wMatrixPlayer = *reinterpret_cast<Gs::AffineMatrix4f*>(player.instance.wMatrix);

        bool isMovementBlocked = false;

        if (player.moveDirStack > 0 && !player.isFalling && !(currentLevel != nullptr && currentLevel->IsCompleted()))
        {
            const int moveStackPos = player.moveDirStack - 1;

            int nextPosX = player.gridPos[0] + player.moveDir[moveStackPos][0];
            int nextPosY = player.gridPos[1] + player.moveDir[moveStackPos][1];

            if (currentLevel != nullptr)
            {
                if (currentLevel->IsTileBlocked(nextPosX, nextPosY))
                {
                    // Block player from moving when hitting a wall or already activated tile
                    nextPosX = player.gridPos[0];
                    nextPosY = player.gridPos[1];
                    isMovementBlocked = true;
                }
            }

            player.moveTransition += (dt / playerMoveSpeed) * static_cast<float>(player.moveDirStack);
            if (player.moveTransition >= 1.0f)
            {
                // Perform tile action
                if (currentLevel != nullptr)
                {
                    if (player.gridPos[0] != nextPosX ||
                        player.gridPos[1] != nextPosY)
                    {
                        // Activate tile and start warp effect when level has been completed
                        if (currentLevel->IsTileHole(nextPosX, nextPosY))
                        {
                            player.isFalling = true;
                        }
                        else if (currentLevel->ActivateTile(nextPosX, nextPosY, playerColor))
                        {
                            effects.StartWarp();
                        }
                    }
                }

                // Finish player movement transition
                player.moveTransition   = 0.0f;
                player.gridPos[0]       = nextPosX;
                player.gridPos[1]       = nextPosY;
                player.moveDirStack--;

                // Cancel remaining movements if they are also blocked in the same direction
                while (isMovementBlocked &&
                    player.moveDirStack > 0 &&
                    player.moveDir[moveStackPos][0] == player.moveDir[player.moveDirStack - 1][0] &&
                    player.moveDir[moveStackPos][1] == player.moveDir[player.moveDirStack - 1][1])
                {
                    player.moveDirStack--;
                }
            }
        }

        if (player.isFalling)
        {
            // Fall animation
            if (player.fallDepth < 100.0f)
            {
                player.fallVelocity += dt * playerFallAcceleration;
                player.fallDepth += player.fallVelocity;
                SetPlayerTransform(wMatrixPlayer, player.gridPos, 0, 0, wallPosY - player.fallDepth, 0.0f);
            }
        }
        else if (player.moveDirStack > 0)
        {
            const int moveStackPos  = player.moveDirStack - 1;
            const int moveDirX      = player.moveDir[moveStackPos][0];
            const int moveDirY      = player.moveDir[moveStackPos][1];
            if (isMovementBlocked)
            {
                const int oppositePosX = player.gridPos[0] - moveDirX;
                const int oppositePosY = player.gridPos[1] - moveDirY;
                if (currentLevel != nullptr && !currentLevel->IsWall(oppositePosX, oppositePosY))
                {
                    // Animate player to bounce off the wall
                    const float bounceTransition = std::abs(std::sinf(player.moveTransition * Gs::pi * 2.0f)) * Gs::SmoothStep(1.0f - player.moveTransition * 0.5f) * 0.2f;
                    SetPlayerTransform(wMatrixPlayer, player.gridPos, -moveDirX, -moveDirY, wallPosY, bounceTransition);
                }
                else
                {
                    // Player is completely blocked, no animation
                    SetPlayerTransform(wMatrixPlayer, player.gridPos, 0, 0, wallPosY, 0.0f);
                }
            }
            else
            {
                // Animate player turn over
                SetPlayerTransform(wMatrixPlayer, player.gridPos, moveDirX, moveDirY, wallPosY, player.moveTransition);
            }
        }
        else
        {
            // No player animation
            SetPlayerTransform(wMatrixPlayer, player.gridPos, 0, 0, wallPosY, 0.0f);
        }

        // Apply warp effect around player position
        if (effects.warpEnabled)
        {
            effects.warpTime += dt / warpEffectDuration;
            float maxWarpIntensity = (1.0f - effects.warpTime) * warpEffectScale;
            if (maxWarpIntensity > 0.0f)
            {
                scene.warpCenter    = Gs::TransformVector(wMatrixPlayer, Gs::Vector3f{});
                scene.warpIntensity = std::sinf(effects.warpTime * Gs::pi * 2.0f * static_cast<float>(warpEffectBounces)) * maxWarpIntensity;
            }
            else
            {
                effects.warpEnabled = false;
                scene.warpIntensity = 0.0f;
            }
        }

        // Animate tree rotation
        effects.treeBendTime = std::fmodf(effects.treeBendTime + dt / treeAnimSpeed, 1.0f);
        const float treeBendAngle = effects.treeBendTime * Gs::pi * 2.0f;
        scene.bendDir.x = std::sinf(treeBendAngle) * treeAnimRadius;
        scene.bendDir.z = std::cosf(treeBendAngle) * treeAnimRadius * 0.5f;

        // Update player color
        player.instance.color[0] = playerColor[0];
        player.instance.color[1] = playerColor[1];
        player.instance.color[2] = playerColor[2];
        player.instance.color[3] = 1.0f;
    }

    void RenderLevel(const Level& level, float worldOffsetX, std::uint32_t instanceOffset)
    {
        uniforms.worldOffset[0] = worldOffsetX;
        uniforms.worldOffset[1] = 0.0f;
        uniforms.worldOffset[2] = 0.0f;

        // Draw all tiles (floor and walls)
        uniforms.firstInstance = 1 + instanceOffset;
        uniforms.bendIntensity = 0.0f;
        commands->SetUniforms(0, &uniforms, sizeof(uniforms));

        const std::uint32_t numTiles = level.tileInstanceRange.Count();
        commands->DrawInstanced(mdlBlock.numVertices, mdlBlock.firstVertex, numTiles);

        // Draw decor (trees)
        uniforms.firstInstance += numTiles;
        uniforms.bendIntensity = 0.5f;
        commands->SetUniforms(0, &uniforms, sizeof(uniforms));

        const std::uint32_t numTrees = level.treeInstanceRange.Count();
        commands->DrawInstanced(mdlTree.numVertices, mdlTree.firstVertex, numTrees);
    }

    void RenderScene(bool renderShadowMap)
    {
        const int psoIndex = (renderShadowMap ? 1 : 0);

        commands->SetPipelineState(*scenePSO[psoIndex]);
        commands->SetResource(0, *cbufferScene);
        commands->SetResource(1, *instanceBuffer);

        if (!renderShadowMap)
        {
            commands->SetResource(2, *shadowMap);
            commands->SetResource(3, *shadowMapSampler);
        }

        if (currentLevel != nullptr)
        {
            // Render current level
            commands->PushDebugGroup("CurrentLevel");
            {
                RenderLevel(*currentLevel, Gs::Lerp(0.0f, -levelDistance, levelTransition), levelInstanceOffset);
            }
            commands->PopDebugGroup();

            // Render next level if there is one
            if (nextLevel != nullptr)
            {
                const std::uint32_t numInstancesCurrentLevel = static_cast<std::uint32_t>(currentLevel->meshInstances.size());
                commands->PushDebugGroup("NextLevel");
                {
                    RenderLevel(*nextLevel, Gs::Lerp(levelDistance, 0.0f, levelTransition), numInstancesCurrentLevel);
                }
                commands->PopDebugGroup();
            }
        }

        // Draw player mesh, but not while transitioning
        commands->PushDebugGroup("Player");
        {
            // Always position player at the next level if there is one
            if (nextLevel == nullptr)
            {
                uniforms.worldOffset[0] = 0.0f;
                uniforms.worldOffset[1] = 0.0f;
                uniforms.worldOffset[2] = 0.0f;
            }
            uniforms.firstInstance = 0;
            uniforms.bendIntensity = 0.0f;
            commands->SetUniforms(0, &uniforms, sizeof(uniforms));

            commands->Draw(mdlPlayer.numVertices, mdlPlayer.firstVertex);
        }
        commands->PopDebugGroup();

        // Draw ground floor last, to cover the rest of the framebuffer
        if (!renderShadowMap)
        {
            commands->PushDebugGroup("Ground");
            {
                commands->SetPipelineState(*groundPSO);
                commands->SetResource(0, *cbufferScene);
                commands->SetResource(1, *groundColorMap);
                commands->SetResource(2, *groundColorMapSampler);
                commands->SetResource(3, *shadowMap);
                commands->SetResource(4, *shadowMapSampler);
                commands->Draw(mdlGround.numVertices, mdlGround.firstVertex);
            }
            commands->PopDebugGroup();
        }
    }

    void OnDrawFrame() override
    {
        // Update scene by user input
        timer.MeasureTime();
        UpdateScene(static_cast<float>(timer.GetDeltaTime()));

        commands->Begin();
        {
            // Bind common input assembly
            commands->SetVertexBuffer(*vertexBuffer);
            commands->UpdateBuffer(*instanceBuffer, 0, &player.instance, sizeof(player.instance));

            // Update mesh instances in dirty range
            if (currentLevel != nullptr)
            {
                const std::uint32_t numInstancesToUpdate = currentLevel->meshInstanceDirtyRange.Count();
                if (numInstancesToUpdate > 0)
                {
                    // Update mesh instance buffer
                    const std::uint32_t firstInstanceToUpdate = (1 + levelInstanceOffset + currentLevel->meshInstanceDirtyRange.begin);

                    commands->UpdateBuffer(
                        *instanceBuffer,
                        sizeof(Instance) * firstInstanceToUpdate,
                        &(currentLevel->meshInstances[currentLevel->meshInstanceDirtyRange.begin]),
                        static_cast<std::uint16_t>(sizeof(Instance) * numInstancesToUpdate)
                    );

                    // Reset dirty range
                    currentLevel->meshInstanceDirtyRange.Invalidate();
                }
            }

            // Render shadow-map
            SetShadowMapView();
            commands->UpdateBuffer(*cbufferScene, 0, &scene, sizeof(scene));

            commands->BeginRenderPass(*shadowMapTarget);
            {
                // Clear depth buffer only, we will render the entire framebuffer
                commands->Clear(LLGL::ClearFlags::Depth);
                commands->SetViewport(shadowMapTarget->GetResolution());
                commands->PushDebugGroup("RenderShadowMap");
                {
                    RenderScene(true);
                }
                commands->PopDebugGroup();
            }
            commands->EndRenderPass();

            // Render everything directly into the swap-chain
            SetCameraView();
            commands->UpdateBuffer(*cbufferScene, 0, &scene, sizeof(scene));

            commands->BeginRenderPass(*swapChain);
            {
                // Clear depth buffer only, we will render the entire framebuffer
                commands->Clear(LLGL::ClearFlags::Depth);
                commands->SetViewport(swapChain->GetResolution());
                commands->PushDebugGroup("RenderScene");
                {
                    RenderScene(false);
                }
                commands->PopDebugGroup();
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_HelloGame);



