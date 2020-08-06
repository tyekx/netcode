#pragma once

#include <Netcode/Modules.h>
#include <Netcode/Graphics/GraphicsContexts.h>
#include <Netcode/HandleTypes.h>
#include <memory>
#include <Netcode/URI/Texture.h>
#include "ConstantBufferTypes.h"

struct GBuffer {
	Ref<Netcode::GpuResource> vertexBuffer;
	Ref<Netcode::GpuResource> indexBuffer;
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
	Netcode::Float3 fresnelR0;
	float roughness;
	int textures;

	BRDF_Data() = default;
};

class BRDF_MaterialBase : public MaterialBase {
protected:
	Ref<Netcode::PipelineState> pipelineState;
	Ref<Netcode::RootSignature> rootSignature;
	Ref<Netcode::ResourceViews> textures;
	Ref<Netcode::GpuResource> resourceRefs[5];
	Netcode::URI::Texture textureIds[5];

	template<size_t N>
	static bool CheckInputLayoutElements(const char * (&cArray)[N], const std::vector<InputElement> & elements) {
		std::vector<const char *> diff;

		struct ILComp {
			bool operator()(const InputElement & a, const char * b) const {
				return a.semanticName == b;
			}

			bool operator()(const char * a, const InputElement & b) const {
				return b.semanticName == a;
			}
		};

		std::set_difference(std::begin(cArray),
			std::end(cArray),
			std::begin(elements),
			std::end(elements),
			std::back_inserter(diff),
			ILComp{});

		return diff.empty();
	}

public:
	BRDF_Data Data;

	BRDF_MaterialBase() = default;

	virtual void Initialize(Netcode::Module::IGraphicsModule * graphics) {
		textures = graphics->resources->CreateShaderResourceViews(ARRAYSIZE(resourceRefs));
	}

	virtual bool MeshIsCompatible(const Mesh & mesh) const = 0;
	
	virtual void Apply(Netcode::Graphics::IRenderContext * renderContext) override {
		renderContext->SetRootSignature(rootSignature);
		renderContext->SetPipelineState(pipelineState);
		renderContext->SetConstants(3, Data);
		renderContext->SetShaderResources(4, textures);
	}

	const Netcode::URI::Texture & GetID(BRDF_TextureType slot) {
		return textureIds[static_cast<uint32_t>(slot)];
	}

	void SetTexture(BRDF_TextureType slot, Netcode::URI::Texture texId, Ref<Netcode::GpuResource> texture) {
		const uint32_t idx = static_cast<uint32_t>(slot);
		textures->CreateSRV(static_cast<uint32_t>(idx), texture.get());
		Data.textures |= (1 << idx);
		std::swap(textureIds[idx], texId);
		std::swap(resourceRefs[idx], texture);
	}

	void RemoveTexture(BRDF_TextureType slot) {
		const uint32_t idx = static_cast<uint32_t>(slot);

		if(resourceRefs[idx] != nullptr) {
			Data.textures &= ~(1 << idx);
			textureIds[idx] = Netcode::URI::Texture{};
			resourceRefs[idx].reset();
			textures->RemoveSRV(idx, Netcode::Graphics::ResourceDimension::TEXTURE2D);
		}
	}
};

/**
* BRDF function that expects a Position, Normal and TexCoord.
* NORMAL_TEXTURE is ignored for this material
*/
class BRDF_DefaultMaterial : public BRDF_MaterialBase {
public:
	using BRDF_MaterialBase::BRDF_MaterialBase;

	virtual void Initialize(Netcode::Module::IGraphicsModule * graphics) override {
		BRDF_MaterialBase::Initialize(graphics);

		auto inputLayoutBuilder = graphics->CreateInputLayoutBuilder();
		inputLayoutBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		inputLayoutBuilder->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
		inputLayoutBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
		inputLayoutBuilder->AddInputElement("WEIGHTS", DXGI_FORMAT_R32G32B32_FLOAT);
		inputLayoutBuilder->AddInputElement("BONEIDS", DXGI_FORMAT_R32_UINT);
		auto inputLayout = inputLayoutBuilder->Build();

		auto shaderBuilder = graphics->CreateShaderBuilder();
		auto vs = shaderBuilder->LoadBytecode(L"Editor_GBufferPass_Vertex.cso");
		auto ps = shaderBuilder->LoadBytecode(L"Editor_GBufferPass_Pixel.cso");

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
		psoBuilder->SetVertexShader(vs);
		psoBuilder->SetPixelShader(ps);
		psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
		pipelineState = psoBuilder->Build();
	}

	virtual bool MeshIsCompatible(const Mesh & mesh) const override {
		const char * expectedElements[] = {
			"POSITION", "NORMAL", "TEXCOORD"
		};

		return CheckInputLayoutElements(expectedElements, mesh.inputLayout);
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
	Ref<MaterialBase> material;

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
