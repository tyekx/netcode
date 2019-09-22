#pragma once

#include "EggProject.h"

namespace EggEditor {


	delegate void ProjectEventCallback(EggProject ^ proj);

	ref class EggProjectManager sealed {
		
		EggProject ^ projectFile;

	public:

		event ProjectEventCallback ^ OnClosed;
		event ProjectEventCallback ^ OnOpened;

		property EggProject ^ Project {
			EggProject ^ get() {
				if(projectFile == nullptr) {
					NewProject();
				}

				return projectFile;
			}
		}

		property bool HasProject {
			bool get() {
				return projectFile != nullptr;
			}
		}



		 Windows::Foundation::IAsyncOperation<bool> ^ OpenProject();
		 Windows::Foundation::IAsyncOperation<bool> ^ SaveProjectAs();
		 Windows::Foundation::IAsyncOperation<bool> ^ SaveProject();
		 Windows::Foundation::IAsyncOperation<bool> ^ CloseProject();
		 Windows::Foundation::IAsyncOperation<bool> ^ NewProject();

	};

}
