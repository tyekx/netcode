#pragma once

/*
Credits: a chunk of the SSAO implementation was based on the book "3D Game Programming with DirectX 12"
ISBN: 978-1942270065
*/

#include <Netcode/PhysXWrapper.h>
#include <Netcode/MathExt.h>
#include "GameObject.h"
#include "Scene.h"
#include "PhysxHelpers.h"

class GameSceneSimulationEventCallback : public physx::PxSimulationEventCallback {
	virtual void onConstraintBreak(physx::PxConstraintInfo * constraints, physx::PxU32 count) override;
	virtual void onWake(physx::PxActor ** actors, physx::PxU32 count) override;
	virtual void onSleep(physx::PxActor ** actors, physx::PxU32 count) override;
	virtual void onContact(const physx::PxContactPairHeader & pairHeader, const physx::PxContactPair * pairs, physx::PxU32 nbPairs) override;
	virtual void onTrigger(physx::PxTriggerPair * pairs, physx::PxU32 count) override;
	virtual void onAdvance(const physx::PxRigidBody * const * bodyBuffer, const physx::PxTransform * poseBuffer, const physx::PxU32 count) override;
};

__declspec(align(16)) class GameScene : public Scene<GameObject> {
protected:
	physx::PxControllerManager * controllerManager;
	physx::PxMaterial * controllerMaterial;
	std::unique_ptr<GameSceneSimulationEventCallback> sceneCallback;
	void * __structPadding0;
	GameScene() = default;
public:
	PerFrameData perFrameData;
	SsaoData ssaoData;

	GameScene(Netcode::Physics::PhysX & px) : GameScene() {
		sceneCallback = std::make_unique<GameSceneSimulationEventCallback>();
		physx::PxSceneDesc sceneDesc{ px.physics->getTolerancesScale() };
		sceneDesc.gravity = physx::PxVec3{ 0.0f, -981.0f, 0.0f };
		sceneDesc.cpuDispatcher = px.dispatcher;
		sceneDesc.filterShader = SimulationFilterShader;
		sceneDesc.simulationEventCallback = sceneCallback.get();
		controllerMaterial = px.physics->createMaterial(0.5f, 0.6f, 0.6f);

		physx::PxScene * pScene = px.physics->createScene(sceneDesc);
		controllerManager = PxCreateControllerManager(*pScene);
		physx::PxPvdSceneClient * pvdClient = pScene->getScenePvdClient();
		if(pvdClient) {
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}

		Scene::SetPhysXScene(pScene);
	}

	std::vector<float> CalcGaussWeights(float sigma)
	{
		float twoSigma2 = 2.0f * sigma * sigma;

		int blurRadius = (int)ceil(2.0f * sigma);

		std::vector<float> weights;
		weights.resize(2 * blurRadius + 1);

		float weightSum = 0.0f;

		for(int i = -blurRadius; i <= blurRadius; ++i)
		{
			float x = (float)i;

			weights[i + blurRadius] = expf(-x * x / twoSigma2);

			weightSum += weights[i + blurRadius];
		}

		for(int i = 0; i < weights.size(); ++i)
		{
			weights[i] /= weightSum;
		}

		return weights;
	}

	void Setup() {
		ssaoData.Offsets[0] = Netcode::Float4(+1.0f, +1.0f, +1.0f, 0.0f);
		ssaoData.Offsets[1] = Netcode::Float4(-1.0f, -1.0f, -1.0f, 0.0f);

		ssaoData.Offsets[2] = Netcode::Float4(-1.0f, +1.0f, +1.0f, 0.0f);
		ssaoData.Offsets[3] = Netcode::Float4(+1.0f, -1.0f, -1.0f, 0.0f);

		ssaoData.Offsets[4] = Netcode::Float4(+1.0f, +1.0f, -1.0f, 0.0f);
		ssaoData.Offsets[5] = Netcode::Float4(-1.0f, -1.0f, +1.0f, 0.0f);

		ssaoData.Offsets[6] = Netcode::Float4(-1.0f, +1.0f, -1.0f, 0.0f);
		ssaoData.Offsets[7] = Netcode::Float4(+1.0f, -1.0f, +1.0f, 0.0f);

		ssaoData.Offsets[8] = Netcode::Float4(-1.0f, 0.0f, 0.0f, 0.0f);
		ssaoData.Offsets[9] = Netcode::Float4(+1.0f, 0.0f, 0.0f, 0.0f);

		ssaoData.Offsets[10] = Netcode::Float4(0.0f, -1.0f, 0.0f, 0.0f);
		ssaoData.Offsets[11] = Netcode::Float4(0.0f, +1.0f, 0.0f, 0.0f);

		ssaoData.Offsets[12] = Netcode::Float4(0.0f, 0.0f, -1.0f, 0.0f);
		ssaoData.Offsets[13] = Netcode::Float4(0.0f, 0.0f, +1.0f, 0.0f);

		for(int i = 0; i < SsaoData::SAMPLE_COUNT; ++i)
		{
			float s = Netcode::RandomFloat(0.3f, 1.0f);
			s *= s;

			Netcode::Vector3 v = ssaoData.Offsets[i];
			ssaoData.Offsets[i] = (v.Normalize() * s).XYZ0();
		}

		ssaoData.occlusionRadius = 10.0f;
		ssaoData.occlusionFadeStart = 4.0f;
		ssaoData.occlusionFadeEnd = 40.0f;
		ssaoData.surfaceEpsilon = 0.5f;

		auto blurWeights = CalcGaussWeights(2.5f);
		ssaoData.weights[0] = Netcode::Float4(&blurWeights[0]);
		ssaoData.weights[1] = Netcode::Float4(&blurWeights[4]);
		ssaoData.weights[2] = Netcode::Float4(&blurWeights[8]);
	}

	void Spawn(GameObject * obj) {
		if(!obj->IsDeletable()) {
			if(obj->HasComponent<Collider>()) {
				SpawnPhysxActor(obj->GetComponent<Collider>()->actorRef);
			}
			obj->Spawn();
		}
	}

	void UpdatePerFrameCb() {
		Transform * transform = cameraRef->GetComponent<Transform>();
		Camera * camComponent = cameraRef->GetComponent<Camera>();

		const Netcode::Vector3 eyePos = transform->worldPosition;

		const Netcode::Matrix view = Netcode::LookToMatrix(eyePos, camComponent->ahead, camComponent->up);
		const Netcode::Matrix proj = Netcode::PerspectiveFovMatrix(camComponent->fov, camComponent->aspect, camComponent->nearPlane, camComponent->farPlane);

		const Netcode::Matrix vp = view * proj;
		const Netcode::Matrix invVp = vp.Invert();

		const Netcode::Matrix tex = Netcode::Float4x4{  0.5f,  0.0f, 0.0f, 0.0f,
													  0.0f, -0.5f, 0.0f, 0.0f,
													  0.0f,  0.0f, 1.0f, 0.0f,
													  0.5f,  0.5f, 0.0f, 1.0f };

		Netcode::Matrix rayDir = Netcode::LookToMatrix(Netcode::Float3{ }, camComponent->ahead, camComponent->up) * proj;
		rayDir = rayDir.Invert();

		perFrameData.farZ = camComponent->farPlane;
		perFrameData.nearZ = camComponent->nearPlane;
		perFrameData.fov = camComponent->fov;
		perFrameData.aspectRatio = camComponent->aspect;

		perFrameData.RayDir = rayDir.Transpose();
		perFrameData.View = view.Transpose();
		perFrameData.Proj = proj.Transpose();
		perFrameData.ViewProj = vp.Transpose();
		perFrameData.ViewProjInv = invVp.Transpose();

		perFrameData.eyePos = eyePos.XYZ1();

		Netcode::Matrix projInv = invVp * view;
		Netcode::Matrix viewInv = proj * invVp;

		perFrameData.ViewInv = viewInv.Transpose();
		perFrameData.ProjInv = projInv.Transpose();
		perFrameData.ProjTex = (proj * tex).Transpose();
	}


	physx::PxController * CreateController() {
		physx::PxCapsuleControllerDesc cd;
		cd.behaviorCallback = NULL;
		cd.climbingMode = physx::PxCapsuleClimbingMode::eEASY;
		cd.contactOffset = 0.1f;
		cd.density = 10.0f;
		cd.invisibleWallHeight = 0.0f;
		cd.material = controllerMaterial;
		cd.position = physx::PxExtendedVec3{ 0.0, 200.0, 0.0 };
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
};
