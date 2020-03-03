#pragma once

#include "DX12Common.h"
#include <d3d12.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <functional>
#include <memory>
#include "Vertex.h"
#include "UploadBatch.h"
#include "Modules.h"

namespace Egg::Graphics::DX12
{
   __declspec(align(256)) struct SpriteCbuffer {
       DirectX::XMFLOAT4X4A transform;
    };

    enum SpriteSortMode
    {
        SpriteSortMode_Deferred,
        SpriteSortMode_Immediate,
        SpriteSortMode_Texture,
        SpriteSortMode_BackToFront,
        SpriteSortMode_FrontToBack,
    };

    enum SpriteEffects : uint32_t
    {
        SpriteEffects_None = 0,
        SpriteEffects_FlipHorizontally = 1,
        SpriteEffects_FlipVertically = 2,
        SpriteEffects_FlipBoth = SpriteEffects_FlipHorizontally | SpriteEffects_FlipVertically,
    };

    class SpriteBatch
    {


    public:
        __declspec(align(16)) struct SpriteInfo
        {
            DirectX::XMFLOAT4A source;
            DirectX::XMFLOAT4A destination;
            DirectX::XMFLOAT4A color;
            DirectX::XMFLOAT4A originRotationDepth;
            ResourceViewsRef texture;
            DirectX::XMVECTOR textureSize;
            unsigned int flags;

            // Combine values from the public SpriteEffects enum with these internal-only flags.
            static const unsigned int SourceInTexels = 4;
            static const unsigned int DestSizeInPixels = 8;

            static_assert((SpriteEffects_FlipBoth & (SourceInTexels | DestSizeInPixels)) == 0, "Flag bits must not overlap");
        };
        static uint64_t indexBuffer;
        uint64_t vertexBuffer;

        IResourceContext * resourceContext;
        IRenderContext * renderContext;

        RootSignatureRef rootSignature;
        PipelineStateRef pipelineState;

        std::unique_ptr<PCT_Vertex[]> vertexData;
        std::unique_ptr<SpriteInfo[]> mSpriteQueue;

        size_t mSpriteQueueCount;
        size_t mSpriteQueueArraySize;

        std::vector<SpriteInfo const *> mSortedSprites;


        // Mode settings from the last Begin call.
        bool mInBeginEndPair;

        SpriteSortMode mSortMode;
        DirectX::XMMATRIX mTransformMatrix;
        

        D3D12_GPU_VIRTUAL_ADDRESS cbufferAddr;
        SpriteCbuffer cbuffer;

        size_t mVertexPageSize;
        size_t mSpriteCount;

        static const size_t MaxBatchSize = 2048;
        static const size_t MinBatchSize = 128;
        static const size_t InitialQueueSize = 64;
        static const size_t VerticesPerSprite = 4;
        static const size_t IndicesPerSprite = 6;

        enum RootParameterIndex
        {
            TextureSRV,
            ConstantBuffer,
            TextureSampler,
            RootParameterCount
        };

        void CreateIndexBuffer(Egg::Module::IGraphicsModule * graphics);

        SpriteBatch(Egg::Module::IGraphicsModule * graphics);

        SpriteBatch(SpriteBatch && moveFrom) = default;
        SpriteBatch & operator= (SpriteBatch && moveFrom) = default;

        SpriteBatch(SpriteBatch const &) = delete;
        SpriteBatch & operator= (SpriteBatch const &) = delete;

        virtual ~SpriteBatch() = default;

        // Begin/End a batch of sprite drawing operations.
        void Begin(SpriteSortMode sortMode = SpriteSortMode_Deferred, DirectX::FXMMATRIX transformMatrix = MatrixIdentity);
        void End();

        // Draw overloads specifying position, origin and scale as XMFLOAT2.
        void Draw(ResourceViewsRef textureSRV, DirectX::XMUINT2 const & textureSize, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color = DirectX::Colors::White);
        void Draw(ResourceViewsRef textureSRV, DirectX::XMUINT2 const & textureSize, DirectX::XMFLOAT2 const & position, _In_opt_ RECT const * sourceRectangle, DirectX::FXMVECTOR color = DirectX::Colors::White, float rotation = 0, DirectX::XMFLOAT2 const & origin = Float2Zero, float scale = 1, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0);
        void Draw(ResourceViewsRef textureSRV, DirectX::XMUINT2 const & textureSize, DirectX::XMFLOAT2 const & position, _In_opt_ RECT const * sourceRectangle, DirectX::FXMVECTOR color, float rotation, DirectX::XMFLOAT2 const & origin, DirectX::XMFLOAT2 const & scale, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0);

        // Draw overloads specifying position, origin and scale via the first two components of an XMVECTOR.
        void Draw(ResourceViewsRef textureSRV, DirectX::XMUINT2 const & textureSize, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color = DirectX::Colors::White);
        void Draw(ResourceViewsRef textureSRV, DirectX::XMUINT2 const & textureSize, DirectX::FXMVECTOR position, _In_opt_ RECT const * sourceRectangle, DirectX::FXMVECTOR color = DirectX::Colors::White, float rotation = 0, DirectX::FXMVECTOR origin = DirectX::g_XMZero, float scale = 1, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0);
        void Draw(ResourceViewsRef textureSRV, DirectX::XMUINT2 const & textureSize, DirectX::FXMVECTOR position, _In_opt_ RECT const * sourceRectangle, DirectX::FXMVECTOR color, float rotation, DirectX::FXMVECTOR origin, DirectX::GXMVECTOR scale, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0);

        // Draw overloads specifying position as a RECT.
        void Draw(ResourceViewsRef textureSRV, DirectX::XMUINT2 const & textureSize, RECT const & destinationRectangle, DirectX::FXMVECTOR color = DirectX::Colors::White);
        void Draw(ResourceViewsRef textureSRV, DirectX::XMUINT2 const & textureSize, RECT const & destinationRectangle, _In_opt_ RECT const * sourceRectangle, DirectX::FXMVECTOR color = DirectX::Colors::White, float rotation = 0, DirectX::XMFLOAT2 const & origin = Float2Zero, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0);

    private:
        // Implementation helper methods.
        void GrowSpriteQueue();
        void PrepareForRendering();
        void FlushBatch();
        void SortSprites();
        void GrowSortedSprites();

        void RenderBatch(ResourceViewsRef texture, DirectX::XMVECTOR textureSize, _In_reads_(count) SpriteInfo const * const * sprites, size_t count);

        static void RenderSprite(_In_ SpriteInfo const * sprite,
                                             _Out_writes_(VerticesPerSprite) PCT_Vertex * vertices,
                                             DirectX::FXMVECTOR textureSize,
                                             DirectX::FXMVECTOR inverseTextureSize);

        void Draw(ResourceViewsRef textureSRV, DirectX::XMUINT2 const & textureSize, DirectX::FXMVECTOR destination, RECT const * sourceRectangle, DirectX::FXMVECTOR color, DirectX::FXMVECTOR originRotationDepth, unsigned int flags);

        static const DirectX::XMMATRIX MatrixIdentity;
        static const DirectX::XMFLOAT2 Float2Zero;
    };
}

