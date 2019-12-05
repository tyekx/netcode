#include <cstdio>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>

#include <Egg/Path.h>
#include <Egg/Common.h>
#include <Egg/Utility.h>
#include <Egg/ProgramArgs.h>
#include <Egg/Vertex.h>
#include <Egg/Exporter.h>
#include <Egg/Importer.h>
#include <Egg/Asset/Animation.h>
#include <Egg/LinearClassifier.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <shellapi.h>

void ToLower(std::wstring & str) {
	std::transform(str.begin(), str.end(), str.begin(), std::tolower);
}

void PrintHelp() {
	printf("Usage(1): $ EggAssetPipeline --name=<string> --lods=<file>[,<file>,...] --animations=[<file>,...] [--generate_tangent_space]\r\n");
	printf("Usage(2): $ EggAssetPipeline --manifest=<file>\r\n");
	printf("Usage(3): $ EggAssetPipeline\r\n\r\n");

	printf("\t(2): will load the expected arguments from a file, the arguments follow the (1) pattern in the manifest file\r\n");
	printf("\t(3): will try to load the default manifest.txt file in the working directory\r\n\r\n");
	
	printf("Parameters:\r\n");
	printf("\t	  --name				   : Name of the output file\r\n");
	printf("\t    --lods				   : Different level of details for the same model, in a descending LOD order");
	printf("\t    --animations			   : List of animations to be used for the mesh");
	printf("\t    --generate_tangent_space : Tangents and binormals will be calculated for each mesh\r\n");
	printf("\t	  --manifest               : Loads the program arguments from a manifest file\r\n");
}

struct ImportedAnimationKey {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 rotation;
	DirectX::XMFLOAT3 scale;
	double time;
};

struct BoneAnimation {
	int BoneId;

	Egg::Asset::AnimationEdge PreState;
	Egg::Asset::AnimationEdge PostState;

	std::vector<ImportedAnimationKey> Keys;
};

struct Animation {

	std::string name;

	double Duration;
	double TicksPerSecond;

	std::vector<BoneAnimation> BoneData;

	unsigned int bonesLength;
	unsigned int keysLength;

	/* these values are for transforming the layout of boneData */
	std::vector<Egg::Asset::AnimationEdge> PreStates;
	std::vector<Egg::Asset::AnimationEdge> PostStates;

	std::vector<double> Times;

	std::vector<Egg::Asset::AnimationKey> Keys;

};

struct ImportedBone {
	std::string name;
	aiMatrix4x4 offsetMatrix;
	uint32_t numVertexWeights;
	aiVertexWeight * vertexWeights;
};

struct ImportedLOD {
	unsigned int vertexCount;
	unsigned int indexCount;
	std::vector<unsigned char> vertices;
	std::vector<uint32_t> indices;
	std::vector<ImportedBone> bones;
};

struct ImportedMesh {
	uint32_t materialIndex;
	unsigned int vertexType;
	unsigned int vertexSize;
	DirectX::BoundingBox boundingBox;
	std::vector<ImportedLOD> lods;
};

struct ImportedMaterial {
	aiColor3D diffuseColor;
	aiColor3D ambientColor;
	aiColor3D specularColor;
	float shininess;
	std::string diffuseTexPath;
	std::string normalTexPath;
};

struct ImportedModel {
	std::vector<ImportedMaterial> materials;
	std::vector<ImportedMesh> meshes;
};

struct ProcessedBone {
	int parentIndex;
	aiMatrix4x4 offsetMatrix;
	std::string name;
};

std::string ExtractFileName(const std::string & src) {
	std::size_t indexOfDot = src.find_last_of('.');

	ASSERT(indexOfDot != std::string::npos, "Failed to extract filename");

	std::string fixedSlashes = src;
	std::replace(fixedSlashes.begin(), fixedSlashes.end(), '\\', '/');

	std::string preExtension = fixedSlashes.substr(0, indexOfDot);
	std::size_t indexOfSlash = preExtension.find_last_of('/');

	if(indexOfSlash == std::string::npos) {
		return preExtension;
	}

	return preExtension.substr(indexOfSlash + 1);
}

Egg::Asset::AnimationEdge ToAnimationState(aiAnimBehaviour behaviour) {
	switch(behaviour) {
		case aiAnimBehaviour_DEFAULT: return Egg::Asset::AnimationEdge::DEFAULT;
		case aiAnimBehaviour_CONSTANT: return Egg::Asset::AnimationEdge::CONSTANT;
		case aiAnimBehaviour_LINEAR: return Egg::Asset::AnimationEdge::LINEAR;
		case aiAnimBehaviour_REPEAT: return Egg::Asset::AnimationEdge::REPEAT;
		default: throw 1;
	}
}

void Explode(std::vector<std::wstring> & dst, const std::wstring & src, wchar_t delim) {

	std::wstringstream ss{ src };

	while(ss.good()) {
		std::wstring substr;
		std::getline(ss, substr, delim);
		dst.push_back(substr);
	}

}

void Process(const wchar_t * file, std::vector<ProcessedBone> & bones, ImportedModel & model, bool generateTangentSpace);
void ProcessAnimation(const wchar_t * file, std::vector<ProcessedBone> & bones, std::vector<Animation> & anims);
void WriteBinary(const char * dest, ImportedModel & model, std::vector<ProcessedBone> & bones, std::vector<Animation> & anims);
void CalculateBoundingBoxes(ImportedModel & model);

int wmain(int argc, wchar_t ** argv) {
	Egg::ProgramArgs pa{ (const wchar_t**) argv, argc };

	if(argc == 1) {
		// try find manifest
		if(!Egg::Path::FileExists(L"manifest.txt")) {
			printf("Error: Manifest was not found\r\n");
			PrintHelp();
			return 1;
		} else {
			const wchar_t * defaultManifest = L"--manifest=manifest.txt";
			pa = Egg::ProgramArgs{ &defaultManifest, 1 };
		}
	}

	if(pa.IsSet(L"help")) {
		PrintHelp();
		return 0;
	}

	if(pa.IsSet(L"manifest")) {
		std::wstring manif = pa.GetArg(L"manifest");
		std::string buffer;
		if(!Egg::Path::FileExists(manif.c_str())) {
			printf("Error: manifest file was not found\r\n");
			return 1;
		}

		Egg::Utility::SlurpFile(buffer, manif);
		std::wstring wbuf = Egg::Utility::ToWideString(buffer);
		std::replace(wbuf.begin(), wbuf.end(), L'\n', L' ');
		std::replace(wbuf.begin(), wbuf.end(), L'\r', L' ');

		LPWSTR* argValues = CommandLineToArgvW(wbuf.c_str(), &argc);

		pa = Egg::ProgramArgs{ (const wchar_t**)argValues, argc };

		LocalFree(argValues);
	}

	if(!pa.IsSet(L"name")) {
		printf("--name argument is required\r\n");
		PrintHelp();
		return 1;
	}

	if(!pa.IsSet(L"lods")) {
		printf("--lods argument is required to have at least 1 reference");
		PrintHelp();
		return 1;
	}

	ImportedModel model;
	std::vector<ProcessedBone> bones;
	std::vector<Animation> anims;

	std::vector<std::wstring> lodFiles;
	std::vector<std::wstring> animationFiles;
	std::wstring name = pa.GetArg(L"name");

	Explode(lodFiles, pa.GetArg(L"lods"), L',');
	Explode(animationFiles, pa.GetArg(L"animations"), L',');

	for(const std::wstring & i : lodFiles) {
		Process(i.c_str(), bones, model, pa.IsSet(L"generate_tangent_space"));
		CalculateBoundingBoxes(model);
	}

	for(const std::wstring & i : animationFiles) {
		ProcessAnimation(i.c_str(), bones, anims);
	}

	name += L".eggasset";

	std::string binaryName = Egg::Utility::ToNarrowString(name);

	WriteBinary(binaryName.c_str(), model, bones, anims);


	return 0;
}

DirectX::XMFLOAT3 ComponentWiseMin(const DirectX::XMFLOAT3 & a, const DirectX::XMFLOAT3 & b) {
	return DirectX::XMFLOAT3{ std::min(a.x,b.x), std::min(a.y,b.y), std::min(a.z,b.z) };
}

DirectX::XMFLOAT3 ComponentWiseMax(const DirectX::XMFLOAT3 & a, const DirectX::XMFLOAT3 & b) {
	return DirectX::XMFLOAT3{ std::max(a.x,b.x), std::max(a.y,b.y), std::max(a.z,b.z) };
}

void CalculateBoundingBoxes(ImportedModel & model) {
	for(auto & mesh : model.meshes) {

		DirectX::XMFLOAT3 maxVec{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
		DirectX::XMFLOAT3 minVec{ FLT_MAX, FLT_MAX, FLT_MAX };

		for(auto & lod : mesh.lods) {
			uint8_t * meshPtr = &(lod.vertices.at(0));
			// would probably be better to use the index buffer if there is one
			for(unsigned int i = 0; i < lod.vertexCount; ++i) {
				Egg::PNT_Vertex * vert = reinterpret_cast<Egg::PNT_Vertex *>(meshPtr);

				maxVec = ComponentWiseMax(maxVec, vert->position);
				minVec = ComponentWiseMin(minVec, vert->position);

				meshPtr += mesh.vertexSize;
			}
		}

		/*
		Idea: once we have the 2 edge-case points, the 2 imaginary point (not neccesairly a vertex), furthest each other,
		then we center it.
		Let A,B vectors, M mesh. No m point from the mesh has a smaller component than point A(x,y,z). B is the same but
		is the greatest of any vertices component.
		If A,B satisfies this, then (A+B)/2 will be the center of the bounding box, and the extents are calculated in
		a way that | B - A | / 2, where the | B - A | is componentwise absolute value and not length. This will ensure the
		tightest bounding box possible
		*/

		DirectX::XMVECTOR maxV = DirectX::XMLoadFloat3(&maxVec);
		DirectX::XMVECTOR minV = DirectX::XMLoadFloat3(&minVec);

		DirectX::XMVECTOR diffVec = DirectX::XMVectorSubtract(maxV, minV);

		DirectX::XMVECTOR len = DirectX::XMVector3Length(diffVec);

		DirectX::XMVECTOR posVector = DirectX::XMVectorScale(DirectX::XMVectorAdd(minV, maxV), 0.5f);
		DirectX::XMVECTOR extentsVector = DirectX::XMVectorScale(DirectX::XMVectorAbs(diffVec), 0.5f);

		DirectX::BoundingBox boundingBox{ };
		DirectX::XMStoreFloat3(&boundingBox.Center, posVector);
		DirectX::XMStoreFloat3(&boundingBox.Extents, extentsVector);

		mesh.boundingBox = boundingBox;

	}
}

const ImportedBone* IsReferenced(const char* boneName, const ImportedModel & im) {
	for(const ImportedMesh & m : im.meshes) {
		int i = 0;
		for(const ImportedBone & ib : m.lods[0].bones) {
			if(ib.name == boneName) {
				return &(m.lods[0].bones.at(i));
			}
			i++;
		}
	}
	return false;
}

const char * GetParentName(aiNode * node) {
	if(node == nullptr) {
		return nullptr;
	}

	if(node->mName.data[0] != '\0') {
		return node->mName.data;
	}

	return GetParentName(node->mParent);
}

int GetIndex(const std::vector<ProcessedBone> & haystack, const char * needle) {
	int i = 0;
	for(const ProcessedBone & b : haystack) {
		if(b.name == needle) {
			return i;
		}
		i++;
	}
	return -1;
}

void ProcessBones(std::vector<ProcessedBone> & dst, const ImportedModel & model, aiNode* node) {
	if(node->mName.data[0] != '\0') {
		const ImportedBone * refBone = IsReferenced(node->mName.C_Str(), model);
		if(refBone != nullptr) {
			printf("parentName: %s\r\n", GetParentName(node->mParent));
			ProcessedBone b;
			b.name = node->mName.C_Str();
			b.parentIndex = GetIndex(dst, GetParentName(node->mParent));
			b.offsetMatrix = refBone->offsetMatrix;

			if(node->mParent != nullptr && b.parentIndex == -1) {
				//non animated bone parent, need to get the transformation
				if(!node->mParent->mTransformation.IsIdentity()) {
					b.offsetMatrix = refBone->offsetMatrix * node->mParent->mTransformation;
				}
			}

			dst.push_back(b);
		} else {
			printf("Bone not referenced: %s\r\n", node->mName.C_Str());
		}
	}

	for(uint32_t i = 0; i < node->mNumChildren; ++i) {
		ProcessBones(dst, model, node->mChildren[i]);
	}
}

int FirstUnusedSlot(const int * src) {
	for(int i = 0; i < 4; ++i) {
		if(src[i] == 0) {
			return i;
		}
	}
	return -1;
}

int GetBoneIndex(const std::vector<ProcessedBone> & bones, const char * name) {
	int i = 0;
	for(const ProcessedBone & b : bones) {
		if(b.name == name) {
			return i;
		}
		++i;
	}
	return -1;
}

int & IndexInt4(DirectX::XMINT4 & v, unsigned int i) {
	switch(i) {
		case 0: return v.x;
		case 1: return v.y;
		case 2: return v.z;
		case 3: return v.w;
		default: throw 1;
	}
}

float & IndexFloat3(DirectX::XMFLOAT3 & v, unsigned int i) {
	switch(i) {
	case 0: return v.x;
	case 1: return v.y;
	case 2: return v.z;
	default: throw 1;
	}
}

void FillVertexWeights(ImportedModel & im, std::vector<ProcessedBone> & bones) {
	for(ImportedMesh & m : im.meshes) {
		for(ImportedLOD & lod : m.lods) {
			for(ImportedBone & ib : lod.bones) {
				for(uint32_t vi = 0; vi < ib.numVertexWeights; ++vi) {
					void * vPtr = &(lod.vertices.at(ib.vertexWeights[vi].mVertexId * m.vertexSize));
					Egg::PNTWB_Vertex * vert = reinterpret_cast<Egg::PNTWB_Vertex *>(vPtr);

					int k = FirstUnusedSlot(reinterpret_cast<int *>(&(vert->boneIds)));
					int l = GetBoneIndex(bones, ib.name.c_str());

					if(k == -1 || l == -1) {
						continue;
					}

					IndexInt4(vert->boneIds, k) = l;

					if(k < 3 && k >= 0) {
						IndexFloat3(vert->weights, k) = ib.vertexWeights[vi].mWeight;
					}
				}
			}
		}
	}
}

unsigned int GetMeshSignature(aiMesh * mesh) {
	unsigned int meshSignature = 0;
	if(mesh->HasPositions()) {
		meshSignature |= 1U;
	}

	if(mesh->HasNormals()) {
		meshSignature |= 2;
	}

	if(mesh->HasTextureCoords(0)) {
		meshSignature |= 4U;
	}

	if(mesh->HasBones()) {
		meshSignature |= 8U;
	}

	if(mesh->HasTangentsAndBitangents()) {
		meshSignature |= 16U;
	}

	return meshSignature;
}

template<typename T>
void SetVertexInfo(unsigned int & type, unsigned int & size) {
	type = T::type;
	size = sizeof(T);
}

void Process(const wchar_t * file, std::vector<ProcessedBone> & bones, ImportedModel & model, bool generateTangentSpace) {
	std::string path = Egg::Utility::ToNarrowString(file);

	Assimp::Importer importer;

	unsigned int flags = (generateTangentSpace) ? aiProcess_CalcTangentSpace : 0;
	flags |= aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights | aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_ImproveCacheLocality;

	const aiScene * scene = importer.ReadFile(path.c_str(), flags);

	ASSERT(scene != nullptr, "Failed to load obj file: '%s'. Assimp error message: '%s'", path.c_str(), importer.GetErrorString());

	ASSERT(scene->HasMeshes(), "FBX file: '%s' does not contain a mesh.", path.c_str());

	for(unsigned int i = 0; i < scene->mMetaData->mNumProperties; ++i) {

		auto & key = scene->mMetaData->mKeys[i];
		auto & value = scene->mMetaData->mValues[i];

		switch(value.mType) {
			case aiMetadataType::AI_AIVECTOR3D:
				{
					aiVector3D v;
					scene->mMetaData->Get(i, v);
					Egg::Utility::Debugf("%s: %f %f %f\r\n", key.C_Str(), v.x, v.y, v.z);
				}
				break;
			case aiMetadataType::AI_AISTRING:
				{
					aiString v;
					scene->mMetaData->Get<aiString>(i, v);
					Egg::Utility::Debugf("%s: %s\r\n", key.C_Str(), v.C_Str());
				}
				break;
			case aiMetadataType::AI_BOOL:
				{
					bool b;
					scene->mMetaData->Get(i, b);
					Egg::Utility::Debugf("%s: %d (bool)\r\n", key.C_Str(), (int)b);
				}
				break;
			case aiMetadataType::AI_INT32:
				{
					int v;
					scene->mMetaData->Get(i, v);
					Egg::Utility::Debugf("%s: %d (int)\r\n", key.C_Str(), v);
				}
				break;
			case aiMetadataType::AI_FLOAT:
				{
					float f;
					scene->mMetaData->Get(i, f);
					Egg::Utility::Debugf("%s: %f (float)\r\n", key.C_Str(), f);
				}
				break;
			case aiMetadataType::AI_DOUBLE:
				{
					double f;
					scene->mMetaData->Get(i, f);
					Egg::Utility::Debugf("%s: %lf (double)\r\n", key.C_Str(), f);
				}
				break;
			case aiMetadataType::AI_UINT64:
				{
					unsigned long long f;
					scene->mMetaData->Get(i, f);
					Egg::Utility::Debugf("%s: %ld (ULL)\r\n", key.C_Str(), f);
				}
				break;

		}

	}

	uint32_t meshCount = scene->mNumMeshes;
	uint32_t animCount = scene->mNumAnimations;
	uint32_t texCount = scene->mNumTextures;
	uint32_t matCount = scene->mNumMaterials;

	bool isLodLevel = true;
	if(model.meshes.size() == 0) {
		isLodLevel = false;
		model.meshes.reserve(meshCount);
		model.materials.reserve(matCount);
	} else {
		ASSERT(model.meshes.size() == meshCount, "A lod level must have the same amount of meshes");
	}


	for(uint32_t i = 0; i < meshCount; ++i) {
		Egg::Utility::Debugf("boneCount: %d\r\n", scene->mMeshes[i]->mNumBones);
		aiMesh * mesh = scene->mMeshes[i];

		unsigned int vType;
		unsigned int vSize;
		unsigned int meshSignature = GetMeshSignature(mesh);

		switch(meshSignature) {
		case 0b00111:
			SetVertexInfo<Egg::PNT_Vertex>(vType, vSize);
			break;
		case 0b01111:
			SetVertexInfo<Egg::PNTWB_Vertex>(vType, vSize);
			break;
		case 0b10111:
			SetVertexInfo<Egg::PNTTB_Vertex>(vType, vSize);
			break;
		case 0b11111:
			SetVertexInfo<Egg::PNTWBTB_Vertex>(vType, vSize);
			break;
		default:
			printf("Error: '%s': mesh type not supported\r\n", path.c_str());
			exit(1);
		}

		/* processing PNT mesh, just the usual stuff */
		
		ImportedMesh source;
		source.vertexSize = vSize;
		source.vertexType = vType;

		ImportedMesh * im;
		if(isLodLevel) {
			im = &(model.meshes[i]);
		} else {
			im = &source;
		}

		ImportedLOD lod;
		lod.vertices.resize(mesh->mNumVertices * im->vertexSize);
		lod.bones.resize(mesh->mNumBones);
		lod.vertexCount = mesh->mNumVertices;

		for(int k = 0; k < mesh->mNumVertices; ++k) {
			unsigned char * ptr = &(lod.vertices.at(k * im->vertexSize));

			Egg::PNT_Vertex * evt = reinterpret_cast<Egg::PNT_Vertex *>(ptr);
			evt->position.x = mesh->mVertices[k].x;
			evt->position.y = mesh->mVertices[k].y;
			evt->position.z = mesh->mVertices[k].z;

			evt->normal.x = mesh->mNormals[k].x;
			evt->normal.y = mesh->mNormals[k].y;
			evt->normal.z = mesh->mNormals[k].z;

			evt->tex.x = mesh->mTextureCoords[0][k].x;
			evt->tex.y = mesh->mTextureCoords[0][k].y;

			switch(meshSignature) {
			case 0b01111:
				{
					Egg::PNTWB_Vertex * wbVertex = reinterpret_cast<Egg::PNTWB_Vertex *>(ptr);
					wbVertex->weights = DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f };
					wbVertex->boneIds = DirectX::XMINT4{ 0, 0, 0, 0 };
				}
				break;
			case 0b10111:
				{
					Egg::PNTTB_Vertex * tbVertex = reinterpret_cast<Egg::PNTTB_Vertex *>(ptr);
					tbVertex->tangent = reinterpret_cast<DirectX::XMFLOAT3 &>(mesh->mTangents[k]);
					tbVertex->binormal = reinterpret_cast<DirectX::XMFLOAT3 &>(mesh->mBitangents[k]);
				}
				break;
			case 0b11111:
				{
					Egg::PNTWBTB_Vertex * wbtbVertex = reinterpret_cast<Egg::PNTWBTB_Vertex *>(ptr);
					wbtbVertex->weights = DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f };
					wbtbVertex->boneIds = DirectX::XMINT4{ 0, 0, 0, 0 };
					wbtbVertex->tangent = reinterpret_cast<DirectX::XMFLOAT3 &>(mesh->mTangents[k]);
					wbtbVertex->binormal = reinterpret_cast<DirectX::XMFLOAT3 &>(mesh->mBitangents[k]);
				}
				break;
			}
		}

		unsigned int nIndices = 0;
		for(int k = 0; k < mesh->mNumFaces; ++k) {
			if(mesh->mFaces[k].mNumIndices == 3) {
				nIndices += 3;
			}
		}
		lod.indices.resize(nIndices);
		lod.indexCount = nIndices;

		unsigned int * ptr = &(lod.indices.at(0));
		for(int k = 0; k < mesh->mNumFaces; ++k) {
			
			unsigned int n = mesh->mFaces[k].mNumIndices;
				
			if(n == 3) {
				ptr[0] = mesh->mFaces[k].mIndices[0];
				ptr[1] = mesh->mFaces[k].mIndices[1];
				ptr[2] = mesh->mFaces[k].mIndices[2];
				ptr += 3;
			}
		}

		im->materialIndex = mesh->mMaterialIndex;

		/* adding the bones, but will process it later on */
		for(uint32_t j = 0; j < mesh->mNumBones; ++j) {
			aiBone * b = mesh->mBones[j];
			
			lod.bones[j].name = b->mName.C_Str();
			lod.bones[j].numVertexWeights = b->mNumWeights;
			lod.bones[j].vertexWeights = b->mWeights;
			lod.bones[j].offsetMatrix = b->mOffsetMatrix;
		}

		if(!isLodLevel) {
			model.meshes.emplace_back(std::move(*im));
		}
		model.meshes[i].lods.push_back(std::move(lod));
	}

	ProcessBones(bones, model, scene->mRootNode);
	FillVertexWeights(model, bones);

	for(uint32_t i = 0; i < matCount; ++i) {
		aiMaterial * mat = scene->mMaterials[i];

		ImportedMaterial imat;
		aiString str;

		if(AI_SUCCESS == mat->Get(AI_MATKEY_NAME, str)) {
			Egg::Utility::Debugf("Prop: name | value '%s'\r\n", str.C_Str());
		}


		unsigned int diffuseCount = mat->GetTextureCount(aiTextureType_DIFFUSE);
		unsigned int normalCount = mat->GetTextureCount(aiTextureType_NORMALS);


		if(diffuseCount > 0) {
			if(diffuseCount > 1) {
				printf("Warning: only 1 diffuse map is supported as of now\r\n\tMultiple found on material: '%s'\r\n", str.C_Str());
			}

			aiString texPath;

			if(AI_SUCCESS == mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath)) {
				imat.diffuseTexPath = texPath.C_Str();
			} else {
				printf("Warning: failed to get diffuse texture path\r\n");
			}

		}

		if(normalCount > 0) {
			if(normalCount > 1) {
				printf("Warning: only 1 normal map is supported as of now\r\n\tMultiple found on material: '%s'\r\n", str.C_Str());
			}

			aiString texPath;
			if(AI_SUCCESS == mat->GetTexture(aiTextureType_NORMALS, 0, &texPath)) {
				imat.normalTexPath = texPath.C_Str();
			} else {
				printf("Warning: failed to get diffuse texture path\r\n");
			}
		}

		

		if(AI_SUCCESS != mat->Get(AI_MATKEY_COLOR_DIFFUSE, imat.diffuseColor)) {
			Egg::Utility::Debugf("Failed to get diffuse color\r\n");
			continue;
		}

		if(AI_SUCCESS != mat->Get(AI_MATKEY_COLOR_AMBIENT, imat.ambientColor)) {
			Egg::Utility::Debugf("Failed to get ambient color\r\n");
			continue;
		}

		if(AI_SUCCESS != mat->Get(AI_MATKEY_COLOR_SPECULAR, imat.specularColor)) {
			Egg::Utility::Debugf("Failed to get specular color\r\n");
			continue;
		}

		if(AI_SUCCESS != mat->Get(AI_MATKEY_SHININESS, imat.shininess)) {
			Egg::Utility::Debugf("Failed to get shininess\r\n");
			continue;
		}

		model.materials.push_back(imat);


	}

	return;
}

bool IsAnimationUniformInTime(aiNodeAnim * nodeAnim) {
	if(nodeAnim->mNumPositionKeys != nodeAnim->mNumRotationKeys || nodeAnim->mNumRotationKeys != nodeAnim->mNumScalingKeys) {
		return false;
	}

	for(uint32_t i = 1; i < nodeAnim->mNumPositionKeys; ++i) {
		double dp = nodeAnim->mPositionKeys[i].mTime - nodeAnim->mPositionKeys[i - 1].mTime;
		double dr = nodeAnim->mRotationKeys[i].mTime - nodeAnim->mRotationKeys[i - 1].mTime;
		double ds = nodeAnim->mScalingKeys[i].mTime - nodeAnim->mScalingKeys[i - 1].mTime;

		/*
		* No need to epsilon check this, should be bit perfect match
		*/
		if(dp != dr || dr != ds) {
			return false;
		}
	}

	return true;
}

void LinearizeAnimation(Animation & anim) {
	
	unsigned int bonesLength = (unsigned int) anim.BoneData.size();
	unsigned int keysLength = (unsigned int) anim.BoneData[0].Keys.size();

	anim.PreStates.resize(bonesLength);
	anim.PostStates.resize(bonesLength);
	anim.Times.resize(keysLength);
	anim.Keys.resize(bonesLength * keysLength);

	anim.keysLength = keysLength;
	anim.bonesLength = bonesLength;

	unsigned int bonesIter = 0;
	unsigned int keysIter = 0;

	for(auto & key : anim.BoneData[0].Keys) {
		anim.Times[keysIter] = key.time;
		++keysIter;
	}

	for(auto & boneAnim : anim.BoneData) {

		anim.PreStates[bonesIter] = boneAnim.PreState;
		anim.PostStates[bonesIter] = boneAnim.PostState;

		keysIter = 0;

		for(auto & animKey : boneAnim.Keys) {
			Egg::Asset::AnimationKey * ptr = &(anim.Keys.at(0));
			ptr += bonesLength * keysIter;
			ptr[bonesIter].position = animKey.position;
			ptr[bonesIter].rotation = animKey.rotation;
			ptr[bonesIter].scale = animKey.scale;
			++keysIter;
		}
		++bonesIter;
	}
}



double NextKeyframe(double prevKeyframe, aiAnimation * anim) {
	double candidate = 500000.0;

	for(unsigned int i = 0; i < anim->mNumChannels; ++i) {
		for(unsigned int j = 0; j < anim->mChannels[i]->mNumPositionKeys; ++j) {
			if((anim->mChannels[i]->mPositionKeys[j].mTime - prevKeyframe) > 0.001) {
				if((anim->mChannels[i]->mPositionKeys[j].mTime - candidate) < -0.001) {
					candidate = anim->mChannels[i]->mPositionKeys[j].mTime;
				}
			}
		}
	}

	return candidate;
}

int IsKeyframePresent(double tl, aiNodeAnim* anim) {
	for(unsigned int i = 0; i < anim->mNumPositionKeys; ++i) {
		if(abs(anim->mPositionKeys[i].mTime - tl) < 0.001) {
			return i;
		}
	}
	return -1;
}

ImportedAnimationKey ToImportedAnimKey(aiNodeAnim * anim, unsigned int keyId) {
	aiVectorKey * posKey = anim->mPositionKeys + keyId;
	aiQuatKey * rotKey = anim->mRotationKeys + keyId;
	aiVectorKey * scaleKey = anim->mScalingKeys + keyId;

	DirectX::XMFLOAT3 posStValue{ posKey->mValue.x, posKey->mValue.y, posKey->mValue.z };
	DirectX::XMFLOAT4 rotStValue{ rotKey->mValue.x, rotKey->mValue.y, rotKey->mValue.z, rotKey->mValue.w };
	DirectX::XMFLOAT3 scaleStValue{ scaleKey->mValue.x, scaleKey->mValue.y, scaleKey->mValue.z };

	ImportedAnimationKey ak;
	ak.position = posStValue;
	ak.rotation = rotStValue;
	ak.scale = scaleStValue;
	ak.time = posKey->mTime;

	return ak;
}

ImportedAnimationKey GenerateKey(double tl, aiNodeAnim * anim) {
	ImportedAnimationKey ak;
	for(unsigned int j = 0; j < anim->mNumPositionKeys; ++j) {
		double t = anim->mPositionKeys[j].mTime;

		if(t > tl) {
			ASSERT(j != 0, "Not implemented case\r\n");

			ImportedAnimationKey ak0 = ToImportedAnimKey(anim, j - 1);
			ImportedAnimationKey ak1 = ToImportedAnimKey(anim, j);

			double lerpArg = (tl - ak0.time) / (ak1.time - ak0.time);

			DirectX::XMVECTOR pos0 = DirectX::XMLoadFloat3(&ak0.position);
			DirectX::XMVECTOR pos1 = DirectX::XMLoadFloat3(&ak1.position);
			DirectX::XMStoreFloat3(&ak.position, DirectX::XMVectorLerp(pos0, pos1, lerpArg));

			DirectX::XMVECTOR quat0 = DirectX::XMLoadFloat4(&ak0.rotation);
			DirectX::XMVECTOR quat1 = DirectX::XMLoadFloat4(&ak1.rotation);
			DirectX::XMStoreFloat4(&ak.rotation, DirectX::XMQuaternionSlerp(quat0, quat1, lerpArg));

			DirectX::XMVECTOR scale0 = DirectX::XMLoadFloat3(&ak0.scale);
			DirectX::XMVECTOR scale1 = DirectX::XMLoadFloat3(&ak1.scale);
			DirectX::XMStoreFloat3(&ak.scale, DirectX::XMVectorLerp(scale0, scale1, lerpArg));

			ak.time = tl;

			return ak;
		}
	}

	if(anim->mNumPositionKeys == 1) {
		ak = ToImportedAnimKey(anim, 0);
		ak.time = tl;
		return ak;
	}
	
	ASSERT(false, "timeline issues\r\n");
}

void ProcessAnimation(const wchar_t* file, std::vector<ProcessedBone> & bones, std::vector<Animation> & anims) {
	std::string path = Egg::Utility::ToNarrowString(file);

	Assimp::Importer importer;

	unsigned int flags = aiProcess_JoinIdenticalVertices | aiProcess_FlipWindingOrder | aiProcess_LimitBoneWeights | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_ImproveCacheLocality;

	const aiScene * scene = importer.ReadFile(path.c_str(), flags);
	
	ASSERT(scene != nullptr, "Failed to load obj file: '%s'. Assimp error message: '%s'", path.c_str(), importer.GetErrorString());

	ASSERT(scene->HasMeshes(), "FBX file: '%s' does not contain a mesh.", path.c_str());

	ASSERT(scene->HasAnimations(), "FBX file: '%s' does not contain animations.", path.c_str());

	uint32_t animCount = scene->mNumAnimations;

	for(uint32_t i = 0; i < animCount; ++i) {
		Animation a;

		aiAnimation * anim = scene->mAnimations[i];
		a.name = ExtractFileName(path);
		a.TicksPerSecond = anim->mTicksPerSecond;
		a.Duration = anim->mDuration;
		a.BoneData.resize(bones.size());

		bool isValid = true;


		double tl = 0.0;

		for(uint32_t j = 0; j < anim->mNumChannels; ++j) {
			aiNodeAnim * nodeAnim = anim->mChannels[j];
			int boneId = GetIndex(bones, nodeAnim->mNodeName.C_Str());

			if(boneId == -1) {
				continue;
			}

			if(!IsAnimationUniformInTime(nodeAnim)) {
				isValid = false;
				printf("Error: %s is not uniformly exported\r\n", a.name.c_str());
				break;
			}
		}

		if(!isValid) {
			continue;
		}

		do {
			for(uint32_t j = 0; j < anim->mNumChannels; ++j) {
				aiNodeAnim * nodeAnim = anim->mChannels[j];
				int boneId = GetIndex(bones, nodeAnim->mNodeName.C_Str());


				if(boneId != -1) {
					int keyFrame = IsKeyframePresent(tl, nodeAnim);

					a.BoneData[boneId].BoneId = boneId;

					if(keyFrame != -1) {
						a.BoneData[boneId].Keys.push_back(ToImportedAnimKey(nodeAnim, keyFrame));
					} else {
						a.BoneData[boneId].Keys.push_back(GenerateKey(tl, nodeAnim));
					}
				}


			}

			tl = NextKeyframe(tl, anim);

		} while(tl < 400000.0);

		std::sort(a.BoneData.begin(), a.BoneData.end(), [](auto a, auto b) -> bool {
			return a.BoneId < b.BoneId;
		});

		LinearizeAnimation(a);

		anims.emplace_back(std::move(a));
	}
}

unsigned int CalculateRequiredMemory(ImportedModel & model, std::vector<ProcessedBone> & bones, std::vector<Animation> & anims) {
	unsigned int acc = 0;

	/*
	everything is linearized here, we just need to set the pointers, for that only the base struct size is needed
	*/

	acc += anims.size() * sizeof(Egg::Asset::Animation);

	acc += bones.size() * sizeof(Egg::Asset::Bone);
	acc += model.materials.size() * sizeof(Egg::Asset::Material);

	acc += model.meshes.size() * sizeof(Egg::Asset::Mesh);
	acc += model.meshes.size() * model.meshes[0].lods.size() * sizeof(Egg::Asset::LODLevel);

	for(auto & mesh : model.meshes) {
		unsigned int verticesSize = 0;
		unsigned int indiciesSize = 0;
		for(auto & lod : mesh.lods) {
			verticesSize += lod.vertexCount * mesh.vertexSize;
			indiciesSize += lod.indexCount * 4U;
		}
		acc += verticesSize;
		acc += indiciesSize;
	}

	return acc;
}

bool Equal(const DirectX::XMFLOAT4X4 & a, const DirectX::XMFLOAT4X4 & b) {
	return true;
}

bool Equal(Egg::Asset::Model & a, Egg::Asset::Model & b) {
	if(a.animationsLength != b.animationsLength ||
	   a.bonesLength != b.bonesLength ||
	   a.materialsLength != b.materialsLength ||
	   a.meshesLength != b.meshesLength) {
		return false;
	}

	printf("Integrity: Lengths OK\r\n");

	printf("Integrity: Animations SKIPPED\r\n");

	for(int i = 0; i < a.bonesLength; ++i) {
		if(a.bones[i].parentId != b.bones[i].parentId ||
		   !Equal(a.bones[i].transform, b.bones[i].transform) ||
		   memcmp(a.bones[i].name, b.bones[i].name, sizeof(Egg::Asset::Bone::name)) != 0) {
			return false;
		}
	}

	printf("Integrity: Bones OK\r\n");

	for(int i = 0; i < a.meshesLength; ++i) {
		if(a.meshes[i].indicesLength != b.meshes[i].indicesLength ||
		   a.meshes[i].verticesLength != b.meshes[i].verticesLength ||
		   a.meshes[i].materialId != b.meshes[i].materialId ||
		   a.meshes[i].vertexSize != b.meshes[i].vertexSize ||
		   a.meshes[i].vertexType != b.meshes[i].vertexType) {
			return false;
		}

		if(memcmp(a.meshes[i].vertices, b.meshes[i].vertices, a.meshes[i].verticesLength) != 0 ||
		   memcmp(a.meshes[i].indices, b.meshes[i].indices, a.meshes[i].indicesLength * sizeof(unsigned int)) != 0) {
			return false;
		}
	}

	printf("Integrity: Meshes OK\r\n");
	printf("Integrity: Materials SKIPPED\r\n");

	return true;
}

/*
* Transforms into Egg::Asset::Model format
*/
void WriteBinary(const char * dest, ImportedModel & model, std::vector<ProcessedBone> & bones, std::vector<Animation> & anims) {

	Egg::Asset::Model m;

	unsigned int s = CalculateRequiredMemory(model, bones, anims);

	Egg::Memory::LinearClassifier allocator{ s };

	m.meshesLength = model.meshes.size();
	m.meshes = reinterpret_cast<Egg::Asset::Mesh *>(allocator.Allocate(m.meshesLength * sizeof(Egg::Asset::Mesh)));
	
	for(unsigned int i = 0; i < model.meshes.size(); ++i) {
		m.meshes[i].vertexType = model.meshes[i].vertexType;
		m.meshes[i].vertexSize = model.meshes[i].vertexSize;

		unsigned int vertexOffset = 0;
		unsigned int indexOffset = 0;

		unsigned int verticesSize = 0;
		unsigned int indicesSize = 0;
		
		m.meshes[i].boundingBox = model.meshes[i].boundingBox;
		m.meshes[i].lodLevelsLength = model.meshes[i].lods.size();
		m.meshes[i].lodLevels = reinterpret_cast<Egg::Asset::LODLevel *>(allocator.Allocate(m.meshes[i].lodLevelsLength * sizeof(Egg::Asset::LODLevel)));

		for(unsigned int j = 0; j < model.meshes[i].lods.size(); ++j) {
			Egg::Asset::LODLevel lvl;
			lvl.indexCount = model.meshes[i].lods[j].indexCount;
			lvl.vertexCount = model.meshes[i].lods[j].vertexCount;
			lvl.indexOffset = indexOffset;
			lvl.vertexOffset = vertexOffset;
			lvl.indicesLength = lvl.indexCount * 4U;
			lvl.verticesLength = lvl.vertexCount * m.meshes[i].vertexSize;
			indexOffset += lvl.indexCount;
			vertexOffset += lvl.vertexCount;
			verticesSize += lvl.verticesLength;
			indicesSize += lvl.indicesLength;
			m.meshes[i].lodLevels[j] = lvl;
		}

		m.meshes[i].vertices = allocator.Allocate(verticesSize);
		m.meshes[i].indices = reinterpret_cast<unsigned int *>(allocator.Allocate(indicesSize));
		m.meshes[i].indicesLength = indicesSize / 4U;
		m.meshes[i].verticesLength = verticesSize;

		for(unsigned int j = 0; j < m.meshes[i].lodLevelsLength; ++j) {
			unsigned int verticesByteOffset = m.meshes[i].lodLevels[j].vertexOffset * m.meshes[i].vertexSize;
			unsigned int indicesOffset = m.meshes[i].lodLevels[j].indexOffset;

			uint8_t * vBuffer = reinterpret_cast<uint8_t *>(m.meshes[i].vertices);
			vBuffer += verticesByteOffset;

			unsigned int * iBuffer = m.meshes[i].indices + indicesOffset;

			memcpy(vBuffer, &(model.meshes[i].lods[j].vertices.at(0)), m.meshes[i].lodLevels[j].verticesLength);
			memcpy(iBuffer, &(model.meshes[i].lods[j].indices.at(0)), m.meshes[i].lodLevels[j].indicesLength);
		}

		m.meshes[i].materialId = model.meshes[i].materialIndex;
	}

	m.animationsLength = anims.size();
	m.animations = reinterpret_cast<Egg::Asset::Animation *>(allocator.Allocate(m.animationsLength * sizeof(Egg::Asset::Animation)));

	for(unsigned int i = 0; i < anims.size(); ++i) {
		Animation & a = anims[i];
		Egg::Asset::Animation & eggAnim = m.animations[i];

		void * p1 = &m.meshes[1];
		void * p2 = m.animations[i].name;

		strcpy_s(m.animations[i].name, a.name.c_str());
		eggAnim.name[_countof(eggAnim.name) - 1] = '\0';

		eggAnim.bonesLength = a.bonesLength;
		eggAnim.keysLength = a.keysLength;

		eggAnim.keys = &(a.Keys.at(0));
		eggAnim.preStates = &(a.PreStates.at(0));
		eggAnim.postStates = &(a.PostStates.at(0));
		eggAnim.times = &(a.Times.at(0));

		eggAnim.duration = a.Duration;
		eggAnim.ticksPerSecond = a.TicksPerSecond;
	}

	m.materialsLength = model.materials.size();
	m.materials = reinterpret_cast<Egg::Asset::Material *>(allocator.Allocate(m.materialsLength * sizeof(Egg::Asset::Material)));
	ZeroMemory(m.materials, sizeof(Egg::Asset::Material) * m.materialsLength);

	for(unsigned int i = 0; i < m.materialsLength; ++i) {
		m.materials[i].diffuseColor = reinterpret_cast<DirectX::XMFLOAT3 &>(model.materials[i].diffuseColor);
		m.materials[i].ambientColor = reinterpret_cast<DirectX::XMFLOAT3 &>(model.materials[i].ambientColor);
		m.materials[i].specularColor = reinterpret_cast<DirectX::XMFLOAT3 &>(model.materials[i].specularColor);
		m.materials[i].shininess = model.materials[i].shininess;

		strcpy_s(m.materials[i].diffuseTexture, model.materials[i].diffuseTexPath.c_str());
		m.materials[i].diffuseTexture[_countof(m.materials[i].diffuseTexture) - 1] = '\0';

		strcpy_s(m.materials[i].normalTexture, model.materials[i].normalTexPath.c_str());
		m.materials[i].normalTexture[_countof(m.materials[i].normalTexture) - 1] = '\0';
	}

	m.bonesLength = bones.size();
	m.bones = reinterpret_cast<Egg::Asset::Bone *>(allocator.Allocate(m.bonesLength * sizeof(Egg::Asset::Bone)));
	
	for(unsigned int i = 0; i < m.bonesLength; ++i) {
		strcpy_s(m.bones[i].name, bones[i].name.c_str());
		m.bones[i].name[_countof(m.bones[i].name) - 1] = '\0';

		m.bones[i].parentId = bones[i].parentIndex;
		auto& r = bones[i].offsetMatrix.Transpose();
		
		m.bones[i].transform = DirectX::XMFLOAT4X4{
			r.a1, r.a2, r.a3, r.a4,
			r.b1, r.b2, r.b3, r.b4,
			r.c1, r.c2, r.c3, r.c4,
			r.d1, r.d2, r.d3, r.d4
		};
	}


	Egg::Exporter::ExportModel(dest, m);

	Egg::Asset::Model imported;
	Egg::Importer::ImportModel(dest, imported);

	if(Equal(imported, m)) {
		printf("Integrity check: OK\r\n");
	} else {
		printf("Integrity check: FAILED\r\n");
	}


	unsigned int nVertices = 0;
	unsigned int verticesInBytes = 0;
	unsigned int indices = 0;
	for(unsigned int i = 0; i < m.meshesLength; ++i) {
		verticesInBytes += m.meshes[i].verticesLength;
		nVertices += (m.meshes[i].verticesLength / m.meshes[i].vertexSize);
		indices += m.meshes[i].indicesLength;
	}

	printf("Meshes: %d\r\n\tVertices: %d (size: %d)\r\n\tIndices: %d (size: %d)\r\n", m.meshesLength, nVertices, verticesInBytes, indices, indices * (unsigned int)sizeof(unsigned int));
	printf("Bones: %d\r\nMaterials: %d\r\nAnimations: %d\r\n", m.bonesLength, m.materialsLength, m.animationsLength);

	for(unsigned int i = 0; i < m.animationsLength; ++i) {
		printf("\t%s\r\n", m.animations[i].name);
	}

	printf("Exported successfully into file: %s\r\n", dest);
	//m.memoryAllocation = allocator.Detach();
}
