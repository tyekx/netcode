#pragma once

#include <json11/json11.hpp>
#include "EggProject.h"

namespace EggEditor {

	class JsonSerializer {
	public:
		static json11::Json Serialize(const EggEditor::EggProjectFolder & pf) {
			return json11::Json::object {
				{ "name", pf.GetName() },
				{ "id", pf.GetId() },
				{ "ownerId", pf.GetOwnerId() }
			};
		}
		static json11::Json Serialize(const EggEditor::EggProject & proj) {
			json11::Json::array arr;
			
			for(const EggEditor::EggProjectFolder & f : proj.GetProjectFolders()) {
				arr.push_back(Serialize(f));
			}

			return json11::Json::object{
				{ "folders", arr }
			};
		}
	};

}
