#include "Material.h"
#include <Netcode/Utility.h>

namespace Netcode {

#define REGISTER_CONSTANT(id, className, variableMember) MaterialParam(static_cast<uint32_t>(id), \
		static_cast<uint16_t>(offsetof(className, variableMember)), static_cast<uint16_t>(sizeof(className::variableMember)))

	static constexpr MaterialParam INVALID_MATERIAL{ static_cast<uint32_t>(MaterialParamId::INVALID_PARAMETER), 0, 0 };
	
	static constexpr MaterialParam brdfReflectionData[] = {
		REGISTER_CONSTANT(MaterialParamId::DIFFUSE_ALBEDO, BrdfMaterial, brdfData.diffuseAlbedo),
		REGISTER_CONSTANT(MaterialParamId::SPECULAR_ALBEDO, BrdfMaterial, brdfData.fresnelR0),
		REGISTER_CONSTANT(MaterialParamId::ROUGHNESS, BrdfMaterial, brdfData.roughness),
		REGISTER_CONSTANT(MaterialParamId::TEXTURE_TILES, BrdfMaterial, brdfData.textureTiles),
		REGISTER_CONSTANT(MaterialParamId::TEXTURE_TILES_OFFSET, BrdfMaterial, brdfData.textureOffset),
		REGISTER_CONSTANT(MaterialParamId::DISPLACEMENT_SCALE, BrdfMaterial, brdfData.displacementScale),
		REGISTER_CONSTANT(MaterialParamId::DISPLACEMENT_BIAS, BrdfMaterial, brdfData.displacementBias),
		REGISTER_CONSTANT(MaterialParamId::TEXTURE_FLAGS, BrdfMaterial, brdfData.textureFlags),
		REGISTER_CONSTANT(MaterialParamId::REFLECTANCE, BrdfMaterial, brdfData.reflectivity),
		REGISTER_CONSTANT(MaterialParamId::METAL_MASK, BrdfMaterial, brdfData.metalMask),
		REGISTER_CONSTANT(MaterialParamId::TEXTURE_DIFFUSE, BrdfMaterial, resources[0]),
		REGISTER_CONSTANT(MaterialParamId::TEXTURE_NORMAL, BrdfMaterial, resources[1]),
		REGISTER_CONSTANT(MaterialParamId::TEXTURE_AMBIENT, BrdfMaterial, resources[2]),
		REGISTER_CONSTANT(MaterialParamId::TEXTURE_SPECULAR, BrdfMaterial, resources[3]),
		REGISTER_CONSTANT(MaterialParamId::TEXTURE_ROUGHNESS, BrdfMaterial, resources[4]),
		REGISTER_CONSTANT(MaterialParamId::TEXTURE_DISPLACEMENT, BrdfMaterial, resources[5]),
		REGISTER_CONSTANT(MaterialParamId::TEXTURE_DIFFUSE_PATH, BrdfMaterial, brdfPathData[0]),
		REGISTER_CONSTANT(MaterialParamId::TEXTURE_NORMAL_PATH, BrdfMaterial, brdfPathData[1]),
		REGISTER_CONSTANT(MaterialParamId::TEXTURE_AMBIENT_PATH, BrdfMaterial, brdfPathData[2]),
		REGISTER_CONSTANT(MaterialParamId::TEXTURE_SPECULAR_PATH, BrdfMaterial, brdfPathData[3]),
		REGISTER_CONSTANT(MaterialParamId::TEXTURE_ROUGHNESS_PATH, BrdfMaterial, brdfPathData[4]),
		REGISTER_CONSTANT(MaterialParamId::TEXTURE_DISPLACEMENT_PATH, BrdfMaterial, brdfPathData[5]),
	};

	Ref<Material> BrdfMaterial::Clone() const {
		Ref<BrdfMaterial> clone = std::make_shared<BrdfMaterial>(type, name);
		clone->brdfData = brdfData;
		for(uint32_t i = 0; i < Utility::ArraySize(brdfPathData); ++i) {
			clone->brdfPathData[i] = brdfPathData[i];
		}

		for(uint32_t i = 0; i < Utility::ArraySize(resources); ++i) {
			clone->resources[i] = resources[i];
		}

		for(uint32_t i = 0; i < Utility::ArraySize(views); ++i) {
			clone->views[i] = views[i];
		}

		return clone;
	}

	uint32_t BrdfMaterial::GetParameterCount() const {
		return Material::GetParameterCount() + Utility::ArraySize(brdfReflectionData);
	}

	MaterialParam BrdfMaterial::GetParameterByIndex(uint32_t idx) const {
		uint32_t numParentParams = Material::GetParameterCount();
		uint32_t numMyParams = Utility::ArraySize(brdfReflectionData);
		uint32_t localIndex = idx - numParentParams;

		if(localIndex < numMyParams) {
			return brdfReflectionData[localIndex];
		} else {
			return Material::GetParameterByIndex(idx);
		}
	}

	const void * BrdfMaterial::GetParameterPointerByIndex(uint32_t idx) const {
		uint32_t numParentParams = Material::GetParameterCount();
		uint32_t numMyParams = Utility::ArraySize(brdfReflectionData);
		uint32_t localIndex = idx - numParentParams;

		if(localIndex < numMyParams) {
			MaterialParam mp = brdfReflectionData[localIndex];

			return reinterpret_cast<const uint8_t *>(this) + mp.offset;
		} else {
			return Material::GetParameterPointerByIndex(idx);
		}
	}

	void * BrdfMaterial::GetParameterPointer(uint32_t id) {
		for(uint32_t i = 0; i < Utility::ArraySize(brdfReflectionData); i++) {
			MaterialParam param = brdfReflectionData[i];

			if(param.id == id) {
				return reinterpret_cast<uint8_t *>(this) + param.offset;
			}
		}

		return Material::GetParameterPointer(id);
	}

	constexpr const MaterialParam & FindParameter(uint32_t id) {
		for(uint32_t i = 0; i < Utility::ArraySize(brdfReflectionData); i++) {
			if(brdfReflectionData[i].id == id) {
				return brdfReflectionData[i];
			}
		}
		return INVALID_MATERIAL;
	}

	const void * BrdfMaterial::GetParameterPointer(uint32_t id) const {
		const MaterialParam & param = FindParameter(id);
		
		if(param.id == id) {
			return reinterpret_cast<const uint8_t *>(this) + param.offset;
		}

		return Material::GetParameterPointer(id);
	}

	Ref<GpuResource> BrdfMaterial::GetResource(uint32_t resourceIndex) const {
		Netcode::OutOfRangeAssertion(resourceIndex < Utility::ArraySize(resources));
		return resources[resourceIndex];
	}

	void BrdfMaterial::SetResource(uint32_t resourceIndex, Ref<GpuResource> resource) {
		Netcode::OutOfRangeAssertion(resourceIndex < Utility::ArraySize(resources));
		std::swap(resources[resourceIndex], resource);
	}

	Ref<ResourceViews> BrdfMaterial::GetResourceView(uint32_t viewIndex) const {
		Netcode::OutOfRangeAssertion(viewIndex < Utility::ArraySize(views));
		return views[viewIndex];
	}

	void BrdfMaterial::SetResourceView(uint32_t viewIndex, Ref<ResourceViews> view) {
		Netcode::OutOfRangeAssertion(viewIndex < Utility::ArraySize(views));
		std::swap(views[viewIndex], view);
	}

}
