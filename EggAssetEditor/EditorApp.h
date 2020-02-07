#pragma once

#include <Egg/Modules.h>
#include <Egg/Path.h>
#include "EditorFrameGraph.h"
#include <algorithm>
#include "Model.h"
#include "BoundingBoxHelpers.h"

namespace Egg::Module {

	class EditorApp : public AApp {
	public:
		BoneData boneData;
		PerFrameData perFrameData;
		PerObjectData perObjectData;
		BoneVisibilityData boneVisibilityData;

		EditorFrameGraph editorFrameGraph;

		DirectX::XMFLOAT3 cameraAhead;
		DirectX::XMFLOAT3 cameraPosition;
		DirectX::XMFLOAT3 cameraUp;

		float cameraFov;
		float cameraFarZ;
		float cameraNearZ;
		float cameraAspect;
		float cameraWorldDistance;

		float cameraPitch;
		float cameraYaw;
		float mouseSpeed;

		std::vector<GBuffer> gbuffers;
		std::vector<GBuffer> boneGbuffers;

		virtual void SetSelectedBones(std::vector<uint32_t> boneIndices) {
			memset(boneVisibilityData.BoneVisibility, 0, sizeof(BoneVisibilityData));
			for(uint32_t boneIndex : boneIndices) {
				boneVisibilityData.BoneVisibility[boneIndex] = 1;
			}
		}

		virtual void SetBoundingBoxes(std::vector<DirectX::BoundingBox> boundingBoxes) {
			BoundingBoxGenerator boundingBoxGen;

			for(const DirectX::BoundingBox & bb : boundingBoxes) {
				boundingBoxGen.UpdateForPoint(DirectX::XMFLOAT3{ bb.Center.x - bb.Extents.x, bb.Center.y - bb.Extents.y, bb.Center.z - bb.Extents.z });
				boundingBoxGen.UpdateForPoint(DirectX::XMFLOAT3{ bb.Center.x + bb.Extents.x, bb.Center.y + bb.Extents.y, bb.Center.z + bb.Extents.z });
			}

			DirectX::BoundingBox boundingBox = boundingBoxGen.GetBoundingBox();


			DirectX::XMVECTOR extentsV = DirectX::XMLoadFloat3(&boundingBox.Extents);
			DirectX::XMVECTOR lengthV = DirectX::XMVector3Length(extentsV);

			float len;
			DirectX::XMStoreFloat(&len, lengthV);

			DirectX::BoundingSphere bs{ boundingBox.Center, len };


			DirectX::XMMATRIX modelMat = DirectX::XMMatrixTranslation(-bs.Center.x, -bs.Center.y, -bs.Center.z);
			DirectX::XMVECTOR modelDet = DirectX::XMMatrixDeterminant(modelMat);
			DirectX::XMMATRIX invModelMat = DirectX::XMMatrixInverse(&modelDet, modelMat);

			DirectX::XMStoreFloat4x4A(&perObjectData.Model, DirectX::XMMatrixTranspose(modelMat));
			DirectX::XMStoreFloat4x4A(&perObjectData.InvModel, DirectX::XMMatrixTranspose(invModelMat));

			float worldDepthDistance = 3.0f * 1.25f * len;
			cameraWorldDistance = 2.0f * len;
			cameraNearZ = 0.1f;
			cameraFarZ = cameraNearZ + worldDepthDistance;

			UpdatePerFrameData();
		}

		virtual BoneData * GetBoneData() {
			return &boneData;
		}

		virtual void SetDrawGeometry(std::vector<LOD*> lodRefs) {
			if(!gbuffers.empty()) {
				for(GBuffer & gb : gbuffers) {
					if(gb.indexBuffer != 0) {
						graphics->resources->ReleaseResource(gb.indexBuffer);
					}
					graphics->resources->ReleaseResource(gb.vertexBuffer);
				}
				gbuffers.clear();
			}

			for(LOD * lod : lodRefs) {
				GBuffer gbuffer;
			    gbuffer.indexBuffer = graphics->resources->CreateIndexBuffer(lod->indexDataSizeInBytes, DXGI_FORMAT_R32_UINT, ResourceType::PERMANENT_UPLOAD, ResourceState::ANY_READ);
				gbuffer.vertexBuffer = graphics->resources->CreateVertexBuffer(lod->vertexDataSizeInBytes, lod->vertexDataSizeInBytes / lod->vertexCount, ResourceType::PERMANENT_UPLOAD, ResourceState::ANY_READ);
				gbuffer.vertexCount = lod->vertexCount;
				gbuffer.indexCount = lod->indexCount;
				gbuffers.push_back(gbuffer);
				graphics->resources->CopyConstants(gbuffer.indexBuffer, lod->indexData.get(), lod->indexDataSizeInBytes);
				graphics->resources->CopyConstants(gbuffer.vertexBuffer, lod->vertexData.get(), lod->vertexDataSizeInBytes);
			}
		}

		/*
		Initialize modules
		*/
		virtual void Setup(IModuleFactory * factory) override {
			Path::SetMediaRoot(L"Media");
			Path::SetShaderRoot(L"Shaders");

			events = std::make_unique<Egg::Module::AppEventSystem>();

			graphics = factory->CreateGraphicsModule(this, 0);

			StartModule(graphics.get());

			for(uint32_t i = 0; i < 128; ++i) {
				DirectX::XMStoreFloat4x4A(&boneData.BindTransform[i], DirectX::XMMatrixIdentity());
				DirectX::XMStoreFloat4x4A(&boneData.ToRootTransform[i], DirectX::XMMatrixIdentity());
			}

			cameraNearZ = 1.0f;
			cameraFarZ = 1000.0f;
			cameraFov = DirectX::XM_PIDIV4;
			cameraAspect = graphics->GetAspectRatio();

			cameraWorldDistance = 1.0f;
			cameraPosition = DirectX::XMFLOAT3{ 0.0f, 0.0f, 180.0f };
			cameraAhead = DirectX::XMFLOAT3{ 0.0f, 0.0f, -1.0f };
			cameraUp = DirectX::XMFLOAT3{ 0.0f, 1.0f, 0.0f };

			mouseSpeed = 0.0005f;

			memset(boneVisibilityData.BoneVisibility, 0, sizeof(BoneVisibilityData));

			UpdatePerFrameData();

			editorFrameGraph.CreatePermanentResources(graphics.get());
		}

		void UpdatePerFrameData() {
			perFrameData.eyePos = DirectX::XMFLOAT4A{ cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f };

			DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovRH(cameraFov, cameraAspect, cameraNearZ, cameraFarZ);

			DirectX::XMVECTOR aheadV = DirectX::XMLoadFloat3(&cameraAhead);
			DirectX::XMVECTOR upV = DirectX::XMLoadFloat3(&cameraUp);
			DirectX::XMVECTOR posV = DirectX::XMVectorScale(aheadV, cameraWorldDistance);
			DirectX::XMVECTOR realAheadV = DirectX::XMVectorNegate(aheadV);

			DirectX::XMMATRIX view = DirectX::XMMatrixLookAtRH(posV, DirectX::g_XMZero, upV);

			DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(view, proj);

			DirectX::XMMATRIX viewFromOrigo = DirectX::XMMatrixLookToRH(DirectX::g_XMZero, realAheadV, upV);
			DirectX::XMMATRIX rayDir = DirectX::XMMatrixMultiply(viewFromOrigo, proj);
			DirectX::XMVECTOR rayDirDet = DirectX::XMMatrixDeterminant(rayDir);
			rayDir = DirectX::XMMatrixInverse(&rayDirDet, rayDir);

			DirectX::XMStoreFloat4x4A(&perFrameData.Proj, DirectX::XMMatrixTranspose(proj));
			DirectX::XMStoreFloat4x4A(&perFrameData.View, DirectX::XMMatrixTranspose(view));
			DirectX::XMStoreFloat4x4A(&perFrameData.ViewProj, DirectX::XMMatrixTranspose(viewProj));
			DirectX::XMStoreFloat4x4A(&perFrameData.RayDir, DirectX::XMMatrixTranspose(rayDir));

			DirectX::XMStoreFloat4x4A(&perFrameData.ProjTex, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4A(&perFrameData.ViewProjInv, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4A(&perFrameData.ViewInv, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4A(&perFrameData.ProjInv, DirectX::XMMatrixIdentity());
			perFrameData.nearZ = cameraNearZ;
			perFrameData.farZ = cameraFarZ;
			perFrameData.fov = cameraFov;
			perFrameData.aspectRatio = cameraAspect;
		}

		void MouseMoved(const DirectX::XMFLOAT2 & mouseDelta) {
			cameraPitch += mouseSpeed * mouseDelta.y;
			cameraPitch = std::clamp(cameraPitch, -(DirectX::XM_PIDIV2 - 0.00001f), (DirectX::XM_PIDIV2 - 0.00001f));
			cameraYaw += mouseSpeed * mouseDelta.x;

			if(cameraYaw < (-DirectX::XM_PI)) {
				cameraYaw += DirectX::XM_2PI;
			}

			if(cameraYaw > (DirectX::XM_PI)) {
				cameraYaw -= DirectX::XM_2PI;
			}

			DirectX::XMVECTOR cameraYawQuat = DirectX::XMQuaternionRotationRollPitchYaw(0.0f, cameraYaw, 0.0f);
			
			DirectX::XMFLOAT3 minusUnitZ{ 0.0f, 0.0f, -1.0f };
			DirectX::XMVECTOR cameraQuat = DirectX::XMQuaternionRotationRollPitchYaw(cameraPitch, cameraYaw, 0.0f);
			DirectX::XMVECTOR aheadStart = DirectX::XMLoadFloat3(&minusUnitZ);

			DirectX::XMStoreFloat3(&cameraAhead, DirectX::XMVector3Normalize(DirectX::XMVector3Rotate(aheadStart, cameraQuat)));

			UpdatePerFrameData();

			Run();
		}

		/*
		Advance simulation, update modules
		*/
		virtual void Run() override {
			graphics->frame->Prepare();
		
			editorFrameGraph.boneData = &boneData;
			editorFrameGraph.perFrameData = &perFrameData;
			editorFrameGraph.perObjectData = &perObjectData;
			editorFrameGraph.boneVisibilityData = &boneVisibilityData;
			editorFrameGraph.gbufferPass_Input = gbuffers;

			FrameGraphBuilder builder;
			editorFrameGraph.CreateFrameGraph(builder);
			FrameGraph graph = builder.Build(graphics->resources);
			graph.Render(graphics->renderer);

			graphics->frame->Render();
			graphics->frame->Present();
		}

		/*
		Properly shutdown the application
		*/
		virtual void Exit() override {
			ShutdownModule(graphics.get());
		}
	};

}
