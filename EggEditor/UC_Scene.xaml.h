//
// UC_Scene.xaml.h
// Declaration of the UC_Scene class
//

#pragma once

#include "UC_Scene.g.h"
#include "Auxiliary.h"
#include <Egg/Scene.h>
#include "UC_GameObject.xaml.h"

namespace EggEditor
{

	public delegate void GameObjectSelectedCallback(UC_GameObject ^ goRef);

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class UC_Scene sealed
	{
		Egg::Scene * sceneRef;
		CxCppSceneObserverAdapter sceneObserverAdapter;

		SceneDataContext ^ TypedDataContext;
	public:
		UC_Scene();

		event GameObjectSelectedCallback ^ GameObjectSelected;

		void SetScene(UINT_PTR scene) {
			sceneRef = reinterpret_cast<Egg::Scene *>(scene);
			sceneRef->SetObserver(&sceneObserverAdapter);
			SceneDataContext^ dc = (SceneDataContext ^)DataContext;
			sceneRef->SetObserver(reinterpret_cast<Egg::ISceneObserver *>(&sceneObserverAdapter));
		}

		void OnGameObjectAdded(UINT_PTR gameObj);

		void CtxBtnNewGameObject_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void OnTapped(Platform::Object ^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs ^ e);
	};
}
