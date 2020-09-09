#include "pch.h"
#include "EditorApp.h"
#include <Windows.h>
#include <fileapifromapp.h>

namespace Netcode::Module {

	struct FileWatch {
		std::wstring name;
		FILETIME lastWriteTime;

		bool operator!=(const FileWatch & rhs) const {
			return name != rhs.name ||
				lastWriteTime.dwLowDateTime != rhs.lastWriteTime.dwLowDateTime ||
				lastWriteTime.dwHighDateTime != rhs.lastWriteTime.dwHighDateTime;
		}
	};

	static std::vector<FileWatch> fileWatches;

	static std::vector<FileWatch> CreateWatchesForShaderObjects() {
		std::wstring queryString{ Netcode::IO::Path::ShaderRoot() };
		queryString += L"*.cso";

		WIN32_FIND_DATAW data;
		HANDLE h = FindFirstFileExFromAppW(queryString.c_str(), FindExInfoStandard, &data, FindExSearchNameMatch, NULL, 0);

		std::vector<FileWatch> v;
		v.reserve(32);

		if(h != INVALID_HANDLE_VALUE) {
			do {
				FileWatch fileWatch;
				fileWatch.lastWriteTime = data.ftLastWriteTime;
				fileWatch.name = data.cFileName;
				v.emplace_back(std::move(fileWatch));
			} while(FindNextFileW(h, &data));
		}

		return v;
	}

	void EditorApp::SetDrawGeometry(std::vector<Intermediate::LOD *> lodRefs) {
		gbuffers.clear();

		int id = 0;

		for(Intermediate::LOD * lod : lodRefs) {
			GBuffer gbuffer;
			gbuffer.indexBuffer = graphics->resources->CreateIndexBuffer(lod->indexDataSizeInBytes, DXGI_FORMAT_R32_UINT, ResourceType::PERMANENT_UPLOAD, ResourceState::ANY_READ);
			gbuffer.vertexBuffer = graphics->resources->CreateVertexBuffer(lod->vertexDataSizeInBytes, lod->vertexDataSizeInBytes / lod->vertexCount, ResourceType::PERMANENT_UPLOAD, ResourceState::ANY_READ);

			std::wstringstream wss;
			wss << "VB#" << id << ":" << (void *)(lod->vertexData.get()) << ":" << lod->vertexDataSizeInBytes;
			graphics->resources->SetDebugName(gbuffer.vertexBuffer, wss.str().c_str());

			std::wstringstream wss2;
			wss2 << "IB#" << id << ":" << (void *)(lod->indexData.get()) << ":" << lod->indexDataSizeInBytes;
			graphics->resources->SetDebugName(gbuffer.indexBuffer, wss2.str().c_str());

			id++;

			gbuffer.vertexCount = lod->vertexCount;
			gbuffer.indexCount = lod->indexCount;
			gbuffers.push_back(gbuffer);
			graphics->resources->CopyConstants(gbuffer.indexBuffer, lod->indexData.get(), lod->indexDataSizeInBytes);
			graphics->resources->CopyConstants(gbuffer.vertexBuffer, lod->vertexData.get(), lod->vertexDataSizeInBytes);
		}
	}

	void EditorApp::ReloadShadersDetail()
	{
		editorFrameGraph.Create_GBufferPass_PipelineState();
		OutputDebugStringW(L"GBuffer reloaded \n");
		InvalidateFrame();
	}

	void EditorApp::ReloadShaders() {
		auto watches = CreateWatchesForShaderObjects();

		if(fileWatches.size() != watches.size()) {
			ReloadShadersDetail();
		}

		for(const auto & [a, b] : ZipConst(fileWatches, watches)) {
			if(a != b) {
				ReloadShadersDetail();
				break;
			}
		}

		std::swap(fileWatches, watches);
	}

	void EditorApp::SetMaterials(const std::vector<Intermediate::Mesh> & meshes, const std::vector<Ref<Material>> & mats, bool forceUpdate) {
		matAssoc.reserve(meshes.size());
		materials = mats;

		using P = MaterialParamId;

		for(const auto & mesh : meshes) {
			uint32_t materialIdx = mesh.materialIdx;

			Ptr<Material> mat = materials[materialIdx].get();

			Ref<ResourceViews> view = mat->GetResourceView(0);
			if(view == nullptr) {
				view = graphics->resources->CreateShaderResourceViews(6);
				for(uint32_t i = 0; i < 6; i++) {
					view->ClearSRV(i, Graphics::ResourceDimension::TEXTURE2D);
				}
				mat->SetResourceView(0, view);
			}

			ApplyTexture(materialIdx, P::TEXTURE_DIFFUSE, P::TEXTURE_DIFFUSE_PATH, materials[materialIdx]->GetRequiredParameter<URI::Texture>(P::TEXTURE_DIFFUSE_PATH), forceUpdate);
			ApplyTexture(materialIdx, P::TEXTURE_NORMAL, P::TEXTURE_NORMAL_PATH, materials[materialIdx]->GetRequiredParameter<URI::Texture>(P::TEXTURE_NORMAL_PATH), forceUpdate);
			ApplyTexture(materialIdx, P::TEXTURE_AMBIENT, P::TEXTURE_AMBIENT_PATH, materials[materialIdx]->GetRequiredParameter<URI::Texture>(P::TEXTURE_AMBIENT_PATH), forceUpdate);
			ApplyTexture(materialIdx, P::TEXTURE_ROUGHNESS, P::TEXTURE_ROUGHNESS_PATH, materials[materialIdx]->GetRequiredParameter<URI::Texture>(P::TEXTURE_ROUGHNESS_PATH), forceUpdate);
			ApplyTexture(materialIdx, P::TEXTURE_SPECULAR, P::TEXTURE_SPECULAR_PATH, materials[materialIdx]->GetRequiredParameter<URI::Texture>(P::TEXTURE_SPECULAR_PATH), forceUpdate);
			ApplyTexture(materialIdx, P::TEXTURE_DISPLACEMENT, P::TEXTURE_DISPLACEMENT_PATH, materials[materialIdx]->GetRequiredParameter<URI::Texture>(P::TEXTURE_DISPLACEMENT_PATH), forceUpdate);
			matAssoc.emplace_back(materials[materialIdx]);
		}
	}

	/*
	Initialize modules
	*/

	void EditorApp::Setup(IModuleFactory * factory) {
		Log::Setup(true);

		Netcode::IO::Path::SetShaderRoot(L"C:/work/directx12/Bin/v142-msvc/Shaders");
		Netcode::IO::Path::SetMediaRoot(L"C:/work/directx12/Media");
		Netcode::IO::Path::SetWorkingDirectiory(L"C:/work/directx12/Bin/v142-msvc/AppX");

		Netcode::IO::File configFile{ L"config.json" };

		fileWatches = CreateWatchesForShaderObjects();

		if(!Netcode::IO::File::Exists(configFile.GetFullPath())) {
			Log::Error("File does not exist");
			return;
		}

		Netcode::JsonDocument doc;
		Netcode::IO::ParseJsonFromFile(doc, configFile.GetFullPath());

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
		lightData.lights[0] = Netcode::DirectionalLight{ Netcode::Float3::One, Netcode::Float3{ 0.0f, 0.0f, 1.0f } };
		lightData.lights[1] = Netcode::OmniLight{ Netcode::Float3{ 0.8f, 0.6f, 0.2f}, Netcode::Float3{ 0.0f, 4.0f, 0.0f }, 2.0f, 10.0f };
		lightData.lights[2] = Netcode::SpotLight{ Netcode::Float3{ 0.1f, 0.6f, 0.8f}, Netcode::Float3{ 4.0f, 0.0f, 0.0f }, Netcode::Float3{ -1.0f, 0.0f, 0.0f }, 2.0f, 10.0f, 0.9f, 0.7f };
		lightData.ambientLightIntensity = Float4{ 0.2f,0.2f, 0.2f, 1.0f };
		lightData.numLights = 1;

		cameraNearZ = 1.0f;
		cameraFarZ = 1000.0f;
		cameraFov = DirectX::XM_PIDIV4;
		cameraAspect = graphics->GetAspectRatio();

		cameraWorldDistance = 1.0f;
		cameraLookAt = Netcode::Float3::Zero;
		cameraPosition = Netcode::Float3{ 0.0f, 0.0f, 180.0f };
		cameraAhead = Netcode::Float3{ 0.0f, 0.0f, -1.0f };
		cameraUp = Netcode::Float3{ 0.0f, 1.0f, 0.0f };

		mouseSpeed = 0.0005f;

		perObjectData.Model = Netcode::Float4x4::Identity;
		perObjectData.InvModel = Netcode::Float4x4::Identity;

		memset(boneVisibilityData.BoneVisibility, 0, sizeof(BoneVisibilityData));

		UpdatePerFrameData();

		editorFrameGraph.CreatePermanentResources(graphics.get());

		Ref<Netcode::FrameGraphBuilder> frameGraphBuilder = graphics->CreateFrameGraphBuilder();

		Ref<Netcode::TextureBuilder> textureBuilder = graphics->CreateTextureBuilder();
		textureBuilder->LoadTextureCube(L"compiled/textures/envmaps/cloudynoon.dds");
		Ref<Netcode::GpuResource> cloudynoonTexture = textureBuilder->Build();
		Ref<Netcode::GpuResource> preIntegratedBrdf = editorFrameGraph.PreIntegrateBrdf(frameGraphBuilder.get());
		Ref<Netcode::GpuResource> preFilteredEnvmap = editorFrameGraph.PrefilterEnvMap(frameGraphBuilder.get(), cloudynoonTexture);

		graphics->frame->Run(frameGraphBuilder->Build(), Netcode::Graphics::FrameGraphCullMode::NONE);
		graphics->frame->DeviceSync();

		editorFrameGraph.SetGlobalEnvMap(preFilteredEnvmap, preIntegratedBrdf);
	}

	/*
	Properly shutdown the application
	*/

	void EditorApp::Exit() {
		ShutdownModule(graphics.get());
	}

}

