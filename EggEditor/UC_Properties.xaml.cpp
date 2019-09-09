//
// UC_Properties.xaml.cpp
// Implementation of the UC_Properties class
//

#include "pch.h"
#include "UC_Properties.xaml.h"
#include "UC_TransformComponent.xaml.h"
#include "UC_MeshComponent.xaml.h"
#include "UC_ScriptComponent.xaml.h"

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

UC_Properties::UC_Properties()
{
	InitializeComponent();
	DataContext = ref new GameObjectDataContext();
}


void EggEditor::UC_Properties::CtxAddTransformComponentBtn_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	TransformComponent * tc = goRef->AddComponent<TransformComponent>();
	transformComponentControl->SetComponent(reinterpret_cast<UINT_PTR>(tc));
}

void EggEditor::UC_Properties::CtxAddScriptComponentBtn_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	//goRef->AddComponent<ScriptComponent>();
}

void EggEditor::UC_Properties::CtxAddMeshComponentBtn_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	//goRef->AddComponent<MeshComponent>();
}
