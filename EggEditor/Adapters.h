#pragma once

#include <Egg/SceneObserver.h>
#include <Egg/GameObjectObserver.h>

/*
* in C++/CX its not allowed to implement native interfaces, to work around we implement it in a native class, that contains a CX ref class which has events defined.
* This native adapter class can then be aggregated into the datacontext class, which signs up to the CX adapter's events.
* By making this work around its enough to call the native classes' functions and the UI will automatically get notifications of the data changes
*/

namespace EggEditor {

	public delegate void GameObjectChangedCallback(UINT_PTR obj);

	public ref class CxCppGameObjectObserverAdapterCx sealed {
	public:
		event GameObjectChangedCallback ^ GameObjectNameChanged;
		event GameObjectChangedCallback ^ GameObjectOwnerChanged;
		event GameObjectChangedCallback ^ GameObjectDestroyed;
		event GameObjectChangedCallback ^ GameObjectSignatureChanged;

		void InvokeGameObjectSignatureChanged(UINT_PTR obj);
		void InvokeGameObjectDestroyed(UINT_PTR obj);
		void InvokeGameObjectOwnerChanged(UINT_PTR obj);
		void InvokeGameObjectNameChanged(UINT_PTR obj);
	};

	class CxCppGameObjectObserverAdapter : public Egg::IGameObjectObserver {
		CxCppGameObjectObserverAdapterCx ^ CxAdapterInstance;

	public:
		CxCppGameObjectObserverAdapter() : CxAdapterInstance{ ref new CxCppGameObjectObserverAdapterCx() } { }

		virtual void GameObjectNameChanged(Egg::GameObject * obj) override;
		virtual void GameObjectDestroyed(Egg::GameObject * obj) override;
		virtual void GameObjectSignatureChanged(Egg::GameObject * obj) override;
		virtual void GameObjectOwnerChanged(Egg::GameObject * obj) override;

		CxCppGameObjectObserverAdapterCx ^ GetCx();
	};

	public ref class CxCppSceneObserverAdapterCx sealed {
	public:
		event GameObjectChangedCallback ^ SceneGameObjectAdded;

		void InvokeSceneGameObjectAdded(UINT_PTR obj);
	};

	class CxCppSceneObserverAdapter : public Egg::ISceneObserver {
		CxCppSceneObserverAdapterCx ^ CxAdapterInstance;

	public:
		CxCppSceneObserverAdapter() : CxAdapterInstance { ref new CxCppSceneObserverAdapterCx() } { }

		virtual void SceneGameObjectAdded(Egg::GameObject * obj) override;

		CxCppSceneObserverAdapterCx ^ GetCx();
	};

}

