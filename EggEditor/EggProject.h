#pragma once

#include "IDialogHandler.h"
#include "IAssetBrowserItem.h"
#include "JsonSerializer.h"

namespace EggEditor {


	public ref class EggProject sealed {
		Platform::String ^ projectName;
		Platform::String ^ projectFile;
		IAssetBrowserItem ^ rootFolder;

	public:

		EggProject() {
			projectName = L"";
			rootFolder = ref new ProjectFolderDataContext();
			rootFolder->Name = L"/";
		}

		property int IsEmpty {
			int get() {
				return rootFolder->Children->Size == 0;
			}
		}

		property IAssetBrowserItem ^ Root {
			IAssetBrowserItem ^ get() {
				return rootFolder;
			}
		}

		property Platform::String ^ Name {
			Platform::String ^ get() {
				return projectName;
			}
			void set(Platform::String ^ val) {
				projectName = val;
			}
		}

		property Platform::String ^ FilePath {
			Platform::String ^ get() {
				return projectFile;
			}
			void set(Platform::String ^ val) {
				projectFile = val;
			}
		}

		/*
		void NewProject(IDialogHandler ^ handler) {
			if(projectOpen) {
				CloseProject(nullptr);
			}
		}

		void OpenProject() {

		}

		void SaveProject() {

		}
		void CloseProject(IDialogHandler ^ handler) {
			EggProject ^ pThis = this;

			if(Root->IsContentDirty) {
				auto dialog = ref new Windows::UI::Xaml::Controls::ContentDialog();
				dialog->Title = L"Unsaved Changes";
				dialog->PrimaryButtonText = L"Save";
				dialog->SecondaryButtonText = L"Discard";
				dialog->CloseButtonText = L"Cancel";
				dialog->DefaultButton = Windows::UI::Xaml::Controls::ContentDialogButton::Primary;
				auto asyncPromise = dialog->ShowAsync();

				auto task = concurrency::create_task(asyncPromise);

				task.then([handler, pThis](Windows::UI::Xaml::Controls::ContentDialogResult result) -> void {
					switch(result) {
					case Windows::UI::Xaml::Controls::ContentDialogResult::Primary:
						if(handler != nullptr) handler->PrimaryOption();
						break;
					case Windows::UI::Xaml::Controls::ContentDialogResult::Secondary:
						if(handler != nullptr) handler->SecondaryOption();
						break;
					default:
						if(handler != nullptr) handler->Cancel();
						break;
					}
				});
			} else {
				if(handler != nullptr) handler->SecondaryOption();
			}
		}*/

		void Serialize(IJsonWriter ^ writer) {

		}

		void Deserialize(IJsonReader ^ reader) {

		}

	};

}
