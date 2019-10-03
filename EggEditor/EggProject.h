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

		void Serialize(IJsonWriter ^ writer) {

		}

		void Deserialize(IJsonReader ^ reader) {

		}

	};

}
