#pragma once

#include "../../Modules.h"
#include "../../Vertex.h"
#include "../UploadBatch.h"

#include <functional>
#include <memory>

#include "DX12Common.h"
#include <d3d12.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

namespace Netcode::Graphics::DX12
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

    class SpriteBatch : public Netcode::SpriteBatch {
        static GpuResourceWeakRef indexBuffer;
    public:

        struct SpriteInfo {
            SpriteDesc spriteDesc;
            BorderDesc borderDesc;
            Float4 source;
            Float4 destination;
            Float4 originRotationDepth;
            Vector2 textureSize;
            Rect scissorRect;
            unsigned int flags;

            // Combine values from the public SpriteEffects enum with these internal-only flags.
            static const unsigned int SourceInTexels = 4;
            static const unsigned int DestSizeInPixels = 8;

            static_assert((SpriteEffects_FlipBoth & (SourceInTexels | DestSizeInPixels)) == 0, "Flag bits must not overlap");
        };

        GpuResourceRef vertexBuffer;
        GpuResourceRef indexBufferRef;

        IResourceContext * resourceContext;
        IRenderContext * renderContext;

        RootSignatureRef rootSignature;
        PipelineStateRef pipelineState;
        
        Rect recordScissorRect;
        Rect currentlyBoundScissorRect;

        std::unique_ptr<PCT_Vertex[]> vertexData;
        std::unique_ptr<SpriteInfo[]> mSpriteQueue;

        uint32_t mSpriteQueueCount;
        uint32_t mSpriteQueueArraySize;

        std::vector<SpriteInfo const *> mSortedSprites;

        SpriteSortMode mSortMode;
        Netcode::Matrix mTransformMatrix;

        D3D12_GPU_VIRTUAL_ADDRESS cbufferAddr;
        SpriteCbuffer cbuffer;

        uint32_t mVertexPageSize;
        uint32_t mSpriteCount;

        bool mInBeginEndPair;
        bool firstDraw;

        static const uint32_t MaxBatchSize = 2048;
        static const uint32_t MinBatchSize = 128;
        static const uint32_t InitialQueueSize = 64;
        static const uint32_t VerticesPerSprite = 4;
        static const uint32_t IndicesPerSprite = 6;

        enum RootParameterIndex
        {
            TextureSRV,
            ConstantBuffer,
            TextureSampler,
            RootParameterCount
        };

        void CreateIndexBuffer(const Netcode::Module::IGraphicsModule * graphics);

        SpriteBatch(const Netcode::Module::IGraphicsModule * graphics, Netcode::RootSignatureRef rootSig, Netcode::PipelineStateRef pso);

        SpriteBatch(SpriteBatch && moveFrom) = default;
        SpriteBatch & operator= (SpriteBatch && moveFrom) = default;

        SpriteBatch(SpriteBatch const &) = delete;
        SpriteBatch & operator= (SpriteBatch const &) = delete;

        virtual ~SpriteBatch() = default;

        // Begin/End a batch of sprite drawing operations.
        void Begin(SpriteSortMode sortMode = SpriteSortMode_Deferred, Netcode::Matrix transformMatrix = Netcode::Matrix{});
        void End();

        virtual void BeginRecord(void * ctx, Netcode::Float4x4 viewProj) override {
            renderContext = static_cast<Netcode::Graphics::IRenderContext *>(ctx);
            Begin(SpriteSortMode_Deferred, viewProj);
        }

        virtual void EndRecord() override {
            End();
        }

        virtual void SetScissorRect(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom)  override;
        virtual void SetScissorRect(const Rect & rect)  override;
        virtual void SetScissorRect()  override;



        virtual void DrawSprite(const SpriteDesc & spriteDesc, const Float2 & position) override;
        virtual void DrawSprite(const SpriteDesc & spriteDesc, const Float2 & position, const Float2 & size) override;
        virtual void DrawSprite(const SpriteDesc & spriteDesc, const Float2 & position, const Float2 & size, const Float2 & rotationOrigin, float rotationZ) override;
        virtual void DrawSprite(const SpriteDesc & spriteDesc, const Float2 & position, const Float2 & size, const Float2 & rotationOrigin, float rotationZ, float layerDepth) override;

        virtual void DrawSprite(const SpriteDesc & spriteDesc, const BorderDesc & borderDesc, const Float2 & position) override;
        virtual void DrawSprite(const SpriteDesc & spriteDesc, const BorderDesc & borderDesc, const Float2 & position, const Float2 & size) override;
        virtual void DrawSprite(const SpriteDesc & spriteDesc, const BorderDesc & borderDesc, const Float2 & position, const Float2 & size, const Float2 & rotationOrigin, float rotationZ) override;
        virtual void DrawSprite(const SpriteDesc & spriteDesc, const BorderDesc & borderDesc, const Float2 & position, const Float2 & size, const Float2 & rotationOrigin, float rotationZ, float layerDepth) override;


    private:
        // Implementation helper methods.
        void GrowSpriteQueue();
        void PrepareForRendering();
        void FlushBatch();
        void SortSprites();
        void GrowSortedSprites();

        void NC_MATH_CALLCONV RenderBatch(ResourceViewsRef texture, Vector2 textureSize, const SpriteInfo * const * sprites, uint32_t count);

        static void NC_MATH_CALLCONV RenderSprite(const SpriteInfo * sprite, PCT_Vertex * vertices, Vector2 textureSize, Vector2 inverseTextureSize);

        void NC_MATH_CALLCONV Draw(const SpriteDesc & spriteDesc, const BorderDesc & borderDesc, Netcode::Vector4 destination, Netcode::Vector4 originRotationDepth, uint32_t flags);
    };

    using DX12SpriteBatch = Netcode::Graphics::DX12::SpriteBatch;
    using DX12SpriteBatchRef = std::shared_ptr<DX12SpriteBatch>;

}

