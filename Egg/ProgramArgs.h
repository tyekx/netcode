#pragma once

#include <string>
#include <map>

namespace Egg {

	class ProgramArgs {
		std::map<std::wstring, std::wstring> Args;
		std::wstring EmptyString;
	public:
		ProgramArgs(const wchar_t ** argv, int argc) {
			for(int i = 0; i < argc; ++i) {
				std::wstring unprocessed = argv[i];

				if(unprocessed.size() < 3 || unprocessed[0] != L'-' || unprocessed[1] != L'-') {
					continue;
				}

				unprocessed = unprocessed.substr(2);

				size_t indexOf = unprocessed.find(L'=');

				std::wstring key;
				std::wstring value;

				if(indexOf != std::wstring::npos) {
					key = unprocessed.substr(0, indexOf);
					value = unprocessed.substr(indexOf + 1);
				} else {
					key = unprocessed;
				}

				Args[key] = value;
			}
		}

		bool IsSet(const std::wstring & key) {
			return Args.find(key) != Args.end();
		}

		const std::wstring & GetArg(const std::wstring & key) {
			if(IsSet(key)) {
				return Args[key];
			}
			return EmptyString;
		}
	};

}
