#pragma once

#include <Netcode/HandleTypes.h>
#include "ConstantBufferTypes.h"

struct GBuffer {
	Netcode::GpuResourceRef vertexBuffer;
	Netcode::GpuResourceRef indexBuffer;
	uint64_t vertexCount;
	uint64_t indexCount;
};

struct MaterialBase {
	virtual ~MaterialBase() = default;
	virtual void Apply(Netcode::Graphics::IRenderContext * renderContext) = 0;
};

enum class BRDF_TextureType {
	DIFFUSE_TEXTURE,
	NORMAL_TEXTURE,
	AMBIENT_TEXTURE,
	SPECULAR_TEXTURE,
	ROUGHNESS_TEXTURE
};

struct BRDF_Data {
	Netcode::Float4 diffuseColor;
	Netcode::Float4 specularColor;
	Netcode::Float4 ambientColor;
	int textures;
	float roughness;
};

class BRDF_MaterialBase : public MaterialBase {
protected:
	Netcode::PipelineStateRef pipelineState;
	Netcode::RootSignatureRef rootSignature;
	Netcode::ResourceViewsRef textures;
	Netcode::GpuResourceRef resourceRefs[5];
	BRDF_Data data;

	BRDF_MaterialBase() = default;

	virtual void Initialize(Netcode::Module::IGraphicsModule * graphics) = 0;

public:
	BRDF_MaterialBase(Netcode::Module::IGraphicsModule * graphics) : BRDF_MaterialBase{} {
		textures = graphics->resources->CreateShaderResourceViews(ARRAYSIZE(resourceRefs));
		Initialize(graphics);
	}

	virtual void Apply(Netcode::Graphics::IRenderContext * renderContext) override {
		renderContext->SetRootSignature(rootSignature);
		renderContext->SetPipelineState(pipelineState);
		renderContext->SetConstants(3, data);
		renderContext->SetShaderResources(4, textures);
	}

	void SetTexture(BRDF_TextureType slot, Netcode::GpuResourceRef texture) {
		const uint32_t idx = static_cast<uint32_t>(slot);
		textures->CreateSRV(static_cast<uint32_t>(idx), texture.get());
		data.textures |= (1 << idx);
		std::swap(resourceRefs[idx], texture);
	}

	void RemoveTexture(BRDF_TextureType slot) {
		const uint32_t idx = static_cast<uint32_t>(slot);
		data.textures &= ~(1 << idx);
		resourceRefs[idx].reset();
		textures->RemoveSRV(idx, Netcode::Graphics::ResourceDimension::TEXTURE2D);
	}
};

/**
* BRDF function that expects a Position, Normal and TexCoord.
* NORMAL_TEXTURE is ignored for this material
*/
class BRDF_DefaultMaterial : public BRDF_MaterialBase {
	virtual void Initialize(Netcode::Module::IGraphicsModule * graphics) override {
		auto inputLayoutBuilder = graphics->CreateInputLayoutBuilder();
		inputLayoutBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		inputLayoutBuilder->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
		inputLayoutBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
		auto inputLayout = inputLayoutBuilder->Build();

		auto shaderBuilder = graphics->CreateShaderBuilder();
		shaderBuilder->SetShaderType(Netcode::ShaderType::VERTEX_SHADER);
		shaderBuilder->SetSource(L"Editor_GBufferPass_Vertex.hlsl");
		shaderBuilder->SetEntrypoint("main");
		auto vs = shaderBuilder->Build();

		shaderBuilder->SetShaderType(Netcode::ShaderType::PIXEL_SHADER);
		shaderBuilder->SetSource(L"Editor_GBufferPass_Pixel.hlsl");
		shaderBuilder->SetEntrypoint("main");
		auto ps = shaderBuilder->Build();

		auto rootSigBuilder = graphics->CreateRootSignatureBuilder();
		rootSignature = rootSigBuilder->BuildFromShader(vs);

		Netcode::DepthStencilDesc depthStencilDesc;
		depthStencilDesc.depthEnable = true;

		auto psoBuilder = graphics->CreateGPipelineStateBuilder();
		psoBuilder->SetInputLayout(inputLayout);
		psoBuilder->SetDepthStencilFormat(graphics->GetDepthStencilFormat());
		psoBuilder->SetNumRenderTargets(1);
		psoBuilder->SetRenderTargetFormat(0, graphics->GetBackbufferFormat());
		psoBuilder->SetDepthStencilState(depthStencilDesc);
		psoBuilder->SetRootSignature(rootSignature);
		pipelineState = psoBuilder->Build();
	}
};

/**
* BRDF function that expects a Position, Normal, TexCoord, Binormal and Tangent
* Handles normal mapping
*/
class BRDF_NormalMaterial : public BRDF_MaterialBase {

};

/**
* BRDF function that handles skinning in the vertex shader.
* expects Position, Normal, TexCoord, Weights and BoneIDs
* NORMAL_TEXTURE is ignored for this material
*/
class BRDF_SkinnedMaterial : public BRDF_MaterialBase {

};

/**
* BRDF function that handles skinning and normal mapping as well.
* expect PNT, BT, WB.
*/
class BRDF_NormalSkinnedMaterial : public BRDF_MaterialBase {

};


struct ShadedMesh {
	GBuffer geometry;
	std::shared_ptr<MaterialBase> material;

	void Render(Netcode::Graphics::IRenderContext * ctx) {
		material->Apply(ctx);
		ctx->SetVertexBuffer(geometry.vertexBuffer);
		if(geometry.indexCount > 0) {
			ctx->SetIndexBuffer(geometry.indexBuffer);
			ctx->DrawIndexed(geometry.indexCount);
		} else {
			ctx->Draw(geometry.vertexCount);
		}
	}
};
