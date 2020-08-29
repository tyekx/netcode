#include "FBXImporter.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <Netcode/Utility.h>
#include <Netcode/FancyIterators.hpp>
#include "BoundingBoxHelpers.h"
#include <Windows.h>

namespace Netcode {

	static Netcode::Float4x4 ToFloat4x4(const aiMatrix4x4 & m) {
		return Netcode::Float4x4{ m.a1, m.b1, m.c1, m.d1,
									m.a2, m.b2, m.c2, m.d2,
									m.a3, m.b3, m.c3, m.d3,
									m.a4, m.b4, m.c4, m.d4 };
	}

	static std::vector<Intermediate::Bone> ImportBones(const aiMesh * mesh) {
		std::vector<Intermediate::Bone> bones;

		if(!mesh->HasBones()) {
			return bones;
		}

		for(uint32_t boneIter = 0; boneIter < mesh->mNumBones; ++boneIter) {
			Intermediate::Bone importedBone;

			importedBone.boneName = mesh->mBones[boneIter]->mName.C_Str();
			importedBone.transform = ToFloat4x4(mesh->mBones[boneIter]->mOffsetMatrix);
			importedBone.envelope.reserve(mesh->mBones[boneIter]->mNumWeights);

			for(uint32_t envelopeIter = 0; envelopeIter < mesh->mBones[boneIter]->mNumWeights; ++envelopeIter) {
				Intermediate::VertexWeight vw;
				vw.vertexIndex = mesh->mBones[boneIter]->mWeights[envelopeIter].mVertexId;
				vw.weight = mesh->mBones[boneIter]->mWeights[envelopeIter].mWeight;
				importedBone.envelope.push_back(vw);
			}

			bones.push_back(std::move(importedBone));
		}

		return bones;
	}

	static std::tuple<std::vector<Intermediate::InputElement>, uint32_t> GetInputLayout(const aiMesh * mesh) {
		std::vector<Intermediate::InputElement> inputLayout;
		uint32_t vertexSize = 0;

		if(mesh->HasPositions()) {
			Intermediate::InputElement ie;
			ie.byteOffset = vertexSize;
			ie.format = DXGI_FORMAT_R32G32B32_FLOAT;
			ie.semanticIndex = 0;
			ie.semanticName = "POSITION";
			inputLayout.push_back(ie);
		}

		vertexSize += sizeof(Netcode::Float3);

		if(mesh->HasNormals()) {
			Intermediate::InputElement ie;
			ie.byteOffset = vertexSize;
			ie.format = DXGI_FORMAT_R32G32B32_FLOAT;
			ie.semanticIndex = 0;
			ie.semanticName = "NORMAL";
			inputLayout.push_back(ie);
		}

		vertexSize += sizeof(Netcode::Float3);

		for(int i = 0; mesh->HasTextureCoords(i); ++i) {
			uint32_t uvwCount = mesh->mNumUVComponents[i];

			Intermediate::InputElement ie;
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
			Intermediate::InputElement ie;
			ie.byteOffset = vertexSize;
			ie.format = DXGI_FORMAT_R32G32B32_FLOAT;
			ie.semanticIndex = 0;
			ie.semanticName = "BINORMAL";
			inputLayout.push_back(ie);

			ie.byteOffset = vertexSize + sizeof(Netcode::Float3);
			ie.format = DXGI_FORMAT_R32G32B32_FLOAT;
			ie.semanticIndex = 0;
			ie.semanticName = "TANGENT";
			inputLayout.push_back(ie);
		}

		vertexSize += 2 * sizeof(Netcode::Float3);

		if(mesh->HasBones()) {
			Intermediate::InputElement ie;
			ie.byteOffset = vertexSize;
			ie.format = DXGI_FORMAT_R32G32B32_FLOAT;
			ie.semanticIndex = 0;
			ie.semanticName = "WEIGHTS";
			inputLayout.push_back(ie);
			ie.byteOffset = vertexSize + sizeof(Netcode::Float3);
			ie.format = DXGI_FORMAT_R32_UINT;
			ie.semanticIndex = 0;
			ie.semanticName = "BONEIDS";
			inputLayout.push_back(ie);
		}

		vertexSize += sizeof(Netcode::Float3);
		vertexSize += sizeof(uint32_t);

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

	static std::vector<Intermediate::Mesh> ImportMeshes(const aiScene * scene) {
		std::vector<Intermediate::Mesh> meshes;

		BoundingBoxGenerator boundingBoxGen;

		for(uint32_t i = 0; i < scene->mNumMeshes; ++i) {
			const aiMesh * mesh = scene->mMeshes[i];
			uint32_t numIndices = GetIndexCount(mesh);
			uint32_t numVertices = mesh->mNumVertices;
			auto [inputLayout, vertexStride] = GetInputLayout(mesh);

			Intermediate::LOD lod0;
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

				Netcode::Float3 * positionPtr = reinterpret_cast<Netcode::Float3 *>(vPtr + byteOffset);
				if(mesh->HasPositions()) {
					positionPtr->x = mesh->mVertices[vertexIter].x;
					positionPtr->y = mesh->mVertices[vertexIter].y;
					positionPtr->z = mesh->mVertices[vertexIter].z;

					boundingBoxGen.UpdateForPoint(*positionPtr);
				} else {
					*positionPtr = Netcode::Float3::Zero;
				}

				byteOffset += sizeof(Netcode::Float3);

				Netcode::Float3 * normalPtr = reinterpret_cast<Netcode::Float3 *>(vPtr + byteOffset);
				if(mesh->HasNormals()) {
					normalPtr->x = mesh->mNormals[vertexIter].x;
					normalPtr->y = mesh->mNormals[vertexIter].y;
					normalPtr->z = mesh->mNormals[vertexIter].z;
				} else {
					*normalPtr = Netcode::Float3::Zero;
				}

				byteOffset += sizeof(Netcode::Float3);

				for(int texCoordIter = 0; mesh->HasTextureCoords(texCoordIter); ++texCoordIter) {
					float * texCoordPtr = reinterpret_cast<float *>(vPtr + byteOffset);
					uint32_t uvwCount = mesh->mNumUVComponents[texCoordIter];

					for(uint32_t uvwIter = 0; uvwIter < uvwCount; ++uvwIter) {
						*texCoordPtr++ = mesh->mTextureCoords[texCoordIter][vertexIter][uvwIter];
					}
				}

				byteOffset += sizeof(Netcode::Float2);

				Netcode::Float3 * tangent = reinterpret_cast<Netcode::Float3 *>(vPtr + byteOffset);
				Netcode::Float3 * binormal = reinterpret_cast<Netcode::Float3 *>(vPtr + byteOffset + sizeof(Netcode::Float3));

				if(mesh->HasTangentsAndBitangents()) {
					tangent->x = mesh->mTangents[vertexIter].x;
					tangent->y = mesh->mTangents[vertexIter].y;
					tangent->z = mesh->mTangents[vertexIter].z;

					binormal->x = mesh->mBitangents[vertexIter].x;
					binormal->y = mesh->mBitangents[vertexIter].y;
					binormal->z = mesh->mBitangents[vertexIter].z;
				} else {
					*tangent = Netcode::Float3::Zero;
					*binormal = Netcode::Float3::Zero;
				}

				byteOffset += 2 * sizeof(Netcode::Float3);

				float * weightsPtr = reinterpret_cast<float *>(vPtr + byteOffset);
				uint32_t * boneIdsPtr = reinterpret_cast<uint32_t *>(vPtr + byteOffset + 3 * sizeof(float));
				*weightsPtr++ = 0;
				*weightsPtr++ = 0;
				*weightsPtr++ = 0;
				*boneIdsPtr = 0xFFFFFFFF;
				boneDataByteOffset = byteOffset;
				byteOffset += sizeof(Netcode::Float3);
				byteOffset += sizeof(uint32_t);
			}

			Intermediate::Mesh importedMesh;
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

	static Intermediate::Skeleton CreateSkeleton(const Intermediate::Model & partiallyImportedModel, const aiScene * scene) {
		Intermediate::Skeleton skeleton;

		std::vector<Intermediate::Bone> uniqueImportedBones;
	
		for(const Intermediate::Mesh & importedMesh : partiallyImportedModel.meshes) {
			if(uniqueImportedBones.size() == 0) {
				uniqueImportedBones = importedMesh.bones;
			} else {
				for(const auto & bone : importedMesh.bones) {
					auto it = std::find_if(std::begin(uniqueImportedBones), std::end(uniqueImportedBones), [&bone](const Intermediate::Bone & b) -> bool {
						return b.boneName == bone.boneName;
					});

					if(it == std::end(uniqueImportedBones)) {
						OutputDebugStringW(L"error, meshes have different skeletons associated to them\r\n");
						throw std::exception();
					}
				}
			}
		}

		std::vector<Intermediate::SkeletonBone> skeletonBones;

		for(const Intermediate::Bone & bone : uniqueImportedBones) {
			const aiNode * node = GetNodeByName(scene, bone.boneName);

			Intermediate::SkeletonBone skBone;
			skBone.boneName = bone.boneName;
			skBone.parentIndex = -1;
			skBone.transform = bone.transform;

			// must find a node in the parent hierarchy which is an actual bone
			const aiNode * parentBoneNode = node->mParent;
			bool parentBoneFound = false;

			while(!parentBoneFound && parentBoneNode != nullptr) {
				for(const Intermediate::Bone & b : uniqueImportedBones) {
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
			for(const Intermediate::SkeletonBone & placedSkeletonBone : skeletonBones) {
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

	static void FillVertexWeights(uint8_t * vPtr, uint32_t vertexStride, uint32_t boneDataByteOffset, const Intermediate::Mesh & importedMesh) {
		uint8_t boneIndex = 0;
		for(const Intermediate::Bone & bone : importedMesh.bones) {
			for(const Intermediate::VertexWeight & weight : bone.envelope) {
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

	static void FillVertexWeights(const Intermediate::Model & model) {

		for(const Intermediate::Mesh & importedMesh : model.meshes) {

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

			for(const Intermediate::InputElement & inputElement : importedMesh.inputLayout) {
				if(inputElement.semanticName == "WEIGHTS") {
					boneDataByteOffset = inputElement.byteOffset;
				}
			}

			for(const Intermediate::LOD & lod : importedMesh.lods) {
				uint8_t * vPtr = lod.vertexData.get();
				FillVertexWeights(vPtr, vertexStride, boneDataByteOffset, importedMesh);
			}
		}
	}

	static Netcode::MaterialType GetMaterialType(const aiMaterial * mat) {
		//aiShadingMode mode;
		//if(mat->Get(AI_MATKEY_SHADING_MODEL, mode) != AI_SUCCESS) {
		//	return Netcode::MaterialType::BRDF;
		//}
		return Netcode::MaterialType::BRDF;
	}

	static Ref<Netcode::Material> ImportMaterial(const aiMaterial * mat) {
		aiString str;
		std::string name;
		if(AI_SUCCESS == mat->Get(AI_MATKEY_NAME, str)) {
			name = str.C_Str();
		}
		Netcode::MaterialType type = GetMaterialType(mat);

		Ref<Netcode::Material> imat = std::make_shared<Netcode::BrdfMaterial>(type, name);

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

		char buffer[256];

		for(unsigned int i = 0; i < mat->mNumProperties; ++i) {
			const aiMaterialProperty * prop = mat->mProperties[i];
			sprintf_s(buffer, "key: %s | idx: %d idx2: %d\r\n", prop->mKey.C_Str(), prop->mIndex, prop->mType);

			OutputDebugStringA(buffer);
			//$raw.ShininessExponent|file
			//$raw.DiffuseColor|file
			switch(prop->mType) {
				case aiPropertyTypeInfo::aiPTI_String:
				{
					aiString & str2 = *reinterpret_cast<aiString *>(prop->mData);
					sprintf_s(buffer, "\t (string[%d]): %s\r\n", prop->mDataLength, str2.C_Str());
				}
				break;
				case aiPropertyTypeInfo::aiPTI_Integer:
				{
					int & v = *reinterpret_cast<int *>(prop->mData);
					sprintf_s(buffer, "\t    (int[%d]): %d\r\n", prop->mDataLength / 4, v);
				}
				break;
				case aiPropertyTypeInfo::aiPTI_Float:
				{
					float * f = reinterpret_cast<float *>(prop->mData);

					switch(prop->mDataLength) {
						case 4:
							sprintf_s(buffer, "\t  (float[%d]): %f\r\n", 1, *f);
							break;
						case 8:
							sprintf_s(buffer, "\t  (float[%d]): %f %f\r\n", 2, f[0], f[1]);
							break;
						case 12:
							sprintf_s(buffer, "\t  (float[%d]): %f %f %f\r\n", 3, f[0], f[1], f[2]);
							break;
						case 16:
							sprintf_s(buffer, "\t  (float[%d]): %f %f %f %f\r\n", 4, f[0], f[1], f[2], f[3]);
							break;
					}
				}
				break;
				default:break;
			}

			OutputDebugStringA(buffer);
		}

		if(diffuseCount > 0) {
			if(diffuseCount > 1) {
				OutputDebugStringW(L"Warning: only 1 diffuse map is supported as of now");
			}

			aiString texPath;

			if(AI_SUCCESS == mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath)) {
				std::string s = texPath.C_Str();
				imat->SetParameter(Netcode::MaterialParamId::TEXTURE_DIFFUSE_PATH, Netcode::Utility::ToWideString(s));
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
				std::string s = texPath.C_Str();
				imat->SetParameter(Netcode::MaterialParamId::TEXTURE_NORMAL_PATH, Netcode::Utility::ToWideString(s));
			} else {
				OutputDebugStringW(L"Warning: failed to get diffuse texture path\r\n");
			}
		}

		aiColor3D diffuseColor{ 0.5f, 0.5f, 0.5f };
		mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
		Netcode::Float4 ncDiffuseColor{ diffuseColor.r, diffuseColor.g, diffuseColor.b, 1.0f };
		imat->SetParameter(Netcode::MaterialParamId::DIFFUSE_ALBEDO, ncDiffuseColor);
		imat->SetParameter(Netcode::MaterialParamId::SPECULAR_ALBEDO, Netcode::Float3{ 0.05f, 0.05f, 0.05f });
		imat->SetParameter(Netcode::MaterialParamId::REFLECTANCE, 0.1f);
		imat->SetParameter(Netcode::MaterialParamId::METAL_MASK, false);
		imat->SetParameter(Netcode::MaterialParamId::TEXTURE_TILES, Netcode::Float2::One);
		imat->SetParameter(Netcode::MaterialParamId::TEXTURE_TILES_OFFSET, Netcode::Float2::Zero);
		imat->SetParameter(Netcode::MaterialParamId::DISPLACEMENT_SCALE, 0.01f);
		imat->SetParameter(Netcode::MaterialParamId::DISPLACEMENT_BIAS, 0.42f);
		imat->SetParameter(Netcode::MaterialParamId::TEXTURE_FLAGS, 0);

		float shininess = 0.0f;
		mat->Get(AI_MATKEY_SHININESS, shininess);
		imat->SetParameter(Netcode::MaterialParamId::ROUGHNESS, std::clamp((256.0f - shininess) / 256.0f, 0.0f, 1.0f));

		return imat;
	}

	static std::vector<Ref<Netcode::Material>> ImportMaterials(const aiScene * scene) {
		std::vector<Ref<Netcode::Material>> materials;

		for(uint32_t i = 0; i < scene->mNumMaterials; ++i) {
			materials.push_back(ImportMaterial(scene->mMaterials[i]));
		}

		return materials;
	}

	static Intermediate::Model ImportModel(const aiScene * scene) {
		Intermediate::Model importedModel;

		importedModel.offlineTransform = Netcode::Float4x4::Identity;

		if(scene != nullptr) {
			importedModel.meshes = ImportMeshes(scene);
		}

		importedModel.skeleton = CreateSkeleton(importedModel, scene);

		FillVertexWeights(importedModel);

		importedModel.materials = ImportMaterials(scene);

		return importedModel;
	}

	Intermediate::Model FBXImporter::FromFile(const std::string & file) {
		Assimp::Importer importer;

		uint32_t flags = aiProcess_Triangulate | aiProcess_FlipWindingOrder | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes;

		const aiScene * scene = importer.ReadFile(file, flags);

		if(scene == nullptr) {
			const char * err = importer.GetErrorString();
			OutputDebugStringA(err);
		}

		return ImportModel(scene);
	}

	Intermediate::Model FBXImporter::FromMemory(const uint8_t * source, uint32_t sizeInBytes)
	{
		Assimp::Importer importer;

		uint32_t flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipWindingOrder | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes;

		const aiScene * scene = importer.ReadFileFromMemory(source, sizeInBytes, flags, ".fbx");

		return ImportModel(scene);
	}


	static std::vector<Intermediate::Animation> ImportAnimations(const aiScene * scene, const Intermediate::Skeleton & skeleton) {
		std::vector<Intermediate::Animation> animations;

		for(uint32_t i = 0; i < scene->mNumAnimations; ++i) {

			const aiAnimation * anim = scene->mAnimations[i];

			Intermediate::Animation importedAnimation;
			importedAnimation.duration = anim->mDuration;
			importedAnimation.framesPerSecond = anim->mTicksPerSecond;
			importedAnimation.name = anim->mName.C_Str();

			for(uint32_t boneIt = 0; boneIt < anim->mNumChannels; ++boneIt) {
				const aiNodeAnim * nodeAnim = anim->mChannels[boneIt];

				Intermediate::BoneAnimation importedBoneAnimation;
				importedBoneAnimation.BoneId = skeleton.GetBoneIndex(nodeAnim->mNodeName.C_Str());
				importedBoneAnimation.PostState = static_cast<Intermediate::AnimationEdge>(nodeAnim->mPostState);
				importedBoneAnimation.PreState = static_cast<Intermediate::AnimationEdge>(nodeAnim->mPreState);

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
					Intermediate::PositionKey pk;
					pk.position.x = nodeAnim->mPositionKeys[keyIt].mValue.x;
					pk.position.y = nodeAnim->mPositionKeys[keyIt].mValue.y;
					pk.position.z = nodeAnim->mPositionKeys[keyIt].mValue.z;

					pk.time = nodeAnim->mPositionKeys[keyIt].mTime;

					importedBoneAnimation.positionKeys.push_back(pk);
				}

				for(uint32_t keyIt = 0; keyIt < nodeAnim->mNumRotationKeys; ++keyIt) {
					Intermediate::RotationKey rk;
					rk.rotation.x = nodeAnim->mRotationKeys[keyIt].mValue.x;
					rk.rotation.y = nodeAnim->mRotationKeys[keyIt].mValue.y;
					rk.rotation.z = nodeAnim->mRotationKeys[keyIt].mValue.z;
					rk.rotation.w = nodeAnim->mRotationKeys[keyIt].mValue.w;

					rk.time = nodeAnim->mRotationKeys[keyIt].mTime;

					importedBoneAnimation.rotationKeys.push_back(rk);
				}

				for(uint32_t keyIt = 0; keyIt < nodeAnim->mNumScalingKeys; ++keyIt) {
					Intermediate::ScaleKey sk;
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

	std::vector<Intermediate::Animation> FBXImporter::ImportAnimationsFromFile(const std::string & file, const Intermediate::Skeleton & skeleton) {

		Assimp::Importer importer;

		uint32_t flags = aiProcess_Triangulate | aiProcess_FlipWindingOrder | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes;

		const aiScene * scene = importer.ReadFile(file, flags);

		return ImportAnimations(scene, skeleton);
	}

	std::vector<Intermediate::Animation> FBXImporter::ImportAnimationsFromMemory(const uint8_t * source, uint32_t sizeInBytes, const Intermediate::Skeleton & skeleton) {
	
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

	static double GetNextTimeKey(const std::vector<Intermediate::BoneAnimation> & boneData, double currentTimeKey) {
		double candidate = std::numeric_limits<double>::infinity();
		for(const Intermediate::BoneAnimation & boneAnim : boneData) {
			for(const Intermediate::PositionKey & pk : boneAnim.positionKeys) {
				if(StrictlyGreater(pk.time, currentTimeKey)) {
					if(StrictlyLess(pk.time, candidate)) {
						candidate = pk.time;
					}
					break;
				}
			}

			for(const Intermediate::RotationKey & rk : boneAnim.rotationKeys) {
				if(StrictlyGreater(rk.time, currentTimeKey)) {
					if(StrictlyLess(rk.time, candidate)) {
						candidate = rk.time;
					}
					break;
				}
			}

			for(const Intermediate::ScaleKey & sk : boneAnim.scaleKeys) {
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

	static Netcode::Float3 SamplePosition(const std::vector<Intermediate::PositionKey> & keys, double t) {
		if(keys.empty()) {
			return Netcode::Float3::Zero;
		}

		if(keys.size() == 1) {
			return keys[0].position;
		}

		int indexEnd = 0;
		for(const Intermediate::PositionKey & pk : keys) {
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

		Netcode::Vector3 endV = keys.at(indexEnd).position;
		Netcode::Vector3 startV = keys.at(indexEnd - 1).position;

		return Netcode::Vector3::Lerp(startV, endV, alphaF);
	}

	static Netcode::Float4 SampleRotation(const std::vector<Intermediate::RotationKey> & keys, double t) {
		if(keys.empty()) {
			return Netcode::Float4(0.0f, 0.0f, 0.0f, 1.0f);
		}

		if(keys.size() == 1) {
			return keys[0].rotation;
		}

		int indexEnd = 0;
		for(const Intermediate::RotationKey & pk : keys) {
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

		const Netcode::Quaternion endV = keys.at(indexEnd).rotation;
		const Netcode::Quaternion startV = keys.at(indexEnd - 1).rotation;

		return Netcode::Quaternion::Slerp(startV, endV, alphaF);
	}

	static Netcode::Float3 SampleScale(const std::vector<Intermediate::ScaleKey> & keys, double t) {
		if(keys.empty()) {
			return Netcode::Float3(1.0f, 1.0f, 1.0f);
		}

		if(keys.size() == 1) {
			return keys[0].scale;
		}

		int indexEnd = 0;
		for(const Intermediate::ScaleKey & pk : keys) {
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

		const Netcode::Vector3 endV = keys.at(indexEnd).scale;
		const Netcode::Vector3 startV = keys.at(indexEnd - 1).scale;

		return Netcode::Vector3::Lerp(startV, endV, alphaF);
	}

	Intermediate::OptimizedAnimation FBXImporter::OptimizeAnimation(const Intermediate::Animation & anim, const Intermediate::Skeleton & skeleton) {
		Intermediate::OptimizedAnimation optAnim;

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
			Intermediate::OptimizedBoneAnimation optBoneAnim;
			optBoneAnim.boneData.resize(skeleton.bones.size());

			// set defaults
			for(size_t i = 0; i < skeleton.bones.size(); ++i) {
				optBoneAnim.boneData[i].position = Netcode::Float4{ 0.0f, 0.0f, 0.0f, 1.0f };
				optBoneAnim.boneData[i].rotation = Netcode::Float4{ 0.0f, 0.0f, 0.0f, 1.0f };
				optBoneAnim.boneData[i].scale = Netcode::Float4{ 1.0f, 1.0f, 1.0f, 0.0f };
			}

			for(const Intermediate::BoneAnimation & bA : anim.boneAnimations) {
				if(bA.BoneId >= 0) {
					DirectX::XMFLOAT3 posSample = SamplePosition(bA.positionKeys, t);
					DirectX::XMFLOAT3 scaleSample = SampleScale(bA.scaleKeys, t);

					optBoneAnim.boneData[bA.BoneId].position = Netcode::Float4{ posSample.x, posSample.y, posSample.z, 1.0f };
					optBoneAnim.boneData[bA.BoneId].rotation = SampleRotation(bA.rotationKeys, t);
					optBoneAnim.boneData[bA.BoneId].scale = Netcode::Float4{ scaleSample.x, scaleSample.y, scaleSample.z, 0.0f };
				}
			}

			optAnim.keys.push_back(std::move(optBoneAnim));
		}

		return optAnim;
	}

}
