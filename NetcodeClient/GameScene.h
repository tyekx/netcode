#pragma once

#include <Netcode/URI/Model.h>
#include <Netcode/PhysXWrapper.h>
#include <Netcode/MathExt.h>
#include <NetcodeFoundation/Json.h>
#include "GameObject.h"
#include "Scene.h"
#include "PhysxHelpers.h"
#include "Snippets.h"
#include "Scripts/DevCameraScript.h"
#include "Asset/GameObjectCatalog.h"

class GameScene : public Scene<GameObject> {
public:
	PerFrameData perFrameData;

	GameScene();
	GameScene(GameScene &&) = default;
	GameScene & operator=(GameScene &&) noexcept = default;
	~GameScene() = default;
	GameScene(const GameScene &) = delete;
	GameScene & operator=(const GameScene &) = delete;

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
		/*ssaoData.Offsets[0] = Netcode::Float4(+1.0f, +1.0f, +1.0f, 0.0f);
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
		ssaoData.weights[2] = Netcode::Float4(&blurWeights[8]);*/
	}

	void Spawn(GameObject * obj) {
		if(!obj->IsDeletable()) {
			if(obj->HasComponent<Collider>()) {
				SpawnPhysxActor(obj->GetComponent<Collider>()->actor.Get());
			}

			if(!obj->IsActive()) {
				if(obj->HasComponent<Script>()) {
					obj->GetComponent<Script>()->BeginPlay(obj);
				}
			}
			obj->Spawn();
		}
	}

	GameObject * CloneWithHierarchy(GameObject * src);

	GameObject * Clone(GameObject * src);

	void UpdatePerFrameCb() {
		Transform * transform = GetCamera()->GetComponent<Transform>();
		Camera * camComponent = GetCamera()->GetComponent<Camera>();

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

		perFrameData.ambientLightIntensity = Netcode::Float4{ 0.1f, 0.1f, 0.1f, 1.0f };
		perFrameData.eyePos = eyePos.XYZ1();

		Netcode::Matrix projInv = invVp * view;
		Netcode::Matrix viewInv = proj * invVp;

		perFrameData.ViewInv = viewInv.Transpose();
		perFrameData.ProjInv = projInv.Transpose();
		perFrameData.ProjTex = (proj * tex).Transpose();
	}


	physx::PxController * CreateController();
};

class GameSceneManager {
	GameScene activeScene;
	GameObjectCatalog catalog;
	GameObject * devCam;
	

	struct CamState {
		Transform tr;
		Camera cam;
		DevCameraScript dcs;
	} cameraState;

	Netcode::URI::Model activeSceneUri;

	void LoadSceneDetail(const Netcode::URI::Model & uri);

public:

 	[[nodiscard]] const Netcode::URI::Model & GetActiveSceneUri() const {
		return activeSceneUri;
	}

	void CloseScene();

	void ClearCache() {
		catalog.Clear();
	}

	void ReloadScene();

	void LoadScene(const Netcode::URI::Model & uri);

	GameScene* GetScene() {
		return &activeScene;
	}
};
