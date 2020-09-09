#include "ClientConverters.h"
#include <NetcodeAssetLib/Model.h>
#include <Netcode/Utility.h>
#include <Netcode/Graphics/ResourceEnums.h>
#include "../GameObject.h"
#include "../Services.h"
#include "../Scripts/DebugScript.h"

ClientAssetConverter::ClientAssetConverter(GameObjectCatalog * catalog, Netcode::JsonValue * json, GameScene * scene) : AssetConverterBase{ scene, json },
	catalog{ catalog } {

}

Ref<Netcode::Material> ClientAssetConverter::ConvertMaterial(const Netcode::JsonValue & values, Ref<Netcode::Material> sourceMaterial) {
	class MaterialProxy {
		Ref<Netcode::Material> mat;
		Ref<Netcode::Material> sourceMat;

	public:
		MaterialProxy(Ref<Netcode::Material> srcMat) : mat { }, sourceMat{ std::move(srcMat) } { }

		Ptr<Netcode::Material> operator->() {
			if(mat == nullptr) {
				mat = sourceMat->Clone();
			}
			return mat.get();
		}

		Ref<Netcode::Material> GetInstance() const {
			if(mat == nullptr) {
				return sourceMat;
			}
			return mat;
		}
	};

	MaterialProxy proxy{ std::move(sourceMaterial) };

	if(const auto it = values.FindMember(L"roughness"); it != values.MemberEnd() && it->value.IsNumber()) {
		proxy->SetParameter(Netcode::MaterialParamId::ROUGHNESS, it->value.GetFloat());
	}

	if(const auto it = values.FindMember(L"reflectance"); it != values.MemberEnd() && it->value.IsNumber()) {
		proxy->SetParameter(Netcode::MaterialParamId::REFLECTANCE, it->value.GetFloat());
	}

	if(const auto it = values.FindMember(L"metal_mask"); it != values.MemberEnd() && it->value.IsBool()) {
		proxy->SetParameter(Netcode::MaterialParamId::METAL_MASK, it->value.GetBool());
	}
	
	return proxy.GetInstance();
}

GameObject * ClientAssetConverter::ConvertColliderComponent(GameObject * gameObject, const Netcode::JsonValue & values) {
	throw Netcode::NotImplementedException{ "Converting from JSON to collider is not implemented" };
}

GameObject * ClientAssetConverter::ConvertModelComponent(GameObject * gameObject, const Netcode::JsonValue & values) {
	std::wstring assetPath;
	Netcode::JsonValueConverter<>::ConvertFromJson(Netcode::GetMember(values, L"asset"), assetPath);
	const Netcode::URI::Model modelUri{ std::move(assetPath) };
	const auto catalogProxy = catalog->Find(modelUri);

	if(catalogProxy != nullptr) {
		gameObject = catalogProxy.Clone();
	} else {
		gameObject = scene->Create();
		AssetManager * assetManager = Service::Get<AssetManager>();
		Netcode::Asset::Model * ncAsset = assetManager->Import(modelUri);
		gameObject = ConvertComponents(gameObject, ncAsset);
	}

	if(const auto it = values.FindMember(L"materials"); it != values.MemberEnd()) {
		if(!it->value.IsArray()) {
			throw Netcode::UndefinedBehaviourException{ "model.materials must be an array" };
		}

		Model * modelComponent = gameObject->GetComponent<Model>();

		for(const auto & mat : it->value.GetArray()) {
			uint32_t materialIndex;
			Netcode::JsonValueConverter<>::ConvertFromJson(Netcode::GetMember(mat, L"material_index"), materialIndex);

			if(modelComponent->materials.size() <= materialIndex) {
				throw Netcode::OutOfRangeException{ "material_index is out of range" };
			}
			
			auto existingMaterial = modelComponent->materials[materialIndex];

			Ref<Netcode::Material> material = ConvertMaterial(mat, existingMaterial);

			modelComponent->materials[materialIndex] = material;

			for(auto & shadedMesh : modelComponent->meshes) {
				if(shadedMesh.material == existingMaterial) {
					shadedMesh.material = material;
				}
			}
		}
	}
	
	return gameObject;
}

GameObject * ClientAssetConverter::ConvertLightComponent(GameObject * gameObject, const Netcode::JsonValue & values) {
	Netcode::Light light{ };

	uint32_t lightTypeInt;
	Netcode::JsonValueConverter<>::ConvertFromJson(Netcode::GetMember(values, L"type"), lightTypeInt);
	const Netcode::LightType type = static_cast<Netcode::LightType>(lightTypeInt);
	
	Netcode::Float3 intensity;
	Netcode::JsonValueConverter<>::ConvertFromJson(Netcode::GetMember(values, L"intensity"), intensity);
	light.intensity = Netcode::Float4{ intensity.x, intensity.y, intensity.z, 1.0f };

	switch(type) {
		case Netcode::LightType::DIRECTIONAL: {
			Netcode::Float3 dir;
			Netcode::JsonValueConverter<>::ConvertFromJson(Netcode::GetMember(values, L"direction"), dir);
			light.position = -Netcode::Vector3{ dir }.Normalize();
		} break;
		case Netcode::LightType::OMNI:
			Netcode::JsonValueConverter<>::ConvertFromJson(Netcode::GetMember(values, L"min_dist"), light.minimumDistance);
			Netcode::JsonValueConverter<>::ConvertFromJson(Netcode::GetMember(values, L"max_dist"), light.maximumDistance);
			Netcode::JsonValueConverter<>::ConvertFromJson(Netcode::GetMember(values, L"reference"), light.referenceDistance);
			Netcode::JsonValueConverter<>::ConvertFromJson(Netcode::GetMember(values, L"position"), light.position);
			break;
		default: break;
	}

	Netcode::Light * lPtr = gameObject->AddComponent<Netcode::Light>();
	lPtr->position = light.position;
	lPtr->referenceDistance = light.referenceDistance;
	lPtr->minimumDistance = light.minimumDistance;
	lPtr->maximumDistance = light.maximumDistance;
	lPtr->angleOffset = light.angleOffset;
	lPtr->angleScale = light.angleScale;
	lPtr->intensity = light.intensity;
	lPtr->direction = light.direction;
	lPtr->type = type;

	return gameObject;
}

GameObject * ClientAssetConverter::ConvertTransformComponent(GameObject * gameObject, const Netcode::JsonValue & values) {
	Transform * transform = gameObject->AddComponent<Transform>();

	if(const auto it = values.FindMember(L"position"); it != values.MemberEnd()) {
		Netcode::JsonValueConverter<>::ConvertFromJson(it->value, transform->position);
	}

	if(const auto it = values.FindMember(L"euler_angles"); it != values.MemberEnd()) {
		Netcode::Float3 eulerAngles;
		Netcode::JsonValueConverter<>::ConvertFromJson(it->value, eulerAngles);
		transform->rotation = Netcode::Quaternion::FromEulerAngles(eulerAngles);
	}
	
	return gameObject;
}

template<typename T>
class ComponentProxy {
	GameObject * gameObj;
public:
	ComponentProxy(GameObject* obj) : gameObj{ obj } { }
	
	T * operator->() {
		if(gameObj->HasComponent<T>()) {
			return gameObj->GetComponent<T>();
		}
		return gameObj->AddComponent<T>();
	}
};

void ClientAssetConverter::ConvertScriptComponent(GameObject* gameObject, const Netcode::JsonValue& values) {
	ComponentProxy<Script> proxy { gameObject };

	std::wstring typeString;
	Netcode::JsonValueConverter<>::ConvertFromJson(Netcode::GetMember(values, L"type"), typeString);

	if(typeString == L"TransformDebugScript") {
		proxy->AddScript(std::make_unique<TransformDebugScript>());
		return;
	}

	Log::Error("Script type: {0} is not registered", Netcode::Utility::ToNarrowString(typeString));
}

GameObject * ClientAssetConverter::ConvertGameObject(const Netcode::JsonValue & values) {
	if(!values.IsObject()) {
		throw Netcode::UndefinedBehaviourException{ "Trying to convert a non object" };
	}
	
	GameObject * gameObj = nullptr;
	if(const auto it = values.FindMember(L"model"); it != values.MemberEnd()) {
		gameObj = ConvertModelComponent(gameObj, it->value);
	} else {
		gameObj = scene->Create();
	}

	if(const auto it = values.FindMember(L"transform"); it != values.MemberEnd()) {
		if(!it->value.IsObject()) {
			throw Netcode::UndefinedBehaviourException{ "transform must be a json object" };
		}
		gameObj = ConvertTransformComponent(gameObj, it->value);
	}

	if(const auto it = values.FindMember(L"light"); it != values.MemberEnd()) {
		if(!it->value.IsObject()) {
			throw Netcode::UndefinedBehaviourException{ "light must be a json object" };
		}
		gameObj = ConvertLightComponent(gameObj, it->value);
	}

	if(const auto it = values.FindMember(L"script"); it != values.MemberEnd()) {
		if(!it->value.IsArray()) {
			throw Netcode::UndefinedBehaviourException{ "script must be a json array" };
		}

		for(const auto & scriptJson : it->value.GetArray()) {
			try {
				ConvertScriptComponent(gameObj, scriptJson);
			} catch(Netcode::ExceptionBase & e) {
				Log::Error("While loading script: {0}", e.ToString());
			}
		}
	}

	if(const auto it = values.FindMember(L"children"); it != values.MemberEnd()) {
		if(!it->value.IsArray()) {
			throw Netcode::UndefinedBehaviourException{ "children node must be a json array" };
		}

		for(const auto & child : it->value.GetArray()) {
			GameObject * childObject;
			try {
				childObject = ConvertGameObject(child);
			} catch(Netcode::ExceptionBase & e) {
				Log::Error("While loading game object: {0}", e.ToString());
				continue;
			}
			gameObj->AddChild(childObject);
		}
	}

	scene->Spawn(gameObj);

	return gameObj;
}

Ref<Netcode::Material> ClientAssetConverter::ConvertMaterial(const Netcode::Asset::Material * mat) {
	auto * graphics = Service::Get<Netcode::Module::IGraphicsModule *>();

	std::string ncMatName{ std::string_view{ mat->name, Netcode::Utility::ArraySize(mat->name) } };
	Ref<Netcode::Material> ncMat = std::make_shared<Netcode::BrdfMaterial>(static_cast<Netcode::MaterialType>(mat->type), ncMatName);
	uint32_t texFlags = 0;
	ncMat->SetParameter<bool>(Netcode::MaterialParamId::METAL_MASK, false);

	for(uint32_t j = 0; j < mat->indicesLength; ++j) {
		Netcode::Asset::MaterialParamIndex param = mat->indices[j];
		void * pptr = ncMat->GetParameterPointer(param.id);

		if(param.id < static_cast<uint32_t>(Netcode::MaterialParamId::SENTINEL_TEXTURE_PATHS_BEGIN)) {
			memcpy(pptr, mat->data + param.offset, param.size);
		} else if(param.id < static_cast<uint32_t>(Netcode::MaterialParamId::SENTINEL_TEXTURE_PATHS_END)) {
			if(param.size > 0) {
				std::wstring fullUriPath{ std::wstring_view {
					reinterpret_cast<wchar_t *>(mat->data + param.offset),
					param.size / sizeof(wchar_t)
				} };

				Netcode::URI::Texture texUri{ std::move(fullUriPath), Netcode::FullPathToken{ } };

				Ref<Netcode::TextureBuilder> texBuilder = graphics->CreateTextureBuilder();
				texBuilder->LoadTexture2D(texUri);
				texBuilder->SetStateAfterUpload(Netcode::Graphics::ResourceState::PIXEL_SHADER_RESOURCE);
				Ref<Netcode::GpuResource> gpuRes = texBuilder->Build();

				uint32_t texId = param.id -
					static_cast<uint32_t>(Netcode::MaterialParamId::SENTINEL_TEXTURE_PATHS_BEGIN);

				Netcode::MaterialParamId pid = static_cast<Netcode::MaterialParamId>(
					texId +
					static_cast<uint32_t>(Netcode::MaterialParamId::SENTINEL_TEXTURES)
					);

				Ref<Netcode::ResourceViews> views = ncMat->GetResourceView(0);

				if(views == nullptr) {
					views = graphics->resources->CreateShaderResourceViews(6);
					ncMat->SetResourceView(0, views);
					views->ClearSRV(0, Netcode::Graphics::ResourceDimension::TEXTURE2D);
					views->ClearSRV(1, Netcode::Graphics::ResourceDimension::TEXTURE2D);
					views->ClearSRV(2, Netcode::Graphics::ResourceDimension::TEXTURE2D);
					views->ClearSRV(3, Netcode::Graphics::ResourceDimension::TEXTURE2D);
					views->ClearSRV(4, Netcode::Graphics::ResourceDimension::TEXTURE2D);
					views->ClearSRV(5, Netcode::Graphics::ResourceDimension::TEXTURE2D);
				}

				texFlags |= 1 << texId;

				views->CreateSRV(texId, gpuRes.get());

				ncMat->SetParameter<Ref<Netcode::GpuResource>>(pid, std::move(gpuRes));

				(*reinterpret_cast<Netcode::URI::Texture *>(pptr)) = std::move(texUri);
			}
		}
	}
	ncMat->SetParameter<uint32_t>(Netcode::MaterialParamId::TEXTURE_FLAGS, texFlags);

	return ncMat;
}

Ref<Mesh> ClientAssetConverter::ConvertMesh(const Netcode::Asset::Mesh * mesh) {
	auto * graphics = Service::Get<Netcode::Module::IGraphicsModule *>();
	auto batch = graphics->resources->CreateUploadBatch();
	auto appMesh = std::make_shared<Mesh>();

	uint8_t * const vBasePtr = reinterpret_cast<uint8_t *>(mesh->vertices);
	uint8_t * const iBasePtr = reinterpret_cast<uint8_t *>(mesh->indices);

	for(unsigned int i = 0; i < mesh->lodLevelsLength; ++i) {
		using Netcode::Graphics::ResourceState;
		using Netcode::Graphics::ResourceType;

		uint8_t * vData = vBasePtr + mesh->lodLevels[i].vertexBufferByteOffset;
		uint8_t * iData = nullptr;
		Ref<Netcode::GpuResource> vbuffer = graphics->resources->CreateVertexBuffer(mesh->lodLevels[i].vertexBufferSizeInBytes, mesh->vertexSize, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
		uint64_t vCount = mesh->lodLevels[i].vertexCount;
		Ref<Netcode::GpuResource> ibuffer{ nullptr };
		uint64_t iCount = 0;

		batch->Upload(vbuffer, vData, mesh->lodLevels[i].vertexBufferSizeInBytes);
		batch->Barrier(vbuffer, ResourceState::COPY_DEST, ResourceState::VERTEX_AND_CONSTANT_BUFFER);

		if(mesh->indices != nullptr) {
			ibuffer = graphics->resources->CreateIndexBuffer(mesh->lodLevels[i].indexBufferSizeInBytes, DXGI_FORMAT_R32_UINT, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
			iData = iBasePtr + mesh->lodLevels[i].indexBufferByteOffset;
			iCount = mesh->lodLevels[i].indexCount;

			batch->Upload(ibuffer, iData, mesh->lodLevels[i].indexBufferSizeInBytes);
			batch->Barrier(ibuffer, ResourceState::COPY_DEST, ResourceState::INDEX_BUFFER);
		}

		GBuffer lod;
		lod.indexBuffer = ibuffer;
		lod.vertexBuffer = vbuffer;
		lod.indexCount = iCount;
		lod.vertexCount = vCount;

		appMesh->AddLOD(lod);
		appMesh->vertexSize = mesh->vertexSize;
	}

	appMesh->selectedLod = 0;
	appMesh->boundingBox = mesh->boundingBox;

	graphics->frame->SyncUpload(std::move(batch));

	return appMesh;
}

GameObject * ClientAssetConverter::ConvertColliderComponent(GameObject * gameObject, const Netcode::Asset::Model * ncAsset) {
	Collider * colliderComponent = gameObject->AddComponent<Collider>();
	auto * pxService = Service::Get<Netcode::Physics::PhysX>();

	physx::PxPhysics * pxp = pxService->physics.Get();
	Netcode::PxPtr<physx::PxRigidDynamic> actor = pxp->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
	actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
	std::vector<ColliderShape> netcodeShapes;
	netcodeShapes.reserve(ncAsset->colliders.Size());

	for(size_t i = 0; i < ncAsset->colliders.Size(); ++i) {
		const Netcode::Asset::Collider * netcodeCollider = ncAsset->colliders.Data() + i;
		netcodeShapes.push_back(*netcodeCollider);
		Netcode::PxPtr<physx::PxShape> shape{ nullptr };
		physx::PxShapeFlags shapeFlags;
		physx::PxFilterData filterData;

		if(netcodeCollider->boneReference >= 0 && netcodeCollider->boneReference < 0x7F) {
			shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eTRIGGER_SHAPE | physx::PxShapeFlag::eVISUALIZATION;
			filterData.word0 = PHYSX_COLLIDER_TYPE_HITBOX;
		} else {
			shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION;
			filterData.word0 = PHYSX_COLLIDER_TYPE_WORLD;
		}

		if(netcodeCollider->type == Netcode::Asset::ColliderType::MESH) {
			const auto & mesh = ncAsset->meshes[0];
			physx::PxConvexMeshDesc cmd;
			cmd.points.count = mesh.lodLevels[0].vertexCount;
			cmd.points.data = mesh.vertices;
			cmd.points.stride = mesh.vertexSize;
			cmd.vertexLimit = 255;
			cmd.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX | physx::PxConvexFlag::eFAST_INERTIA_COMPUTATION;
			cmd.indices.data = mesh.indices;
			cmd.indices.count = mesh.lodLevels[0].indexCount;
			cmd.indices.stride = 4;

			physx::PxDefaultMemoryOutputStream buf;
			physx::PxConvexMeshCookingResult::Enum r;
			pxService->cooking->cookConvexMesh(cmd, buf, &r);

			physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
			Netcode::PxPtr<physx::PxConvexMesh> convexMesh = pxp->createConvexMesh(input);

			physx::PxConvexMeshGeometry pcmg{ convexMesh.Get() };

			shape = pxp->createShape(pcmg, *pxService->defaultMaterial, true, shapeFlags);

		} else {
			shape = CreatePrimitiveShapeFromAsset(*netcodeCollider, pxp, pxService->defaultMaterial.Get(), shapeFlags);
		}

		shape->setLocalPose(physx::PxTransform{
				Netcode::ToPxVec3(netcodeCollider->localPosition),
				Netcode::ToPxQuat(netcodeCollider->localRotation)
			});

		shape->setQueryFilterData(filterData);
		shape->userData = nullptr;
		actor->attachShape(*shape);
	}

	colliderComponent->actor = actor.Release();
	colliderComponent->shapes = std::move(netcodeShapes);
	colliderComponent->actor->userData = gameObject;
	return gameObject;
}

GameObject * ClientAssetConverter::ConvertModelComponent(GameObject * gameObject, const Netcode::Asset::Model * ncAsset) {
	Model * modelComponent = gameObject->AddComponent<Model>();

	for(size_t matIdx = 0; matIdx < ncAsset->materials.Size(); matIdx++) {
		auto ncMat = ConvertMaterial(ncAsset->materials.Data() + matIdx);
		modelComponent->materials.push_back(ncMat);
	}

	for(size_t meshIdx = 0; meshIdx < ncAsset->meshes.Size(); ++meshIdx) {
		const Netcode::Asset::Mesh * mesh = &ncAsset->meshes[meshIdx];
		modelComponent->perObjectData.Model = Netcode::Float4x4::Identity;
		modelComponent->perObjectData.InvModel = Netcode::Float4x4::Identity;

		auto appMesh = ConvertMesh(mesh);

		modelComponent->AddShadedMesh(appMesh, modelComponent->materials[mesh->materialId]);
	}
	
	return gameObject;
}

GameObject * ClientAssetConverter::ConvertComponents(GameObject * gameObject, const Netcode::Asset::Model * ncAsset) {
	if(ncAsset->meshes.Size() > 0) {
		gameObject = ConvertModelComponent(gameObject, ncAsset);
	}

	if(ncAsset->colliders.Size() > 0) {
		gameObject = ConvertColliderComponent(gameObject, ncAsset);
	}
	
	return gameObject;
}

void ClientAssetConverter::ConvertScene() {
	if(auto it = sourceJson->FindMember(L"objects"); it != sourceJson->MemberEnd() && it->value.IsArray()) {
		const auto & objArray = it->value.GetArray();

		for(const auto & obj : objArray) {
			try {
				ConvertGameObject(obj);
			} catch(Netcode::ExceptionBase & e) {
				Log::Error("While converting scene: {0}", e.ToString());
			}
		}
	}
}
