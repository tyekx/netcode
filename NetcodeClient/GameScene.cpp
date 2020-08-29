#include "GameScene.h"
#include "Services.h"
#include "DevCameraScript.h"
#include <NetcodeAssetLib/JsonUtility.h>
#include <memory>

GameScene::GameScene() : Scene<GameObject>{} {
	auto * pxService = Service::Get<Netcode::Physics::PhysX>();

	physx::PxSceneDesc sceneDesc{ pxService->physics->getTolerancesScale() };
	sceneDesc.gravity = physx::PxVec3{ 0.0f, -981.0f, 0.0f };
	sceneDesc.cpuDispatcher = pxService->dispatcher.Get();
	sceneDesc.filterShader = SimulationFilterShader;

	physx::PxScene * pScene = pxService->physics->createScene(sceneDesc);
	controllerManager = PxCreateControllerManager(*pScene);
	physx::PxPvdSceneClient * pvdClient = pScene->getScenePvdClient();
	if(pvdClient) {
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	Scene::SetPhysXScene(pScene);
}

GameObject * GameScene::CloneWithHierarchy(GameObject * src)
{
	GameObject * pThis = Clone(src);
	for(auto * child : src->Children()) {
		pThis->AddChild(Clone(child));
	}
	return pThis;
}

GameObject * GameScene::Clone(GameObject * src) {
	GameObject * obj = Create();

	if(src->HasComponent<Transform>()) {
		Transform * tr = obj->AddComponent<Transform>();
		Transform * srcTr = src->GetComponent<Transform>();
		tr->position = srcTr->position;
		tr->rotation = srcTr->rotation;
		tr->scale = srcTr->scale;
	}

	if(src->HasComponent<Collider>()) {
		Collider * dstCollider = obj->AddComponent<Collider>();
		Collider * srcCollider = src->GetComponent<Collider>();

		physx::PxTransform t{ physx::PxIdentity };
		if(obj->HasComponent<Transform>()) {
			Transform * transform = obj->GetComponent<Transform>();
			t = physx::PxTransform{
				Netcode::ToPxVec3(transform->position), Netcode::ToPxQuat(transform->rotation)
			};
		}

		auto * ptr = physx::PxCloneDynamic(pxScene->getPhysics(), t, *static_cast<physx::PxRigidDynamic *>(srcCollider->actor.Get()));
		ptr->userData = nullptr;
		dstCollider->actor.Reset(ptr);
	}

	if(src->HasComponent<Model>()) {
		Model * m = obj->AddComponent<Model>();
		const Model * s = src->GetComponent<Model>();
		m->meshes = s->meshes;
		m->materials = s->materials;
	}

	return obj;
}

physx::PxController * GameScene::CreateController() {
	auto * pxService = Service::Get<Netcode::Physics::PhysX>();
	physx::PxCapsuleControllerDesc cd;
	cd.behaviorCallback = NULL;
	cd.climbingMode = physx::PxCapsuleClimbingMode::eEASY;
	cd.contactOffset = 0.1f;
	cd.density = 10.0f;
	cd.invisibleWallHeight = 0.0f;
	cd.material = pxService->defaultMaterial.Get();
	cd.position = physx::PxExtendedVec3{ 0.0, 300.0, 0.0 };
	cd.nonWalkableMode = physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING;
	cd.registerDeletionListener = true;
	cd.reportCallback = NULL;
	cd.scaleCoeff = 0.8f;
	cd.slopeLimit = 0.7071f;
	cd.stepOffset = 5.0f;
	cd.upDirection = physx::PxVec3{ 0.0f, 1.0f, 0.0f };
	cd.volumeGrowth = 1.5f;
	cd.height = 80.0f;
	cd.radius = 60.0f;
	return controllerManager->createController(cd);
}

void GameSceneManager::CompleteSceneLoading() {
	auto it = std::remove_if(std::begin(gameObjectCatalog), std::end(gameObjectCatalog), [](const GameObjectMetadata & mData) -> bool {
		return mData.touched == 0;
	});

	gameObjectCatalog.erase(it, std::end(gameObjectCatalog));

	for(auto & i : gameObjectCatalog) {
		i.touched = 0;
	}
}

static void LoadComponents(Netcode::Asset::Model * model, GameObject * gameObject);

inline Netcode::Float3 LoadFloat3FromJson(const json11::Json::object & obj, const std::string & key, Netcode::Float3 defValue = Netcode::Float3::Zero) {
	if(auto it = obj.find(key); it != obj.end() && it->second.is_array()) {
		return Netcode::Asset::LoadFloat3(it->second.array_items());
	}
	return defValue;
}

inline float LoadFloatFromJson(const json11::Json::object & obj, const std::string & key, float defValue = 0.0f) {
	if(auto it = obj.find(key); it != obj.end() && it->second.is_number()) {
		return static_cast<float>(it->second.number_value());
	}
	return defValue;
}

static void LoadMaterialDataFromJson(const json11::Json::object & obj, Ptr<Netcode::Material> ncMat) {
	if(auto it = obj.find("roughness"); it != obj.end() && it->second.is_number()) {
		float roughness = static_cast<float>(it->second.number_value());
		ncMat->SetParameter<float>(Netcode::MaterialParamId::ROUGHNESS, roughness);
	}

	if(auto it = obj.find("reflectance"); it != obj.end() && it->second.is_number()) {
		float reflectance = static_cast<float>(it->second.number_value());
		ncMat->SetParameter<float>(Netcode::MaterialParamId::REFLECTANCE, reflectance);
	}

	if(auto it = obj.find("metal_mask"); it != obj.end() && it->second.is_bool()) {
		ncMat->SetParameter<bool>(Netcode::MaterialParamId::METAL_MASK, it->second.bool_value());
	}
}

GameObject * GameSceneManager::LoadLightFromJson(GameScene * scene, const json11::Json::object & values) {

	if(auto it = values.find("type"); it != values.end() && it->second.is_number()) {
		Netcode::LightType type = static_cast<Netcode::LightType>(it->second.int_value());

		if(type != Netcode::LightType::DIRECTIONAL && type != Netcode::LightType::OMNI) {
			return nullptr;
		}

		GameObject * gameObj = scene->Create();
		Transform * t = gameObj->AddComponent<Transform>();
		Netcode::Light * l = gameObj->AddComponent<Netcode::Light>();
		Netcode::Float3 intensity = LoadFloat3FromJson(values, "intensity");
		l->intensity = Netcode::Float4{ intensity.x, intensity.y, intensity.z, 1.0f };
		l->type = type;
		switch(type) {
			case Netcode::LightType::DIRECTIONAL: {
				Netcode::Vector3 dirV = LoadFloat3FromJson(values, "direction");
				Netcode::Float3 dir = dirV.Normalize();
				l->position = Netcode::Float3{ -dir.x, -dir.y, -dir.z };
			} break;
			case Netcode::LightType::OMNI:
				l->minimumDistance = LoadFloatFromJson(values, "minDist");
				l->maximumDistance = LoadFloatFromJson(values, "maxDist");
				l->referenceDistance = LoadFloatFromJson(values, "reference");
				l->position = LoadFloat3FromJson(values, "position");
				break;
			default: break;
		}

		scene->Spawn(gameObj);
		return gameObj;
	}

	return nullptr;
}

GameObject* GameSceneManager::LoadGameObjectFromJson(GameScene * scene, const json11::Json::object & values) {
	AssetManager * assetManager = Service::Get<AssetManager>();
	GameObject * gameObj = nullptr;

	if(auto it = values.find("asset"); it != values.end() && it->second.is_string()) {
		std::wstring assetPath = Netcode::Utility::ToWideString(it->second.string_value());

		Netcode::URI::Model modelUri{ std::move(assetPath) };

		auto cit = std::find_if(std::begin(gameObjectCatalog), std::end(gameObjectCatalog), [&modelUri](const GameObjectMetadata & mdata) -> bool {
			return mdata.uri.GetFullPath() == modelUri.GetFullPath();
		});

		if(cit != std::end(gameObjectCatalog)) {
			gameObj = scene->Clone(cit->gameObject);

			Log::Debug("Cloned asset for scene: {0}", it->second.string_value());

			if(cit->touched == 0) {
				cit->touched = 1;
				cit->gameObject = gameObj;
			}

		} else {
			Log::Debug("Loading asset from scene: {0}", it->second.string_value());

			gameObj = scene->Create();
			LoadComponents(assetManager->Import(modelUri), gameObj);

			GameObjectMetadata mData;
			mData.gameObject = gameObj;
			mData.touched = 1;
			mData.uri = modelUri;

			gameObjectCatalog.emplace_back(std::move(mData));
		}
	} else {
		Log::Warn("field 'asset' was not found, skipping object");
	}

	if(Transform * tr = gameObj->GetComponent<Transform>(); tr != nullptr) {
		if(auto it = values.find("position"); it != values.end() && it->second.is_array()) {
			tr->position = Netcode::Asset::LoadFloat3(it->second);
		}

		if(auto it = values.find("eulerAngles"); it != values.end() && it->second.is_array()) {
			tr->rotation = Netcode::Quaternion::FromEulerAngles(Netcode::Asset::LoadFloat3(it->second));
		}
	}

	if(Model * modelComponent = gameObj->GetComponent<Model>(); modelComponent != nullptr) {
		if(auto it = values.find("materialData"); it != values.end() && it->second.is_array()) {
			const auto & matDataArray = it->second.array_items();

			for(const auto & matDataObj : matDataArray) {
				if(matDataObj.is_object()) {
					const auto & matData = matDataObj.object_items();
					if(auto matIdx = matData.find("materialIndex"); matIdx != matData.end() && matIdx->second.is_number()) {
						size_t materialIndex = static_cast<size_t>(matIdx->second.int_value());

						if(materialIndex < modelComponent->materials.size()) {
							Ref<Netcode::Material> ncMat = modelComponent->materials[materialIndex]->Clone();
							Log::Debug("Cloned material: {0}", ncMat->GetName());

							for(auto & shadedMesh : modelComponent->meshes) {
								if(shadedMesh.material == modelComponent->materials[materialIndex]) {
									shadedMesh.material = ncMat;
								}
							}

							if(ncMat != nullptr) {
								LoadMaterialDataFromJson(matData, ncMat.get());
							}

							modelComponent->materials[materialIndex] = ncMat;
						}
					}
				}
			}
		}
	}

	if(auto it = values.find("name"); it != values.end() && it->second.is_string()) {
		gameObj->name = it->second.string_value();
	}

	if(auto it = values.find("children"); it != values.end() && it->second.is_array()) {
		for(const auto & cit : it->second.array_items()) {
			if(cit.is_object()) {
				GameObject * child = LoadGameObjectFromJson(scene, cit.object_items());

				if(child != nullptr) {
					gameObj->AddChild(child);
				}
			}
		}
	}

	scene->Spawn(gameObj);
	return gameObj;
}

void GameSceneManager::LoadSceneDetail(const json11::Json::object & json) {
	if(auto it = json.find("objects"); it != json.end() && it->second.is_array()) {
		const auto & objArray = it->second.array_items();

		for(const auto & obj : objArray) {
			if(obj.is_object()) {
				LoadGameObjectFromJson(&activeScene, obj.object_items());
			}
		}
	}

	if(auto it = json.find("lights"); it != json.end() && it->second.is_array()) {
		const auto & objArray = it->second.array_items();

		for(const auto & obj : objArray) {
			if(obj.is_object()) {
				LoadLightFromJson(&activeScene, obj.object_items());
			}
		}
	}
}

void GameSceneManager::LoadSceneDetail(const Netcode::URI::Model & uri) {
	if(!Netcode::IO::File::Exists(activeSceneUri.GetModelPath())) {
		Log::Error("Scene file was not found");
		return;
	}

	json11::Json json = LoadJsonFile(activeSceneUri.GetModelPath());

	if(json.is_null()) {
		Log::Error("Failed to parse scene file");
		return;
	}

	if(!json.is_object()) {
		Log::Error("Sanity check failed, scene file root is not an object");
		return;
	}

	LoadSceneDetail(json.object_items());
	CompleteSceneLoading();
}

void GameSceneManager::CloseScene() {
	cameraState.tr = *devCam->GetComponent<Transform>();
	cameraState.cam = *devCam->GetComponent<Camera>();
	cameraState.dcs = *static_cast<DevCameraScript *>(devCam->GetComponent<Script>()->behavior.get());

	activeScene.Clear();
	gameObjectCatalog.clear();
}

void GameSceneManager::ReloadScene() {
	devCam = activeScene.Create();
	auto [tr, cam, script] = devCam->AddComponents<Transform, Camera, Script>();
	std::unique_ptr<DevCameraScript> sc = std::make_unique<DevCameraScript>();
	*sc = cameraState.dcs;
	sc->transform = tr;
	sc->camera = cam;
	script->SetBehavior(std::move(sc));
	tr->position = cameraState.tr.position;
	tr->rotation = cameraState.tr.rotation;
	cam->ahead = cameraState.cam.ahead;
	cam->aspect = cameraState.cam.aspect;
	cam->farPlane = cameraState.cam.farPlane;
	cam->nearPlane = cameraState.cam.nearPlane;
	cam->fov = cameraState.cam.fov;
	cam->up = cameraState.cam.up;
	activeScene.SetCamera(devCam);
	activeScene.Spawn(devCam);

	LoadSceneDetail(activeSceneUri);
}

void GameSceneManager::LoadScene(const Netcode::URI::Model & uri) {
	activeSceneUri = uri;

	devCam = activeScene.Create();
	auto [tr, cam, script] = devCam->AddComponents<Transform, Camera, Script>();
	script->SetBehavior(std::make_unique<DevCameraScript>());
	script->Setup(devCam);
	activeScene.SetCamera(devCam);
	activeScene.Spawn(devCam);

	LoadSceneDetail(activeSceneUri);
}

static void LoadModelComponent(Netcode::Asset::Model * model, Model * modelComponent);
static void LoadColliderComponent(Netcode::Asset::Model * model, Collider * colliderComponent);

void LoadComponents(Netcode::Asset::Model * model, GameObject * gameObject) {
	gameObject->AddComponent<Transform>();

	if(model->meshes.Size() > 0) {
		Model * modelComponent = gameObject->AddComponent<Model>();
		LoadModelComponent(model, modelComponent);
	}

	if(model->colliders.Size() > 0) {
		Collider * colliderComponent = gameObject->AddComponent<Collider>();
		LoadColliderComponent(model, colliderComponent);
		colliderComponent->actor->userData = gameObject;
	}
}

void LoadColliderComponent(Netcode::Asset::Model * model, Collider * colliderComponent) {
	if(model->colliders.Size() == 0) {
		return;
	}

	auto * pxService = Service::Get<Netcode::Physics::PhysX>();

	physx::PxPhysics * pxp = pxService->physics.Get();
	Netcode::PxPtr<physx::PxRigidDynamic> actor = pxp->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
	actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
	std::vector<ColliderShape> netcodeShapes;
	netcodeShapes.reserve(model->colliders.Size());

	for(size_t i = 0; i < model->colliders.Size(); ++i) {
		const Netcode::Asset::Collider * netcodeCollider = model->colliders.Data() + i;
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
			const auto & mesh = model->meshes[0];
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
}

void LoadModelComponent(Netcode::Asset::Model * model, Model * modelComponent) {
	auto * graphics = Service::Get<Netcode::Module::IGraphicsModule *>();

	for(size_t matIdx = 0; matIdx < model->materials.Size(); matIdx++) {
		const Netcode::Asset::Material * mat = model->materials.Data() + matIdx;

		std::string ncMatName{ std::string_view{ mat->name, Netcode::Utility::ArraySize(mat->name) } };
		Ref<Netcode::Material> ncMat = std::make_shared<Netcode::BrdfMaterial>(static_cast<Netcode::MaterialType>(mat->type), ncMatName);
		uint32_t texFlags = 0;

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
		ncMat->SetParameter<bool>(Netcode::MaterialParamId::METAL_MASK, false);
		ncMat->SetParameter<float>(Netcode::MaterialParamId::ROUGHNESS, ncMat->GetRequiredParameter<float>(Netcode::MaterialParamId::ROUGHNESS) / 255.0f);

		modelComponent->materials.push_back(ncMat);
	}

	for(size_t meshIdx = 0; meshIdx < model->meshes.Size(); ++meshIdx) {
		const Netcode::Asset::Mesh * mesh = model->meshes.Data() + meshIdx;

		modelComponent->perObjectData.Model = Netcode::Float4x4::Identity;
		modelComponent->perObjectData.InvModel = Netcode::Float4x4::Identity;

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


		modelComponent->AddShadedMesh(appMesh, modelComponent->materials[mesh->materialId]);
	}
}

