#include <cstdio>
#include <string>
#include <algorithm>
#include <vector>

#include <Egg/Path.h>
#include <Egg/Common.h>
#include <Egg/Utility.h>
#include <Egg/ProgramArgs.h>
#include <Egg/Vertex.h>
#include <Egg/Exporter.h>
#include <Egg/Importer.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

void ToLower(std::wstring & str) {
	std::transform(str.begin(), str.end(), str.begin(), std::tolower);
}

void PrintHelp() {
	printf("Usage: $ EggAssetPipeline <media_file> [<media_file>...] [--generate_tangent_space]\r\n");
	printf("\tThis utility does preprocessing on assets to make it faster to load in a real scenario\r\n");
	printf("\t<media_file>                 : This can be a relative or absolute file path. Output will be <media_file>.eggasset in the current working directory. For now only FBX files are supported\r\n");
	printf("\t    --generate_tangent_space : Tangents and binormals will be calculated for each mesh\r\n");
}


struct BoneAnimation {
	int BoneId;

	Egg::Asset::AnimationState PreState;
	Egg::Asset::AnimationState PostState;

	std::vector<Egg::Asset::AnimationKey> Keys;
};

struct Animation {

	std::string name;

	double Duration;
	double TicksPerSecond;

	std::vector<BoneAnimation> BoneData;

};

struct ImportedBone {
	std::string name;
	aiMatrix4x4 offsetMatrix;
	uint32_t numVertexWeights;
	aiVertexWeight * vertexWeights;
};

struct ImportedMesh {
	uint32_t materialIndex;
	unsigned int vertexType;
	unsigned int vertexSize;
	std::vector<unsigned char> vertices;
	std::vector<uint32_t> indices;
	std::vector<ImportedBone> bones;
};

struct ImportedMaterial {
	aiColor3D diffuseColor;
	aiColor3D ambientColor;
	aiColor3D specularColor;
	float shininess;
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

Egg::Asset::AnimationState ToAnimationState(aiAnimBehaviour behaviour) {
	switch(behaviour) {
		case aiAnimBehaviour_DEFAULT: return Egg::Asset::AnimationState::DEFAULT;
		case aiAnimBehaviour_CONSTANT: return Egg::Asset::AnimationState::CONSTANT;
		case aiAnimBehaviour_LINEAR: return Egg::Asset::AnimationState::LINEAR;
		case aiAnimBehaviour_REPEAT: return Egg::Asset::AnimationState::REPEAT;
		default: throw 1;
	}
}

void Process(const wchar_t * file, std::vector<ProcessedBone> & bones, ImportedModel & model, bool generateTangentSpace);
void ProcessAnimation(const wchar_t * file, std::vector<ProcessedBone> & bones, std::vector<Animation> & anims);
void WriteBinary(const char * dest, ImportedModel & model, std::vector<ProcessedBone> & bones, std::vector<Animation> & anims);

int wmain(int argc, wchar_t ** argv) {
	Egg::ProgramArgs pa{ (const wchar_t**) argv, argc };

	if(pa.IsSet(L"help") || argc < 2) {
		PrintHelp();
		return 0;
	}

	ImportedModel model;
	std::vector<ProcessedBone> bones;
	std::vector<Animation> anims;

	std::string binaryName;

	for(int i = 1; i < argc; ++i) {
		std::wstring mediaFile{ argv[i] };

		if(mediaFile[0] == L'-' && mediaFile[1] == L'-') {
			continue;
		}

		std::size_t indexOfDot = mediaFile.find_last_of('.');

		if(indexOfDot == std::wstring::npos) {
			printf("Error: input file must be a file with extension\r\n");
			return 1;
		}

		std::wstring extension = mediaFile.substr(indexOfDot);
		ToLower(extension);

		if(extension != L".fbx") {
			printf("Error: input file must be an FBX file\r\n");
			return 1;
		}

		if(!Egg::Path::FileExists(mediaFile.c_str())) {
			printf("Error: input file (%S) was not found\r\n", mediaFile.c_str());
			continue;
		}

		if(i == 1) {
			// first should be T pose
			Process(mediaFile.c_str(), bones, model, pa.IsSet(L"generate_tangent_space"));
			binaryName = ExtractFileName(Egg::Utility::ToNarrowString(mediaFile));
		} else {
			ProcessAnimation(mediaFile.c_str(), bones, anims);
		}
	}

	binaryName += ".eggasset";

	WriteBinary(binaryName.c_str(), model, bones, anims);


	return 0;
}

const ImportedBone* IsReferenced(const char* boneName, const ImportedModel & im) {
	for(const ImportedMesh & m : im.meshes) {
		int i = 0;
		for(const ImportedBone & ib : m.bones) {
			if(ib.name == boneName) {
				return &(m.bones.at(i));
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
			ProcessedBone b;
			b.name = node->mName.C_Str();
			b.parentIndex = GetIndex(dst, GetParentName(node->mParent));
			b.offsetMatrix = refBone->offsetMatrix;
			dst.push_back(b);
		} else {

			printf("Bone not referenced: %s\r\n", node->mName.C_Str());
		}
	}

	for(uint32_t i = 0; i < node->mNumChildren; ++i) {
		ProcessBones(dst, model, node->mChildren[i]);
	}
}

bool CheckTransformConsistency(const std::vector<ProcessedBone> & bones, const ImportedModel & im) {
	for(const ProcessedBone & b : bones) {
		for(const ImportedMesh & m : im.meshes) {
			for(const ImportedBone & ib : m.bones) {
				if(ib.name == b.name) {
					if(b.offsetMatrix != ib.offsetMatrix) {
						return false;
					}
				}
			}
		}
	}
	return true;
}

int FirstUnusedSlot(const int * src) {
	for(int i = 0; i < 4; ++i) {
		if(src[i] == -1) {
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

int & IndexInt4(Egg::Math::Int4 & v, unsigned int i) {
	switch(i) {
		case 0: return v.x;
		case 1: return v.y;
		case 2: return v.z;
		case 3: return v.w;
		default: throw 1;
	}
}

float & IndexFloat3(Egg::Math::Float3 & v, unsigned int i) {
	switch(i) {
	case 0: return v.x;
	case 1: return v.y;
	case 2: return v.z;
	default: throw 1;
	}
}

void FillVertexWeights(ImportedModel & im, std::vector<ProcessedBone> & bones) {
	for(ImportedMesh & m : im.meshes) {
		for(ImportedBone & ib : m.bones) {
			for(uint32_t vi = 0; vi < ib.numVertexWeights; ++vi) {
				void * vPtr = &(m.vertices.at(ib.vertexWeights[vi].mVertexId * m.vertexSize));
				Egg::PNTWB_Vertex * vert = reinterpret_cast<Egg::PNTWB_Vertex *>(vPtr);

				int k = FirstUnusedSlot(reinterpret_cast<int*>(&(vert->boneIds)));
				int l = GetBoneIndex(bones, ib.name.c_str());

				if(k == -1 || l == -1) {
					OutputDebugString("Overflow\r\n");
					throw 1;
				}

				IndexInt4(vert->boneIds, k) = l;
				
				if(k < 3 && k >= 0) {
					IndexFloat3(vert->weights, k) = ib.vertexWeights[vi].mWeight;
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
	flags |= aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_ImproveCacheLocality;

	const aiScene * scene = importer.ReadFile(path.c_str(), flags);

	ASSERT(scene != nullptr, "Failed to load obj file: '%s'. Assimp error message: '%s'", path.c_str(), importer.GetErrorString());

	ASSERT(scene->HasMeshes(), "FBX file: '%s' does not contain a mesh.", path.c_str());

	uint32_t meshCount = scene->mNumMeshes;
	uint32_t animCount = scene->mNumAnimations;
	uint32_t texCount = scene->mNumTextures;
	uint32_t matCount = scene->mNumMaterials;

	model.meshes.reserve(meshCount);
	model.materials.reserve(matCount);


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
		
		ImportedMesh im;
		im.vertexSize = vSize;
		im.vertexType = vType;
		im.vertices.resize(mesh->mNumVertices * im.vertexSize);
		im.bones.resize(mesh->mNumBones);

		for(int k = 0; k < mesh->mNumVertices; ++k) {
			unsigned char * ptr = &(im.vertices.at(k * im.vertexSize));

			Egg::PNT_Vertex * evt = reinterpret_cast<Egg::PNT_Vertex *>(ptr);
			evt->position = reinterpret_cast<Egg::Math::Float3&>(mesh->mVertices[k]);
			evt->normal = reinterpret_cast<Egg::Math::Float3&>(mesh->mNormals[k]);
			evt->tex = reinterpret_cast<Egg::Math::Float2&>(mesh->mTextureCoords[0][k]);

			switch(meshSignature) {
			case 0b01111:
				{
					Egg::PNTWB_Vertex * wbVertex = reinterpret_cast<Egg::PNTWB_Vertex *>(ptr);
					wbVertex->weights = Egg::Math::Float3::Zero;
					wbVertex->boneIds = Egg::Math::Int4{ -1, -1, -1, -1 };
				}
				break;
			case 0b10111:
				{
					Egg::PNTTB_Vertex * tbVertex = reinterpret_cast<Egg::PNTTB_Vertex *>(ptr);
					tbVertex->tangent = reinterpret_cast<Egg::Math::Float3 &>(mesh->mTangents[k]);
					tbVertex->binormal = reinterpret_cast<Egg::Math::Float3 &>(mesh->mBitangents[k]);
				}
				break;
			case 0b11111:
				{
					Egg::PNTWBTB_Vertex * wbtbVertex = reinterpret_cast<Egg::PNTWBTB_Vertex *>(ptr);
					wbtbVertex->weights = Egg::Math::Float3::Zero;
					wbtbVertex->boneIds = Egg::Math::Int4{ -1, -1, -1, -1 };
					wbtbVertex->tangent = reinterpret_cast<Egg::Math::Float3 &>(mesh->mTangents[k]);
					wbtbVertex->binormal = reinterpret_cast<Egg::Math::Float3 &>(mesh->mBitangents[k]);
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
		im.indices.resize(nIndices);

		unsigned int * ptr = &(im.indices.at(0));
		for(int k = 0; k < mesh->mNumFaces; ++k) {
			
			unsigned int n = mesh->mFaces[k].mNumIndices;
				
			if(n == 3) {
				ptr[0] = mesh->mFaces[k].mIndices[0];
				ptr[1] = mesh->mFaces[k].mIndices[1];
				ptr[2] = mesh->mFaces[k].mIndices[2];
				ptr += 3;
			}
		}

		im.materialIndex = mesh->mMaterialIndex;

		/* adding the bones, but will process it later on */
		for(uint32_t j = 0; j < mesh->mNumBones; ++j) {
			aiBone * b = mesh->mBones[j];
			
			im.bones[j].name = b->mName.C_Str();
			im.bones[j].numVertexWeights = b->mNumWeights;
			im.bones[j].vertexWeights = b->mWeights;
			im.bones[j].offsetMatrix = b->mOffsetMatrix;
		}

		model.meshes.emplace_back(std::move(im));
	}

	ProcessBones(bones, model, scene->mRootNode);
	FillVertexWeights(model, bones);

	if(!CheckTransformConsistency(bones, model)) {
		// error!
		OutputDebugString("Error\r\n");
	}

	for(uint32_t i = 0; i < matCount; ++i) {
		aiMaterial * mat = scene->mMaterials[i];

		ImportedMaterial imat;
		aiString str;
		if(AI_SUCCESS == mat->Get(AI_MATKEY_NAME, str)) {
			Egg::Utility::Debugf("Prop: name | value '%s'\r\n", str.C_Str());
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

void ProcessAnimation(const wchar_t* file, std::vector<ProcessedBone> & bones, std::vector<Animation> & anims) {
	std::string path = Egg::Utility::ToNarrowString(file);

	Assimp::Importer importer;

	const aiScene * scene = importer.ReadFile(path.c_str(), aiProcess_LimitBoneWeights | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes);
	
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
		a.BoneData.resize(anim->mNumChannels);

		bool isValid = true;

		for(uint32_t j = 0; j < anim->mNumChannels; ++j) {
			aiNodeAnim * nodeAnim = anim->mChannels[j];

			if(!IsAnimationUniformInTime(nodeAnim)) {
				isValid = false;
				printf("Error: %s is not uniformly exported\r\n", a.name.c_str());
				break;
			}

			a.BoneData[j].Keys.resize(nodeAnim->mNumPositionKeys);

			for(uint32_t k = 0; k < nodeAnim->mNumPositionKeys; ++k) {
				a.BoneData[j].Keys[k].position.x = nodeAnim->mPositionKeys[k].mValue.x;
				a.BoneData[j].Keys[k].position.y = nodeAnim->mPositionKeys[k].mValue.y;
				a.BoneData[j].Keys[k].position.z = nodeAnim->mPositionKeys[k].mValue.z;

				a.BoneData[j].Keys[k].rotation.x = nodeAnim->mRotationKeys[k].mValue.x;
				a.BoneData[j].Keys[k].rotation.y = nodeAnim->mRotationKeys[k].mValue.y;
				a.BoneData[j].Keys[k].rotation.z = nodeAnim->mRotationKeys[k].mValue.z;
				a.BoneData[j].Keys[k].rotation.w = nodeAnim->mRotationKeys[k].mValue.w;

				a.BoneData[j].Keys[k].scale.x = nodeAnim->mScalingKeys[k].mValue.x;
				a.BoneData[j].Keys[k].scale.y = nodeAnim->mScalingKeys[k].mValue.y;
				a.BoneData[j].Keys[k].scale.z = nodeAnim->mScalingKeys[k].mValue.z;

				// the animation must be uniform for this
				a.BoneData[j].Keys[k].time = nodeAnim->mPositionKeys[k].mTime;
			}

			a.BoneData[j].PreState = ToAnimationState(nodeAnim->mPreState);
			a.BoneData[j].PostState = ToAnimationState(nodeAnim->mPostState);
			a.BoneData[j].BoneId = GetBoneIndex(bones, nodeAnim->mNodeName.C_Str());

			ASSERT(a.BoneData[j].BoneId != -1, "Bone named '%s' was not found.", nodeAnim->mNodeName.C_Str());
		}

		if(!isValid) {
			continue;
		}

		std::sort(a.BoneData.begin(), a.BoneData.end(), [](auto a, auto b) -> bool {
			return a.BoneId < b.BoneId;
		});

		anims.emplace_back(std::move(a));
	}

	return;
}

unsigned int CalculateRequiredMemory(ImportedModel & model, std::vector<ProcessedBone> & bones, std::vector<Animation> & anims) {
	unsigned int acc = 0;

	acc += anims.size() * sizeof(Egg::Asset::Animation);
	for(Animation & a : anims) {
		acc += a.BoneData.size() * sizeof(Egg::Asset::BoneAnimation);
	 /*
		for(BoneAnimation b : a.BoneData) {
			acc += b.Keys.size() * sizeof(Egg::Asset::AnimationKey);
		}*/
	}

	acc += bones.size() * sizeof(Egg::Asset::Bone);
	acc += model.materials.size() * sizeof(Egg::Asset::Material);

	acc += model.meshes.size() * sizeof(Egg::Asset::Mesh);
	/*
	for(ImportedMesh & m : model.meshes) {
		acc += m.indices.size() * sizeof(unsigned int);
		acc += m.vertices.size() * sizeof(Egg::PNTWB_Vertex);
	}*/

	return acc;
}

bool Equal(const Egg::Math::Float4x4 & a, const Egg::Math::Float4x4 & b) {
	for(int i = 0; i < 16; ++i) {
		if(a.l[0] != b.l[0]) {
			return false;
		}
	}
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

	for(int i = 0; i < a.animationsLength; ++i) {
		if(a.animations[i].boneDataLength != b.animations[i].boneDataLength ||
		   a.animations[i].duration != b.animations[i].duration ||
		   memcmp(a.animations[i].name, b.animations[i].name, sizeof(Egg::Asset::Animation::name)) != 0 ||
		   a.animations[i].ticksPerSecond != b.animations[i].ticksPerSecond) {
			return false;
		}

		for(int j = 0; j < a.animations[i].boneDataLength; ++j) {
			auto & boneA = a.animations[i].boneData[j];
			auto & boneB = b.animations[i].boneData[j];

			if(boneA.boneId != boneB.boneId ||
			   boneA.keysLength != boneB.keysLength ||
			   boneA.postState != boneB.postState ||
			   boneA.preState != boneB.preState) {
				return false;
			}

			for(int k = 0; k < a.animations[i].boneData[j].keysLength; ++k) {
				auto & keyA = boneA.keys[k];
				auto & keyB = boneB.keys[k];

				if((keyA.position != keyB.position).Any() ||
				   (keyA.rotation != keyB.rotation).Any() ||
				   (keyA.scale != keyB.scale).Any() ||
				   keyA.time != keyB.time) {
					return false;
				}
			}

		}

	}

	printf("Integrity: Animations OK\r\n");

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
	printf("Integrity: Materials Skipped\r\n");

	return true;
}

/*
* Transforms into Egg::Asset::Model format
*/
void WriteBinary(const char * dest, ImportedModel & model, std::vector<ProcessedBone> & bones, std::vector<Animation> & anims) {

	Egg::Asset::Model m;

	unsigned int s = CalculateRequiredMemory(model, bones, anims);

	Egg::Importer::LinearAllocator allocator{ s };
	m.memoryAllocation = allocator.ptr;

	m.meshesLength = model.meshes.size();
	m.meshes = reinterpret_cast<Egg::Asset::Mesh *>(allocator.Allocate(m.meshesLength * sizeof(Egg::Asset::Mesh)));
	
	for(unsigned int i = 0; i < model.meshes.size(); ++i) {
		m.meshes[i].vertexType = model.meshes[i].vertexType;
		m.meshes[i].vertexSize = model.meshes[i].vertexSize;

		m.meshes[i].indices = &(model.meshes[i].indices.at(0));
		m.meshes[i].indicesLength = model.meshes[i].indices.size();

		m.meshes[i].vertices = &(model.meshes[i].vertices.at(0));
		m.meshes[i].verticesLength = model.meshes[i].vertices.size();

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

		eggAnim.boneDataLength = a.BoneData.size();
		eggAnim.boneData = reinterpret_cast<Egg::Asset::BoneAnimation *>(allocator.Allocate(eggAnim.boneDataLength * sizeof(Egg::Asset::BoneAnimation)));

		for(unsigned int j = 0; j < a.BoneData.size(); ++j) {
			BoneAnimation & b = a.BoneData[j];
			Egg::Asset::BoneAnimation & eggBone = eggAnim.boneData[j];
			eggBone.boneId = b.BoneId;
			eggBone.keys = &(b.Keys.at(0));
			eggBone.keysLength = b.Keys.size();
			eggBone.postState = b.PostState;
			eggBone.preState = b.PreState;
		}

		eggAnim.duration = a.Duration;
		eggAnim.ticksPerSecond = a.TicksPerSecond;
	}

	m.materialsLength = model.materials.size();
	m.materials = reinterpret_cast<Egg::Asset::Material *>(allocator.Allocate(m.materialsLength * sizeof(Egg::Asset::Material)));
	ZeroMemory(m.materials, sizeof(Egg::Asset::Material) * m.materialsLength);

	for(unsigned int i = 0; i < m.materialsLength; ++i) {
		m.materials[i].diffuseColor = reinterpret_cast<Egg::Math::Float3 &>(model.materials[i].diffuseColor);
		m.materials[i].ambientColor = reinterpret_cast<Egg::Math::Float3 &>(model.materials[i].ambientColor);
		m.materials[i].specularColor = reinterpret_cast<Egg::Math::Float3 &>(model.materials[i].specularColor);
		m.materials[i].shininess = model.materials[i].shininess;
	}

	m.bonesLength = bones.size();
	m.bones = reinterpret_cast<Egg::Asset::Bone *>(allocator.Allocate(m.bonesLength * sizeof(Egg::Asset::Bone)));
	
	for(unsigned int i = 0; i < m.bonesLength; ++i) {
		strcpy_s(m.bones[i].name, bones[i].name.c_str());
		m.bones[i].name[_countof(m.bones[i].name) - 1] = '\0';

		m.bones[i].parentId = bones[i].parentIndex;
		bones[i].offsetMatrix.Transpose();
		m.bones[i].transform = reinterpret_cast<Egg::Math::Float4x4&>(bones[i].offsetMatrix);
	}


	Egg::Exporter::ExportModel(dest, m);

	Egg::Asset::Model imported;
	Egg::Importer::ImportModel(dest, imported);

	if(Equal(imported, m)) {
		printf("Integrity check: OK\r\n");
	} else {
		printf("Integrity check: FAILED\r\n");
	}


	unsigned int verticesInBytes = 0;
	unsigned int indices = 0;
	for(unsigned int i = 0; i < m.meshesLength; ++i) {
		verticesInBytes += m.meshes[i].verticesLength;
		indices += m.meshes[i].indicesLength;
	}

	printf("Meshes: %d\r\n\tVertices: %d (size: %d)\r\n\tIndices: %d (size: %d)\r\n", m.meshesLength, verticesInBytes / (unsigned int)sizeof(Egg::PNTWB_Vertex), verticesInBytes, indices, indices * (unsigned int)sizeof(unsigned int));
	printf("Bones: %d\r\nMaterials: %d\r\nAnimations: %d\r\n", m.bonesLength, m.materialsLength, m.animationsLength);

	for(unsigned int i = 0; i < m.animationsLength; ++i) {
		printf("\t%s\r\n", m.animations[i].name);
	}

	printf("Exported successfully into file: %s\r\n", dest);
}
