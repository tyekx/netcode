#include "DX12DebugContext.h"
#include <Netcode/HandleTypes.h>
#include <Netcode/Config.h>
#include <Netcode/Vertex.h>
#include <Netcode/Modules.h>
#include <Netcode/BasicGeometry.h>
#include <Netcode/Graphics/ResourceEnums.h>
#include "DX12GraphicsModule.h"

namespace Netcode::Graphics::DX12 {

	static inline Vector3 HTransform(Vector4 v, Matrix m) {
		v = v.Transform(m);
		return (v / v.Swizzle<3, 3, 3, 3>()).XYZ();
	}

	void DebugContext::PushVertex(const PC_Vertex & vertex, bool depthEnabled) {
		// assert: bufferSize > (numNoDepthVertices + numDepthVertices)
		if(depthEnabled) {
			vertices[numDepthVertices++] = vertex;
		} else {
			vertices[bufferSize - 1 - numNoDepthVertices++] = vertex;
		}
	}

	PC_Vertex * DebugContext::GetBufferForVertices(size_t numVertices, bool depthEnabled) {
		// assert: bufferSize >= (numNoDepthVertices + numDepthVertices + numVertices)
		PC_Vertex * tmp;
		if(depthEnabled) {
			tmp = vertices.data() + numDepthVertices;
			numDepthVertices += numVertices;
		} else {
			tmp = vertices.data() + (bufferSize - numNoDepthVertices - numVertices);
			numNoDepthVertices += numVertices;
		}
		return tmp;
	}

	void DebugContext::CreateD2DContext(Module::IGraphicsModule * graphics)
	{
		DX12GraphicsModule * dx12Module = dynamic_cast<DX12GraphicsModule *>(graphics);

		com_ptr<ID3D11Device> tmpDev;
		
		DX_API("Failed to create D11Device")
			D3D11On12CreateDevice(dx12Module->device.Get(),
				D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT,
				nullptr,
				0,
				reinterpret_cast<IUnknown **>(dx12Module->commandQueue.GetAddressOf()),
				1, 0, tmpDev.GetAddressOf(), d11Context.GetAddressOf(), nullptr);
		
		DX_API("Failed to cast to D3D11on12Device")
			tmpDev.As(&d11Device);
		
		D2D1_FACTORY_OPTIONS factoryOptions;
		factoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
		D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;

		DX_API("Failed to create D2DFactory")
			D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &factoryOptions, reinterpret_cast<void **>(d2dFactory.GetAddressOf()));

		DX_API("Failed to cast D3D11 device to DXGI device")
			d11Device.As(&dxgiDevice);

		DX_API("Failed to create D2DDevice")
			d2dFactory->CreateDevice(dxgiDevice.Get(), d2dDevice.GetAddressOf());

		DX_API("Failed to create D2DDeviceContext")
			d2dDevice->CreateDeviceContext(deviceOptions, d2dDeviceContext.GetAddressOf());

		DX_API("Failed to create DWriteFactory")
			DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown **>(dwriteFactory.GetAddressOf()));

		d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &textBrush);
		dwriteFactory->CreateTextFormat(
			L"Verdana",
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			16,
			L"en-us",
			textFormat.GetAddressOf()
		);
		textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	}

	void DebugContext::CreateResources(Module::IGraphicsModule * graphics) {
		graphicsModule = graphics;
		CreateD2DContext(graphics);
		
		Ref<Netcode::InputLayoutBuilder> ilBuilder = graphics->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("COLOR", DXGI_FORMAT_R32G32B32_FLOAT);
		Ref<Netcode::InputLayout> il = ilBuilder->Build();

		Netcode::DepthStencilDesc depthStencilDesc;
		Netcode::RasterizerDesc rasterizerDesc;
		rasterizerDesc.antialiasedLineEnable = true;
		depthStencilDesc.depthWriteMaskZero = true;

		Ref<Netcode::ShaderBuilder> shaderBuilder = graphics->CreateShaderBuilder();
		Ref<Netcode::ShaderBytecode> vertexShader = shaderBuilder->LoadBytecode(L"Netcode_DebugPrimVS.cso");
		Ref<Netcode::ShaderBytecode> pixelShader = shaderBuilder->LoadBytecode(L"Netcode_DebugPrimPS.cso");

		Ref<Netcode::RootSignatureBuilder> rootSigBuilder = graphics->CreateRootSignatureBuilder();
		rootSignature = rootSigBuilder->BuildFromShader(vertexShader);

		Ref<Netcode::GPipelineStateBuilder> gpsoBuilder = graphics->CreateGPipelineStateBuilder();
		gpsoBuilder->SetNumRenderTargets(1);
		gpsoBuilder->SetRenderTargetFormat(0, graphics->GetBackbufferFormat());
		gpsoBuilder->SetDepthStencilFormat(graphics->GetDepthStencilFormat());
		gpsoBuilder->SetInputLayout(il);
		gpsoBuilder->SetPrimitiveTopologyType(PrimitiveTopologyType::LINE);
		gpsoBuilder->SetRasterizerState(rasterizerDesc);
		gpsoBuilder->SetDepthStencilState(depthStencilDesc);
		gpsoBuilder->SetVertexShader(vertexShader);
		gpsoBuilder->SetPixelShader(pixelShader);
		gpsoBuilder->SetRootSignature(rootSignature);
		depthPso = gpsoBuilder->Build();

		depthStencilDesc.depthEnable = false;

		gpsoBuilder->SetNumRenderTargets(1);
		gpsoBuilder->SetRenderTargetFormat(0, graphics->GetBackbufferFormat());
		gpsoBuilder->SetDepthStencilFormat(graphics->GetDepthStencilFormat());
		gpsoBuilder->SetInputLayout(il);
		gpsoBuilder->SetPrimitiveTopologyType(PrimitiveTopologyType::LINE);
		gpsoBuilder->SetRasterizerState(rasterizerDesc);
		gpsoBuilder->SetDepthStencilState(depthStencilDesc);
		gpsoBuilder->SetVertexShader(vertexShader);
		gpsoBuilder->SetPixelShader(pixelShader);
		gpsoBuilder->SetRootSignature(rootSignature);
		noDepthPso = gpsoBuilder->Build();

		defaultColor = Config::Get<Float3>(L"graphics.debug.defaultColor:Float3");
		bufferSize = Config::Get<uint32_t>(L"graphics.debug.primitiveBufferDepth:u32");
		uploadBuffer = graphics->resources->CreateVertexBuffer(bufferSize * sizeof(PC_Vertex),
			sizeof(PC_Vertex),
			ResourceType::PERMANENT_UPLOAD,
			ResourceState::ANY_READ);

		vertices.resize(bufferSize);
	}
	void DebugContext::UploadResources(IResourceContext * context) {
		if(numDepthVertices > 0) {
			context->CopyConstants(uploadBuffer, vertices.data(), numDepthVertices * sizeof(PC_Vertex), 0);
		}

		if(numNoDepthVertices > 0) {
			context->CopyConstants(uploadBuffer, vertices.data() + (bufferSize - numNoDepthVertices), numNoDepthVertices * sizeof(PC_Vertex), numDepthVertices * sizeof(PC_Vertex));
		}
	}
	
	void DebugContext::InternalSwapChainResourcesChanged(Module::IGraphicsModule * graphics) {
		if(graphics == nullptr) {
			d2dRenderTargets[0].Reset();
			d2dRenderTargets[1].Reset();
			d2dRenderTargets[2].Reset();
			wrappedResources[0].Reset();
			wrappedResources[1].Reset();
			wrappedResources[2].Reset();

			d2dDeviceContext->SetTarget(nullptr);
			d11Context->Flush();
			return;
		}
		
		DX12GraphicsModule * g = dynamic_cast<DX12GraphicsModule *>(graphics);
		graphicsModule = graphics;
		
		const float dpi = static_cast<float>(g->windowDpi);
		
		D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
			dpi,
			dpi
		);
		
		for(UINT i = 0; i < g->backbufferDepth; i++) {
			com_ptr<ID3D12Resource> resource;
			
			g->swapChain->GetBuffer(i, IID_PPV_ARGS(resource.GetAddressOf()));

			D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };

			DX_API("Failed to create wrapped resource")
			d11Device->CreateWrappedResource(resource.Get(),
				&d3d11Flags,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				IID_PPV_ARGS(wrappedResources[i].ReleaseAndGetAddressOf())
			);

			com_ptr<IDXGISurface> dxgiSurface;
			
			DX_API("Failed to cast to DXGI surface")
				wrappedResources[i].As(&dxgiSurface);

			DX_API("Failed to create bitmap from dxgi surface")
				d2dDeviceContext->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), &bitmapProperties, d2dRenderTargets[i].ReleaseAndGetAddressOf());
		}
	}
	
	void DebugContext::Draw(IRenderContext * context, const Netcode::Float4x4 & viewProjMatrix) {
		if(numDepthVertices == 0 && numNoDepthVertices == 0) {
			return;
		}

		viewProj = viewProjMatrix;

		context->SetPrimitiveTopology(PrimitiveTopology::LINELIST);
		context->SetScissorRect();
		context->SetViewport();
		context->SetRootSignature(rootSignature.get());
		context->SetRootConstants(0, &viewProj, 16);
		context->SetVertexBuffer(uploadBuffer.get());

		if(numDepthVertices > 0) {
			context->SetPipelineState(depthPso.get());
			context->Draw(numDepthVertices);
		}

		if(numNoDepthVertices > 0) {
			context->SetPipelineState(noDepthPso.get());
			context->Draw(numNoDepthVertices, numDepthVertices);
		}

		numDepthVertices = 0;
		numNoDepthVertices = 0;
	}

	void DebugContext::DrawDebugText(std::wstring text, const Float2 & topLeftPosInWindowCoords)
	{
		DebugText dt;
		dt.content = std::move(text);
		dt.pos = topLeftPosInWindowCoords;
		debugTextBatch.emplace_back(std::move(dt));
	}

	void DebugContext::InternalPostRender()
	{
		if(debugTextBatch.empty()) {
			return;
		}
		
		UINT frameIndex = dynamic_cast<DX12GraphicsModule *>(graphicsModule)->backbufferIndex;

		D2D1_SIZE_F rtSize = d2dRenderTargets[frameIndex]->GetSize();

		d11Device->AcquireWrappedResources(wrappedResources[frameIndex].GetAddressOf(), 1);
		
		d2dDeviceContext->SetTarget(d2dRenderTargets[frameIndex].Get());
		d2dDeviceContext->BeginDraw();
		d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

		for(const DebugText & dt: debugTextBatch) {
			D2D1_RECT_F textRect = D2D1::RectF(dt.pos.x, dt.pos.y, rtSize.width - dt.pos.x, rtSize.height - dt.pos.y);
			d2dDeviceContext->DrawTextA(
				dt.content.c_str(),
				dt.content.size(),
				textFormat.Get(),
				&textRect,
				textBrush.Get()
			);
		}

		debugTextBatch.clear();

		DX_API("Failed to end draw")
			d2dDeviceContext->EndDraw();

		d11Device->ReleaseWrappedResources(wrappedResources[frameIndex].GetAddressOf(), 1);

		d11Context->Flush();
	}

	void DebugContext::DrawPoint(const Float3 & point, float extent) {
		DrawPoint(point, extent, Float4x4::Identity, true);
	}

	void DebugContext::DrawPoint(const Float3 & point, float extent, bool depthEnabled) {
		DrawPoint(point, extent, Float4x4::Identity, depthEnabled);
	}

	void DebugContext::DrawPoint(const Float3 & point, float extent, const Float4x4 & transform)
	{
		DrawPoint(point, extent, transform, true);
	}

	void DebugContext::DrawPoint(const Float3 & worldPos, float extent, const Float4x4 & transform, bool depthEnabled)
	{
		Float3 point = HTransform(Vector3{ worldPos }.XYZ1(), transform);

		DrawLine(
			Float3{ point.x - extent, point.y, point.z },
			Float3{ point.x + extent, point.y, point.z },
			Float3{ 1.0f, 0.0f, 0.0f },
			depthEnabled
		);

		DrawLine(
			Float3{ point.x, point.y - extent, point.z },
			Float3{ point.x, point.y + extent, point.z },
			Float3{ 0.0f, 1.0f, 0.0f },
			depthEnabled
		);

		DrawLine(
			Float3{ point.x, point.y, point.z - extent },
			Float3{ point.x, point.y, point.z + extent },
			Float3{ 0.0f, 0.0f, 1.0f },
			depthEnabled
		);
	}

	void DebugContext::DrawLine(const Netcode::Float3 & worldPosStart, const Netcode::Float3 & worldPosEnd) {
		DrawLine(worldPosStart, worldPosEnd, true);
	}

	void DebugContext::DrawLine(const Netcode::Float3 & worldPosStart, const Netcode::Float3 & worldPosEnd, bool depthEnabled) {
		DrawLine(worldPosStart, worldPosEnd, defaultColor, depthEnabled);
	}

	void DebugContext::DrawLine(const Netcode::Float3 & worldPosStart, const Netcode::Float3 & worldPosEnd, const Netcode::Float3 & color) {
		DrawLine(worldPosStart, worldPosEnd, color, true);
	}

	void DebugContext::DrawLine(const Netcode::Float3 & worldPosStart, const Netcode::Float3 & worldPosEnd, const Netcode::Float3 & color, bool depthEnabled) {
		PC_Vertex vert0;
		vert0.position = worldPosStart;
		vert0.color = color;

		PC_Vertex vert1;
		vert1.position = worldPosEnd;
		vert1.color = color;

		PushVertex(vert0, depthEnabled);
		PushVertex(vert1, depthEnabled);
	}

	void DebugContext::DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd, const Float3 & color, const Float4x4 & transform)
	{
		DrawLine(worldPosStart, worldPosEnd, color, transform, true);
	}

	void DebugContext::DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd, const Float3 & color, const Float4x4 & transform, bool depthEnabled)
	{
		PC_Vertex vert0;
		vert0.position = HTransform(Vector3{ worldPosStart }.XYZ1(), transform);
		vert0.color = color;

		PC_Vertex vert1;
		vert1.position = HTransform(Vector3{ worldPosEnd }.XYZ1(), transform);
		vert1.color = color;

		PushVertex(vert0, depthEnabled);
		PushVertex(vert1, depthEnabled);
	}

	void DebugContext::DrawSphere(Vector3 worldPosOrigin, float radius)
	{
		DrawSphere(worldPosOrigin, radius, defaultColor, true);
	}

	void DebugContext::DrawSphere(Vector3 worldPosOrigin, float radius, bool depthEnabled)
	{
		DrawSphere(worldPosOrigin, radius, defaultColor, depthEnabled);
	}

	void DebugContext::DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color)
	{
		DrawSphere(worldPosOrigin, radius, color, true);
	}

	void DebugContext::DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color, bool depthEnabled)
	{
		const uint32_t numSlices = Config::Get<uint32_t>(L"graphics.debug.sphereSlices:u32");

		uint32_t numVertices = BasicGeometry::GetSphereWireframeSize(numSlices);

		Netcode::PC_Vertex * vData = GetBufferForVertices(numVertices, depthEnabled);

		BasicGeometry::CreateSphereWireframe(vData, sizeof(Netcode::PC_Vertex), radius, numSlices, 0);

		for(uint32_t i = 0; i < numVertices; ++i) {
			vData[i].position = worldPosOrigin + vData[i].position;
			vData[i].color = color;
		}
	}

	void DebugContext::DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color, const Float4x4 & transform)
	{
		DrawSphere(worldPosOrigin, radius, color, transform, true);
	}

	void DebugContext::DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color, const Float4x4 & transform, bool depthEnabled)
	{
		const uint32_t numSlices = Config::Get<uint32_t>(L"graphics.debug.sphereSlices:u32");

		uint32_t numVertices = BasicGeometry::GetSphereWireframeSize(numSlices);

		PC_Vertex * vData = GetBufferForVertices(numVertices, depthEnabled);

		BasicGeometry::CreateSphereWireframe(vData, sizeof(PC_Vertex), radius, numSlices, 0);

		for(uint32_t i = 0; i < numVertices; ++i) {
			vData[i].position = HTransform((worldPosOrigin + vData[i].position).XYZ1(), transform);
			vData[i].color = color;
		}
	}

	void DebugContext::DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents)
	{
		DrawBoundingBox(worldPosOrigin, halfExtents, defaultColor, true);
	}

	void DebugContext::DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, bool depthEnabled)
	{
		DrawBoundingBox(worldPosOrigin, halfExtents, defaultColor, depthEnabled);
	}

	void DebugContext::DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color)
	{
		DrawBoundingBox(worldPosOrigin, halfExtents, color, true);
	}

	void DebugContext::DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, bool depthEnabled)
	{
		DrawBox(Quaternion{}, worldPosOrigin, halfExtents, color, depthEnabled);
	}

	void DebugContext::DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, const Float4x4 & transform)
	{
		DrawBox(Quaternion{}, worldPosOrigin, halfExtents, color, transform, true);
	}

	void DebugContext::DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, const Float4x4 & transform, bool depthEnabled)
	{
		DrawBox(Quaternion{}, worldPosOrigin, halfExtents, color, transform, depthEnabled);
	}

	void DebugContext::DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents)
	{
		DrawBox(orientation, worldPosOrigin, halfExtents, defaultColor, true);
	}

	void DebugContext::DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, bool depthEnabled)
	{
		DrawBox(orientation, worldPosOrigin, halfExtents, defaultColor, depthEnabled);
	}

	void DebugContext::DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color)
	{
		DrawBox(orientation, worldPosOrigin, halfExtents, color, true);
	}

	void DebugContext::DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, bool depthEnabled)
	{
		const Vector3 negExt = -halfExtents;

		DrawLine(worldPosOrigin + halfExtents.Permute<0, 1, 2>(negExt).Rotate(orientation),
			     worldPosOrigin + halfExtents.Permute<3, 1, 2>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<3, 1, 2>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<3, 4, 2>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<0, 1, 2>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<0, 4, 2>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<0, 4, 2>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<3, 4, 2>(negExt).Rotate(orientation), color, depthEnabled);

		DrawLine(worldPosOrigin + halfExtents.Permute<0, 1, 5>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<3, 1, 5>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<3, 1, 5>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<3, 4, 5>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<0, 1, 5>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<0, 4, 5>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<0, 4, 5>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<3, 4, 5>(negExt).Rotate(orientation), color, depthEnabled);

		DrawLine(worldPosOrigin + halfExtents.Permute<0, 1, 2>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<0, 1, 5>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<0, 4, 2>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<0, 4, 5>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<3, 1, 2>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<3, 1, 5>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<3, 4, 2>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<3, 4, 5>(negExt).Rotate(orientation), color, depthEnabled);
	}

	void DebugContext::DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, const Float4x4 & transform)
	{
		DrawBox(orientation, worldPosOrigin, halfExtents, color, transform, true);
	}

	void DebugContext::DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, const Float4x4 & transform, bool depthEnabled)
	{
		const Vector3 negExt = -halfExtents;

		DrawLine(worldPosOrigin + halfExtents.Permute<0, 1, 2>(negExt).Rotate(orientation),
			worldPosOrigin + halfExtents.Permute<3, 1, 2>(negExt).Rotate(orientation), color, transform, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<3, 1, 2>(negExt).Rotate(orientation),
			worldPosOrigin + halfExtents.Permute<3, 4, 2>(negExt).Rotate(orientation), color, transform, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<0, 1, 2>(negExt).Rotate(orientation),
			worldPosOrigin + halfExtents.Permute<0, 4, 2>(negExt).Rotate(orientation), color, transform, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<0, 4, 2>(negExt).Rotate(orientation),
			worldPosOrigin + halfExtents.Permute<3, 4, 2>(negExt).Rotate(orientation), color, transform, depthEnabled);

		DrawLine(worldPosOrigin + halfExtents.Permute<0, 1, 5>(negExt).Rotate(orientation),
			worldPosOrigin + halfExtents.Permute<3, 1, 5>(negExt).Rotate(orientation), color, transform, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<3, 1, 5>(negExt).Rotate(orientation),
			worldPosOrigin + halfExtents.Permute<3, 4, 5>(negExt).Rotate(orientation), color, transform, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<0, 1, 5>(negExt).Rotate(orientation),
			worldPosOrigin + halfExtents.Permute<0, 4, 5>(negExt).Rotate(orientation), color, transform, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<0, 4, 5>(negExt).Rotate(orientation),
			worldPosOrigin + halfExtents.Permute<3, 4, 5>(negExt).Rotate(orientation), color, transform, depthEnabled);

		DrawLine(worldPosOrigin + halfExtents.Permute<0, 1, 2>(negExt).Rotate(orientation),
			worldPosOrigin + halfExtents.Permute<0, 1, 5>(negExt).Rotate(orientation), color, transform, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<0, 4, 2>(negExt).Rotate(orientation),
			worldPosOrigin + halfExtents.Permute<0, 4, 5>(negExt).Rotate(orientation), color, transform, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<3, 1, 2>(negExt).Rotate(orientation),
			worldPosOrigin + halfExtents.Permute<3, 1, 5>(negExt).Rotate(orientation), color, transform, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<3, 4, 2>(negExt).Rotate(orientation),
			worldPosOrigin + halfExtents.Permute<3, 4, 5>(negExt).Rotate(orientation), color, transform, depthEnabled);
	}

	void DebugContext::DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight)
	{
		DrawCapsule(rotation, position, radius, halfHeight, defaultColor, true);
	}

	void DebugContext::DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, bool depthEnabled)
	{
		DrawCapsule(rotation, position, radius, halfHeight, defaultColor, depthEnabled);
	}

	void DebugContext::DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color)
	{
		DrawCapsule(rotation, position, radius, halfHeight, color, true);
	}

	void DebugContext::DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color, bool depthEnabled)
	{
		const uint32_t numSlices = Config::Get<uint32_t>(L"graphics.debug.capsuleSlices:u32");

		uint32_t numVertices = BasicGeometry::GetCapsuleWireframeSize(numSlices);

		PC_Vertex * vData = GetBufferForVertices(numVertices, depthEnabled);

		BasicGeometry::CreateCapsuleWireframe(vData, sizeof(PC_Vertex), numSlices, radius, halfHeight, 0);

		for(uint32_t i = 0; i < numVertices; ++i) {
			vData[i].position = Vector3(vData[i].position).Rotate(rotation) + position;
			vData[i].color = color;
		}
	}

	void DebugContext::DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color, const Float4x4 & transform)
	{
		DrawCapsule(rotation, position, radius, halfHeight, color, transform, true);
	}

	void DebugContext::DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color, const Float4x4 & transform, bool depthEnabled)
	{
		const uint32_t numSlices = Config::Get<uint32_t>(L"graphics.debug.capsuleSlices:u32");

		uint32_t numVertices = BasicGeometry::GetCapsuleWireframeSize(numSlices);

		PC_Vertex * vData = GetBufferForVertices(numVertices, depthEnabled);

		BasicGeometry::CreateCapsuleWireframe(vData, sizeof(PC_Vertex), numSlices, radius, halfHeight, 0);

		for(uint32_t i = 0; i < numVertices; ++i) {
			vData[i].position = HTransform((Vector3(vData[i].position).Rotate(rotation) + position).XYZ1(), transform);
			vData[i].color = color;
		}
	}

}
