//
// UC_Scene.xaml.cpp
// Implementation of the UC_Scene class
//

#include "pch.h"
#include "UC_Scene.xaml.h"

using namespace EggEditor;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

UC_Scene::UC_Scene()
{
	InitializeComponent();
	TypedDataContext = (SceneDataContext ^)DataContext;
	sceneObserverAdapter.GetCx()->SceneGameObjectAdded += ref new GameObjectChangedCallback(this, &UC_Scene::OnGameObjectAdded);
}

void UC_Scene::OnGameObjectAdded(UINT_PTR gameObj) {
	UC_GameObject ^ newGoRef = ref new UC_GameObject(gameObj);
	newGoRef->Tapped += ref new Windows::UI::Xaml::Input::TappedEventHandler(this, &EggEditor::UC_Scene::OnTapped);
	TypedDataContext->GameObjects->Append(newGoRef);
}

void EggEditor::UC_Scene::CtxBtnNewGameObject_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e) {
	if(sceneRef) {
		Egg::GameObject * go = sceneRef->Next();
		go->SetName("Unnamed Object");
	}
}

void EggEditor::UC_Scene::OnTapped(Platform::Object ^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs ^ e)
{
	GameObjectSelected((UC_GameObject ^)sender);
}
