#pragma once

#include <Netcode/Graphics/GraphicsContexts.h>
#include <Netcode/Vertex.h>
#include <vector>
#include <memory>

#include "DX12Common.h"
#include "DX12Includes.h"
#include <d3d11on12.h>
#include <d2d1_3.h>
#include <dwrite.h>

namespace Netcode::Module {

	class IGraphicsModule;

}

namespace Netcode::Graphics::DX12 {

	class DebugContext : public IDebugContext {

		struct DebugText {
			std::wstring content;
			Float2 pos;
		};

		Module::IGraphicsModule * graphicsModule;
		com_ptr<ID3D11On12Device> d11Device;
		com_ptr<ID3D11DeviceContext> d11Context;
		com_ptr<ID2D1Factory3> d2dFactory;
		com_ptr<IDXGIDevice> dxgiDevice;
		com_ptr<ID2D1Device> d2dDevice;
		com_ptr<ID2D1DeviceContext> d2dDeviceContext;
		com_ptr<IDWriteFactory> dwriteFactory;
		com_ptr<ID3D11Resource> wrappedResources[3];
		com_ptr<ID2D1Bitmap1> d2dRenderTargets[3];
		com_ptr<ID2D1SolidColorBrush> textBrush;
		com_ptr<IDWriteTextFormat> textFormat;
		
		Float3 defaultColor;
		Float4x4 viewProj;
		size_t bufferSize;
		size_t numNoDepthVertices;
		size_t numDepthVertices;
		uint32_t capsuleSlices;
		uint32_t sphereSlices;
		std::vector<PC_Vertex> vertices;

		Ref<GpuResource> uploadBuffer;
		Ref<PipelineState> depthPso;
		Ref<PipelineState> noDepthPso;
		Ref<RootSignature> rootSignature;

		std::vector<DebugText> debugTextBatch;

		void PushVertex(const PC_Vertex & vertex, bool depthEnabled);

		PC_Vertex * GetBufferForVertices(size_t numVertices, bool depthEnabled);

		void CreateD2DContext(Module::IGraphicsModule * graphics);
		
	public:
		void CreateResources(Module::IGraphicsModule * graphics);
		
		virtual void InternalSwapChainResourcesChanged(Module::IGraphicsModule * graphics) override;
		virtual void InternalPostRender() override;

		virtual void UploadResources(IResourceContext * context) override;

		virtual void Draw(IRenderContext * context, const Float4x4 & viewProjMatrix) override;

		virtual void DrawDebugText(std::wstring text, const Float2 & topLeftPosInWindowCoords) override;

		virtual void DrawPoint(const Float3 & point, float extent) override;
		virtual void DrawPoint(const Float3 & point, float extent, bool depthEnabled) override;
		virtual void DrawPoint(const Float3 & worldPos, float extents, const Float4x4 & transform) override;
		virtual void DrawPoint(const Float3 & worldPos, float extents, const Float4x4 & transform, bool depthEnabled) override;

		virtual void DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd) override;
		virtual void DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd, bool depthEnabled) override;
		virtual void DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd, const Float3 & color) override;
		virtual void DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd, const Float3 & color, bool depthEnabled) override;
		virtual void DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd, const Float3 & color, const Float4x4 & transform) override;
		virtual void DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd, const Float3 & color, const Float4x4 & transform, bool depthEnabled) override;

		virtual void DrawSphere(Vector3 worldPosOrigin, float radius) override;
		virtual void DrawSphere(Vector3 worldPosOrigin, float radius, bool depthEnabled) override;
		virtual void DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color) override;
		virtual void DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color, bool depthEnabled) override;
		virtual void DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color, const Float4x4 & transform) override;
		virtual void DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color, const Float4x4 & transform, bool depthEnabled) override;

		virtual void DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents) override;
		virtual void DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, bool depthEnabled) override;
		virtual void DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color) override;
		virtual void DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, bool depthEnabled) override;
		virtual void DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, const Float4x4 & transform) override;
		virtual void DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, const Float4x4 & transform, bool depthEnabled) override;

		virtual void DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents) override;
		virtual void DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, bool depthEnabled) override;
		virtual void DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color) override;
		virtual void DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, bool depthEnabled) override;
		virtual void DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, const Float4x4 & transform) override;
		virtual void DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, const Float4x4 & transform, bool depthEnabled) override;
	
		virtual void DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight) override;
		virtual void DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, bool depthEnabled) override;
		virtual void DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color) override;
		virtual void DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color, bool depthEnabled) override;
		virtual void DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color, const Float4x4 & transform) override;
		virtual void DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color, const Float4x4 & transform, bool depthEnabled) override;
	};

}
