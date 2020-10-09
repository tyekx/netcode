#include "GraphicsEngine.h"

using ParamId = Netcode::MaterialParamId;

using Netcode::Graphics::ResourceDesc;
using Netcode::ResourceViews;
using Netcode::Graphics::ResourceType;
using Netcode::Graphics::ResourceDimension;
using Netcode::Graphics::ResourceState;
using Netcode::Graphics::ResourceFlags;
using Netcode::Graphics::PrimitiveTopology;

struct BrdfMaterialData {
	Netcode::Float3 diffuseColor;
	float reflectance;
	Netcode::Float3 specularColor;
	float roughness;
	Netcode::Float2 tiles;
	Netcode::Float2 tilesOffset;
	float displacementScale;
	float displacementBias;
	float textureSpaceChirality;
	uint32_t textureFlags;

	BrdfMaterialData() = default;

	BrdfMaterialData(Ptr<Netcode::Material> mat) : BrdfMaterialData{} {
		Assign(mat);
	}

	void Assign(Ptr<Netcode::Material> mat) {
		diffuseColor = mat->GetRequiredParameter<Netcode::Float3>(ParamId::DIFFUSE_ALBEDO);
		reflectance = mat->GetOptionalParameter<float>(ParamId::REFLECTANCE, 0.2f);
		specularColor = mat->GetRequiredParameter<Netcode::Float3>(ParamId::SPECULAR_ALBEDO);
		roughness = mat->GetOptionalParameter<float>(ParamId::ROUGHNESS, 0.2f);
		tiles = mat->GetOptionalParameter<Netcode::Float2>(ParamId::TEXTURE_TILES, Netcode::Float2::One);
		tilesOffset = mat->GetOptionalParameter<Netcode::Float2>(ParamId::TEXTURE_TILES_OFFSET, Netcode::Float2::Zero);
		displacementScale = mat->GetOptionalParameter<float>(ParamId::DISPLACEMENT_SCALE, 0.01f);
		displacementBias = mat->GetOptionalParameter<float>(ParamId::DISPLACEMENT_BIAS, 0.5f);
		textureSpaceChirality = 1.0f;
		textureFlags = mat->GetOptionalParameter<uint32_t>(ParamId::TEXTURE_FLAGS, 0);

		bool metallicBit = mat->GetOptionalParameter<bool>(ParamId::METAL_MASK, false);

		if(metallicBit) {
			textureFlags |= (1 << 31);
		}
	}
};


void GraphicsEngine::SetPerFrameCb(IRenderContext * context, int slot) {
	if(perFrameCbuffer == 0) {
		perFrameCbuffer = context->SetConstants(slot, *perFrameData);
	} else {
		context->SetConstants(slot, perFrameCbuffer);
	}
}

void GraphicsEngine::SetMaterialData(Ptr<Netcode::Material> mat, int32_t slot, IRenderContext * ctx) {
	switch(mat->GetType()) {
		case Netcode::MaterialType::BRDF:
			ctx->SetConstants(slot, BrdfMaterialData{ mat });
			break;
		default: break;
	}
}

void GraphicsEngine::CreateFSQuad() {
	struct PT_Vert {
		Netcode::Float3 position;
		Netcode::Float2 texCoord;
	};

	PT_Vert vData[6] = {
		{ { 1.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
		{ { -1.0f, -1.0f, 0.0f },{ 0.0f, 1.0f } },
		{ { -1.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } },
		{ { 1.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
		{ { 1.0f, -1.0f, 0.0f },{ 1.0f, 1.0f } },
		{ { -1.0f, -1.0f, 0.0f },{ 0.0f, 1.0f } }
	};

	fsQuad.vertexBuffer = graphics->resources->CreateVertexBuffer(sizeof(vData), sizeof(PT_Vert), ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
	fsQuad.vertexCount = 6;
	fsQuad.indexBuffer = 0;
	fsQuad.indexCount = 0;

	auto uploadBatch = graphics->resources->CreateUploadBatch();
	uploadBatch->Upload(fsQuad.vertexBuffer, vData, sizeof(vData));
	uploadBatch->Barrier(fsQuad.vertexBuffer, ResourceState::COPY_DEST, ResourceState::VERTEX_AND_CONSTANT_BUFFER);
	graphics->frame->SyncUpload(uploadBatch);
}

void GraphicsEngine::CreateSkinnedGbufferPassPermanentResources() {
	/*auto ilBuilder = graphics->CreateInputLayoutBuilder();
	ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	ilBuilder->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	ilBuilder->AddInputElement("WEIGHTS", DXGI_FORMAT_R32G32B32_FLOAT);
	ilBuilder->AddInputElement("BONEIDS", DXGI_FORMAT_R32_UINT);
	Ref<Netcode::InputLayout> inputLayout = ilBuilder->Build();

	auto shaderBuilder = graphics->CreateShaderBuilder();
	Ref<Netcode::ShaderBytecode> vs = shaderBuilder->LoadBytecode(L"skinningPass_Vertex.cso");
	Ref<Netcode::ShaderBytecode> ps = shaderBuilder->LoadBytecode(L"gbufferPass_Pixel.cso");

	auto rsBuilder = graphics->CreateRootSignatureBuilder();
	skinnedGbufferPass_RootSignature = rsBuilder->BuildFromShader(vs);

	Netcode::DepthStencilDesc depthStencilDesc;
	depthStencilDesc.backFace.stencilDepthFailOp = Netcode::StencilOp::KEEP;
	depthStencilDesc.backFace.stencilFailOp = Netcode::StencilOp::KEEP;
	depthStencilDesc.backFace.stencilPassOp = Netcode::StencilOp::KEEP;
	depthStencilDesc.backFace.stencilFunc = Netcode::ComparisonFunc::NEVER;

	depthStencilDesc.frontFace.stencilDepthFailOp = Netcode::StencilOp::KEEP;
	depthStencilDesc.frontFace.stencilFailOp = Netcode::StencilOp::KEEP;
	depthStencilDesc.frontFace.stencilPassOp = Netcode::StencilOp::REPLACE;
	depthStencilDesc.frontFace.stencilFunc = Netcode::ComparisonFunc::ALWAYS;

	depthStencilDesc.depthEnable = true;
	depthStencilDesc.stencilEnable = true;
	depthStencilDesc.stencilWriteMask = 0xFF;
	depthStencilDesc.stencilReadMask = 0xFF;

	auto psoBuilder = graphics->CreateGPipelineStateBuilder();
	psoBuilder->SetRootSignature(skinnedGbufferPass_RootSignature);
	psoBuilder->SetInputLayout(inputLayout);
	psoBuilder->SetVertexShader(vs);
	psoBuilder->SetPixelShader(ps);
	psoBuilder->SetDepthStencilState(depthStencilDesc);
	psoBuilder->SetDepthStencilFormat(gbufferPass_DepthStencilFormat);
	psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32G32B32A32_FLOAT });
	psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
	skinnedGbufferPass_PipelineState = psoBuilder->Build();*/
}

void GraphicsEngine::CreateSkinningPassPermanentResources() {
	auto shaderBuilder = graphics->CreateShaderBuilder();
	Ref<Netcode::ShaderBytecode> interpolateCS = shaderBuilder->LoadBytecode(L"skinningPassInterpolate_Compute.cso");
	Ref<Netcode::ShaderBytecode> blendCS = shaderBuilder->LoadBytecode(L"skinningPassBlend_Compute.cso");

	auto rootSigBuilder = graphics->CreateRootSignatureBuilder();
	skinningPass_RootSignature = rootSigBuilder->BuildFromShader(interpolateCS);

	auto cpsoBuilder = graphics->CreateCPipelineStateBuilder();
	cpsoBuilder->SetRootSignature(skinningPass_RootSignature);
	cpsoBuilder->SetComputeShader(interpolateCS);
	skinningInterpolatePass_PipelineState = cpsoBuilder->Build();

	cpsoBuilder->SetRootSignature(skinningPass_RootSignature);
	cpsoBuilder->SetComputeShader(blendCS);
	skinningBlendPass_PipelineState = cpsoBuilder->Build();
}

void GraphicsEngine::CreateGbufferPassPermanentResources() {
	gbufferPass_DepthStencilFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

	auto shaderBuilder = graphics->CreateShaderBuilder();
	Ref<Netcode::ShaderBytecode> vs = shaderBuilder->LoadBytecode(L"brdfMaterial_Vertex.cso");
	Ref<Netcode::ShaderBytecode> ps = shaderBuilder->LoadBytecode(L"brdfMaterial_Pixel.cso");

	auto rootSigBuilder = graphics->CreateRootSignatureBuilder();
	gbufferPass_RootSignature = rootSigBuilder->BuildFromShader(vs);

	auto ilBuilder = graphics->CreateInputLayoutBuilder();
	ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	ilBuilder->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	ilBuilder->AddInputElement("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT);
	Ref<Netcode::InputLayout> inputLayout = ilBuilder->Build();

	Netcode::DepthStencilDesc depthStencilDesc;
	depthStencilDesc.backFace.stencilDepthFailOp = Netcode::StencilOp::KEEP;
	depthStencilDesc.backFace.stencilFailOp = Netcode::StencilOp::KEEP;
	depthStencilDesc.backFace.stencilPassOp = Netcode::StencilOp::KEEP;
	depthStencilDesc.backFace.stencilFunc = Netcode::ComparisonFunc::NEVER;

	depthStencilDesc.frontFace.stencilDepthFailOp = Netcode::StencilOp::KEEP;
	depthStencilDesc.frontFace.stencilFailOp = Netcode::StencilOp::KEEP;
	depthStencilDesc.frontFace.stencilPassOp = Netcode::StencilOp::REPLACE;
	depthStencilDesc.frontFace.stencilFunc = Netcode::ComparisonFunc::ALWAYS;

	depthStencilDesc.depthEnable = true;
	depthStencilDesc.stencilEnable = true;
	depthStencilDesc.stencilWriteMask = 0xFF;
	depthStencilDesc.stencilReadMask = 0xFF;

	auto psoBuilder = graphics->CreateGPipelineStateBuilder();
	psoBuilder->SetRootSignature(gbufferPass_RootSignature);
	psoBuilder->SetInputLayout(inputLayout);
	psoBuilder->SetVertexShader(vs);
	psoBuilder->SetPixelShader(ps);
	psoBuilder->SetDepthStencilState(depthStencilDesc);
	psoBuilder->SetDepthStencilFormat(gbufferPass_DepthStencilFormat);
	psoBuilder->SetRenderTargetFormats({ graphics->GetBackbufferFormat() });
	psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
	gbufferPass_PipelineState = psoBuilder->Build();

	gbufferPass_DepthStencilView = graphics->resources->CreateDepthStencilView();
	CreateGbufferPassSizeDependentResources();

	/*gbufferPass_DepthStencilFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

	auto shaderBuilder = graphics->CreateShaderBuilder();
	Ref<Netcode::ShaderBytecode> vs = shaderBuilder->LoadBytecode(L"gbufferPass_Vertex.cso");
	Ref<Netcode::ShaderBytecode> ps = shaderBuilder->LoadBytecode(L"gbufferPass_Pixel.cso");

	auto rootSigBuilder = graphics->CreateRootSignatureBuilder();
	gbufferPass_RootSignature = rootSigBuilder->BuildFromShader(vs);

	auto ilBuilder = graphics->CreateInputLayoutBuilder();
	ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	ilBuilder->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	Ref<Netcode::InputLayout> inputLayout = ilBuilder->Build();

	Netcode::DepthStencilDesc depthStencilDesc;
	depthStencilDesc.backFace.stencilDepthFailOp = Netcode::StencilOp::KEEP;
	depthStencilDesc.backFace.stencilFailOp = Netcode::StencilOp::KEEP;
	depthStencilDesc.backFace.stencilPassOp = Netcode::StencilOp::KEEP;
	depthStencilDesc.backFace.stencilFunc = Netcode::ComparisonFunc::NEVER;

	depthStencilDesc.frontFace.stencilDepthFailOp = Netcode::StencilOp::KEEP;
	depthStencilDesc.frontFace.stencilFailOp = Netcode::StencilOp::KEEP;
	depthStencilDesc.frontFace.stencilPassOp = Netcode::StencilOp::REPLACE;
	depthStencilDesc.frontFace.stencilFunc = Netcode::ComparisonFunc::ALWAYS;

	depthStencilDesc.depthEnable = true;
	depthStencilDesc.stencilEnable = true;
	depthStencilDesc.stencilWriteMask = 0xFF;
	depthStencilDesc.stencilReadMask = 0xFF;

	auto psoBuilder = graphics->CreateGPipelineStateBuilder();
	psoBuilder->SetRootSignature(gbufferPass_RootSignature);
	psoBuilder->SetInputLayout(inputLayout);
	psoBuilder->SetVertexShader(vs);
	psoBuilder->SetPixelShader(ps);
	psoBuilder->SetDepthStencilState(depthStencilDesc);
	psoBuilder->SetDepthStencilFormat(gbufferPass_DepthStencilFormat);
	psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32G32B32A32_FLOAT });
	psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
	gbufferPass_PipelineState = psoBuilder->Build();

	gbufferPass_RenderTargetViews = graphics->resources->CreateRenderTargetViews(2);
	gbufferPass_DepthStencilView = graphics->resources->CreateDepthStencilView();
*/
}

void GraphicsEngine::CreateGbufferPassSizeDependentResources() {
	gbufferPass_DepthBuffer.reset();
	gbufferPass_DepthBuffer = graphics->resources->CreateDepthStencil(gbufferPass_DepthStencilFormat, ResourceType::PERMANENT_DEFAULT, ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::DEPTH_READ);
	graphics->resources->SetDebugName(gbufferPass_DepthBuffer, L"GBuffer:Depth");
	gbufferPass_DepthStencilView->CreateDSV(gbufferPass_DepthBuffer.get());

	/*gbufferPass_ColorRenderTarget.reset();
	gbufferPass_NormalsRenderTarget.reset();
	gbufferPass_DepthBuffer.reset();

	gbufferPass_ColorRenderTarget = graphics->resources->CreateRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, ResourceType::PERMANENT_DEFAULT, ResourceState::PIXEL_SHADER_RESOURCE);
	gbufferPass_NormalsRenderTarget = graphics->resources->CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT, ResourceType::PERMANENT_DEFAULT, ResourceState::PIXEL_SHADER_RESOURCE);
	gbufferPass_DepthBuffer = graphics->resources->CreateDepthStencil(gbufferPass_DepthStencilFormat, ResourceType::PERMANENT_DEFAULT, ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::DEPTH_READ);

	graphics->resources->SetDebugName(gbufferPass_ColorRenderTarget, L"GBuffer:Color");
	graphics->resources->SetDebugName(gbufferPass_NormalsRenderTarget, L"GBuffer:Normals");
	graphics->resources->SetDebugName(gbufferPass_DepthBuffer, L"GBuffer:Depth");

	gbufferPass_RenderTargetViews->CreateRTV(0, gbufferPass_ColorRenderTarget.get());
	gbufferPass_RenderTargetViews->CreateRTV(1, gbufferPass_NormalsRenderTarget.get());
	gbufferPass_DepthStencilView->CreateDSV(gbufferPass_DepthBuffer.get());*/
}

void GraphicsEngine::CreateSSAOBlurPassPermanentResources() {
	auto shaderBuilder = graphics->CreateShaderBuilder();
	Ref<Netcode::ShaderBytecode> vs = shaderBuilder->LoadBytecode(L"ssaoPass_Vertex.cso");
	Ref<Netcode::ShaderBytecode> ps = shaderBuilder->LoadBytecode(L"ssaoBlurPass_Pixel.cso");

	auto rootSigBuilder = graphics->CreateRootSignatureBuilder();
	ssaoBlurPass_RootSignature = rootSigBuilder->BuildFromShader(ps);

	auto ilBuilder = graphics->CreateInputLayoutBuilder();
	ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

	Netcode::DepthStencilDesc depthDesc;
	depthDesc.depthEnable = false;
	depthDesc.stencilEnable = false;

	Ref<Netcode::InputLayout> inputLayout = ilBuilder->Build();
	auto psoBuilder = graphics->CreateGPipelineStateBuilder();
	psoBuilder->SetRootSignature(ssaoBlurPass_RootSignature);
	psoBuilder->SetInputLayout(inputLayout);
	psoBuilder->SetVertexShader(vs);
	psoBuilder->SetPixelShader(ps);
	psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R32_FLOAT });
	psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
	psoBuilder->SetDepthStencilState(depthDesc);
	ssaoBlurPass_PipelineState = psoBuilder->Build();
}

void GraphicsEngine::CreateSSAOOcclusionPassPermanentResources() {
	auto shaderBuilder = graphics->CreateShaderBuilder();
	Ref<Netcode::ShaderBytecode> vs = shaderBuilder->LoadBytecode(L"ssaoPass_Vertex.cso");
	Ref<Netcode::ShaderBytecode> ps = shaderBuilder->LoadBytecode(L"ssaoOcclusionPass_Pixel.cso");

	auto rootSigBuilder = graphics->CreateRootSignatureBuilder();
	ssaoOcclusionPass_RootSignature = rootSigBuilder->BuildFromShader(ps);

	auto ilBuilder = graphics->CreateInputLayoutBuilder();
	ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

	Netcode::DepthStencilDesc depthDesc;
	depthDesc.depthEnable = false;
	depthDesc.stencilEnable = false;

	Ref<Netcode::InputLayout> inputLayout = ilBuilder->Build();
	auto psoBuilder = graphics->CreateGPipelineStateBuilder();
	psoBuilder->SetRootSignature(ssaoOcclusionPass_RootSignature);
	psoBuilder->SetInputLayout(inputLayout);
	psoBuilder->SetVertexShader(vs);
	psoBuilder->SetPixelShader(ps);
	psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R32_FLOAT });
	psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
	psoBuilder->SetDepthStencilState(depthDesc);
	ssaoOcclusionPass_PipelineState = psoBuilder->Build();

	ssaoPass_RandomVectorTexture = graphics->resources->CreateTexture2D(256, 256, 1, DXGI_FORMAT_R8G8B8A8_UNORM, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST, ResourceFlags::NONE);

	std::unique_ptr<DirectX::PackedVector::XMCOLOR[]> colors = std::make_unique<DirectX::PackedVector::XMCOLOR[]>(256 * 256);
	for(int i = 0; i < 256; ++i)
	{
		for(int j = 0; j < 256; ++j)
		{
			Netcode::Float3 v(Netcode::RandomFloat(), Netcode::RandomFloat(), Netcode::RandomFloat());

			colors[i * 256 + j] = DirectX::PackedVector::XMCOLOR(v.z, v.y, v.x, 0.0f);
		}
	}

	auto batch = graphics->resources->CreateUploadBatch();
	batch->Upload(ssaoPass_RandomVectorTexture, colors.get(), 256 * 256 * sizeof(DirectX::PackedVector::XMCOLOR));
	batch->Barrier(ssaoPass_RandomVectorTexture, ResourceState::COPY_DEST, ResourceState::PIXEL_SHADER_RESOURCE);
	graphics->frame->SyncUpload(std::move(batch));

	CreateSSAOOcclusionPassSizeDependentResources();
}

void GraphicsEngine::CreateSSAOOcclusionPassSizeDependentResources() {
	ssaoRenderTargetSize = Netcode::UInt2{ backbufferSize.x / 2, backbufferSize.y / 2 };

	ssaoRenderTargetSize.x = (ssaoRenderTargetSize.x == 0) ? 1 : ssaoRenderTargetSize.x;
	ssaoRenderTargetSize.y = (ssaoRenderTargetSize.y == 0) ? 1 : ssaoRenderTargetSize.y;

	ssaoPass_OcclusionRenderTarget.reset();
	ssaoPass_BlurRenderTarget.reset();

	ssaoPass_OcclusionRenderTarget = graphics->resources->CreateRenderTarget(ssaoRenderTargetSize.x, ssaoRenderTargetSize.y, DXGI_FORMAT_R32_FLOAT, ResourceType::PERMANENT_DEFAULT, ResourceState::PIXEL_SHADER_RESOURCE);
	ssaoPass_BlurRenderTarget = graphics->resources->CreateRenderTarget(ssaoRenderTargetSize.x, ssaoRenderTargetSize.y, DXGI_FORMAT_R32_FLOAT, ResourceType::PERMANENT_DEFAULT, ResourceState::PIXEL_SHADER_RESOURCE);

	lightingPass_ShaderResourceViews->CreateSRV(3, ssaoPass_BlurRenderTarget.get());
}

/*
void GraphicsEngine::CreateLightingPassPermanentResources() {
	auto shaderBuilder = graphics->CreateShaderBuilder();
	Ref<Netcode::ShaderBytecode> vs = shaderBuilder->LoadBytecode(L"lightingPass_Vertex.cso");
	Ref<Netcode::ShaderBytecode> ps = shaderBuilder->LoadBytecode(L"lightingPass_Pixel.cso");

	auto rootSigBuilder = graphics->CreateRootSignatureBuilder();
	lightingPass_RootSignature = rootSigBuilder->BuildFromShader(vs);

	auto ilBuilder = graphics->CreateInputLayoutBuilder();
	ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

	Netcode::DepthStencilDesc depthDesc;
	depthDesc.depthEnable = false;
	depthDesc.stencilEnable = true;
	depthDesc.frontFace.stencilDepthFailOp = Netcode::StencilOp::KEEP;
	depthDesc.frontFace.stencilFailOp = Netcode::StencilOp::KEEP;
	depthDesc.frontFace.stencilPassOp = Netcode::StencilOp::KEEP;
	depthDesc.frontFace.stencilFunc = Netcode::ComparisonFunc::EQUAL;
	depthDesc.stencilReadMask = 0xFF;
	depthDesc.stencilWriteMask = 0x00;
	depthDesc.depthWriteMaskZero = true;
	depthDesc.backFace = depthDesc.frontFace;
	depthDesc.backFace.stencilFunc = Netcode::ComparisonFunc::NEVER;

	Ref<Netcode::InputLayout> inputLayout = ilBuilder->Build();
	auto psoBuilder = graphics->CreateGPipelineStateBuilder();
	psoBuilder->SetRootSignature(lightingPass_RootSignature);
	psoBuilder->SetInputLayout(inputLayout);
	psoBuilder->SetVertexShader(vs);
	psoBuilder->SetPixelShader(ps);
	psoBuilder->SetDepthStencilFormat(gbufferPass_DepthStencilFormat);
	psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM });
	psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
	psoBuilder->SetDepthStencilState(depthDesc);
	lightingPass_PipelineState = psoBuilder->Build();

	lightingPass_ShaderResourceViews = graphics->resources->CreateShaderResourceViews(4);
	CreateLightingPassResourceViews();
}

void GraphicsEngine::CreateLightingPassResourceViews() {
	lightingPass_ShaderResourceViews->CreateSRV(0, gbufferPass_ColorRenderTarget.get());
	lightingPass_ShaderResourceViews->CreateSRV(1, gbufferPass_NormalsRenderTarget.get());
	lightingPass_ShaderResourceViews->CreateSRV(2, gbufferPass_DepthBuffer.get());
}*/

void GraphicsEngine::CreateBackgroundPassPermanentResources() {

	auto shaderBuilder = graphics->CreateShaderBuilder();
	Ref<Netcode::ShaderBytecode> vs = shaderBuilder->LoadBytecode(L"envmapPass_Vertex.cso");
	Ref<Netcode::ShaderBytecode> ps = shaderBuilder->LoadBytecode(L"envmapPass_Pixel.cso");

	auto rootSigBuilder = graphics->CreateRootSignatureBuilder();
	envmapPass_RootSignature = rootSigBuilder->BuildFromShader(vs);

	auto ilBuilder = graphics->CreateInputLayoutBuilder();
	ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	Ref<Netcode::InputLayout> inputLayout = ilBuilder->Build();


	Netcode::DepthStencilDesc depthDesc;
	depthDesc.depthEnable = false;
	depthDesc.stencilEnable = true;
	depthDesc.frontFace.stencilDepthFailOp = Netcode::StencilOp::KEEP;
	depthDesc.frontFace.stencilFailOp = Netcode::StencilOp::KEEP;
	depthDesc.frontFace.stencilPassOp = Netcode::StencilOp::KEEP;
	depthDesc.frontFace.stencilFunc = Netcode::ComparisonFunc::EQUAL;
	depthDesc.stencilReadMask = 0xFF;
	depthDesc.stencilWriteMask = 0x00;
	depthDesc.depthWriteMaskZero = true;
	depthDesc.backFace = depthDesc.frontFace;
	depthDesc.backFace.stencilFunc = Netcode::ComparisonFunc::NEVER;

	auto psoBuilder = graphics->CreateGPipelineStateBuilder();
	psoBuilder->SetInputLayout(inputLayout);
	psoBuilder->SetVertexShader(vs);
	psoBuilder->SetPixelShader(ps);
	psoBuilder->SetDepthStencilFormat(gbufferPass_DepthStencilFormat);
	psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM });
	psoBuilder->SetRootSignature(envmapPass_RootSignature);
	psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
	psoBuilder->SetDepthStencilState(depthDesc);

	envmapPass_PipelineState = psoBuilder->Build();
}

void GraphicsEngine::CreateUIPassPermanentResources() {
	auto ilBuilder = graphics->CreateInputLayoutBuilder();
	ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	ilBuilder->AddInputElement("COLOR", DXGI_FORMAT_R32G32B32A32_FLOAT);
	ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	Ref<Netcode::InputLayout> inputLayout = ilBuilder->Build();

	auto shaderBuilder = graphics->CreateShaderBuilder();
	Ref<Netcode::ShaderBytecode> vs = shaderBuilder->LoadBytecode(L"Netcode_SpriteVS.cso");
	Ref<Netcode::ShaderBytecode> ps = shaderBuilder->LoadBytecode(L"Netcode_SpritePS.cso");

	auto rootSigBuilder = graphics->CreateRootSignatureBuilder();
	auto rootSignature = rootSigBuilder->BuildFromShader(vs);

	Netcode::BlendDesc blendState;
	Netcode::RenderTargetBlendDesc rt0Blend;
	rt0Blend.blendEnable = true;
	rt0Blend.logicOpEnable = false;
	rt0Blend.srcBlend = Netcode::BlendMode::SRC_ALPHA;
	rt0Blend.destBlend = Netcode::BlendMode::INV_SRC_ALPHA;
	rt0Blend.blendOp = Netcode::BlendOp::ADD;
	rt0Blend.srcBlendAlpha = Netcode::BlendMode::ONE;
	rt0Blend.destBlendAlpha = Netcode::BlendMode::INV_SRC_ALPHA;
	rt0Blend.blendOpAlpha = Netcode::BlendOp::ADD;
	rt0Blend.logicOp = Netcode::LogicOp::NOOP;
	rt0Blend.renderTargetWriteMask = 0x0F;

	blendState.alphaToCoverageEnabled = false;
	blendState.independentAlphaEnabled = false;
	blendState.rtBlend[0] = rt0Blend;

	Netcode::RasterizerDesc rasterizerState;
	rasterizerState.fillMode = Netcode::FillMode::SOLID;
	rasterizerState.cullMode = Netcode::CullMode::NONE;
	rasterizerState.frontCounterClockwise = false;
	rasterizerState.depthBias = 0;
	rasterizerState.depthBiasClamp = 0.0f;
	rasterizerState.slopeScaledDepthBias = 0.0f;
	rasterizerState.depthClipEnable = true;
	rasterizerState.multisampleEnable = true;
	rasterizerState.antialiasedLineEnable = false;
	rasterizerState.forcedSampleCount = 0;
	rasterizerState.conservativeRaster = false;

	Netcode::DepthStencilDesc depthStencilDesc;
	depthStencilDesc.depthEnable = false;
	depthStencilDesc.stencilEnable = false;
	depthStencilDesc.depthWriteMaskZero = true;

	auto psoBuilder = graphics->CreateGPipelineStateBuilder();
	psoBuilder->SetInputLayout(inputLayout);
	psoBuilder->SetRootSignature(rootSignature);
	psoBuilder->SetVertexShader(vs);
	psoBuilder->SetPixelShader(ps);
	psoBuilder->SetDepthStencilFormat(DXGI_FORMAT_D32_FLOAT);
	psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM });
	psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
	psoBuilder->SetBlendState(blendState);
	psoBuilder->SetRasterizerState(rasterizerState);
	psoBuilder->SetDepthStencilState(depthStencilDesc);

	auto pipelineState = psoBuilder->Build();

	Ref<Netcode::SpriteBatchBuilder> spriteBatchBuilder = graphics->CreateSpriteBatchBuilder();
	spriteBatchBuilder->SetPipelineState(std::move(pipelineState));
	spriteBatchBuilder->SetRootSignature(std::move(rootSignature));
	uiPass_SpriteBatch = spriteBatchBuilder->Build();
}

void GraphicsEngine::CreateSkinningPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder) {
	frameGraphBuilder->CreateRenderPass("Skinning",
		[&](IResourceContext * context) -> void {

		context->UseComputeContext();

		for(Ref<AnimationSet> & animSet : skinningPass_Input) {
			animSet->UploadConstants(context);
		}

	},
		[&](IRenderContext * context) -> void {

		context->SetRootSignature(skinningPass_RootSignature);

		for(Ref<AnimationSet> & animSet : skinningPass_Input) {
			animSet->BindResources(context);

			context->SetPipelineState(skinningInterpolatePass_PipelineState);
			context->Dispatch(8, animSet->GetNumInstances(), 1);

			context->UnorderedAccessBarrier(animSet->GetIntermediateResource());
			context->FlushResourceBarriers();

			context->SetPipelineState(skinningBlendPass_PipelineState);
			context->Dispatch(1, animSet->GetNumInstances(), 1);

			animSet->CopyResults(context);
		}

	});
}

void GraphicsEngine::CreateSkinnedGbufferPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder) {
	if(skinnedGbufferPass_Input.empty()) {
		return;
	}

	frameGraphBuilder->CreateRenderPass("Skinned Gbuffer",
		[&](IResourceContext * context) -> void {

		context->Writes(gbufferPass_DepthBuffer.get());
		context->Writes(nullptr);

	},
		[&](IRenderContext * context) -> void {
		context->SetRootSignature(skinnedGbufferPass_RootSignature);
		context->SetPipelineState(skinnedGbufferPass_PipelineState);

		context->SetRenderTargets(nullptr, gbufferPass_DepthStencilView);
		context->SetScissorRect();
		context->SetViewport();
		context->ClearRenderTarget(0);
		context->ClearDepthStencil();
		context->SetStencilReference(0xFF);

		context->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);

		for(RenderItem & item : skinnedGbufferPass_Input) {
			if(item.material->GetType() == Netcode::MaterialType::BRDF) {
				SetMaterialData(item.material.get(), 0, context);
				context->SetConstants(1, *item.objectData);
				SetPerFrameCb(context, 2);
				context->SetConstants(3, *item.debugBoneData);
				//context->SetShaderResources(3, item.boneData, item.boneDataOffset);
				context->SetVertexBuffer(item.gbuffer.vertexBuffer);
				context->SetIndexBuffer(item.gbuffer.indexBuffer);
				context->DrawIndexed(item.gbuffer.indexCount);
			}
		}
	}
	);
}

void GraphicsEngine::CreateGbufferPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder) {
	frameGraphBuilder->CreateRenderPass("Gbuffer", [&](IResourceContext * context) -> void {

		context->Writes(nullptr);
		context->Writes(gbufferPass_DepthBuffer.get());

	},
		[&](IRenderContext * context) -> void {

		bool isBound = false;
		void * objectData = nullptr;

		context->ResourceBarrier(gbufferPass_DepthBuffer, ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::DEPTH_READ, ResourceState::DEPTH_WRITE);
		context->FlushResourceBarriers();

		context->SetRootSignature(gbufferPass_RootSignature);
		context->SetPipelineState(gbufferPass_PipelineState);
		context->SetRenderTargets(nullptr, gbufferPass_DepthStencilView);
		context->ClearRenderTarget(0);
		context->ClearDepthStencil();
		context->SetScissorRect();
		context->SetViewport();
		context->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
		context->SetStencilReference(0xFF);
		context->SetShaderResources(4, lightingData_StructuredBuffer);
		context->SetShaderResources(5, prefilteredSplitSumViews);

		for(RenderItem & item : gbufferPass_Input) {
			if(item.material->GetType() != Netcode::MaterialType::BRDF) {
				continue;
			}

			SetMaterialData(item.material.get(), 0, context);

			if(objectData != item.objectData) {
				item.objectData->lightsCount = std::min(sceneLights->size(), 1024ull);
				item.objectData->lightsOffset = 0;
				context->SetConstants(1, *item.objectData);
				objectData = item.objectData;
			}

			if(!isBound) {
				SetPerFrameCb(context, 2);
				isBound = true;
			}

			auto srv = item.material->GetResourceView(0);

			if(srv != nullptr) {
				context->SetShaderResources(3, srv);
			}

			context->SetVertexBuffer(item.gbuffer.vertexBuffer);
			context->SetIndexBuffer(item.gbuffer.indexBuffer);
			context->DrawIndexed(item.gbuffer.indexCount);
		}

		context->ResourceBarrier(gbufferPass_DepthBuffer, ResourceState::DEPTH_WRITE, ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::DEPTH_READ);
		context->FlushResourceBarriers();
	});
}
/*
void GraphicsEngine::CreateSSAOOcclusionPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder) {
	frameGraphBuilder->CreateRenderPass("SSAO Occlusion", [&](IResourceContext * context) -> void {

		context->Reads(gbufferPass_NormalsRenderTarget.get());
		context->Reads(gbufferPass_DepthBuffer.get());

		context->Writes(ssaoPass_OcclusionRenderTarget.get());

	},
		[&](IRenderContext * context) -> void {
		context->SetRootSignature(ssaoOcclusionPass_RootSignature);
		context->SetPipelineState(ssaoOcclusionPass_PipelineState);

		context->SetViewport(ssaoRenderTargetSize.x, ssaoRenderTargetSize.y);
		context->SetScissorRect(0, ssaoRenderTargetSize.x, 0, ssaoRenderTargetSize.y);
		context->ResourceBarrier(ssaoPass_OcclusionRenderTarget, ResourceState::PIXEL_SHADER_RESOURCE, ResourceState::RENDER_TARGET);
		context->FlushResourceBarriers();
		context->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
		context->SetRenderTargets(ssaoPass_OcclusionRenderTarget, 0);
		context->ClearRenderTarget(0);

		SetPerFrameCb(context, 0);
		context->SetConstants(1, *ssaoData);
		context->SetShaderResources(2, { gbufferPass_NormalsRenderTarget, gbufferPass_DepthBuffer, ssaoPass_RandomVectorTexture });

		context->SetVertexBuffer(fsQuad.vertexBuffer);
		context->Draw(fsQuad.vertexCount);

		context->SetRenderTargets(0, 0);
		context->ResourceBarrier(ssaoPass_OcclusionRenderTarget, ResourceState::RENDER_TARGET, ResourceState::PIXEL_SHADER_RESOURCE);
		context->FlushResourceBarriers();
	});
}

void GraphicsEngine::CreateSSAOBlurPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder) {
	frameGraphBuilder->CreateRenderPass("SSAO Blur", [&](IResourceContext * context) -> void {

		context->Reads(ssaoPass_OcclusionRenderTarget.get());
		context->Writes(ssaoPass_BlurRenderTarget.get());

	},
		[&](IRenderContext * context) -> void {

		context->SetRootSignature(ssaoBlurPass_RootSignature);
		context->SetPipelineState(ssaoBlurPass_PipelineState);
		context->SetViewport(ssaoRenderTargetSize.x, ssaoRenderTargetSize.y);
		context->SetScissorRect(0, ssaoRenderTargetSize.x, 0, ssaoRenderTargetSize.y);
		context->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);


		ssaoData->invRenderTargetSize = Netcode::Float2{
			1.0f / static_cast<float>(ssaoRenderTargetSize.x),
			1.0f / static_cast<float>(ssaoRenderTargetSize.y)
		};

		uint32_t isHorizontal = 0;

		Ref<GpuResource> renderTargets[2] = {
			ssaoPass_OcclusionRenderTarget, ssaoPass_BlurRenderTarget
		};

		for(uint32_t i = 0; i < 3; ++i) {
			Ref<GpuResource> currentRenderTarget = renderTargets[(i + 1) % 2];
			Ref<GpuResource> sourceTexture = renderTargets[i % 2];

			context->ResourceBarrier(currentRenderTarget, ResourceState::PIXEL_SHADER_RESOURCE, ResourceState::RENDER_TARGET);
			context->FlushResourceBarriers();

			context->SetRenderTargets(currentRenderTarget, 0);
			context->ClearRenderTarget(0);

			context->SetShaderResources(0, { sourceTexture, gbufferPass_NormalsRenderTarget, gbufferPass_DepthBuffer });
			context->SetConstants(1, *ssaoData);

			SetPerFrameCb(context, 2);
			context->SetRootConstants(3, &isHorizontal, 1);
			context->SetVertexBuffer(fsQuad.vertexBuffer);
			context->Draw(fsQuad.vertexCount);

			context->ResourceBarrier(currentRenderTarget, ResourceState::RENDER_TARGET, ResourceState::PIXEL_SHADER_RESOURCE);
			context->FlushResourceBarriers();

			if(isHorizontal == 0) {
				isHorizontal = 0xFFFFFFFF;
			} else {
				isHorizontal = 0;
			}
		}
	});
}

void GraphicsEngine::CreateLightingPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder) {
	frameGraphBuilder->CreateRenderPass("Lighting", [&](IResourceContext * context) -> void {

		context->Reads(ssaoPass_BlurRenderTarget.get());
		context->Reads(gbufferPass_ColorRenderTarget.get());
		context->Reads(gbufferPass_DepthBuffer.get());
		context->Reads(gbufferPass_NormalsRenderTarget.get());

		context->Writes(2);
		context->Writes(nullptr);

	}, [&](IRenderContext * context) -> void {
		context->SetRootSignature(lightingPass_RootSignature);
		context->SetPipelineState(lightingPass_PipelineState);
		context->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
		context->SetScissorRect();
		context->SetViewport();
		context->SetStencilReference(0xFF);
		context->SetRenderTargets(nullptr, gbufferPass_DepthStencilView);
		SetPerFrameCb(context, 0);
		context->SetShaderResources(1, lightingPass_ShaderResourceViews);

		// @TODO: set light data

		context->SetVertexBuffer(fsQuad.vertexBuffer);
		context->Draw(fsQuad.vertexCount);
	});
}*/

void GraphicsEngine::CreateUIPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder) {
	frameGraphBuilder->CreateRenderPass("uiPass", [&](IResourceContext * context) -> void {

		context->Writes(nullptr);

	},
		[&](IRenderContext * context) -> void {

		Netcode::UInt2 ss = ui_Input->WindowSize();
		Netcode::Matrix vp =
			Netcode::LookToMatrix(Netcode::Float3::Zero, Netcode::Float3{ 0.0f, 0.0f, 1.0f }, Netcode::Float3::UnitY) *
			Netcode::OrtographicMatrix(static_cast<float>(ss.x), static_cast<float>(ss.y), 0.0f, Netcode::UI::Control::MAX_DEPTH);

		Netcode::Matrix tex = Netcode::Float4x4{
			-1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f, 1.0f
		};


		context->SetRenderTargets(nullptr, nullptr);
		uiPass_SpriteBatch->BeginRecord(context, (vp * tex).Transpose());
		ui_Input->Render(uiPass_SpriteBatch.get());
		uiPass_SpriteBatch->EndRecord();
	});
}

void GraphicsEngine::CreatePostProcessPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder) {
	frameGraphBuilder->CreateRenderPass("postProcessPass", [&](IResourceContext * context) -> void {



	},
		[&](IRenderContext * context) -> void {

		//context->SetRenderTargets(0, 0);

	});
}

void GraphicsEngine::CreateBackgroundPass(Ptr<Netcode::FrameGraphBuilder> builder) {
	builder->CreateRenderPass("Background", [this](IResourceContext * ctx) ->void {
		ctx->Reads(gbufferPass_DepthBuffer.get());
		ctx->Writes(nullptr);
	},
		[this](IRenderContext * ctx) -> void {

		ctx->SetRenderTargets(nullptr, gbufferPass_DepthStencilView);
		ctx->SetScissorRect();
		ctx->SetViewport();
		ctx->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
		ctx->SetStencilReference(0);

		ctx->SetRootSignature(envmapPass_RootSignature);
		ctx->SetPipelineState(envmapPass_PipelineState);

		SetPerFrameCb(ctx, 0);
		ctx->SetShaderResources(1, prefilteredSplitSumViews);

		ctx->SetVertexBuffer(fsQuad.vertexBuffer);
		ctx->Draw(fsQuad.vertexCount);
	});
}

void GraphicsEngine::CreateDebugPrimPass(Ptr<Netcode::FrameGraphBuilder> builder) {
	builder->CreateRenderPass("Debug", [this](IResourceContext * ctx) ->void {
		ctx->Writes(nullptr);
		graphics->debug->UploadResources(ctx);
	},
		[this](IRenderContext * ctx) -> void {
		ctx->SetRenderTargets(nullptr, gbufferPass_DepthStencilView);
		graphics->debug->Draw(ctx, perFrameData->ViewProj);
	});
}

void GraphicsEngine::Reset() {
	skinningPass_Input.clear();
	skinnedGbufferPass_Input.clear();
	gbufferPass_Input.clear();
	perFrameCbuffer = 0;
	sceneLights->clear();
}

void GraphicsEngine::OnResize(int x, int y) {
	Netcode::UInt2 newSize{ static_cast<uint32_t>(x), static_cast<uint32_t>(y) };

	if(newSize.x != backbufferSize.x || newSize.y != backbufferSize.y) {
		backbufferSize = newSize;
		if(backbufferSize.x != 0 && backbufferSize.y != 0) {
			CreateGbufferPassSizeDependentResources();
			//CreateLightingPassResourceViews();
			//CreateSSAOOcclusionPassSizeDependentResources();
		}
	}
}

void GraphicsEngine::CreatePermanentResources(Netcode::Module::IGraphicsModule * g) {
	graphics = g;
	backbufferSize = graphics->GetBackbufferSize();

	lightingData_StructuredBuffer = g->resources->CreateStructuredBuffer(1024 * sizeof(Netcode::Light), sizeof(Netcode::Light), ResourceType::PERMANENT_UPLOAD, ResourceState::ANY_READ, ResourceFlags::NONE);

	Netcode::Light dirL = Netcode::DirectionalLight{ Netcode::Float3::One, Netcode::Float3{ 0.0f, 0.2f, 1.0f } };
	g->resources->CopyConstants(lightingData_StructuredBuffer, &dirL, sizeof(dirL));

	CreateGbufferPassPermanentResources();
	CreateSkinningPassPermanentResources();
	CreateSkinnedGbufferPassPermanentResources();
	//CreateLightingPassPermanentResources();
	CreateBackgroundPassPermanentResources();
	//CreateSSAOBlurPassPermanentResources();
	//CreateSSAOOcclusionPassPermanentResources();
	CreateUIPassPermanentResources();
	CreateFSQuad();
}

void GraphicsEngine::CreateFrameGraph(Ptr<Netcode::FrameGraphBuilder> builder) {
	if(!sceneLights->empty()) {
		graphics->resources->CopyConstants(lightingData_StructuredBuffer, sceneLights->data(),
			std::min(1024ull, sceneLights->size()) * sizeof(Netcode::Light));
	}

	CreateSkinnedGbufferPass(builder);
	CreateGbufferPass(builder);
	//CreateSSAOOcclusionPass(builder);
	//CreateSSAOBlurPass(builder);
	//CreateLightingPass(builder);
	CreateBackgroundPass(builder);
	CreateUIPass(builder);

#if defined(NETCODE_DEBUG)
	CreateDebugPrimPass(builder);
#endif
}

void GraphicsEngine::CreateComputeFrameGraph(Ptr<Netcode::FrameGraphBuilder> builder) {
	CreateSkinningPass(builder);
}

Ref<GpuResource> GraphicsEngine::PreIntegrateBrdf(Ptr<Netcode::FrameGraphBuilder> builder)
{
	if(IBLPreIntegratePass_RootSignature == nullptr) {
		Ref<Netcode::ShaderBuilder> shaderBuilder = graphics->CreateShaderBuilder();
		Ref<Netcode::ShaderBytecode> prefilterShader = shaderBuilder->LoadBytecode(L"IBL_brdfPreIntegrate_Compute.cso");
		Ref<Netcode::CPipelineStateBuilder> cBuilder = graphics->CreateCPipelineStateBuilder();
		Ref<Netcode::RootSignatureBuilder> rootSigBuilder = graphics->CreateRootSignatureBuilder();
		IBLPreIntegratePass_RootSignature = rootSigBuilder->BuildFromShader(prefilterShader);
		cBuilder->SetComputeShader(prefilterShader);
		cBuilder->SetRootSignature(IBLPreIntegratePass_RootSignature);
		IBLPreIntegratePass_PipelineState = cBuilder->Build();
	}

	const uint32_t SIZE = 512;

	Ref<GpuResource> resource = graphics->resources->CreateTexture2D(SIZE, SIZE, 1,
		DXGI_FORMAT_R16G16_FLOAT, ResourceType::PERMANENT_DEFAULT, ResourceState::UNORDERED_ACCESS, ResourceFlags::ALLOW_UNORDERED_ACCESS);

	Ref<ResourceViews> tempViews = graphics->resources->CreateShaderResourceViews(1);
	tempViews->CreateUAV(0, resource.get());



	builder->CreateRenderPass("CubemapPrefilter",
		[resource](IResourceContext * ctx) -> void {
		ctx->UseComputeContext();
		ctx->Writes(resource.get());
	},
		[this, SIZE, tempViews, resource](IRenderContext * ctx) -> void {

		ctx->SetPipelineState(IBLPreIntegratePass_PipelineState);
		ctx->SetRootSignature(IBLPreIntegratePass_RootSignature);
		ctx->SetShaderResources(0, tempViews);
		ctx->Dispatch(SIZE / 8, SIZE / 8, 1);

	});

	builder->CreateRenderPass("CubemapPrefilter:StateChange",
		[resource](IResourceContext * ctx) -> void {
		ctx->Reads(resource.get());
	},
		[resource](IRenderContext * ctx) -> void {
		ctx->ResourceBarrier(resource, ResourceState::UNORDERED_ACCESS, ResourceState::PIXEL_SHADER_RESOURCE);
		ctx->FlushResourceBarriers();
	});

	return resource;
}

Ref<GpuResource> GraphicsEngine::PrefilterEnvMap(Ptr<Netcode::FrameGraphBuilder> builder, Ref<GpuResource> sourceTexture)
{
	if(IBLPreFilterPass_RootSignature == nullptr) {
		Ref<Netcode::ShaderBuilder> shaderBuilder = graphics->CreateShaderBuilder();
		Ref<Netcode::ShaderBytecode> prefilterShader = shaderBuilder->LoadBytecode(L"IBL_brdfEnvmapFilter_Compute.cso");
		Ref<Netcode::CPipelineStateBuilder> cBuilder = graphics->CreateCPipelineStateBuilder();
		Ref<Netcode::RootSignatureBuilder> rootSigBuilder = graphics->CreateRootSignatureBuilder();
		IBLPreFilterPass_RootSignature = rootSigBuilder->BuildFromShader(prefilterShader);
		cBuilder->SetComputeShader(prefilterShader);
		cBuilder->SetRootSignature(IBLPreFilterPass_RootSignature);
		IBLPreFilterPass_PipelineState = cBuilder->Build();
	}

	ResourceDesc resourceDesc = sourceTexture->GetDesc();

	if((resourceDesc.width != resourceDesc.height) ||
		!Netcode::Utility::IsPowerOf2(resourceDesc.width)) {
		Log::Error("PrefilterEnvMap: The faces of the texture cube must be quadratic and a power of 2.");
		return nullptr;
	}

	if(resourceDesc.depth % 6 != 0 || resourceDesc.dimension != ResourceDimension::TEXTURE2D) {
		Log::Error("PrefilterEnvMap: input resource is not a TextureCube");
		return nullptr;
	}

	uint32_t minWidth = std::max(resourceDesc.height, 8u);
	int possibleMipCount = static_cast<int>(Netcode::Utility::HighestActiveBitIndex(resourceDesc.width)) - 3;
	uint16_t mipCount = static_cast<uint16_t>(std::clamp(possibleMipCount, 1, 12));

	Ref<GpuResource> texResource = graphics->resources->CreateTextureCube(minWidth, minWidth, mipCount, DXGI_FORMAT_R8G8B8A8_UNORM,
		ResourceType::PERMANENT_DEFAULT, ResourceState::UNORDERED_ACCESS, ResourceFlags::ALLOW_UNORDERED_ACCESS);

	Ref<ResourceViews> srv = graphics->resources->CreateShaderResourceViews(mipCount + 1);

	for(uint16_t i = 0; i < mipCount; i++) {
		srv->CreateUAV(i, texResource.get(), i);
	}

	srv->CreateSRV(mipCount, sourceTexture.get());

	builder->CreateRenderPass("CubemapPrefilter",
		[texResource](IResourceContext * ctx) -> void {
		ctx->UseComputeContext();
		ctx->Writes(texResource.get());
	},
		[this, srv, minWidth, mipCount](IRenderContext * ctx) -> void {
		ctx->SetPipelineState(IBLPreFilterPass_PipelineState);
		ctx->SetRootSignature(IBLPreFilterPass_RootSignature);

		uint32_t groupSize = minWidth / 8;
		uint32_t numMipLevels = mipCount;
		ctx->SetRootConstants(2, &numMipLevels, 1, 0);
		for(uint32_t i = 0; i < mipCount; i++) {
			ctx->SetShaderResources(0, srv, static_cast<int>(i));
			ctx->SetShaderResources(1, srv, static_cast<int>(mipCount));
			ctx->SetRootConstants(2, &i, 1, 1);
			ctx->Dispatch(groupSize, groupSize, 1);
			groupSize >>= 1;
		}

	});

	builder->CreateRenderPass("CubemapPrefilter:StateChange",
		[texResource](IResourceContext * ctx) -> void {
		ctx->Reads(texResource.get());
	},
		[=](IRenderContext * ctx) -> void {
		ctx->ResourceBarrier(texResource, ResourceState::UNORDERED_ACCESS, ResourceState::PIXEL_SHADER_RESOURCE);
		ctx->FlushResourceBarriers();
	});

	return texResource;
}

void GraphicsEngine::SetGlobalEnvMap(Ref<GpuResource> preEnvMap, Ref<GpuResource> preBrdfIntegral)
{
	if(prefilteredSplitSumViews == nullptr) {
		prefilteredSplitSumViews = graphics->resources->CreateShaderResourceViews(2);
	}

	prefilteredEnvmap = preEnvMap;
	prefilteredSplitSumViews->CreateSRV(0, preEnvMap.get());

	preIntegratedBrdf = preBrdfIntegral;
	prefilteredSplitSumViews->CreateSRV(1, preBrdfIntegral.get());
}

void GraphicsEngine::ReadbackComputeResults() {
	for(auto & animSet : skinningPass_Input) {
		graphics->resources->Readback(animSet->GetResultReadbackBuffer(), animSet->GetData()->BindTransform, animSet->GetNumInstances() * sizeof(BoneData));
		animSet->Clear();
	}
}
