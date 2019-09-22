#pragma once

#include "EggProject.h"

namespace EggEditor {

	public ref class Static sealed {
		static EggProject ^ project;
	public:
		static property EggProject ^ Project {
			EggProject ^ get() {
				if(project == nullptr) {
					project = ref new EggProject();
				}
				return project;
			}
		}
	};
}
