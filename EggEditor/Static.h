#pragma once

#include "EggProjectManager.h"

namespace EggEditor {

	ref class Static sealed {
		static EggProjectManager ^ projectManager;
	internal:
		static property EggProjectManager ^ ProjectManager {
			EggProjectManager ^ get() {
				if(projectManager == nullptr) {
					projectManager = ref new EggProjectManager();
				}
				return projectManager;
			}
		}
	};
}
