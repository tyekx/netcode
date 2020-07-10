#include "../../Utility.h"

#include <algorithm>

#include "DX12SpriteBatch.h"
#include "DX12ResourceViews.h"

namespace Netcode::Graphics::DX12 {

	struct ControlDisplayData {
		float borderRadius;
		float borderWidth;
		Float2 spriteSize;
		Float2 screenSize;
		Float2 screenPosition;
		Float4 borderColor;
		int borderType;
		int backgroundType;
	};

	GpuResourceWeakRef SpriteBatch::indexBuffer{ };

void SpriteBatch::CreateIndexBuffer(const Netcode::Module::IGraphicsModule * graphics)
{
	if(!Netcode::Utility::IsWeakRefEmpty(indexBuffer)) {
		indexBufferRef = indexBuffer.lock();
		return;
	}

	std::vector<short> indices;

	indices.reserve(MaxBatchSize * IndicesPerSprite);

	for(uint32_t j = 0; j < MaxBatchSize * VerticesPerSprite; j += VerticesPerSprite)
	{
		short i = static_cast<short>(j);

		indices.push_back(i);
		indices.push_back(i + 1);
		indices.push_back(i + 2);

		indices.push_back(i + 1);
		indices.push_back(i + 3);
		indices.push_back(i + 2);
	}
	uint32_t ibufferSize = sizeof(short) * MaxBatchSize * IndicesPerSprite;

	indexBufferRef = graphics->resources->CreateIndexBuffer(ibufferSize, DXGI_FORMAT_R16_UINT, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
	
	UploadBatch upload;
	upload.Upload(indexBufferRef, indices.data(), ibufferSize);
	upload.ResourceBarrier(indexBufferRef, ResourceState::COPY_DEST, ResourceState::INDEX_BUFFER);
	graphics->frame->SyncUpload(upload);

	indexBuffer = indexBufferRef;
}

SpriteBatch::SpriteBatch(const Netcode::Module::IGraphicsModule * graphics, Netcode::RootSignatureRef rootSig, Netcode::PipelineStateRef pso)
			: vertexBuffer{ 0 },
			resourceContext{ nullptr },
			renderContext{ nullptr },
			rootSignature{ std::move(rootSig) },
			pipelineState{ std::move(pso) },
			recordScissorRect{},
			currentlyBoundScissorRect{},
			vertexData{ nullptr },
			mSpriteQueue{ nullptr },
			mSpriteQueueCount{ 0 },
			mSpriteQueueArraySize{ 0 },
			mSortMode{ SpriteSortMode::SpriteSortMode_Deferred },
			mTransformMatrix{ },
			cbufferAddr{},
			cbuffer{},
			mVertexPageSize{ 6 * 4 * MaxBatchSize * VerticesPerSprite },
			mSpriteCount{ 0 },
			mInBeginEndPair{ false },
			firstDraw{ true }
	{
		resourceContext = graphics->resources;

		vertexBuffer = graphics->resources->CreateVertexBuffer(sizeof(PCT_Vertex) * MaxBatchSize * VerticesPerSprite, sizeof(PCT_Vertex), ResourceType::PERMANENT_UPLOAD, ResourceState::ANY_READ);
		vertexData = std::make_unique<PCT_Vertex[]>(MaxBatchSize * VerticesPerSprite);

		CreateIndexBuffer(graphics);
	}

	void SpriteBatch::Begin(SpriteSortMode sortMode, Netcode::Matrix transformMatrix)
	{
		if(mInBeginEndPair)
			throw std::exception("Cannot nest Begin calls on a single SpriteBatch");

		mSortMode = sortMode;
		mTransformMatrix = transformMatrix;
		mSpriteCount = 0;

		if(sortMode == SpriteSortMode_Immediate)
		{
			PrepareForRendering();
		}

		mInBeginEndPair = true;
		firstDraw = true;
		recordScissorRect.Clear();
	}

	void SpriteBatch::End()
	{
		if(!mInBeginEndPair)
			throw std::exception("Begin must be called before End");

		if(mSortMode != SpriteSortMode_Immediate)
		{
			PrepareForRendering();
			FlushBatch();
		}

		mInBeginEndPair = false;
	}

	void SpriteBatch::PrepareForRendering()
	{
		renderContext->SetRootSignature(rootSignature);
		renderContext->SetPipelineState(pipelineState);
		renderContext->SetViewport();
		renderContext->SetScissorRect();
		renderContext->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
		renderContext->SetIndexBuffer(indexBufferRef);
	}

	// Sorts the array of queued sprites.
	void SpriteBatch::SortSprites()
	{
		// Fill the mSortedSprites vector.
		if(mSortedSprites.size() < mSpriteQueueCount)
		{
			GrowSortedSprites();
		}

		switch(mSortMode)
		{
		case SpriteSortMode_Texture:
			// Sort by texture.
			std::sort(mSortedSprites.begin(),
					  mSortedSprites.begin() + static_cast<int>(mSpriteQueueCount),
					  [](SpriteInfo const * x, SpriteInfo const * y) -> bool
			{
				return x->spriteDesc.texture.get() < y->spriteDesc.texture.get();
			});
			break;

		case SpriteSortMode_BackToFront:
			// Sort back to front.
			std::sort(mSortedSprites.begin(),
					  mSortedSprites.begin() + static_cast<int>(mSpriteQueueCount),
					  [](SpriteInfo const * x, SpriteInfo const * y) -> bool
			{
				return x->originRotationDepth.w > y->originRotationDepth.w;
			});
			break;

		case SpriteSortMode_FrontToBack:
			// Sort front to back.
			std::sort(mSortedSprites.begin(),
					  mSortedSprites.begin() + static_cast<int>(mSpriteQueueCount),
					  [](SpriteInfo const * x, SpriteInfo const * y) -> bool
			{
				return x->originRotationDepth.w < y->originRotationDepth.w;
			});
			break;

		default:
			break;
		}
	}

	// Sends queued sprites to the graphics device.
	void SpriteBatch::FlushBatch()
	{
		if(!mSpriteQueueCount)
			return;

		SortSprites();

		// Walk through the sorted sprite list, looking for adjacent entries that share a texture.
		ResourceViewsRef batchTexture;
		Vector2 batchTextureSize = {};
		BorderDesc batchBorderDesc;
		SpriteDesc batchSpriteDesc;
		SpriteScissorRect batchSpr = Rect{};
		uint32_t batchStart = 0;

		for(uint32_t pos = 0; pos < mSpriteQueueCount; pos++)
		{
			ResourceViewsRef texture = mSortedSprites[pos]->spriteDesc.texture;
			SpriteScissorRect spr = mSortedSprites[pos]->scissorRect;
			BorderDesc borderDesc = mSortedSprites[pos]->borderDesc;
			SpriteDesc spriteDesc = mSortedSprites[pos]->spriteDesc;

			Netcode::Vector2 textureSize = Netcode::Float2::Zero;

			if(mSortedSprites[pos]->spriteDesc.type == BackgroundType::SOLID) {
				textureSize = Netcode::Float2{ mSortedSprites[pos]->destination.z, mSortedSprites[pos]->destination.w };
			}

			if(mSortedSprites[pos]->spriteDesc.type == BackgroundType::TEXTURE) {
				textureSize = mSortedSprites[pos]->textureSize;
			}

			bool scDiff = spr != batchSpr;
			bool borderDiff = batchBorderDesc != borderDesc;
			bool spriteDiff = batchSpriteDesc != spriteDesc;

			// Flush whenever the texture changes or when the scissor rect changes
			if(scDiff || borderDiff || spriteDiff) {
				if(pos > batchStart)
				{
					RenderBatch(batchTexture, batchTextureSize, &mSortedSprites[batchStart], pos - batchStart);
				}

				batchTexture = texture;
				batchTextureSize = textureSize;
				batchStart = pos;
				batchSpr = spr;
				batchSpriteDesc = spriteDesc;
				batchBorderDesc = borderDesc;
			}
		}

		// Flush the final batch.
		RenderBatch(batchTexture, batchTextureSize, &mSortedSprites[batchStart], mSpriteQueueCount - batchStart);

		for(uint32_t i = 0; i < mSpriteQueueCount; ++i) {
			mSpriteQueue[i].spriteDesc.texture.reset();
		}
		// Reset the queue.
		mSpriteQueueCount = 0;

		// When sorting is disabled, we persist mSortedSprites data from one batch to the next, to avoid
		// uneccessary work in GrowSortedSprites. But we never reuse these when sorting, because re-sorting
		// previously sorted items gives unstable ordering if some sprites have identical sort keys.
		if(mSortMode != SpriteSortMode_Deferred)
		{
			mSortedSprites.clear();
		}
	}

	void NC_MATH_CALLCONV SpriteBatch::RenderBatch(ResourceViewsRef texture, Vector2 textureSize, const SpriteInfo * const * sprites, uint32_t count)
	{

		Vector2 inverseTextureSize = textureSize.Reciprocal();

		while(count > 0)
		{
			// How many sprites do we want to draw?
			uint32_t batchSize = count;

			// How many sprites does the D3D vertex buffer have room for?
			uint32_t remainingSpace = MaxBatchSize - mSpriteCount;

			if(batchSize > remainingSpace)
			{
				if(remainingSpace < MinBatchSize)
				{
					// If we are out of room, or about to submit an excessively small batch, wrap back to the start of the vertex buffer.
					mSpriteCount = 0;

					batchSize = std::min(count, MaxBatchSize);
				} else
				{
					// Take however many sprites fit in what's left of the vertex buffer.
					batchSize = remainingSpace;
				}
			}

			uint32_t vertexOffset = (mSpriteCount)*VerticesPerSprite;

			PCT_Vertex * vertices = vertexData.get() + vertexOffset;

			// Generate sprite vertex data.
			for(uint32_t i = 0; i < batchSize; i++) {
				UndefinedBehaviourAssertion(i < count);

				RenderSprite(sprites[i], vertices, textureSize, inverseTextureSize);

				vertices += VerticesPerSprite;
			}

			uint32_t spriteVertexTotalSize = sizeof(PCT_Vertex) * VerticesPerSprite;
			resourceContext->CopyConstants(vertexBuffer, vertexData.get() + vertexOffset, batchSize * spriteVertexTotalSize, vertexOffset * sizeof(PCT_Vertex));

			UINT indexCount = static_cast<UINT>(batchSize * IndicesPerSprite);

			if(firstDraw) {
				firstDraw = false;
				currentlyBoundScissorRect.Clear();
				renderContext->SetScissorRect();
			}

			if(currentlyBoundScissorRect != sprites[0]->scissorRect) {
				currentlyBoundScissorRect = sprites[0]->scissorRect;

				if(currentlyBoundScissorRect == 0) {
					renderContext->SetScissorRect();
				} else {
					Rect r = currentlyBoundScissorRect.GetRect();
					renderContext->SetScissorRect(static_cast<uint32_t>(r.left),
						static_cast<uint32_t>(r.right),
						static_cast<uint32_t>(r.top),
						static_cast<uint32_t>(r.bottom));
				}
			}

			Float4 screenSize = renderContext->GetViewport();

			ControlDisplayData controlDisplay;
			controlDisplay.borderType = static_cast<int>(sprites[0]->borderDesc.type);
			controlDisplay.backgroundType = static_cast<int>(sprites[0]->spriteDesc.type);
			controlDisplay.borderColor = sprites[0]->borderDesc.color;
			controlDisplay.borderRadius = sprites[0]->borderDesc.borderRadius;
			controlDisplay.borderWidth = sprites[0]->borderDesc.borderWidth;
			controlDisplay.screenPosition = Float2{ sprites[0]->destination.x, sprites[0]->destination.y };
			controlDisplay.screenSize = Float2{ screenSize.z, screenSize.w };
			controlDisplay.spriteSize = Float2{ sprites[0]->destination.z, sprites[0]->destination.w };

			cbuffer.transform = mTransformMatrix;
			if(texture != nullptr) {
				renderContext->SetShaderResources(0, texture);
			}
			renderContext->SetRootConstants(1, &controlDisplay, 14);
			renderContext->SetRootConstants(2, &cbuffer.transform, 16);
			renderContext->SetVertexBuffer(vertexBuffer);
			renderContext->DrawIndexed(indexCount, vertexOffset);

			// Advance the buffer position.
			mSpriteCount += batchSize;

			sprites += batchSize;
			count -= batchSize;
		}
	}

	void NC_MATH_CALLCONV SpriteBatch::RenderSprite(const SpriteInfo * sprite, PCT_Vertex * vertices, Vector2 textureSize, Vector2 inverseTextureSize)
	{
		Netcode::Vector4 source = sprite->source;
		Netcode::Vector4 destination = sprite->destination;
		Netcode::Vector4 color = sprite->spriteDesc.color;
		Netcode::Vector4 originRotationDepth = sprite->originRotationDepth;

		float rotation = sprite->originRotationDepth.z;
		unsigned int flags = sprite->flags;

		// Extract the source and destination sizes into separate vectors.
		DirectX::XMVECTOR sourceSize = DirectX::XMVectorSwizzle<2, 3, 2, 3>(source.v);
		DirectX::XMVECTOR destinationSize = DirectX::XMVectorSwizzle<2, 3, 2, 3>(destination.v);

		// Scale the origin offset by source size, taking care to avoid overflow if the source region is zero.
		DirectX::XMVECTOR isZeroMask = DirectX::XMVectorEqual(sourceSize, DirectX::XMVectorZero());
		DirectX::XMVECTOR nonZeroSourceSize = DirectX::XMVectorSelect(sourceSize, DirectX::g_XMEpsilon, isZeroMask);

		DirectX::XMVECTOR origin = DirectX::XMVectorDivide(originRotationDepth.v, nonZeroSourceSize);

		// Convert the source region from texels to mod-1 texture coordinate format.
		if(flags & SpriteInfo::SourceInTexels)
		{
			source = DirectX::XMVectorMultiply(source.v, inverseTextureSize.v);
			sourceSize = DirectX::XMVectorMultiply(sourceSize, inverseTextureSize.v);
		} else
		{
			origin = DirectX::XMVectorMultiply(origin, inverseTextureSize.v);
		}

		// If the destination size is relative to the source region, convert it to pixels.
		if(!(flags & SpriteInfo::DestSizeInPixels))
		{
			destinationSize = DirectX::XMVectorMultiply(destinationSize, textureSize.v);
		}

		// Compute a 2x2 rotation matrix.
		DirectX::XMVECTOR rotationMatrix1;
		DirectX::XMVECTOR rotationMatrix2;

		if(rotation != 0)
		{
			float sin, cos;

			DirectX::XMScalarSinCos(&sin, &cos, rotation);

			DirectX::XMVECTOR sinV = DirectX::XMLoadFloat(&sin);
			DirectX::XMVECTOR cosV = DirectX::XMLoadFloat(&cos);

			rotationMatrix1 = DirectX::XMVectorMergeXY(cosV, sinV);
			rotationMatrix2 = DirectX::XMVectorMergeXY(DirectX::XMVectorNegate(sinV), cosV);
		} else
		{
			rotationMatrix1 = DirectX::g_XMIdentityR0;
			rotationMatrix2 = DirectX::g_XMIdentityR1;
		}

		// The four corner vertices are computed by transforming these unit-square positions.
		static DirectX::XMVECTORF32 cornerOffsets[VerticesPerSprite] =
		{
			{ { { 0, 0, 0, 0 } } },
			{ { { 1, 0, 0, 0 } } },
			{ { { 0, 1, 0, 0 } } },
			{ { { 1, 1, 0, 0 } } },
		};

		static_assert(SpriteEffects_FlipHorizontally == 1 &&
					  SpriteEffects_FlipVertically == 2, "If you change these enum values, the mirroring implementation must be updated to match");

		const unsigned int mirrorBits = flags & 3u;

		// Generate the four output vertices.
		for(uint32_t i = 0; i < VerticesPerSprite; i++)
		{
			// Calculate position.
			DirectX::XMVECTOR cornerOffset = DirectX::XMVectorMultiply(XMVectorSubtract(cornerOffsets[i], origin), destinationSize);

			// Apply 2x2 rotation matrix.
			DirectX::XMVECTOR position1 = DirectX::XMVectorMultiplyAdd(DirectX::XMVectorSplatX(cornerOffset), rotationMatrix1, destination.v);
			DirectX::XMVECTOR position2 = DirectX::XMVectorMultiplyAdd(DirectX::XMVectorSplatY(cornerOffset), rotationMatrix2, position1);

			// Set z = depth.
			DirectX::XMVECTOR position = DirectX::XMVectorPermute<0, 1, 7, 6>(position2, originRotationDepth.v);

			// Write position as a Float4, even though VertexPositionColor::position is an XMFLOAT3.
			// This is faster, and harmless as we are just clobbering the first element of the
			// following color field, which will immediately be overwritten with its correct value.
			DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4 *>(&vertices[i].position), position);

			vertices[i].color = color;

			// Compute and write the texture coordinate.
			DirectX::XMVECTOR textureCoordinate = DirectX::XMVectorMultiplyAdd(cornerOffsets[static_cast<unsigned int>(i) ^ mirrorBits], sourceSize, source.v);

			DirectX::XMStoreFloat2(&vertices[i].textureCoordinate, textureCoordinate);
		}
	}

	inline DirectX::XMVECTOR LoadRect(const Rect & rect)
	{
		DirectX::XMVECTOR v = DirectX::XMLoadInt4(reinterpret_cast<uint32_t const *>(&rect));

		v = DirectX::XMConvertVectorIntToFloat(v, 0);

		v = DirectX::XMVectorSubtract(v, DirectX::XMVectorPermute<0, 1, 4, 5>(DirectX::g_XMZero, v));

		return v;
	}

	void NC_MATH_CALLCONV SpriteBatch::Draw(const SpriteDesc & spriteDesc, const BorderDesc & borderDesc, Netcode::Vector4 destination, Netcode::Vector4 originRotationDepth, uint32_t flags)
	{
		if(!mInBeginEndPair)
			throw std::exception("Begin must be called before Draw");


		// Get a pointer to the output sprite.
		if(mSpriteQueueCount >= mSpriteQueueArraySize)
		{
			GrowSpriteQueue();
		}

		SpriteInfo * sprite = &mSpriteQueue[mSpriteQueueCount];
		Netcode::Vector4 dest = destination.v;
		Netcode::Vector4 source = Netcode::Float4::Zero;

		switch(spriteDesc.type) {
			case BackgroundType::SOLID:
				{
					Netcode::Float4 dst = destination;
					source = Netcode::Float4{ 0.0f, 0.0f, dst.z, dst.w };
				}
				break;
			case BackgroundType::TEXTURE:
				source = LoadRect(spriteDesc.sourceRect);
				break;
			default:
				break;
		}

		// If the destination size is relative to the source region, convert it to pixels.
		if(!(flags & SpriteInfo::DestSizeInPixels))
		{
			dest = DirectX::XMVectorPermute<0, 1, 6, 7>(dest.v, (dest * source).v); // dest.zw *= source.zw
		}

		flags |= SpriteInfo::SourceInTexels | SpriteInfo::DestSizeInPixels;

		sprite->source = source;
		sprite->borderDesc = borderDesc;
		sprite->textureSize = DirectX::XMLoadUInt2(&spriteDesc.textureSize);
		sprite->destination = dest;
		sprite->originRotationDepth = originRotationDepth;
		sprite->spriteDesc = spriteDesc;
		sprite->flags = flags;
		sprite->scissorRect = recordScissorRect;

		if(mSortMode == SpriteSortMode_Immediate)
		{
			// If we are in immediate mode, draw this sprite straight away.
			RenderBatch(sprite->spriteDesc.texture, sprite->textureSize, &sprite, 1);
			sprite->spriteDesc.texture.reset();
		} else {
			// Queue this sprite for later sorting and batched rendering.
			mSpriteQueueCount++;
		}
	}

	void SpriteBatch::GrowSortedSprites()
	{
		uint32_t previousSize = static_cast<uint32_t>(mSortedSprites.size());

		mSortedSprites.resize(mSpriteQueueCount);

		for(uint32_t i = previousSize; i < mSpriteQueueCount; i++)
		{
			mSortedSprites[i] = &mSpriteQueue[i];
		}
	}

	void SpriteBatch::DrawSprite(const SpriteDesc & spriteDesc, const Float2 & position)
	{
		DrawSprite(spriteDesc, BorderDesc{}, position, Netcode::Float2{ static_cast<float>(spriteDesc.textureSize.x), static_cast<float>(spriteDesc.textureSize.y) }, Netcode::Float2::Zero, 0.0f, 0.0f);
	}

	void SpriteBatch::DrawSprite(const SpriteDesc & spriteDesc, const Float2 & position, const Float2 & size)
	{
		DrawSprite(spriteDesc, BorderDesc{}, position, size, Netcode::Float2::Zero, 0.0f, 0.0f);
	}

	void SpriteBatch::DrawSprite(const SpriteDesc & spriteDesc, const Float2 & position, const Float2 & size, const Float2 & rotationOrigin, float rotationZ)
	{
		DrawSprite(spriteDesc, BorderDesc{}, position, size, rotationOrigin, rotationZ, 0.0f);
	}

	void SpriteBatch::DrawSprite(const SpriteDesc & spriteDesc, const Float2 & position, const Float2 & size, const Float2 & rotationOrigin, float rotationZ, float layerDepth)
	{
		DrawSprite(spriteDesc, BorderDesc{}, position, size, rotationOrigin, rotationZ, layerDepth);
	}

	void SpriteBatch::DrawSprite(const SpriteDesc & spriteDesc, const BorderDesc & borderDesc, const Float2 & position)
	{
		DrawSprite(spriteDesc, borderDesc, position, Netcode::Float2{ static_cast<float>(spriteDesc.textureSize.x), static_cast<float>(spriteDesc.textureSize.y) }, Netcode::Float2::Zero, 0.0f, 0.0f);
	}

	void SpriteBatch::DrawSprite(const SpriteDesc & spriteDesc, const BorderDesc & borderDesc, const Float2 & position, const Float2 & size)
	{
		DrawSprite(spriteDesc, borderDesc, position, size, Netcode::Float2::Zero, 0.0f, 0.0f);
	}

	void SpriteBatch::DrawSprite(const SpriteDesc & spriteDesc, const BorderDesc & borderDesc, const Float2 & position, const Float2 & size, const Float2 & rotationOrigin, float rotationZ)
	{
		DrawSprite(spriteDesc, borderDesc, position, size, rotationOrigin, rotationZ, 0);
	}

	void SpriteBatch::DrawSprite(const SpriteDesc & spriteDesc, const BorderDesc & borderDesc, const Float2 & position, const Float2 & size, const Float2 & rotationOrigin, float rotationZ, float layerDepth)
	{
		Netcode::Vector4 destination = DirectX::XMVectorPermute<0, 1, 4, 5>(DirectX::XMLoadFloat2(&position), DirectX::XMLoadFloat2(&size));
		Netcode::Vector4 originRotationDepth = DirectX::XMVectorSet(rotationOrigin.x, rotationOrigin.y, rotationZ, layerDepth);

		Draw(spriteDesc, borderDesc, destination, originRotationDepth, static_cast<uint32_t>(SpriteEffects_None) | SpriteInfo::DestSizeInPixels);
	}

	// Dynamically expands the array used to store pending sprite information.
	void SpriteBatch::GrowSpriteQueue()
	{
		// Grow by a factor of 2.
		uint32_t newSize = std::max(InitialQueueSize, mSpriteQueueArraySize * 2);

		// Allocate the new array.
		std::unique_ptr<SpriteInfo[]> newArray(new SpriteInfo[newSize]);

		// Copy over any existing sprites.
		for(uint32_t i = 0; i < mSpriteQueueCount; i++)
		{
			newArray[i] = mSpriteQueue[i];
		}

		// Replace the previous array with the new one.
		mSpriteQueue = std::move(newArray);
		mSpriteQueueArraySize = newSize;

		// Clear any dangling SpriteInfo pointers left over from previous rendering.
		mSortedSprites.clear();
	}

	void SpriteBatch::SetScissorRect(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom) {
		Rect r;
		r.left = left;
		r.right = right;
		r.top = top;
		r.bottom = bottom;
		SetScissorRect(r);
	}

	void SpriteBatch::SetScissorRect(const Rect & rect) {
		recordScissorRect = rect;
	}

	void SpriteBatch::SetScissorRect() {
		recordScissorRect.Clear();
	}

}
