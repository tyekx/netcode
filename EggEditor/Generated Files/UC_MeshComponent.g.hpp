﻿//------------------------------------------------------------------------------
//     This code was generated by a tool.
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
//------------------------------------------------------------------------------
#include "pch.h"

#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BINDING_DEBUG_OUTPUT
extern "C" __declspec(dllimport) int __stdcall IsDebuggerPresent();
#endif

#include "UC_MeshComponent.xaml.h"

void ::EggEditor::UC_MeshComponent::InitializeComponent()
{
    if (_contentLoaded)
    {
        return;
    }
    _contentLoaded = true;
    ::Windows::Foundation::Uri^ resourceLocator = ref new ::Windows::Foundation::Uri(L"ms-appx:///UC_MeshComponent.xaml");
    ::Windows::UI::Xaml::Application::LoadComponent(this, resourceLocator, ::Windows::UI::Xaml::Controls::Primitives::ComponentResourceLocation::Application);
}

void ::EggEditor::UC_MeshComponent::Connect(int __connectionId, ::Platform::Object^ __target)
{
    switch (__connectionId)
    {
    case 2:
        {
            this->meshRef = safe_cast<::Windows::UI::Xaml::Controls::TextBox^>(__target);
            (safe_cast<::Windows::UI::Xaml::Controls::TextBox^>(this->meshRef))->DragOver += ref new ::Windows::UI::Xaml::DragEventHandler(this, (void (::EggEditor::UC_MeshComponent::*)
                (::Platform::Object^, ::Windows::UI::Xaml::DragEventArgs^))&UC_MeshComponent::ScriptRefTextBox_DragOver);
            (safe_cast<::Windows::UI::Xaml::Controls::TextBox^>(this->meshRef))->Drop += ref new ::Windows::UI::Xaml::DragEventHandler(this, (void (::EggEditor::UC_MeshComponent::*)
                (::Platform::Object^, ::Windows::UI::Xaml::DragEventArgs^))&UC_MeshComponent::ScriptRefTextBox_Drop);
        }
        break;
    }
    _contentLoaded = true;
}

::Windows::UI::Xaml::Markup::IComponentConnector^ ::EggEditor::UC_MeshComponent::GetBindingConnector(int __connectionId, ::Platform::Object^ __target)
{
    __connectionId;         // unreferenced
    __target;               // unreferenced
    return nullptr;
}

#pragma warning(pop)


