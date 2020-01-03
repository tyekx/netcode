#include "DX12SpriteBatch.h"
#include "Utility.h"

namespace {

#include "Shaders/Compiled/SpriteFont_SpritePixelShader.inc"
#include "Shaders/Compiled/SpriteFont_SpriteVertexShader.inc"

}

namespace Egg::Graphics::DX12 {

	static const D3D12_INPUT_ELEMENT_DESC PCT_InputElements[] =
	{
		{ "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",       0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	const DirectX::XMMATRIX SpriteBatch::MatrixIdentity = DirectX::XMMatrixIdentity();
	const DirectX::XMFLOAT2 SpriteBatch::Float2Zero(0, 0);

	const D3D12_SHADER_BYTECODE SpriteBatch::s_DefaultVertexShaderByteCodeStatic = { SpriteFont_SpriteVertexShader, sizeof(SpriteFont_SpriteVertexShader) };
	const D3D12_SHADER_BYTECODE SpriteBatch::s_DefaultPixelShaderByteCodeStatic = { SpriteFont_SpritePixelShader, sizeof(SpriteFont_SpritePixelShader) };

	const D3D12_BLEND_DESC SpriteBatchPipelineStateDescription::s_DefaultBlendDesc =
	{
		FALSE, // AlphaToCoverageEnable
		FALSE, // IndependentBlendEnable
		{ {
			TRUE, // BlendEnable
			FALSE, // LogicOpEnable
			D3D12_BLEND_SRC_ALPHA, // SrcBlend
			D3D12_BLEND_INV_SRC_ALPHA, // DestBlend
			D3D12_BLEND_OP_ADD, // BlendOp
			D3D12_BLEND_ONE, // SrcBlendAlpha
			D3D12_BLEND_INV_SRC_ALPHA, // DestBlendAlpha
			D3D12_BLEND_OP_ADD, // BlendOpAlpha
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL
		} }
	};

	// Same to CommonStates::CullCounterClockwise
	const D3D12_RASTERIZER_DESC SpriteBatchPipelineStateDescription::s_DefaultRasterizerDesc = {
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_BACK,
		FALSE, // FrontCounterClockwise
		D3D12_DEFAULT_DEPTH_BIAS,
		D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		TRUE, // DepthClipEnable
		TRUE, // MultisampleEnable
		FALSE, // AntialiasedLineEnable
		0, // ForcedSampleCount
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	};

	// Same as CommonStates::DepthNone
	const D3D12_DEPTH_STENCIL_DESC SpriteBatchPipelineStateDescription::s_DefaultDepthStencilDesc =
	{
		FALSE, // DepthEnable
		D3D12_DEPTH_WRITE_MASK_ZERO,
		D3D12_COMPARISON_FUNC_LESS_EQUAL, // DepthFunc
		FALSE, // StencilEnable
		D3D12_DEFAULT_STENCIL_READ_MASK,
		D3D12_DEFAULT_STENCIL_WRITE_MASK,
		{
			D3D12_STENCIL_OP_KEEP, // StencilFailOp
			D3D12_STENCIL_OP_KEEP, // StencilDepthFailOp
			D3D12_STENCIL_OP_KEEP, // StencilPassOp
			D3D12_COMPARISON_FUNC_ALWAYS // StencilFunc
		}, // FrontFace
		{
			D3D12_STENCIL_OP_KEEP, // StencilFailOp
			D3D12_STENCIL_OP_KEEP, // StencilDepthFailOp
			D3D12_STENCIL_OP_KEEP, // StencilPassOp
			D3D12_COMPARISON_FUNC_ALWAYS // StencilFunc
		} // BackFace
	};

	SpriteBatch::DeviceResources::DeviceResources(_In_ ID3D12Device * device, Resource::IResourceUploader * upload) :
		indexBufferView{},
		mDevice(device)
	{
		CreateIndexBuffer(device, upload);
		CreateRootSignatures(device);
	}


	// Creates the SpriteBatch index buffer.
	void SpriteBatch::DeviceResources::CreateIndexBuffer(_In_ ID3D12Device * device, Resource::IResourceUploader * upload)
	{
		static_assert((MaxBatchSize * VerticesPerSprite) < USHRT_MAX, "MaxBatchSize too large for 16-bit indices");

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(short) * MaxBatchSize * IndicesPerSprite);

		DX_API("Failed to create index buffer")
			device->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(indexBuffer.ReleaseAndGetAddressOf()));

		indexBuffer->SetName(L"SpriteBatch");

		static auto indexValues = CreateIndexValues();

		D3D12_SUBRESOURCE_DATA indexDataDesc = {};
		indexDataDesc.pData = indexValues.data();
		indexDataDesc.RowPitch = static_cast<LONG_PTR>(bufferDesc.Width);
		indexDataDesc.SlicePitch = indexDataDesc.RowPitch;

		// Upload the resource
		upload->Upload(indexBuffer.Get(), reinterpret_cast<const BYTE*>(indexValues.data()), static_cast<UINT>(bufferDesc.Width));
		upload->Transition(indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

		// Create the index buffer view
		indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		indexBufferView.Format = DXGI_FORMAT_R16_UINT;
		indexBufferView.SizeInBytes = static_cast<UINT>(bufferDesc.Width);
	}


	void SpriteBatch::DeviceResources::CreateRootSignatures(_In_ ID3D12Device * device)
	{
		DX_API("Failed to create root signature")
			device->CreateRootSignature(0, SpriteFont_SpritePixelShader, sizeof(SpriteFont_SpritePixelShader), IID_PPV_ARGS(rootSignature.GetAddressOf()));
	}

	std::vector<short> SpriteBatch::DeviceResources::CreateIndexValues()
	{
		std::vector<short> indices;

		indices.reserve(MaxBatchSize * IndicesPerSprite);

		for(size_t j = 0; j < MaxBatchSize * VerticesPerSprite; j += VerticesPerSprite)
		{
			short i = static_cast<short>(j);

			indices.push_back(i);
			indices.push_back(i + 1);
			indices.push_back(i + 2);

			indices.push_back(i + 1);
			indices.push_back(i + 3);
			indices.push_back(i + 2);
		}

		return indices;
	}

	SpriteBatch::SpriteBatch(ID3D12Device * device, Resource::IResourceUploader * upload, const SpriteBatchPipelineStateDescription & psoDesc, const D3D12_VIEWPORT * viewport)
		: mRotation(DXGI_MODE_ROTATION_IDENTITY),
		mSetViewport(false),
		mViewPort{},
		mSpriteQueueCount(0),
		mSpriteQueueArraySize(0),
		mInBeginEndPair(false),
		mSortMode(SpriteSortMode_Deferred),
		mSampler{},
		mTransformMatrix(MatrixIdentity),
		mVertexPageSize(6 * 4 * MaxBatchSize * VerticesPerSprite),
		mSpriteCount(0),
		mDeviceResources(std::make_unique<DeviceResources>(device, upload))
	{
		if(viewport != nullptr)
		{
			mViewPort = *viewport;
			mSetViewport = true;
		}

		DX_API("Failed to create vertex buffer")
			device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(sizeof(PCT_Vertex) * MaxBatchSize * VerticesPerSprite),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(vertexBuffer.GetAddressOf())
			);

		CD3DX12_RANGE readValue{ 0,0 };

		DX_API("Failed to map vertex buffer")
			vertexBuffer->Map(0, &readValue, &mappedVertexBuffer);

		//auto handle = cbufferAlloc->AllocateCbuffer(sizeof(SpriteCbuffer));
		//cbuffer = reinterpret_cast<SpriteCbuffer *>(cbufferAlloc->GetCbufferPointer(handle));
		//cbufferAddr = cbufferAlloc->GetAddress(handle);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dDesc = {};

		D3D12_INPUT_LAYOUT_DESC il;
		il.NumElements = ARRAYSIZE(PCT_InputElements);
		il.pInputElementDescs = PCT_InputElements;

		d3dDesc.InputLayout = il;
		d3dDesc.BlendState = psoDesc.blendDesc;
		d3dDesc.DepthStencilState = psoDesc.depthStencilDesc;
		d3dDesc.RasterizerState = psoDesc.rasterizerDesc;
		d3dDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		d3dDesc.NodeMask = 0;
		d3dDesc.NumRenderTargets = 1;
		d3dDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		d3dDesc.SampleDesc.Count = 1;
		d3dDesc.SampleDesc.Quality = 0;
		d3dDesc.SampleMask = UINT_MAX;
		d3dDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		d3dDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		if(psoDesc.customRootSignature)
		{
			mRootSignature = psoDesc.customRootSignature;
		} else
		{
			mRootSignature = mDeviceResources->rootSignature.Get();
		}
		d3dDesc.pRootSignature = mRootSignature.Get();

		if(psoDesc.customVertexShader.pShaderBytecode)
		{
			d3dDesc.VS = psoDesc.customVertexShader;
		} else
		{
			ASSERT(psoDesc.samplerDescriptor.ptr == 0, "Not supported");
			d3dDesc.VS = s_DefaultVertexShaderByteCodeStatic;
		}

		if(psoDesc.customPixelShader.pShaderBytecode)
		{
			d3dDesc.PS = psoDesc.customPixelShader;
		} else
		{
			ASSERT(psoDesc.samplerDescriptor.ptr == 0, "Not supported");
			d3dDesc.PS = s_DefaultPixelShaderByteCodeStatic;
		}

		if(psoDesc.samplerDescriptor.ptr)
		{
			mSampler = psoDesc.samplerDescriptor;
		}

		DX_API("Failed to create GPSO")
			device->CreateGraphicsPipelineState(
				&d3dDesc,
				IID_PPV_ARGS(mPSO.GetAddressOf()));

		mPSO->SetName(L"SpriteBatch");
	}


	void XM_CALLCONV SpriteBatch::Begin(ID3D12GraphicsCommandList * commandList, SpriteSortMode sortMode, DirectX::FXMMATRIX transformMatrix)
	{
		if(mInBeginEndPair)
			throw std::exception("Cannot nest Begin calls on a single SpriteBatch");

		mSortMode = sortMode;
		mTransformMatrix = transformMatrix;
		mCommandList = commandList;
		mSpriteCount = 0;

		if(sortMode == SpriteSortMode_Immediate)
		{
			PrepareForRendering();
		}

		mInBeginEndPair = true;
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

		// Release this memory
		//mVertexSegment.Reset();

		// Break circular reference chains, in case the state lambda closed
		// over an object that holds a reference to this SpriteBatch.
		mCommandList = nullptr;
		mInBeginEndPair = false;
	}

	void SpriteBatch::PrepareForRendering()
	{
		auto commandList = mCommandList.Get();

		// Set root signature
		commandList->SetGraphicsRootSignature(mRootSignature.Get());

		// Set render state
		commandList->SetPipelineState(mPSO.Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Set the index buffer.
		commandList->IASetIndexBuffer(&mDeviceResources->indexBufferView);

		DirectX::XMMATRIX transformMatrix = (mRotation == DXGI_MODE_ROTATION_UNSPECIFIED)
			? mTransformMatrix
			: (mTransformMatrix * GetViewportTransform(mRotation));

		DirectX::XMStoreFloat4x4A(&cbuffer->transform, DirectX::XMMatrixTranspose(transformMatrix));
		commandList->SetGraphicsRootConstantBufferView(RootParameterIndex::ConstantBuffer, cbufferAddr);
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
				return x->texture.ptr < y->texture.ptr;
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
		D3D12_GPU_DESCRIPTOR_HANDLE batchTexture = {};
		DirectX::XMVECTOR batchTextureSize = {};
		size_t batchStart = 0;

		for(size_t pos = 0; pos < mSpriteQueueCount; pos++)
		{
			D3D12_GPU_DESCRIPTOR_HANDLE texture = mSortedSprites[pos]->texture;
			assert(texture.ptr != 0);
			DirectX::XMVECTOR textureSize = mSortedSprites[pos]->textureSize;

			// Flush whenever the texture changes.
			if(texture.ptr != batchTexture.ptr)
			{
				if(pos > batchStart)
				{
					RenderBatch(batchTexture, batchTextureSize, &mSortedSprites[batchStart], pos - batchStart);
				}

				batchTexture = texture;
				batchTextureSize = textureSize;
				batchStart = pos;
			}
		}

		// Flush the final batch.
		RenderBatch(batchTexture, batchTextureSize, &mSortedSprites[batchStart], mSpriteQueueCount - batchStart);

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

	void SpriteBatch::RenderBatch(D3D12_GPU_DESCRIPTOR_HANDLE texture, DirectX::XMVECTOR textureSize, SpriteInfo const * const * sprites, size_t count)
	{
		auto commandList = mCommandList.Get();

		// Draw using the specified texture.
		// **NOTE** If D3D asserts or crashes here, you probably need to call commandList->SetDescriptorHeaps() with the required descriptor heap(s)
		commandList->SetGraphicsRootDescriptorTable(RootParameterIndex::TextureSRV, texture);

		if(mSampler.ptr)
		{
			commandList->SetGraphicsRootDescriptorTable(RootParameterIndex::TextureSampler, mSampler);
		}

		// Convert to vector format.
		DirectX::XMVECTOR inverseTextureSize = DirectX::XMVectorReciprocal(textureSize);

		while(count > 0)
		{
			// How many sprites do we want to draw?
			size_t batchSize = count;

			// How many sprites does the D3D vertex buffer have room for?
			size_t remainingSpace = MaxBatchSize - mSpriteCount;

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

			PCT_Vertex * vertices = reinterpret_cast<PCT_Vertex *>(mappedVertexBuffer) + (UINT64(mSpriteCount)) * VerticesPerSprite;

			// Generate sprite vertex data.
			for(size_t i = 0; i < batchSize; i++)
			{
				assert(i < count);
				_Analysis_assume_(i < count);
				RenderSprite(sprites[i], vertices, textureSize, inverseTextureSize);

				vertices += VerticesPerSprite;
			}

			// Set the vertex buffer view
			D3D12_VERTEX_BUFFER_VIEW vbv;
			size_t spriteVertexTotalSize = sizeof(PCT_Vertex) * VerticesPerSprite;
			vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress() + (UINT64(mSpriteCount) * UINT64(spriteVertexTotalSize));
			vbv.StrideInBytes = sizeof(PCT_Vertex);
			vbv.SizeInBytes = static_cast<UINT>(batchSize * spriteVertexTotalSize);
			commandList->IASetVertexBuffers(0, 1, &vbv);

			// Ok lads, the time has come for us draw ourselves some sprites!
			UINT indexCount = static_cast<UINT>(batchSize * IndicesPerSprite);

			commandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);

			// Advance the buffer position.
			mSpriteCount += batchSize;

			sprites += batchSize;
			count -= batchSize;
		}
	}

	void XM_CALLCONV SpriteBatch::RenderSprite(SpriteInfo const * sprite, PCT_Vertex * vertices, DirectX::FXMVECTOR textureSize, DirectX::FXMVECTOR inverseTextureSize)
	{
		// Load sprite parameters into SIMD registers.
		DirectX::XMVECTOR source = XMLoadFloat4A(&sprite->source);
		DirectX::XMVECTOR destination = XMLoadFloat4A(&sprite->destination);
		DirectX::XMVECTOR color = XMLoadFloat4A(&sprite->color);
		DirectX::XMVECTOR originRotationDepth = XMLoadFloat4A(&sprite->originRotationDepth);

		float rotation = sprite->originRotationDepth.z;
		unsigned int flags = sprite->flags;

		// Extract the source and destination sizes into separate vectors.
		DirectX::XMVECTOR sourceSize = DirectX::XMVectorSwizzle<2, 3, 2, 3>(source);
		DirectX::XMVECTOR destinationSize = DirectX::XMVectorSwizzle<2, 3, 2, 3>(destination);

		// Scale the origin offset by source size, taking care to avoid overflow if the source region is zero.
		DirectX::XMVECTOR isZeroMask = DirectX::XMVectorEqual(sourceSize, DirectX::XMVectorZero());
		DirectX::XMVECTOR nonZeroSourceSize = DirectX::XMVectorSelect(sourceSize, DirectX::g_XMEpsilon, isZeroMask);

		DirectX::XMVECTOR origin = DirectX::XMVectorDivide(originRotationDepth, nonZeroSourceSize);

		// Convert the source region from texels to mod-1 texture coordinate format.
		if(flags & SpriteInfo::SourceInTexels)
		{
			source = DirectX::XMVectorMultiply(source, inverseTextureSize);
			sourceSize = DirectX::XMVectorMultiply(sourceSize, inverseTextureSize);
		} else
		{
			origin = DirectX::XMVectorMultiply(origin, inverseTextureSize);
		}

		// If the destination size is relative to the source region, convert it to pixels.
		if(!(flags & SpriteInfo::DestSizeInPixels))
		{
			destinationSize = DirectX::XMVectorMultiply(destinationSize, textureSize);
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

		// Tricksy alert! Texture coordinates are computed from the same cornerOffsets
		// table as vertex positions, but if the sprite is mirrored, this table
		// must be indexed in a different order. This is done as follows:
		//
		//    position = cornerOffsets[i]
		//    texcoord = cornerOffsets[i ^ SpriteEffects]

		static_assert(SpriteEffects_FlipHorizontally == 1 &&
					  SpriteEffects_FlipVertically == 2, "If you change these enum values, the mirroring implementation must be updated to match");

		const unsigned int mirrorBits = flags & 3u;

		// Generate the four output vertices.
		for(size_t i = 0; i < VerticesPerSprite; i++)
		{
			// Calculate position.
			DirectX::XMVECTOR cornerOffset = DirectX::XMVectorMultiply(XMVectorSubtract(cornerOffsets[i], origin), destinationSize);

			// Apply 2x2 rotation matrix.
			DirectX::XMVECTOR position1 = DirectX::XMVectorMultiplyAdd(DirectX::XMVectorSplatX(cornerOffset), rotationMatrix1, destination);
			DirectX::XMVECTOR position2 = DirectX::XMVectorMultiplyAdd(DirectX::XMVectorSplatY(cornerOffset), rotationMatrix2, position1);

			// Set z = depth.
			DirectX::XMVECTOR position = DirectX::XMVectorPermute<0, 1, 7, 6>(position2, originRotationDepth);

			// Write position as a Float4, even though VertexPositionColor::position is an XMFLOAT3.
			// This is faster, and harmless as we are just clobbering the first element of the
			// following color field, which will immediately be overwritten with its correct value.
			DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4 *>(&vertices[i].position), position);

			// Write the color.
			DirectX::XMStoreFloat4(&vertices[i].color, color);

			// Compute and write the texture coordinate.
			DirectX::XMVECTOR textureCoordinate = DirectX::XMVectorMultiplyAdd(cornerOffsets[static_cast<unsigned int>(i) ^ mirrorBits], sourceSize, source);

			DirectX::XMStoreFloat2(&vertices[i].textureCoordinate, textureCoordinate);
		}
	}

	void SpriteBatch::GrowSortedSprites()
	{
		size_t previousSize = mSortedSprites.size();

		mSortedSprites.resize(mSpriteQueueCount);

		for(size_t i = previousSize; i < mSpriteQueueCount; i++)
		{
			mSortedSprites[i] = &mSpriteQueue[i];
		}
	}



	// Dynamically expands the array used to store pending sprite information.
	void SpriteBatch::GrowSpriteQueue()
	{
		// Grow by a factor of 2.
		size_t newSize = std::max(InitialQueueSize, mSpriteQueueArraySize * 2);

		// Allocate the new array.
		std::unique_ptr<SpriteInfo[]> newArray(new SpriteInfo[newSize]);

		// Copy over any existing sprites.
		for(size_t i = 0; i < mSpriteQueueCount; i++)
		{
			newArray[i] = mSpriteQueue[i];
		}

		// Replace the previous array with the new one.
		mSpriteQueue = std::move(newArray);
		mSpriteQueueArraySize = newSize;

		// Clear any dangling SpriteInfo pointers left over from previous rendering.
		mSortedSprites.clear();
	}


	DirectX::XMMATRIX SpriteBatch::GetViewportTransform(_In_ DXGI_MODE_ROTATION rotation)
	{
		if(!mSetViewport)
			throw std::exception("Viewport not set.");

		// Compute the matrix.
		float xScale = (mViewPort.Width > 0) ? 2.0f / mViewPort.Width : 0.0f;
		float yScale = (mViewPort.Height > 0) ? 2.0f / mViewPort.Height : 0.0f;

		switch(rotation)
		{
		case DXGI_MODE_ROTATION_ROTATE90:
			return DirectX::XMMATRIX
			(
				0, -yScale, 0, 0,
				-xScale, 0, 0, 0,
				0, 0, 1, 0,
				1, 1, 0, 1
			);

		case DXGI_MODE_ROTATION_ROTATE270:
			return DirectX::XMMATRIX
			(
				0, yScale, 0, 0,
				xScale, 0, 0, 0,
				0, 0, 1, 0,
				-1, -1, 0, 1
			);

		case DXGI_MODE_ROTATION_ROTATE180:
			return DirectX::XMMATRIX
			(
				-xScale, 0, 0, 0,
				0, yScale, 0, 0,
				0, 0, 1, 0,
				1, -1, 0, 1
			);

		default:
			return DirectX::XMMATRIX
			(
				xScale, 0, 0, 0,
				0, -yScale, 0, 0,
				0, 0, 1, 0,
				-1, 1, 0, 1
			);
		}
	}

	inline DirectX::XMVECTOR LoadRect(_In_ RECT const * rect)
	{
		DirectX::XMVECTOR v = DirectX::XMLoadInt4(reinterpret_cast<uint32_t const *>(rect));

		v = DirectX::XMConvertVectorIntToFloat(v, 0);

		// Convert right/bottom to width/height.
		v = DirectX::XMVectorSubtract(v, DirectX::XMVectorPermute<0, 1, 4, 5>(DirectX::g_XMZero, v));

		return v;
	}

	void XM_CALLCONV SpriteBatch::Draw(D3D12_GPU_DESCRIPTOR_HANDLE texture,
									   DirectX::XMUINT2 const & textureSize,
									   DirectX::FXMVECTOR destination,
									   RECT const * sourceRectangle,
									   DirectX::FXMVECTOR color,
									   DirectX::FXMVECTOR originRotationDepth,
									   unsigned int flags)
	{
		if(!mInBeginEndPair)
			throw std::exception("Begin must be called before Draw");

		if(!texture.ptr)
			throw std::exception("Invalid texture for Draw");

		// Get a pointer to the output sprite.
		if(mSpriteQueueCount >= mSpriteQueueArraySize)
		{
			GrowSpriteQueue();
		}

		SpriteInfo * sprite = &mSpriteQueue[mSpriteQueueCount];

		DirectX::XMVECTOR dest = destination;

		if(sourceRectangle)
		{
			// User specified an explicit source region.
			DirectX::XMVECTOR source = LoadRect(sourceRectangle);

			XMStoreFloat4A(&sprite->source, source);

			// If the destination size is relative to the source region, convert it to pixels.
			if(!(flags & SpriteInfo::DestSizeInPixels))
			{
				dest = DirectX::XMVectorPermute<0, 1, 6, 7>(dest, DirectX::XMVectorMultiply(dest, source)); // dest.zw *= source.zw
			}

			flags |= SpriteInfo::SourceInTexels | SpriteInfo::DestSizeInPixels;
		} else
		{
			// No explicit source region, so use the entire texture.
			static const DirectX::XMVECTORF32 wholeTexture = { { {0, 0, 1, 1} } };

			XMStoreFloat4A(&sprite->source, wholeTexture);
		}

		// Convert texture size
		DirectX::XMVECTOR textureSizeV = XMLoadUInt2(&textureSize);

		// Store sprite parameters.
		XMStoreFloat4A(&sprite->destination, dest);
		XMStoreFloat4A(&sprite->color, color);
		XMStoreFloat4A(&sprite->originRotationDepth, originRotationDepth);

		sprite->texture = texture;
		sprite->textureSize = textureSizeV;
		sprite->flags = flags;

		if(mSortMode == SpriteSortMode_Immediate)
		{
			// If we are in immediate mode, draw this sprite straight away.
			RenderBatch(texture, textureSizeV, &sprite, 1);
		} else
		{
			// Queue this sprite for later sorting and batched rendering.
			mSpriteQueueCount++;
		}
	}


	void XM_CALLCONV SpriteBatch::Draw(D3D12_GPU_DESCRIPTOR_HANDLE texture,
									   DirectX::XMUINT2 const & textureSize,
									   DirectX::XMFLOAT2 const & position,
									   DirectX::FXMVECTOR color)
	{
		DirectX::XMVECTOR destination = DirectX::XMVectorPermute<0, 1, 4, 5>(DirectX::XMLoadFloat2(&position), DirectX::g_XMOne); // x, y, 1, 1

		Draw(texture, textureSize, destination, nullptr, color, DirectX::g_XMZero, 0);
	}

	void XM_CALLCONV SpriteBatch::Draw(D3D12_GPU_DESCRIPTOR_HANDLE texture,
									   DirectX::XMUINT2 const & textureSize,
									   DirectX::XMFLOAT2 const & position,
									   RECT const * sourceRectangle,
									   DirectX::FXMVECTOR color,
									   float rotation,
									   DirectX::XMFLOAT2 const & origin,
									   float scale,
									   SpriteEffects effects,
									   float layerDepth)
	{
		DirectX::XMVECTOR destination = DirectX::XMVectorPermute<0, 1, 4, 4>(DirectX::XMLoadFloat2(&position), DirectX::XMLoadFloat(&scale)); // x, y, scale, scale

		DirectX::XMVECTOR originRotationDepth = DirectX::XMVectorSet(origin.x, origin.y, rotation, layerDepth);

		Draw(texture, textureSize, destination, sourceRectangle, color, originRotationDepth, static_cast<unsigned int>(effects));
	}


	void XM_CALLCONV SpriteBatch::Draw(D3D12_GPU_DESCRIPTOR_HANDLE texture,
									   DirectX::XMUINT2 const & textureSize,
									   DirectX::XMFLOAT2 const & position,
									   RECT const * sourceRectangle,
									   DirectX::FXMVECTOR color,
									   float rotation,
									   DirectX::XMFLOAT2 const & origin,
									   DirectX::XMFLOAT2 const & scale,
									   SpriteEffects effects,
									   float layerDepth)
	{
		DirectX::XMVECTOR destination = DirectX::XMVectorPermute<0, 1, 4, 5>(DirectX::XMLoadFloat2(&position), DirectX::XMLoadFloat2(&scale)); // x, y, scale.x, scale.y

		DirectX::XMVECTOR originRotationDepth = DirectX::XMVectorSet(origin.x, origin.y, rotation, layerDepth);

		Draw(texture, textureSize, destination, sourceRectangle, color, originRotationDepth, static_cast<unsigned int>(effects));
	}


	void XM_CALLCONV SpriteBatch::Draw(D3D12_GPU_DESCRIPTOR_HANDLE texture, DirectX::XMUINT2 const & textureSize, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color)
	{
		DirectX::XMVECTOR destination = DirectX::XMVectorPermute<0, 1, 4, 5>(position, DirectX::g_XMOne); // x, y, 1, 1

		Draw(texture, textureSize, destination, nullptr, color, DirectX::g_XMZero, 0);
	}


	void XM_CALLCONV SpriteBatch::Draw(D3D12_GPU_DESCRIPTOR_HANDLE texture,
									   DirectX::XMUINT2 const & textureSize,
									   DirectX::FXMVECTOR position,
									   RECT const * sourceRectangle,
									   DirectX::FXMVECTOR color,
									   float rotation,
									   DirectX::FXMVECTOR origin,
									   float scale,
									   SpriteEffects effects,
									   float layerDepth)
	{
		DirectX::XMVECTOR destination = DirectX::XMVectorPermute<0, 1, 4, 4>(position, DirectX::XMLoadFloat(&scale)); // x, y, scale, scale

		DirectX::XMVECTOR rotationDepth = DirectX::XMVectorMergeXY(DirectX::XMVectorReplicate(rotation), DirectX::XMVectorReplicate(layerDepth));

		DirectX::XMVECTOR originRotationDepth = DirectX::XMVectorPermute<0, 1, 4, 5>(origin, rotationDepth);

		Draw(texture, textureSize, destination, sourceRectangle, color, originRotationDepth, static_cast<unsigned int>(effects));
	}


	void XM_CALLCONV SpriteBatch::Draw(D3D12_GPU_DESCRIPTOR_HANDLE texture,
									   DirectX::XMUINT2 const & textureSize,
									   DirectX::FXMVECTOR position,
									   RECT const * sourceRectangle,
									   DirectX::FXMVECTOR color,
									   float rotation,
									   DirectX::FXMVECTOR origin,
									   DirectX::GXMVECTOR scale,
									   SpriteEffects effects,
									   float layerDepth)
	{
		DirectX::XMVECTOR destination = DirectX::XMVectorPermute<0, 1, 4, 5>(position, scale); // x, y, scale.x, scale.y

		DirectX::XMVECTOR rotationDepth = DirectX::XMVectorMergeXY(DirectX::XMVectorReplicate(rotation), DirectX::XMVectorReplicate(layerDepth));

		DirectX::XMVECTOR originRotationDepth = DirectX::XMVectorPermute<0, 1, 4, 5>(origin, rotationDepth);

		Draw(texture, textureSize, destination, sourceRectangle, color, originRotationDepth, static_cast<unsigned int>(effects));
	}


	void XM_CALLCONV SpriteBatch::Draw(D3D12_GPU_DESCRIPTOR_HANDLE texture,
									   DirectX::XMUINT2 const & textureSize,
									   RECT const & destinationRectangle,
									   DirectX::FXMVECTOR color)
	{
		DirectX::XMVECTOR destination = LoadRect(&destinationRectangle); // x, y, w, h

		Draw(texture, textureSize, destination, nullptr, color, DirectX::g_XMZero, SpriteInfo::DestSizeInPixels);
	}


	void XM_CALLCONV SpriteBatch::Draw(D3D12_GPU_DESCRIPTOR_HANDLE texture,
									   DirectX::XMUINT2 const & textureSize,
									   RECT const & destinationRectangle,
									   RECT const * sourceRectangle,
									   DirectX::FXMVECTOR color,
									   float rotation,
									   DirectX::XMFLOAT2 const & origin,
									   SpriteEffects effects,
									   float layerDepth)
	{
		DirectX::XMVECTOR destination = LoadRect(&destinationRectangle); // x, y, w, h

		DirectX::XMVECTOR originRotationDepth = DirectX::XMVectorSet(origin.x, origin.y, rotation, layerDepth);

		Draw(texture, textureSize, destination, sourceRectangle, color, originRotationDepth, static_cast<unsigned int>(effects) | SpriteInfo::DestSizeInPixels);
	}

	void SpriteBatch::SetRotation(DXGI_MODE_ROTATION mode)
	{
		mRotation = mode;
	}


	DXGI_MODE_ROTATION SpriteBatch::GetRotation() const
	{
		return mRotation;
	}


	void SpriteBatch::SetViewport(const D3D12_VIEWPORT & viewPort)
	{
		mSetViewport = true;
		mViewPort = viewPort;
	}

}
