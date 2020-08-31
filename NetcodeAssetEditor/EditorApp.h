#pragma once

#include <Netcode/Modules.h>
#include "EditorFrameGraph.h"
#include <algorithm>
#include <NetcodeAssetLib/IntermediateModel.h>
#include "BoundingBoxHelpers.h"
#include <Netcode/BasicGeometry.h>
#include <Netcode/URI/Texture.h>
#include <Netcode/IO/Path.h>
#include <Netcode/IO/Json.h>
#include <Netcode/IO/File.h>
#include <Netcode/Config.h>

namespace Netcode::Module {

	class EditorApp : public AApp, TAppEventHandler {
	public:
		BoneData boneData;
		PerFrameData perFrameData;
		PerObjectData perObjectData;
		BoneVisibilityData boneVisibilityData;
		LightData lightData;

		EditorFrameGraph editorFrameGraph;

		Netcode::Float3 cameraLookAt;
		Netcode::Float3 cameraAhead;
		Netcode::Float3 cameraPosition;
		Netcode::Float3 cameraUp;

		float cameraFov;
		float cameraFarZ;
		float cameraNearZ;
		float cameraAspect;
		float cameraWorldDistance;

		float cameraPitch;
		float cameraYaw;
		float mouseSpeed;

		bool frameValid;

		std::vector<GBuffer> gbuffers;
		std::vector<Ref<Netcode::Material>> materials;
		std::vector<Ref<Netcode::Material>> matAssoc;
		std::vector<Intermediate::Collider> colliders;

		Netcode::Float4x4 currentOfflineTransform;

		virtual void SetMaterials(const std::vector<Intermediate::Mesh> & meshes, const std::vector<Ref<Material>> & mats, bool forceUpdate = false);

		void ApplyTexture(uint32_t materialIndex, Netcode::MaterialParamId texType, Netcode::MaterialParamId texPath, const URI::Texture & texUri, bool forceUpdate = false) {
			Ptr<Material> mat = materials[materialIndex].get();

			const Netcode::URI::Texture & existingUri = mat->GetRequiredParameter<Netcode::URI::Texture>(texPath);

			if(!forceUpdate && texUri.GetFullPath() == existingUri.GetFullPath()) {
				return;
			}

			uint32_t texIdx = static_cast<uint32_t>(texType) - static_cast<uint32_t>(MaterialParamId::TEXTURE_DIFFUSE);

			if(texUri.Empty()) {
				mat->SetParameter(texType, nullptr);
				mat->SetParameter(texPath, texUri);
				mat->GetResourceView(0)->ClearSRV(texIdx, Graphics::ResourceDimension::TEXTURE2D);
				return;
			}

			if(!Netcode::IO::File::Exists(texUri.GetTexturePath())) {
				return;
			}

			auto textureBuilder = graphics->CreateTextureBuilder();
			textureBuilder->LoadTexture2D(texUri);
			textureBuilder->SetMipLevels(6);

			Ref<Netcode::GpuResource> texResource = textureBuilder->Build();
			mat->SetParameter(texType, texResource);
			mat->SetParameter(texPath, texUri);
			mat->GetResourceView(0)->CreateSRV(texIdx, texResource.get());
		}

		virtual void SetColliders(std::vector<Intermediate::Collider> colls) {
			colliders = std::move(colls);
		}

		virtual void SetSelectedBones(std::vector<uint32_t> boneIndices) {
			memset(boneVisibilityData.BoneVisibility, 0, sizeof(BoneVisibilityData));
			for(uint32_t boneIndex : boneIndices) {
				boneVisibilityData.BoneVisibility[boneIndex] = 1;
			}
		}

		virtual void SetBoundingBoxes(std::vector<DirectX::BoundingBox> boundingBoxes) {
			BoundingBoxGenerator boundingBoxGen;

			for(const DirectX::BoundingBox & bb : boundingBoxes) {
				boundingBoxGen.UpdateForPoint(Netcode::Float3{ bb.Center.x - bb.Extents.x, bb.Center.y - bb.Extents.y, bb.Center.z - bb.Extents.z });
				boundingBoxGen.UpdateForPoint(Netcode::Float3{ bb.Center.x + bb.Extents.x, bb.Center.y + bb.Extents.y, bb.Center.z + bb.Extents.z });
			}

			DirectX::BoundingBox boundingBox = boundingBoxGen.GetBoundingBox();


			DirectX::XMVECTOR extentsV = DirectX::XMLoadFloat3(&boundingBox.Extents);
			DirectX::XMVECTOR lengthV = DirectX::XMVector3Length(extentsV);

			float len;
			DirectX::XMStoreFloat(&len, lengthV);

			DirectX::BoundingSphere bs{ boundingBox.Center, len };

			cameraLookAt = Netcode::Float3{ bs.Center.x, bs.Center.y, bs.Center.z };

			float worldDepthDistance = 3.0f * 1.25f * len;
			cameraWorldDistance = 2.0f * len;
			cameraNearZ = 0.1f;
			cameraFarZ = cameraNearZ + worldDepthDistance;

			UpdatePerFrameData();
		}

		virtual BoneData * GetBoneData() {
			return &boneData;
		}

		virtual void InvalidateFrame() {
			frameValid = false;
		}

		virtual void SetDrawGeometry(std::vector<Intermediate::LOD *> lodRefs);

		virtual void OnResized(int32_t x, int32_t y) override {
			cameraAspect = graphics->GetAspectRatio();
			editorFrameGraph.OnResized(x, y);
			InvalidateFrame();
		}

		void ReloadShadersDetail();

		virtual void ReloadShaders();

		/*
		Initialize modules
		*/
		virtual void Setup(IModuleFactory * factory) override;

		void UpdatePerFrameData() {
			Netcode::Matrix proj = Netcode::PerspectiveFovMatrix(cameraFov, cameraAspect, cameraNearZ, cameraFarZ);
			Netcode::Vector3 ahead = cameraAhead;

			cameraPosition = ahead * cameraWorldDistance + cameraLookAt;
			//lightData.lights[0].position = Netcode::Float4{ cameraPosition.x, cameraPosition.y, cameraPosition.z, 0.0f };

			Netcode::Vector3 lp = lightData.lights[0].position;
			lp *= 3.0f;
			graphics->debug->DrawPoint(lp, 0.2f);
			
			Netcode::Matrix view = Netcode::LookAtMatrix(cameraPosition, cameraLookAt, cameraUp);
			Netcode::Matrix viewFromOrigo = Netcode::LookToMatrix(Netcode::Float3::Zero, -ahead, cameraUp);

			perFrameData.Proj = proj.Transpose();
			perFrameData.View = view.Transpose();
			perFrameData.ViewProj = (view * proj).Transpose();
			perFrameData.RayDir = (viewFromOrigo * proj).Invert().Transpose();
			perFrameData.nearZ = cameraNearZ;
			perFrameData.farZ = cameraFarZ;
			perFrameData.fov = cameraFov;
			perFrameData.aspectRatio = cameraAspect;
			perFrameData.eyePos = Netcode::Float4{ cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f };
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

			Netcode::Quaternion cameraYawQuat{ cameraPitch, cameraYaw, 0.0f };
			Netcode::Vector3 minusUnitZ = Netcode::Float3{ 0.0f, 0.0f, -1.0f };

			cameraAhead = minusUnitZ.Rotate(cameraYawQuat).Normalize();

			UpdatePerFrameData();

			InvalidateFrame();
		}

		/*
		Advance simulation, update modules
		*/
		virtual void Run() override {
			events->Dispatch();

			if(!frameValid) {
				graphics->frame->Prepare();


				editorFrameGraph.boneData = &boneData;
				editorFrameGraph.perFrameData = &perFrameData;
				editorFrameGraph.perObjectData = &perObjectData;
				editorFrameGraph.boneVisibilityData = &boneVisibilityData;
				editorFrameGraph.lightData = &lightData;
				editorFrameGraph.gbufferPass_Input = gbuffers;
				editorFrameGraph.colliderPass_Input = colliders;
				editorFrameGraph.gbufferPass_InputMaterials = matAssoc;
				editorFrameGraph.cameraWorldDistance = cameraWorldDistance;

				Ref<FrameGraphBuilder> builder = graphics->CreateFrameGraphBuilder();
				editorFrameGraph.CreateFrameGraph(builder.get());
				Ref<FrameGraph> graph = builder->Build();

				graphics->frame->Run(std::move(graph), Netcode::Graphics::FrameGraphCullMode::NONE);
				graphics->frame->Present();
				graphics->frame->DeviceSync();
				graphics->frame->CompleteFrame();
				frameValid = true;
			}
		}

		/*
		Properly shutdown the application
		*/
		virtual void Exit() override;
	};

}
