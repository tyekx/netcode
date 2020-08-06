#pragma once

#include <Netcode/Modules.h>
#include "EditorFrameGraph.h"
#include <algorithm>
#include "Model.h"
#include "BoundingBoxHelpers.h"
#include <Netcode/BasicGeometry.h>
#include <Netcode/IO/Path.h>
#include <Netcode/IO/Json.h>
#include <Netcode/IO/File.h>
#include <Netcode/Config.h>

namespace Netcode::Module {

	class EditorApp : public AApp {
	public:
		BoneData boneData;
		PerFrameData perFrameData;
		PerObjectData perObjectData;
		BoneVisibilityData boneVisibilityData;

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

		std::vector<GBuffer> gbuffers;
		std::vector<GBuffer> boneGbuffers;
		std::vector<GBuffer> colliderGbuffers;
		std::vector<ColliderData> colliderData;
		
		virtual void UpdateColliderData(const std::vector<Collider> & colls) {
			colliderData.clear();

			for(const Collider & c : colls) {
				ColliderData cd;
				cd.Color = Netcode::Float4{ 0.0f, 0.0f, 1.0f, 1.0f };
				cd.BoneReference = c.boneReference;

				Netcode::Matrix R = Netcode::RotationMatrix(c.localRotation);
				Netcode::Matrix T = Netcode::TranslationMatrix(c.localPosition);
				cd.LocalTransform = (R * T).Transpose();

				colliderData.push_back(cd);
			}
		}

		virtual void SetColliders(std::vector<Collider> colls) {
			colliderGbuffers.clear();

			UpdateColliderData(colls);

			auto upload = graphics->resources->CreateUploadBatch();
			std::vector<std::unique_ptr<Netcode::Float3[]>> data;

			for(const Collider & c : colls) {
				GBuffer g;

				switch(c.type) {
					case ColliderType::BOX:
					{
						std::unique_ptr<Netcode::Float3[]> boxVData = std::make_unique<Netcode::Float3[]>(24);
						Netcode::Graphics::BasicGeometry::CreateBoxWireframe(boxVData.get(), sizeof(DirectX::XMFLOAT3), c.boxArgs);
						g.vertexBuffer = graphics->resources->CreateVertexBuffer(24 * sizeof(DirectX::XMFLOAT3), sizeof(DirectX::XMFLOAT3), ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
						g.vertexCount = 24;
						upload->Upload(g.vertexBuffer, boxVData.get(), 24 * sizeof(DirectX::XMFLOAT3));
						upload->Barrier(g.vertexBuffer, ResourceState::COPY_DEST, ResourceState::VERTEX_AND_CONSTANT_BUFFER);
						data.emplace_back(std::move(boxVData));
					}
					break;
					case ColliderType::CAPSULE:
					{
						const uint32_t vCount = Netcode::Graphics::BasicGeometry::GetCapsuleWireframeSize(12);
						constexpr uint32_t stride = sizeof(Netcode::Float3);
						uint32_t sizeInBytes = vCount * stride;
						std::unique_ptr<Netcode::Float3[]> boxVData = std::make_unique<Netcode::Float3[]>(vCount);
						Netcode::Graphics::BasicGeometry::CreateCapsuleWireframe(boxVData.get(), stride, 12, c.capsuleArgs.x, c.capsuleArgs.y, 0);
						g.vertexBuffer = graphics->resources->CreateVertexBuffer(sizeInBytes, stride, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
						g.vertexCount = vCount;
						upload->Upload(g.vertexBuffer, boxVData.get(), sizeInBytes);
						upload->Barrier(g.vertexBuffer, ResourceState::COPY_DEST, ResourceState::VERTEX_AND_CONSTANT_BUFFER);
						data.emplace_back(std::move(boxVData));
					}
					break;
					case ColliderType::SPHERE:
					{
						const uint32_t numVertices = Netcode::Graphics::BasicGeometry::GetSphereWireframeSize(12);
						constexpr uint32_t stride = sizeof(Netcode::Float3);
						uint32_t sizeInBytes = numVertices * stride;
						std::unique_ptr<Netcode::Float3[]> boxVData = std::make_unique<Netcode::Float3[]>(numVertices);
						Netcode::Graphics::BasicGeometry::CreateSphereWireframe(boxVData.get(), stride, c.sphereArgs, 12, 0);
						g.vertexBuffer = graphics->resources->CreateVertexBuffer(sizeInBytes, stride, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
						g.vertexCount = numVertices;
						upload->Upload(g.vertexBuffer, boxVData.get(), sizeInBytes);
						upload->Barrier(g.vertexBuffer, ResourceState::COPY_DEST, ResourceState::VERTEX_AND_CONSTANT_BUFFER);
						data.emplace_back(std::move(boxVData));
					}
					break;
					default:
						continue;
				}
				colliderGbuffers.push_back(g);
			}

			graphics->frame->SyncUpload(upload);
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

		/*
		Initialize modules
		*/
		virtual void Setup(IModuleFactory * factory) override {
			Netcode::IO::Path::SetShaderRoot(L"C:/work/directx12/Bin/v142-msvc/AppX/Shaders");
			Netcode::IO::Path::SetMediaRoot(L"C:/work/directx12/Bin/v142-msvc/AppX/Media");
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

			for(uint32_t i = 0; i < 128; ++i) {
				boneData.BindTransform[i] = Netcode::Float4x4::Identity;
				boneData.ToRootTransform[i] = Netcode::Float4x4::Identity;
			}

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
			perFrameData.eyePos = Netcode::Float4{ cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f };

			Netcode::Matrix proj = Netcode::PerspectiveFovMatrix(cameraFov, cameraAspect, cameraNearZ, cameraFarZ);
			Netcode::Vector3 ahead = cameraAhead;
			
			Netcode::Matrix view = Netcode::LookAtMatrix(ahead * cameraWorldDistance, Netcode::Float3::Zero, cameraUp);
			Netcode::Matrix viewFromOrigo = Netcode::LookToMatrix(Netcode::Float3::Zero, -ahead, cameraUp);

			perFrameData.Proj = proj.Transpose();
			perFrameData.View = view.Transpose();
			perFrameData.ViewProj = (view * proj).Transpose();
			perFrameData.RayDir = (viewFromOrigo * proj).Invert().Transpose();
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

			Netcode::Quaternion cameraYawQuat{ 0.0f, cameraYaw, 0.0f };
			Netcode::Vector3 minusUnitZ = Netcode::Float3{ 0.0f, 0.0f, -1.0f };

			cameraAhead = minusUnitZ.Rotate(cameraYawQuat).Normalize();

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
			editorFrameGraph.colliderPass_Input = colliderGbuffers;
			editorFrameGraph.colliderPass_DataInput = colliderData;

			Ref<FrameGraphBuilder> builder = graphics->CreateFrameGraphBuilder();
			editorFrameGraph.CreateFrameGraph(builder.get());
			Ref<FrameGraph> graph = builder->Build();

			graphics->frame->Run(std::move(graph), Netcode::Graphics::FrameGraphCullMode::NONE);
			graphics->frame->Present();
			graphics->frame->DeviceSync();
			graphics->frame->CompleteFrame();
		}

		/*
		Properly shutdown the application
		*/
		virtual void Exit() override {
			ShutdownModule(graphics.get());
		}
	};

}
