#pragma once

#include <Egg/HandleTypes.h>
#include "ConstantBufferTypes.h"
#include <memory>

struct Model {
	
	struct ShadedMesh {
		Egg::HITEM mesh;
		MaterialCb * materialData;
	};

	unsigned int meshesLength;
	PerObjectCb * perObjectCb;
	BoneDataCb * boneDataCb;
	ShadedMesh meshes[16];

	Model() : meshesLength{ 0 }, perObjectCb{ nullptr }, boneDataCb{ nullptr }, meshes{ 0 } { }
	~Model() = default;

	void AddShadedMesh(Egg::HITEM item, MaterialCb * matData) {
		ASSERT(meshesLength <= 16, "Maximum number of meshes reached");
		meshes[meshesLength].mesh = item;
		meshes[meshesLength].materialData = matData;
		meshesLength += 1;
	}

};
