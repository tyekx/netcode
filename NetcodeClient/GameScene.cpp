#include "GameScene.h"
#include "Services.h"
#include "Scripts/DevCameraScript.h"
#include <NetcodeAssetLib/JsonUtility.h>
#include <memory>
#include <Netcode/IO/Json.h>
#include "Asset/ClientConverters.h"

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

GameObject* GameScene::FindByName(const std::string& name) {
	for(auto it = storage.begin(); it != nullptr; ++it) {
		if(it->name == name) {
			return it.operator->();
		}
	}
	return nullptr;
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

void GameSceneManager::LoadSceneDetail(const Netcode::URI::Model & uri) {
	if(!Netcode::IO::File::Exists(activeSceneUri.GetModelPath())) {
		Log::Error("Scene file was not found");
		return;
	}

	Netcode::JsonDocument json;
	Netcode::IO::ParseJsonFromFile(json, activeSceneUri.GetModelPath());

	if(json.IsNull()) {
		Log::Error("Failed to parse scene file");
		return;
	}

	if(!json.IsObject()) {
		Log::Error("Sanity check failed, scene file root is not an object");
		return;
	}

	ClientAssetConverter converter{ &catalog, &json, &activeScene };
	converter.ConvertScene();
	
	catalog.Reset();
}

void GameSceneManager::CloseScene() {
	cameraState.tr = *devCam->GetComponent<Transform>();
	cameraState.cam = *devCam->GetComponent<Camera>();
	cameraState.dcs = *static_cast<DevCameraScript *>(devCam->GetComponent<Script>()->scripts[0].get());

	activeScene.Clear();
	catalog.Clear();
}

void GameSceneManager::ReloadScene() {
	devCam = activeScene.Create();
	auto [tr, cam, script] = devCam->AddComponents<Transform, Camera, Script>();
	std::unique_ptr<DevCameraScript> sc = std::make_unique<DevCameraScript>();
	*sc = cameraState.dcs;
	sc->transform = tr;
	sc->camera = cam;
	script->AddScript(std::move(sc));
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
	script->AddScript(std::make_unique<DevCameraScript>());
	activeScene.SetCamera(devCam);
	activeScene.Spawn(devCam);

	catalog.SetScene(&activeScene);
	
	LoadSceneDetail(activeSceneUri);
}
