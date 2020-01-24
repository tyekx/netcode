#pragma once

#include <memory>
#include <Egg/GraphicsContexts.h>
#include "ConstantBufferTypes.h"

#define CB_SLOT_PEROBJECT 1
#define CB_SLOT_PERMESH 0
#define CB_SLOT_BONEDATA 2
#define CB_SLOT_PERFRAME 3

struct Material {
protected:
public:
	Egg::RootSignatureRef rootSignature;
	Egg::PipelineStateRef pipelineState;

	virtual void Apply(Egg::Graphics::IRenderContext * ctx) {
			
	}
};

struct TestMaterial : public Material {
	MaterialData data;

	virtual void Apply(Egg::Graphics::IRenderContext * ctx) override {
		ctx->SetConstants(0, data);
	}
};
