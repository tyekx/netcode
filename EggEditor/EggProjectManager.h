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

	internal:
		 concurrency::task<bool> OpenProject(Windows::UI::Core::CoreDispatcher^ dispatcher);
		 concurrency::task<bool> SaveProjectAs();
		 concurrency::task<bool> SaveProject();
		 concurrency::task<bool> CloseProject();
		 concurrency::task<bool> NewProject();
	};

}
