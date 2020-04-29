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

    class RectHash {
        uint64_t value;
    public:
        RectHash() = default;

        RectHash(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom) {
            constexpr uint64_t mask = 0xFFFF;

            value = (static_cast<uint64_t>(left) & mask) << 48 |
                (static_cast<uint64_t>(right) & mask) << 32 |
                (static_cast<uint64_t>(top) & mask) << 16 |
                (static_cast<uint64_t>(bottom) & mask);
        }

        RectHash(const RECT & rect) : RectHash(rect.left, rect.right, rect.top, rect.bottom) {

        }

        bool operator==(const RectHash & rhs) const {
            return value == rhs.value;
        }

        bool operator!=(const RectHash & rhs) const {
            return !operator==(rhs);
        }

        bool operator==(uint64_t v) const {
            return value == v;
        }
    };

    class SpriteScissorRect {
        RectHash hash;
        RECT rect;
    public:
        SpriteScissorRect() = default;

        SpriteScissorRect(const RECT & r) :hash{ r }, rect{ r } {}

        void Clear() {
            rect = RECT{ 0,0,0,0 };
        }

        void SetRect(const RECT & r) {
            rect = r;
            hash = RectHash(r);
        }

        RECT GetRect() const {
            return rect;
        }

        bool operator!=(const SpriteScissorRect & rhs) const {
            return hash != rhs.hash;
        }

        bool operator==(const SpriteScissorRect & rhs) const {
            return hash == rhs.hash;
        }

        bool operator==(uint64_t value) const {
            return hash == value;
        }
    };

    class SpriteBatch : public Netcode::SpriteBatch {
        static GpuResourceRef indexBuffer;
        static const DirectX::XMMATRIX MatrixIdentity;
        static const DirectX::XMFLOAT2 Float2Zero;
    public:

        __declspec(align(16)) struct SpriteInfo
        {
            DirectX::XMFLOAT4A source;
            DirectX::XMFLOAT4A destination;
            DirectX::XMFLOAT4A color;
            DirectX::XMFLOAT4A originRotationDepth;
            ResourceViewsRef texture;
            DirectX::XMVECTOR textureSize;
            SpriteScissorRect scissorRect;
            unsigned int flags;

            // Combine values from the public SpriteEffects enum with these internal-only flags.
            static const unsigned int SourceInTexels = 4;
            static const unsigned int DestSizeInPixels = 8;

            static_assert((SpriteEffects_FlipBoth & (SourceInTexels | DestSizeInPixels)) == 0, "Flag bits must not overlap");
        };

        GpuResourceRef vertexBuffer;

        IResourceContext * resourceContext;
        IRenderContext * renderContext;

        RootSignatureRef rootSignature;
        PipelineStateRef pipelineState;
        
        SpriteScissorRect recordScissorRect;
        SpriteScissorRect currentlyBoundScissorRect;

        std::unique_ptr<PCT_Vertex[]> vertexData;
        std::unique_ptr<SpriteInfo[]> mSpriteQueue;

        uint32_t mSpriteQueueCount;
        uint32_t mSpriteQueueArraySize;

        std::vector<SpriteInfo const *> mSortedSprites;

        SpriteSortMode mSortMode;
        DirectX::XMMATRIX mTransformMatrix;

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
        void Begin(SpriteSortMode sortMode = SpriteSortMode_Deferred, DirectX::FXMMATRIX transformMatrix = MatrixIdentity);
        void End();

        virtual void BeginRecord(void * ctx, DirectX::XMFLOAT4X4 viewProj) override {
            renderContext = static_cast<Netcode::Graphics::IRenderContext *>(ctx);
            DirectX::FXMMATRIX tMat = DirectX::XMLoadFloat4x4(&viewProj);
            Begin(SpriteSortMode_Deferred, tMat);
        }

        virtual void EndRecord() override {
            End();
        }

        virtual void SetScissorRect(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom)  override;
        virtual void SetScissorRect(const RECT & rect)  override;
        virtual void SetScissorRect()  override;

        virtual void DrawSprite(ResourceViewsRef texture, const DirectX::XMUINT2 & textureSize, const DirectX::XMFLOAT2 & position) override;

        virtual void DrawSprite(ResourceViewsRef texture, const DirectX::XMUINT2 & textureSize, const DirectX::XMFLOAT2 & position, const DirectX::XMFLOAT2 & size)  override;

        virtual void DrawSprite(ResourceViewsRef texture, const DirectX::XMUINT2 & textureSize, const DirectX::XMFLOAT2 & position, const DirectX::XMFLOAT2 & size, const DirectX::XMFLOAT4 & color)  override;

        virtual void DrawSprite(ResourceViewsRef texture,
            const DirectX::XMUINT2 & textureSize,
            const DirectX::XMFLOAT2 & position,
            const RECT * sourceRectangle,
            const DirectX::XMFLOAT4 & color,
            float rotation,
            const DirectX::XMFLOAT2 & origin,
            float scale,
            float layerDepth) override;

        virtual void DrawSprite(ResourceViewsRef texture,
            const DirectX::XMUINT2 & textureSize,
            const DirectX::XMFLOAT2 & destPosition,
            const DirectX::XMFLOAT2 & destSize,
            const RECT * sourceRectangle,
            const DirectX::XMFLOAT4 & color,
            float rotation,
            const DirectX::XMFLOAT2 & origin,
            float layerDepth) override;
        /*
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
        */
    private:
        // Implementation helper methods.
        void GrowSpriteQueue();
        void PrepareForRendering();
        void FlushBatch();
        void SortSprites();
        void GrowSortedSprites();

        void RenderBatch(ResourceViewsRef texture, DirectX::XMVECTOR textureSize, SpriteInfo const * const * sprites, uint32_t count);

        static void RenderSprite(SpriteInfo const * sprite,
                                 PCT_Vertex * vertices,
                                 DirectX::FXMVECTOR textureSize,
                                 DirectX::FXMVECTOR inverseTextureSize);

        void Draw(ResourceViewsRef textureSRV, DirectX::XMUINT2 const & textureSize, DirectX::FXMVECTOR destination, RECT const * sourceRectangle, DirectX::FXMVECTOR color, DirectX::FXMVECTOR originRotationDepth, unsigned int flags);

    };

    using DX12SpriteBatch = Netcode::Graphics::DX12::SpriteBatch;
    using DX12SpriteBatchRef = std::shared_ptr<DX12SpriteBatch>;

}

