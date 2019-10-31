#pragma once

#include <map>
#include <string>

namespace Egg::Graphics::Internal {

	struct PreprocessorDefinitions {
		std::map<std::string, std::string> defs;

		bool operator==(const PreprocessorDefinitions & o) const {
			if(defs.size() != o.defs.size()) {
				return false;
			}

			for(const auto & i : defs) {
				const auto & oi = o.defs.find(i.first);

				if(oi == o.defs.end()) {
					return false;
				}

				if(oi->second != i.second) {
					return false;
				}
			}
			return true;
		}

		inline bool operator!=(const PreprocessorDefinitions & o) const {
			return !operator==(o);
		}

		void Define(const std::string & key, float value) {
			Define(key, std::to_string(value));
		}

		void Define(const std::string & key, int value) {
			Define(key, std::to_string(value));
		}

		void Define(const std::string & key, const std::string & value) {
			defs[key] = value;
		}

		void Define(const std::string & key) {
			defs[key];
		}
	};

}
