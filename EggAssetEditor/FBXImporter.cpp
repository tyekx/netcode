#include "pch.h"
#include "FBXImporter.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <DirectXMath.h>
#include <Egg/FancyIterators.hpp>
#include "BoundingBoxHelpers.h"

static DirectX::XMFLOAT4X4 ToFloat4x4(const aiMatrix4x4 & m) {
/*	return DirectX::XMFLOAT4X4{ m.a1, m.a2, m.a3, m.a4,
								m.b1, m.b2, m.b3, m.a4,
								m.c1, m.c2, m.c3, m.c4,
								m.d1, m.d2, m.d3, m.d4 };*/
	return DirectX::XMFLOAT4X4{ m.a1, m.b1, m.c1, m.d1,
								m.a2, m.b2, m.c2, m.d2,
								m.a3, m.b3, m.c3, m.d3,
								m.a4, m.b4, m.c4, m.d4 };
}

static std::vector<Bone> ImportBones(const aiMesh * mesh) {
	std::vector<Bone> bones;

	if(!mesh->HasBones()) {
		return bones;
	}

	for(uint32_t boneIter = 0; boneIter < mesh->mNumBones; ++boneIter) {
		Bone importedBone;

		importedBone.boneName = mesh->mBones[boneIter]->mName.C_Str();
		importedBone.transform = ToFloat4x4(mesh->mBones[boneIter]->mOffsetMatrix);
		importedBone.envelope.reserve(mesh->mBones[boneIter]->mNumWeights);

		for(uint32_t envelopeIter = 0; envelopeIter < mesh->mBones[boneIter]->mNumWeights; ++envelopeIter) {
			VertexWeight vw;
			vw.vertexIndex = mesh->mBones[boneIter]->mWeights[envelopeIter].mVertexId;
			vw.weight = mesh->mBones[boneIter]->mWeights[envelopeIter].mWeight;
			importedBone.envelope.push_back(vw);
		}

		bones.push_back(std::move(importedBone));
	}

	return bones;
}

static std::tuple<std::vector<InputElement>, uint32_t> GetInputLayout(const aiMesh * mesh) {
	std::vector<InputElement> inputLayout;
	uint32_t vertexSize = 0;

	if(mesh->HasPositions()) {
		InputElement ie;
		ie.byteOffset = vertexSize;
		ie.format = DXGI_FORMAT_R32G32B32_FLOAT;
		ie.semanticIndex = 0;
		ie.semanticName = "POSITION";
		inputLayout.push_back(ie);
		vertexSize += sizeof(DirectX::XMFLOAT3);
	}

	if(mesh->HasNormals()) {
		InputElement ie;
		ie.byteOffset = vertexSize;
		ie.format = DXGI_FORMAT_R32G32B32_FLOAT;
		ie.semanticIndex = 0;
		ie.semanticName = "NORMAL";
		inputLayout.push_back(ie);
		vertexSize += sizeof(DirectX::XMFLOAT3);
	}

	for(int i = 0; mesh->HasTextureCoords(i); ++i) {
		uint32_t uvwCount = mesh->mNumUVComponents[i];

		InputElement ie;
		ie.byteOffset = vertexSize;
		ie.semanticIndex = i;
		ie.semanticName = "TEXCOORD";

		if(uvwCount == 2) {
			ie.format = DXGI_FORMAT_R32G32_FLOAT;
		} else if(uvwCount == 3) {
			ie.format = DXGI_FORMAT_R32G32B32_FLOAT;
		} else {
			ie.format = DXGI_FORMAT_UNKNOWN;
		}

		inputLayout.push_back(ie);
		vertexSize += sizeof(float) * uvwCount;
	}

	if(mesh->HasTangentsAndBitangents()) {
		InputElement ie;
		ie.byteOffset = vertexSize;
		ie.format = DXGI_FORMAT_R32G32B32_FLOAT;
		ie.semanticIndex = 0;
		ie.semanticName = "BINORMAL";
		inputLayout.push_back(ie);
		vertexSize += sizeof(DirectX::XMFLOAT3);
		ie.byteOffset = vertexSize;
		ie.format = DXGI_FORMAT_R32G32B32_FLOAT;
		ie.semanticIndex = 0;
		ie.semanticName = "TANGENT";
		inputLayout.push_back(ie);
		vertexSize += sizeof(DirectX::XMFLOAT3);
	}

	if(mesh->HasBones()) {
		InputElement ie;
		ie.byteOffset = vertexSize;
		ie.format = DXGI_FORMAT_R32G32B32_FLOAT;
		ie.semanticIndex = 0;
		ie.semanticName = "WEIGHTS";
		inputLayout.push_back(ie);
		vertexSize += sizeof(DirectX::XMFLOAT3);
		ie.byteOffset = vertexSize;
		ie.format = DXGI_FORMAT_R32_UINT;
		ie.semanticIndex = 0;
		ie.semanticName = "BONEIDS";
		inputLayout.push_back(ie);
		vertexSize += sizeof(uint32_t);
	}

	return std::tie(inputLayout, vertexSize);
}

static uint32_t GetIndexCount(const aiMesh * mesh) {
	if(!mesh->HasFaces()) {
		return 0;
	}

	uint32_t indexCount = 0;
	for(uint32_t i = 0; i < mesh->mNumFaces; ++i) {
		if(mesh->mFaces[i].mNumIndices == 3) {
			indexCount += mesh->mFaces[i].mNumIndices;
		}
	}

	return indexCount;
}

static void SetNthByte(uint32_t & v, unsigned int i, uint8_t k) {
	uint32_t mask = ~(0xFF << (i * 8));
	v &= (mask | (static_cast<uint32_t>(k) << (i * 8)));
}

static int GetFirstUnusedSlot(uint32_t v) {
	for(int i = 0; i < 4; ++i) {
		if(((v >> (i * 8)) & 0xFF) == 0xFF) {
			return i;
		}
	}
	return -1;
}

static std::vector<Mesh> ImportMeshes(const aiScene * scene) {
	std::vector<Mesh> meshes;

	BoundingBoxGenerator boundingBoxGen;

	for(uint32_t i = 0; i < scene->mNumMeshes; ++i) {
		const aiMesh * mesh = scene->mMeshes[i];
		uint32_t numIndices = GetIndexCount(mesh);
		uint32_t numVertices = mesh->mNumVertices;
		auto [inputLayout, vertexStride] = GetInputLayout(mesh);

		LOD lod0;
		lod0.indexCount = numIndices;
		lod0.indexDataSizeInBytes = 4 * lod0.indexCount;
		lod0.vertexCount = numVertices;
		lod0.vertexDataSizeInBytes = numVertices * vertexStride;
		lod0.indexData = std::make_unique<uint8_t[]>(lod0.indexDataSizeInBytes);
		lod0.vertexData = std::make_unique<uint8_t[]>(lod0.vertexDataSizeInBytes);

		uint32_t * indexDataPtr = reinterpret_cast<uint32_t *>(lod0.indexData.get());

		for(uint32_t indexIter = 0; indexIter < mesh->mNumFaces; ++indexIter) {
			
			if(mesh->mFaces[indexIter].mNumIndices == 3) {
				*indexDataPtr++ = mesh->mFaces[indexIter].mIndices[0];
				*indexDataPtr++ = mesh->mFaces[indexIter].mIndices[1];
				*indexDataPtr++ = mesh->mFaces[indexIter].mIndices[2];
			}
			
		}

		uint8_t * vertexDataPtr = lod0.vertexData.get();
		uint8_t * vPtr;
		uint32_t byteOffset;
		uint32_t boneDataByteOffset;

		for(uint32_t vertexIter = 0; vertexIter < mesh->mNumVertices; ++vertexIter) {
			vPtr = vertexDataPtr + vertexIter * vertexStride;
			byteOffset = 0;

			if(mesh->HasPositions()) {
				DirectX::XMFLOAT3 * positionPtr = reinterpret_cast<DirectX::XMFLOAT3 *>(vPtr + byteOffset);
				positionPtr->x = mesh->mVertices[vertexIter].x;
				positionPtr->y = mesh->mVertices[vertexIter].y;
				positionPtr->z = mesh->mVertices[vertexIter].z;

				boundingBoxGen.UpdateForPoint(*positionPtr);

				byteOffset += sizeof(DirectX::XMFLOAT3);
			}

			if(mesh->HasNormals()) {
				DirectX::XMFLOAT3 * normalPtr = reinterpret_cast<DirectX::XMFLOAT3 *>(vPtr + byteOffset);
				normalPtr->x = mesh->mNormals[vertexIter].x;
				normalPtr->y = mesh->mNormals[vertexIter].y;
				normalPtr->z = mesh->mNormals[vertexIter].z;

				byteOffset += sizeof(DirectX::XMFLOAT3);
			}

			for(int texCoordIter = 0; mesh->HasTextureCoords(texCoordIter); ++texCoordIter) {
				float * texCoordPtr = reinterpret_cast<float *>(vPtr + byteOffset);
				uint32_t uvwCount = mesh->mNumUVComponents[texCoordIter];

				for(uint32_t uvwIter = 0; uvwIter < uvwCount; ++uvwIter) {
					*texCoordPtr++ = mesh->mTextureCoords[texCoordIter][vertexIter][uvwIter];
				}

				byteOffset += sizeof(float) * uvwCount;
			}

			if(mesh->HasTangentsAndBitangents()) {
				float * tanbinormal = reinterpret_cast<float *>(vPtr + byteOffset);
				
				*tanbinormal++ = mesh->mTangents[vertexIter].x;
				*tanbinormal++ = mesh->mTangents[vertexIter].y;
				*tanbinormal++ = mesh->mTangents[vertexIter].z;

				*tanbinormal++ = mesh->mBitangents[vertexIter].x;
				*tanbinormal++ = mesh->mBitangents[vertexIter].y;
				*tanbinormal++ = mesh->mBitangents[vertexIter].z;

				byteOffset += sizeof(float) * 6;
			}

			if(mesh->HasBones()) {
				float * weightsPtr = reinterpret_cast<float *>(vPtr + byteOffset);
				uint32_t * boneIdsPtr = reinterpret_cast<uint32_t *>(vPtr + byteOffset + 3 * sizeof(float));

				*weightsPtr++ = 0;
				*weightsPtr++ = 0;
				*weightsPtr++ = 0;
				*boneIdsPtr = 0xFFFFFFFF;

				boneDataByteOffset = byteOffset;
				byteOffset += sizeof(DirectX::XMFLOAT3);
				byteOffset += sizeof(uint32_t);
			}

		}

		Mesh importedMesh;
		importedMesh.name = mesh->mName.C_Str();
		importedMesh.boundingBox = boundingBoxGen.GetBoundingBox();
		importedMesh.bones = ImportBones(mesh);
		importedMesh.inputLayout = std::move(inputLayout);
		importedMesh.vertexStride = vertexStride;
		importedMesh.materialIdx = mesh->mMaterialIndex;
		importedMesh.lods.push_back(std::move(lod0));
		meshes.push_back(std::move(importedMesh));
	}

	return meshes;
}

static const aiNode * GetNodeByNameImpl(const aiNode * node, const std::string & name) {
	if(name == node->mName.C_Str()) {
		return node;
	}

	for(uint32_t i = 0; i < node->mNumChildren; ++i) {
		const aiNode* ptr = GetNodeByNameImpl(node->mChildren[i], name);
		if(ptr) {
			return ptr;
		}
	}

	return nullptr;
}

static const aiNode * GetNodeByName(const aiScene * scene, const std::string & name) {
	return GetNodeByNameImpl(scene->mRootNode, name);
}

static Skeleton CreateSkeleton(const Model & partiallyImportedModel, const aiScene * scene) {
	Skeleton skeleton;

	std::vector<Bone> uniqueImportedBones;
	
	for(const Mesh & importedMesh : partiallyImportedModel.meshes) {
		if(uniqueImportedBones.size() == 0) {
			uniqueImportedBones = importedMesh.bones;
		} else {
			for(const auto & bone : importedMesh.bones) {
				auto it = std::find_if(std::begin(uniqueImportedBones), std::end(uniqueImportedBones), [&bone](const Bone & b) -> bool {
					return b.boneName == bone.boneName;
				});

				if(it == std::end(uniqueImportedBones)) {
					OutputDebugStringW(L"error, meshes have different skeletons associated to them\r\n");
					throw std::exception();
				}
			}
		}
	}

	std::vector<SkeletonBone> skeletonBones;

	for(const Bone & bone : uniqueImportedBones) {
		const aiNode * node = GetNodeByName(scene, bone.boneName);

		SkeletonBone skBone;
		skBone.boneName = bone.boneName;
		skBone.parentIndex = -1;
		skBone.transform = bone.transform;

		// must find a node in the parent hierarchy which is an actual bone
		const aiNode * parentBoneNode = node->mParent;
		bool parentBoneFound = false;

		while(!parentBoneFound && parentBoneNode != nullptr) {
			for(const Bone & b : uniqueImportedBones) {
				if(b.boneName == parentBoneNode->mName.C_Str()) {
					parentBoneFound = true;
					break;
				}
			}

			if(!parentBoneFound) {
				parentBoneNode = parentBoneNode->mParent;
			}
		}

		// check if we are processing the actual root node
		if(parentBoneNode == nullptr) {
			if(!skeletonBones.empty()) {
				OutputDebugStringW(L"\r\nOnly a single root is supported\r\n");
			}

			skeletonBones.emplace_back(std::move(skBone));
			continue;
		}

		// get the index of the parent
		skBone.parentIndex = 0;
		bool parentFound = false;
		for(const SkeletonBone & placedSkeletonBone : skeletonBones) {
			if(placedSkeletonBone.boneName == parentBoneNode->mName.C_Str()) {
				parentFound = true;
				break;
			}
			skBone.parentIndex += 1;
		}

		if(parentFound) {
			skeletonBones.emplace_back(std::move(skBone));
		}
	}

	skeleton.bones = std::move(skeletonBones);

	return skeleton;
}

static void FillVertexWeights(uint8_t * vPtr, uint32_t vertexStride, uint32_t boneDataByteOffset, const Mesh & importedMesh) {
	uint8_t boneIndex = 0;
	for(const Bone & bone : importedMesh.bones) {
		for(const VertexWeight & weight : bone.envelope) {
			uint32_t vertexIndex = weight.vertexIndex;
			uint32_t weightsByteOffset = vertexIndex * vertexStride + boneDataByteOffset;
			float * weights = reinterpret_cast<float *>(vPtr + weightsByteOffset);
			uint32_t * boneIds = reinterpret_cast<uint32_t *>(vPtr + weightsByteOffset + sizeof(DirectX::XMFLOAT3));

			int unusedSlot = GetFirstUnusedSlot(*boneIds);

			if(unusedSlot < 3) {
				weights[unusedSlot] = weight.weight;
			}

			if(unusedSlot < 4) {
				SetNthByte(*boneIds, static_cast<uint32_t>(unusedSlot), static_cast<uint8_t>(boneIndex));
			}
		}
		++boneIndex;
	}
}

static void FillVertexWeights(const Model & model) {

	for(const Mesh & importedMesh : model.meshes) {

		if(importedMesh.bones.empty()) {
			continue;
		}

		if(importedMesh.bones.size() != model.skeleton.bones.size()) {
			throw std::exception{ "Assumption about bone lengths" };
		}

		for(size_t i = 0; i < model.skeleton.bones.size(); ++i) {
			if(model.skeleton.bones[i].boneName != importedMesh.bones[i].boneName) {
				throw std::exception{ "Assumption about bone reordering failed" };
			}
		}

		uint32_t vertexStride = importedMesh.vertexStride;
		uint32_t boneDataByteOffset = 0;

		for(const InputElement & inputElement : importedMesh.inputLayout) {
			if(inputElement.semanticName == "WEIGHTS") {
				boneDataByteOffset = inputElement.byteOffset;
			}
		}

		for(const LOD & lod : importedMesh.lods) {
			uint8_t * vPtr = lod.vertexData.get();
			FillVertexWeights(vPtr, vertexStride, boneDataByteOffset, importedMesh);
		}
	}
}


static Material ImportMaterial(const aiMaterial * mat) {
	Material imat;
	aiString str;

	if(AI_SUCCESS == mat->Get(AI_MATKEY_NAME, str)) {
		imat.name = str.C_Str();
	}


	unsigned int diffuseCount = mat->GetTextureCount(aiTextureType_DIFFUSE);
	unsigned int normalCount = mat->GetTextureCount(aiTextureType_NORMALS);
	unsigned int emissiveColorCount = mat->GetTextureCount(aiTextureType_EMISSION_COLOR);
	unsigned int emissiveCount = mat->GetTextureCount(aiTextureType_EMISSIVE);
	unsigned int displacementCount = mat->GetTextureCount(aiTextureType_DISPLACEMENT);
	unsigned int ambientCount = mat->GetTextureCount(aiTextureType_AMBIENT);
	unsigned int ambientOccCount = mat->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION);
	unsigned int specularCount = mat->GetTextureCount(aiTextureType_SPECULAR);
	unsigned int lightmapCount = mat->GetTextureCount(aiTextureType_LIGHTMAP);
	unsigned int heightCount = mat->GetTextureCount(aiTextureType_HEIGHT);
	unsigned int reflectionCount = mat->GetTextureCount(aiTextureType_REFLECTION);
	unsigned int kd = mat->GetTextureCount(aiTextureType_BASE_COLOR);
	unsigned int metalnessCount = mat->GetTextureCount(aiTextureType_METALNESS);
	unsigned int unknownTexCount = mat->GetTextureCount(aiTextureType_UNKNOWN);
	unsigned int shininessCount = mat->GetTextureCount(aiTextureType_SHININESS);
	unsigned int opacity = mat->GetTextureCount(aiTextureType_OPACITY);
	unsigned int outOfIdeas = mat->GetTextureCount(_aiTextureType_Force32Bit);
	/*
	for(unsigned int i = 0; i < mat->mNumProperties; ++i) {
		const aiMaterialProperty * prop = mat->mProperties[i];
		printf("key: %s | idx: %d idx2: %d\r\n", prop->mKey.C_Str(), prop->mIndex, prop->mType);
		//$raw.ShininessExponent|file
		//$raw.DiffuseColor|file
		switch(prop->mType) {
			case aiPropertyTypeInfo::aiPTI_String:
			{
				aiString & str2 = *reinterpret_cast<aiString *>(prop->mData);
				printf("\t (string[%d]): %s\r\n", prop->mDataLength, str2.C_Str());
			}
			break;
			case aiPropertyTypeInfo::aiPTI_Integer:
			{
				int & v = *reinterpret_cast<int *>(prop->mData);
				printf("\t    (int[%d]): %d\r\n", prop->mDataLength / 4, v);
			}
			break;
			case aiPropertyTypeInfo::aiPTI_Float:
			{
				float * f = reinterpret_cast<float *>(prop->mData);

				switch(prop->mDataLength) {
					case 4:
						printf("\t  (float[%d]): %f\r\n", 1, *f);
						break;
					case 8:
						printf("\t  (float[%d]): %f %f\r\n", 2, f[0], f[1]);
						break;
					case 12:
						printf("\t  (float[%d]): %f %f %f\r\n", 3, f[0], f[1], f[2]);
						break;
					case 16:
						printf("\t  (float[%d]): %f %f %f %f\r\n", 4, f[0], f[1], f[2], f[3]);
						break;
				}
				//printf("\t  (float[%d]): %f\r\n", prop->mDataLength / 4, f);
			}
			break;
		}
	}*/

	if(diffuseCount > 0) {
		if(diffuseCount > 1) {
			OutputDebugStringW(L"Warning: only 1 diffuse map is supported as of now");
		}

		aiString texPath;

		if(AI_SUCCESS == mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath)) {
			imat.diffuseMapReference = texPath.C_Str();
		} else {
			OutputDebugStringW(L"Warning: failed to get diffuse texture path\r\n");
		}

	}

	if(normalCount > 0) {
		if(normalCount > 1) {
			OutputDebugStringW(L"Warning: only 1 normal map is supported as of now\r\n");
		}

		aiString texPath;
		if(AI_SUCCESS == mat->GetTexture(aiTextureType_NORMALS, 0, &texPath)) {
			imat.normalMapReference = texPath.C_Str();
		} else {
			OutputDebugStringW(L"Warning: failed to get diffuse texture path\r\n");
		}
	}

	aiColor3D diffuseColor;
	if(AI_SUCCESS != mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor)) {
		OutputDebugStringW(L"Failed to get diffuseColor\r\n");
	} else {
		imat.diffuseColor.x = diffuseColor.r;
		imat.diffuseColor.y = diffuseColor.g;
		imat.diffuseColor.z = diffuseColor.b;
		imat.diffuseColor.w = 1.0f;
	}

	if(AI_SUCCESS != mat->Get(AI_MATKEY_SHININESS, imat.shininess)) {
		OutputDebugStringW(L"Failed to get shininess\r\n");
	}

	return imat;
}

static std::vector<Material> ImportMaterials(const aiScene * scene) {
	std::vector<Material> materials;

	for(uint32_t i = 0; i < scene->mNumMaterials; ++i) {
		materials.push_back(ImportMaterial(scene->mMaterials[i]));
	}

	return materials;
}

static Model ImportModel(const aiScene * scene) {
	Model importedModel;

	if(scene != nullptr) {
		importedModel.meshes = ImportMeshes(scene);
	}

	importedModel.skeleton = CreateSkeleton(importedModel, scene);

	FillVertexWeights(importedModel);

	importedModel.materials = ImportMaterials(scene);

	return importedModel;
}

Model FBXImporter::FromFile(const std::string & file) {
	Assimp::Importer importer;

	uint32_t flags = aiProcess_Triangulate | aiProcess_FlipWindingOrder | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes;

	const aiScene * scene = importer.ReadFile(file, flags);

	if(scene == nullptr) {
		const char * err = importer.GetErrorString();
		OutputDebugStringA(err);
	}

	return ImportModel(scene);
}

Model FBXImporter::FromMemory(const uint8_t * source, uint32_t sizeInBytes)
{
	Assimp::Importer importer;

	uint32_t flags = aiProcess_Triangulate | aiProcess_FlipWindingOrder | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes;

	const aiScene * scene = importer.ReadFileFromMemory(source, sizeInBytes, flags, ".fbx");

	return ImportModel(scene);
}


static std::vector<Animation> ImportAnimations(const aiScene * scene, const Skeleton & skeleton) {
	std::vector<Animation> animations;

	for(uint32_t i = 0; i < scene->mNumAnimations; ++i) {

		const aiAnimation * anim = scene->mAnimations[i];

		Animation importedAnimation;
		importedAnimation.duration = anim->mDuration;
		importedAnimation.framesPerSecond = anim->mTicksPerSecond;
		importedAnimation.name = anim->mName.C_Str();

		for(uint32_t boneIt = 0; boneIt < anim->mNumChannels; ++boneIt) {
			const aiNodeAnim * nodeAnim = anim->mChannels[boneIt];

			BoneAnimation importedBoneAnimation;
			importedBoneAnimation.BoneId = skeleton.GetBoneIndex(nodeAnim->mNodeName.C_Str());
			importedBoneAnimation.PostState = static_cast<AnimationEdge>(nodeAnim->mPostState);
			importedBoneAnimation.PreState = static_cast<AnimationEdge>(nodeAnim->mPreState);

			if(nodeAnim->mNumPositionKeys > 0) {
				importedBoneAnimation.positionKeys.reserve(nodeAnim->mNumPositionKeys);
			}

			if(nodeAnim->mNumRotationKeys > 0) {
				importedBoneAnimation.rotationKeys.reserve(nodeAnim->mNumRotationKeys);
			}

			if(nodeAnim->mNumScalingKeys > 0) {
				importedBoneAnimation.scaleKeys.reserve(nodeAnim->mNumScalingKeys);
			}

			for(uint32_t keyIt = 0; keyIt < nodeAnim->mNumPositionKeys; ++keyIt) {
				PositionKey pk;
				pk.position.x = nodeAnim->mPositionKeys[keyIt].mValue.x;
				pk.position.y = nodeAnim->mPositionKeys[keyIt].mValue.y;
				pk.position.z = nodeAnim->mPositionKeys[keyIt].mValue.z;

				pk.time = nodeAnim->mPositionKeys[keyIt].mTime;

				importedBoneAnimation.positionKeys.push_back(pk);
			}

			for(uint32_t keyIt = 0; keyIt < nodeAnim->mNumRotationKeys; ++keyIt) {
				RotationKey rk;
				rk.rotation.x = nodeAnim->mRotationKeys[keyIt].mValue.x;
				rk.rotation.y = nodeAnim->mRotationKeys[keyIt].mValue.y;
				rk.rotation.z = nodeAnim->mRotationKeys[keyIt].mValue.z;
				rk.rotation.w = nodeAnim->mRotationKeys[keyIt].mValue.w;

				rk.time = nodeAnim->mRotationKeys[keyIt].mTime;

				importedBoneAnimation.rotationKeys.push_back(rk);
			}

			for(uint32_t keyIt = 0; keyIt < nodeAnim->mNumScalingKeys; ++keyIt) {
				ScaleKey sk;
				sk.scale.x = nodeAnim->mScalingKeys[keyIt].mValue.x;
				sk.scale.y = nodeAnim->mScalingKeys[keyIt].mValue.y;
				sk.scale.z = nodeAnim->mScalingKeys[keyIt].mValue.z;

				sk.time = nodeAnim->mScalingKeys[keyIt].mTime;

				importedBoneAnimation.scaleKeys.push_back(sk);
			}

			importedAnimation.boneAnimations.push_back(std::move(importedBoneAnimation));
		}

		animations.push_back(std::move(importedAnimation));

	}

	return animations;
}

std::vector<Animation> FBXImporter::ImportAnimationsFromFile(const std::string & file, const Skeleton & skeleton) {

	Assimp::Importer importer;

	uint32_t flags = aiProcess_Triangulate | aiProcess_FlipWindingOrder | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes;

	const aiScene * scene = importer.ReadFile(file, flags);

	return ImportAnimations(scene, skeleton);
}

std::vector<Animation> FBXImporter::ImportAnimationsFromMemory(const uint8_t * source, uint32_t sizeInBytes, const Skeleton & skeleton) {
	
	Assimp::Importer importer;

	uint32_t flags = aiProcess_Triangulate | aiProcess_FlipWindingOrder | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes;

	const aiScene * scene = importer.ReadFileFromMemory(source, sizeInBytes, flags, ".fbx");

	return ImportAnimations(scene, skeleton);
}

static bool StrictlyGreater(double a, double b, double epsilon = 0.001) {
	return a > (b + epsilon);
}

static bool RoughlyEqual(double a, double b, double epsilon = 0.001) {
	return std::abs(a - b) < epsilon;
}

static bool StrictlyLess(double a, double b, double epsilon = 0.001) {
	return a < (b - epsilon);
}

static double GetNextTimeKey(const std::vector<BoneAnimation> & boneData, double currentTimeKey) {
	double candidate = std::numeric_limits<double>::infinity();
	for(const BoneAnimation & boneAnim : boneData) {
		for(const PositionKey & pk : boneAnim.positionKeys) {
			if(StrictlyGreater(pk.time, currentTimeKey)) {
				if(StrictlyLess(pk.time, candidate)) {
					candidate = pk.time;
				}
				break;
			}
		}

		for(const RotationKey & rk : boneAnim.rotationKeys) {
			if(StrictlyGreater(rk.time, currentTimeKey)) {
				if(StrictlyLess(rk.time, candidate)) {
					candidate = rk.time;
				}
				break;
			}
		}

		for(const ScaleKey & sk : boneAnim.scaleKeys) {
			if(StrictlyGreater(sk.time, currentTimeKey)) {
				if(StrictlyLess(sk.time, candidate)) {
					candidate = sk.time;
				}
				break;
			}
		}
	}

	return candidate;
}

static DirectX::XMFLOAT3 SamplePosition(const std::vector<PositionKey> & keys, double t) {
	if(keys.empty()) {
		return DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	if(keys.size() == 1) {
		return keys[0].position;
	}

	int indexEnd = 0;
	for(const PositionKey & pk : keys) {
		if(RoughlyEqual(pk.time, t)) {
			return pk.position;
		}

		if(StrictlyGreater(t, pk.time)) {
			indexEnd += 1;
		} else break;
	}

	if(indexEnd == 0) {
		return keys[0].position;
	}

	double end = keys.at(indexEnd).time;
	double start = keys.at(indexEnd - 1).time;

	double alpha = (end - t) / (end - start);

	float alphaF = static_cast<float>(alpha);

	DirectX::XMVECTOR endV = DirectX::XMLoadFloat3(&keys.at(indexEnd).position);
	DirectX::XMVECTOR startV = DirectX::XMLoadFloat3(&keys.at(indexEnd - 1).position);

	DirectX::XMVECTOR lerpedV = DirectX::XMVectorLerp(startV, endV, alphaF);

	DirectX::XMFLOAT3 lerped;
	DirectX::XMStoreFloat3(&lerped, lerpedV);

	return lerped;
}

static DirectX::XMFLOAT4 SampleRotation(const std::vector<RotationKey> & keys, double t) {
	if(keys.empty()) {
		return DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	if(keys.size() == 1) {
		return keys[0].rotation;
	}

	int indexEnd = 0;
	for(const RotationKey & pk : keys) {
		if(RoughlyEqual(pk.time, t)) {
			return pk.rotation;
		}

		if(StrictlyGreater(t, pk.time)) {
			indexEnd += 1;
		} else break;
	}

	if(indexEnd == 0) {
		return keys[0].rotation;
	}

	double end = keys.at(indexEnd).time;
	double start = keys.at(indexEnd - 1).time;

	double alpha = (end - t) / (end - start);

	float alphaF = static_cast<float>(alpha);

	DirectX::XMVECTOR endV = DirectX::XMLoadFloat4(&keys.at(indexEnd).rotation);
	DirectX::XMVECTOR startV = DirectX::XMLoadFloat4(&keys.at(indexEnd - 1).rotation);

	DirectX::XMVECTOR slerpedV = DirectX::XMQuaternionSlerp(startV, endV, alphaF);

	DirectX::XMFLOAT4 slerped;
	DirectX::XMStoreFloat4(&slerped, slerpedV);

	return slerped;
}

static DirectX::XMFLOAT3 SampleScale(const std::vector<ScaleKey> & keys, double t) {
	if(keys.empty()) {
		return DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	}

	if(keys.size() == 1) {
		return keys[0].scale;
	}

	int indexEnd = 0;
	for(const ScaleKey & pk : keys) {
		if(RoughlyEqual(pk.time, t)) {
			return pk.scale;
		}

		if(StrictlyGreater(t, pk.time)) {
			indexEnd += 1;
		} else break;
	}

	if(indexEnd == 0) {
		return keys[0].scale;
	}

	double end = keys.at(indexEnd).time;
	double start = keys.at(indexEnd - 1).time;

	double alpha = (end - t) / (end - start);

	float alphaF = static_cast<float>(alpha);

	DirectX::XMVECTOR endV = DirectX::XMLoadFloat3(&keys.at(indexEnd).scale);
	DirectX::XMVECTOR startV = DirectX::XMLoadFloat3(&keys.at(indexEnd - 1).scale);

	DirectX::XMVECTOR lerpedV = DirectX::XMVectorLerp(startV, endV, alphaF);

	DirectX::XMFLOAT3 lerped;
	DirectX::XMStoreFloat3(&lerped, lerpedV);

	return lerped;
}

OptimizedAnimation FBXImporter::OptimizeAnimation(const Animation & anim, const Skeleton & skeleton) {
	OptimizedAnimation optAnim;

	if(anim.boneAnimations.empty()) {
		return optAnim;
	}

	// if optimization step is slow
	//std::unique_ptr<uint32_t[]> indexCache = std::make_unique<uint32_t[]>(anim.boneAnimations.size());

	std::vector<double> timeKeys;
	timeKeys.push_back(0.0);
	double ct = 0.0;

	while(true) {
		ct = GetNextTimeKey(anim.boneAnimations, ct);

		// no proper candidate was found
		if(ct == std::numeric_limits<double>::infinity()) {
			break;
		}

		timeKeys.push_back(ct);
	}

	std::vector<float> timeKeysF;
	timeKeysF.reserve(timeKeys.size());

	for(double d : timeKeys) {
		timeKeysF.push_back(static_cast<float>(d));
	}

	optAnim.keys.reserve(timeKeys.size());
	optAnim.keyTimes = std::move(timeKeysF);
	optAnim.duration = static_cast<float>(anim.duration);
	optAnim.framesPerSecond = static_cast<float>(anim.framesPerSecond);

	for(double t : timeKeys) {
		OptimizedBoneAnimation optBoneAnim;
		optBoneAnim.boneData.resize(skeleton.bones.size());

		// set defaults
		for(size_t i = 0; i < skeleton.bones.size(); ++i) {
			optBoneAnim.boneData[i].position = DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f };
			optBoneAnim.boneData[i].rotation = DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f };
			optBoneAnim.boneData[i].scale = DirectX::XMFLOAT3{ 1.0f, 1.0f, 1.0f };
		}

		for(const BoneAnimation & bA : anim.boneAnimations) {
			if(bA.BoneId > 0) {
				optBoneAnim.boneData[bA.BoneId].position = SamplePosition(bA.positionKeys, t);
				optBoneAnim.boneData[bA.BoneId].rotation = SampleRotation(bA.rotationKeys, t);
				optBoneAnim.boneData[bA.BoneId].scale = SampleScale(bA.scaleKeys, t);
			}
		}

		optAnim.keys.push_back(std::move(optBoneAnim));
	}

	return optAnim;
}
