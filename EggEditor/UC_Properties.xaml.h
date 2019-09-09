//
// UC_Properties.xaml.h
// Declaration of the UC_Properties class
//

#pragma once

#include "UC_Properties.g.h"
#include "Auxiliary.h"
#include "UC_GameObject.xaml.h"

namespace EggEditor
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class UC_Properties sealed
	{
		Egg::GameObject * goRef;
		UC_GameObject ^ currentGameObject;
	public:
		UC_Properties();

		void OnGameObjectSelected(UC_GameObject ^ gameObject) {
			currentGameObject = gameObject;
			goRef = reinterpret_cast<Egg::GameObject *>(gameObject->GetGameObject());
			DataContext = gameObject->DataContext;
		}
	private:
		void CtxAddTransformComponentBtn_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void CtxAddScriptComponentBtn_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void CtxAddMeshComponentBtn_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
	};
}
