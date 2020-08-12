#pragma once

#include <Netcode/Modules.h>
#include "EditorFrameGraph.h"
#include <algorithm>
#include "Model.h"
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
		std::vector<Ref<BRDF_MaterialBase>> materials;
		std::vector<Ref<BRDF_MaterialBase>> matAssoc;
		std::vector<GBuffer> boneGbuffers;
		std::vector<Collider> colliders;

		virtual void SetMaterials(const std::vector<Mesh> & meshes, const std::vector<Material> & mats) {
			materials.clear();
			materials.resize(mats.size());
			matAssoc.reserve(meshes.size());

			for(const auto & mesh : meshes) {
				uint32_t materialIdx = mesh.materialIdx;

				if(materials[materialIdx] == nullptr) {
					Ref<BRDF_DefaultMaterial> dm = std::make_shared<BRDF_DefaultMaterial>();
					dm->Initialize(graphics.get());
					materials[materialIdx] = dm;
					ApplyMaterial(materialIdx, mats[materialIdx]);
				}

				matAssoc.emplace_back(materials[materialIdx]);
			}
		}

		void ApplyTexture(uint32_t materialIndex, BRDF_TextureType texType, const std::wstring & ref) {
			if(ref.empty()) {
				materials[materialIndex]->RemoveTexture(texType);
				return;
			}

			Netcode::URI::Texture texUri{ ref };

			if(!Netcode::IO::File::Exists(texUri.GetTexturePath())) {
				return;
			}

			if(materials[materialIndex]->GetID(texType).GetFullPath() == texUri.GetFullPath()) {
				return;
			}

			auto textureBuilder = graphics->CreateTextureBuilder();
			textureBuilder->LoadTexture2D(texUri);
			Ref<Netcode::Texture> texture = textureBuilder->Build();

			Ref<Netcode::GpuResource> texResource = graphics->resources->CreateTexture2D(texture->GetImage(0, 0, 0));

			auto uploadBatch = graphics->resources->CreateUploadBatch();
			uploadBatch->Upload(texResource, texture);
			uploadBatch->Barrier(texResource, Netcode::Graphics::ResourceState::COPY_DEST, Netcode::Graphics::ResourceState::ANY_READ);
			graphics->frame->SyncUpload(uploadBatch);

			materials[materialIndex]->SetTexture(texType, std::move(texUri), std::move(texResource));
		}

		virtual void ApplyMaterialData(uint32_t materialIndex, const Material & mat) {
			if(materials.size() <= materialIndex) {
				return;
			}

			Ptr<BRDF_MaterialBase> brdfMat = materials[materialIndex].get();
			brdfMat->Data.diffuseColor = mat.diffuseColor;
			brdfMat->Data.roughness = 1.0f - (mat.shininess / 256.0f);
			brdfMat->Data.fresnelR0 = mat.fresnelR0;
		}

		virtual void ApplyMaterial(uint32_t materialIndex, const Material & mat) {
			if(materials.size() <= materialIndex) {
				return;
			}

			ApplyMaterialData(materialIndex, mat);

			ApplyTexture(materialIndex, BRDF_TextureType::DIFFUSE_TEXTURE, mat.diffuseMapReference);
			ApplyTexture(materialIndex, BRDF_TextureType::NORMAL_TEXTURE, mat.normalMapReference);
			ApplyTexture(materialIndex, BRDF_TextureType::AMBIENT_TEXTURE, mat.ambientMapReference);
			ApplyTexture(materialIndex, BRDF_TextureType::ROUGHNESS_TEXTURE, mat.roughnessMapReference);
			ApplyTexture(materialIndex, BRDF_TextureType::SPECULAR_TEXTURE, mat.specularMapReference);
			ApplyTexture(materialIndex, BRDF_TextureType::HEIGHT_TEXTURE, mat.heightMapReference);
		}

		virtual void SetColliders(std::vector<Collider> colls) {
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


			Netcode::Matrix modelMat = Netcode::TranslationMatrix(Netcode::Float3{ -bs.Center.x, -bs.Center.y, -bs.Center.z });

			perObjectData.Model = modelMat.Transpose();
			perObjectData.InvModel = modelMat.Invert().Transpose();

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

		virtual void SetDrawGeometry(std::vector<LOD*> lodRefs) {
			gbuffers.clear();

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

		virtual void OnResized(int32_t x, int32_t y) override {
			cameraAspect = graphics->GetAspectRatio();
			editorFrameGraph.OnResized(x, y);
			InvalidateFrame();
		}

		/*
		Initialize modules
		*/
		virtual void Setup(IModuleFactory * factory) override {
			Netcode::IO::Path::SetShaderRoot(L"C:/work/directx12/Bin/v142-msvc/AppX/Shaders");
			Netcode::IO::Path::SetMediaRoot(L"C:/work/directx12/Media");
			Netcode::IO::Path::SetWorkingDirectiory(L"C:/work/directx12/Bin/v142-msvc/AppX");

			Netcode::IO::File configFile{ L"config.json" };

			if(!Netcode::IO::File::Exists(configFile.GetFullPath())) {
				Log::Error("File does not exist");
				return;
			}

			rapidjson::Document doc;
			Netcode::IO::ParseJson(doc, configFile.GetFullPath());

			Netcode::Config::LoadJson(doc);

			events = std::make_unique<Netcode::Module::AppEventSystem>();

			graphics = factory->CreateGraphicsModule(this, 0);

			StartModule(graphics.get());
			AddAppEventHandlers(events.get());
			events->AddHandler(this);

			for(uint32_t i = 0; i < 128; ++i) {
				boneData.BindTransform[i] = Netcode::Float4x4::Identity;
				boneData.ToRootTransform[i] = Netcode::Float4x4::Identity;
			}

			memset(&lightData, 0, sizeof(lightData));
			lightData.lights[0] = Netcode::DirectionalLight{ Netcode::Float3::One, Netcode::Float4{ 0.0f, 0.0f, -1.0f, 0.0f} };
			lightData.ambientLightIntensity = Float4{ 0.2f, 0.2f, 0.2f, 1.0f };
			lightData.numLights = 1;

			cameraNearZ = 1.0f;
			cameraFarZ = 1000.0f;
			cameraFov = DirectX::XM_PIDIV4;
			cameraAspect = graphics->GetAspectRatio();

			cameraWorldDistance = 1.0f;
			cameraPosition = Netcode::Float3{ 0.0f, 0.0f, 180.0f };
			cameraAhead = Netcode::Float3{ 0.0f, 0.0f, -1.0f };
			cameraUp = Netcode::Float3{ 0.0f, 1.0f, 0.0f };

			mouseSpeed = 0.0005f;

			memset(boneVisibilityData.BoneVisibility, 0, sizeof(BoneVisibilityData));

			UpdatePerFrameData();

			editorFrameGraph.CreatePermanentResources(graphics.get());
		}

		void UpdatePerFrameData() {
			Netcode::Matrix proj = Netcode::PerspectiveFovMatrix(cameraFov, cameraAspect, cameraNearZ, cameraFarZ);
			Netcode::Vector3 ahead = cameraAhead;

			cameraPosition = ahead * cameraWorldDistance;
			lightData.lights[0].position = Netcode::Float4{ cameraPosition.x, cameraPosition.y, cameraPosition.z, 0.0f };
			
			Netcode::Matrix view = Netcode::LookAtMatrix(cameraPosition, Netcode::Float3::Zero, cameraUp);
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

			Run();
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
				editorFrameGraph.gbufferPass_MaterialsInput = matAssoc;
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
		virtual void Exit() override {
			ShutdownModule(graphics.get());
		}
	};

}
