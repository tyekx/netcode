#pragma once

#include "../Common.h"
#include "../Utility.h"

namespace Egg {
	namespace Mesh {

		GG_CLASS(Geometry)
		protected:
			std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
			D3D12_INPUT_LAYOUT_DESC inputLayout;
			D3D12_PRIMITIVE_TOPOLOGY topology;
		public:
			Geometry() : inputElements{}, inputLayout{}, topology{ D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST } {}

			void SetTopology(D3D12_PRIMITIVE_TOPOLOGY top) {
				topology = top;
			}

			D3D12_PRIMITIVE_TOPOLOGY GetTopology() const {
				return topology;
			}

			void AddInputElement(const D3D12_INPUT_ELEMENT_DESC & ied) {
				inputElements.push_back(ied);
			}

			virtual void Draw(ID3D12GraphicsCommandList * commandList) = 0;
			
			const D3D12_INPUT_LAYOUT_DESC & GetInputLayout() {
				inputLayout.NumElements = (unsigned int)inputElements.size();
				inputLayout.pInputElementDescs = &(inputElements.at(0));
				return inputLayout;
			}

		GG_ENDCLASS

		GG_SUBCLASS(VertexStreamGeometry, Geometry)
		protected:
			com_ptr<ID3D12Resource> vertexBuffer;
			D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
			unsigned int vertexCount;
		public:

			VertexStreamGeometry(ID3D12Device * device, void * data, unsigned int sizeInBytes, unsigned int stride) : Geometry{}, vertexBuffer{ nullptr },
				vertexBufferView{}, vertexCount{ 0 } {
				static int id = 0;
				ASSERT((sizeInBytes % stride) == 0, "VertexStreamGeometry: size and stride is inconsistent. Size % stride must be 0.");

				vertexCount = sizeInBytes / stride;

				DX_API("Failed to create committed resource (VertexStreamGeometry)")
					device->CreateCommittedResource(
						&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
						D3D12_HEAP_FLAG_NONE,
						&CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes),
						D3D12_RESOURCE_STATE_GENERIC_READ,
						nullptr,
						IID_PPV_ARGS(vertexBuffer.GetAddressOf())
					);

				CD3DX12_RANGE readRange{ 0, 0 };
				void * mappedPtr = nullptr;

				DX_API("Failed to map vertex buffer (VertexStreamGeometry)")
					vertexBuffer->Map(0, &readRange, &mappedPtr);

				memcpy(mappedPtr, data, sizeInBytes);
				vertexBuffer->Unmap(0, nullptr);

				vertexBuffer->SetName(Egg::Utility::WFormat(L"VB(VertexStreamGeometry)#%d", id++).c_str());

				vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
				vertexBufferView.SizeInBytes = sizeInBytes;
				vertexBufferView.StrideInBytes = stride;
			}
			
			virtual void Draw(ID3D12GraphicsCommandList * commandList) override {
				commandList->IASetPrimitiveTopology(topology);
				commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
				commandList->DrawInstanced(vertexCount, 1, 0, 0);
			}
	
		GG_ENDCLASS

		GG_SUBCLASS(IndexedGeometry, Geometry)
		protected:
			com_ptr<ID3D12Resource> vertexBuffer;
			com_ptr<ID3D12Resource> indexBuffer;
			D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
			D3D12_INDEX_BUFFER_VIEW indexBufferView;
			unsigned int indexCount;
		public:

			IndexedGeometry(ID3D12Device * device, void * data, unsigned int sizeInBytes, unsigned int stride,
												   void * indexData, unsigned int indexDataSizeInBytes, DXGI_FORMAT indexFormat = DXGI_FORMAT_R32_UINT) {
				static int id = 0;
				DX_API("Failed to create vertex buffer (IndexedGeometry)")
					device->CreateCommittedResource(
						&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
						D3D12_HEAP_FLAG_NONE,
						&CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes),
						D3D12_RESOURCE_STATE_GENERIC_READ,
						nullptr,
						IID_PPV_ARGS(vertexBuffer.GetAddressOf()));

				DX_API("Failed to create index buffer (IndexedGeometry")
					device->CreateCommittedResource(
						&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
						D3D12_HEAP_FLAG_NONE,
						&CD3DX12_RESOURCE_DESC::Buffer(indexDataSizeInBytes),
						D3D12_RESOURCE_STATE_GENERIC_READ,
						nullptr,
						IID_PPV_ARGS(indexBuffer.GetAddressOf()));

				CD3DX12_RANGE range{ 0,0 };
				void * mappedPtr;
				DX_API("Failed to map vertex buffer (IndexedGeometry)")
					vertexBuffer->Map(0, &range, &mappedPtr);

				memcpy(mappedPtr, data, sizeInBytes);
				vertexBuffer->Unmap(0, nullptr);

				DX_API("Failed to set name for vertex buffer (IndexedGeometry)")
					vertexBuffer->SetName(Egg::Utility::WFormat(L"VB(IndexedGeometry)#%d", id).c_str());

				DX_API("Failed to map index buffer (IndexedGeometry")
					indexBuffer->Map(0, &range, &mappedPtr);
				
				memcpy(mappedPtr, indexData, indexDataSizeInBytes);
				indexBuffer->Unmap(0, nullptr);

				DX_API("Failed to set name for index buffer (IndexedGeometry)")
					indexBuffer->SetName(Egg::Utility::WFormat(L"IB(IndexedGeometry)#%d", id++).c_str());

				vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
				vertexBufferView.SizeInBytes = sizeInBytes;
				vertexBufferView.StrideInBytes = stride;

				indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
				indexBufferView.Format = indexFormat;
				indexBufferView.SizeInBytes = indexDataSizeInBytes;

				ASSERT(indexFormat == DXGI_FORMAT_R16_UINT ||
					   indexFormat == DXGI_FORMAT_R32_UINT, "index format must be DXGI_FORMAT_R16_UINT or DGXI_FORMAT_R32_UINT");

				unsigned int formatSize = (indexFormat == DXGI_FORMAT_R16_UINT) ? 2 : 4;

				ASSERT(indexDataSizeInBytes % formatSize == 0, "index buffer size must be divisible by its format size");

				indexCount = (indexDataSizeInBytes / formatSize);
			}

			virtual void Draw(ID3D12GraphicsCommandList * commandList) override {
				commandList->IASetPrimitiveTopology(topology);
				commandList->IASetIndexBuffer(&indexBufferView);
				commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
				commandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
			}

		GG_ENDCLASS

	}
}
