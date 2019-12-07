#pragma once

#include <memory>
#include "HandleTypes.h"
#include <string>
#include <map>

namespace Egg::Module {

	class AApp;
	class IModule;
	class IModuleFactory;
	class IWindowModule;
	class IGraphicsModule;
	class INetworkModule;
	class IPhysicsModule;
	class IAudioModule;

	/*
	Abstract App, provides the user convenient way to use certain subsystems, currently supplied:
	window, graphics, network, physics, audio
	*/
	class AApp {
	protected:
		void StartModule(IModule * m);
		void ShutdownModule(IModule * m);

	public:
		std::unique_ptr<IWindowModule> window;
		std::unique_ptr<IGraphicsModule> graphics;
		std::unique_ptr<INetworkModule> network;
		std::unique_ptr<IPhysicsModule> physics;
		std::unique_ptr<IAudioModule> audio;

		virtual ~AApp() = default;

		/*
		Initialize modules
		*/
		virtual void Setup(IModuleFactory * factory) = 0;

		/*
		Advance simulation, update modules
		*/
		virtual void Run() = 0;

		/*
		Properly shutdown the application
		*/
		virtual void Exit() = 0;
	};

	class IModule {
	public:
		virtual ~IModule() = default;
		virtual void Start(AApp * app) = 0;
		virtual void Shutdown() = 0;
	};

	class IWindowModule : public IModule {
	public:
		virtual ~IWindowModule() = default;
		virtual void * GetUnderlyingPointer() = 0;
		virtual void ProcessMessages() = 0;
		virtual void CompleteFrame() = 0;
		virtual bool KeepRunning() = 0;
	};

	class IGraphicsModule : public IModule {
	public:
		virtual ~IGraphicsModule() = default;
		virtual HITEM CreateItem() = 0;

		virtual HTEXTURE LoadTexture(const std::wstring & textureMediaPath) = 0;
		virtual HSHADER LoadShader(const std::wstring & shaderPath) = 0;

		virtual HINCOMPLETESHADER CreateVertexShader() = 0;
		virtual HINCOMPLETESHADER CreatePixelShader() = 0;
		virtual HINCOMPLETESHADER CreateGeometryShader() = 0;
		virtual HINCOMPLETESHADER CreateDomainShader() = 0;
		virtual HINCOMPLETESHADER CreateHullShader() = 0;

		virtual void SetShaderEntry(HINCOMPLETESHADER shader, const std::string & entryFunction) = 0;
		virtual void SetShaderSource(HINCOMPLETESHADER shader, const std::wstring & shaderPath) = 0;
		virtual void SetShaderMacros(HINCOMPLETESHADER shader, const std::map<std::string, std::string> & defines) = 0;
		virtual HSHADER CompileShader(HINCOMPLETESHADER shader) = 0;

		virtual HPSO CreatePipelineState() = 0;
		virtual void SetVertexShader(HPSO pso, HSHADER vertexShader) = 0;
		virtual void SetPixelShader(HPSO pso, HSHADER pixelShader) = 0;
		virtual void SetGeometryShader(HPSO pso, HSHADER geometryShader) = 0;
		virtual void SetHullShader(HPSO pso, HSHADER hullShader) = 0;
		virtual void SetDomainShader(HPSO pso, HSHADER domainShader) = 0;

		virtual HGEOMETRY CreateGeometry(EGeometryType type = EGeometryType::INDEXED) = 0;
		virtual void AddVertexBufferLOD(HGEOMETRY geometry, void * ptr, unsigned int sizeInBytes, unsigned int strideInBytes) = 0;
		virtual void AddIndexBufferLOD(HGEOMETRY geometry, void * ptr, unsigned int sizeInBytes, unsigned int format) = 0;
		virtual void AddInputElement(HGEOMETRY geometry, const char * name, unsigned int semanticIndex, unsigned int format, unsigned int byteOffset) = 0;
		virtual void AddInputElement(HGEOMETRY geometry, const char * name, unsigned int format, unsigned int byteOffset) = 0;

		virtual HMATERIAL CreateMaterial(HPSO pso, HGEOMETRY geometry) = 0;
		virtual void SetMaterial(HITEM item, HMATERIAL material) = 0;
		virtual void SetGeometry(HITEM item, HGEOMETRY geometry) = 0;

		virtual void AllocateTextures(HITEM item, unsigned int numTextures) = 0;
		virtual void SetTexture(HITEM item, unsigned int slot, HTEXTURE texture) = 0;
		virtual HTEXTURE SetTexture(HITEM item, unsigned int slot, const std::wstring & texturePath) = 0;

		virtual HCBUFFER AllocateCbuffer(unsigned int sizeInBytes) = 0;
		virtual void * GetCbufferPointer(HCBUFFER cbuffer) = 0;

		virtual void AddCbuffer(HITEM item, HCBUFFER cbuffer, unsigned int slot) = 0;
		virtual void SetCbuffer(HITEM item, HCBUFFER cbuffer, unsigned int idx, unsigned int slot) = 0;
		virtual unsigned int GetCbufferSlot(HITEM item, const std::string & cbufferName) = 0;

		virtual void Prepare() = 0;
		virtual void SetRenderTarget() = 0;
		virtual void SetRenderTarget(HRENDERTARGET rt) = 0;
		virtual void ClearRenderTarget() = 0;
		virtual void Record(HITEM item) = 0;
		virtual void Render() = 0;
		virtual void Present() = 0;
	};

	class IAudioModule : public IModule {
	public:
		virtual ~IAudioModule() = default;

	};

	class INetworkModule : public IModule {
	public:
		virtual ~INetworkModule() = default;
	};

	class IPhysicsModule : public IModule {
	public:
		virtual ~IPhysicsModule() = default;
		virtual void * GetUnderlyingPointer() = 0;
		virtual void Simulate(float dt) = 0;
	};

	class IModuleFactory {
	public:
		virtual ~IModuleFactory() = default;
		virtual std::unique_ptr<IWindowModule> CreateWindowModule(AApp * app, int windowType) = 0;
		virtual std::unique_ptr<IGraphicsModule> CreateGraphicsModule(AApp * app, int graphicsType) = 0;
		virtual std::unique_ptr<INetworkModule> CreateNetworkModule(AApp * app, int networkType) = 0;
		virtual std::unique_ptr<IAudioModule> CreateAudioModule(AApp * app, int audioType) = 0;
		virtual std::unique_ptr<IPhysicsModule> CreatePhysicsModule(AApp * app, int physicsType) = 0;
	};

}
