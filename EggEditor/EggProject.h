#pragma once

#include <vector>
#include <map>

namespace EggEditor {

	class EggProjectFolder {
	private:
		int Id;
		int OwnerId;
		std::vector<int> Children;
		std::string Name;
	public:
		EggProjectFolder() : Id{ -1 }, OwnerId{ 0 }, Children{}, Name{} { }

		void SetId(int id) {
			if(id == -1) {
				Id = id;
			}
		}

		int GetOwnerId() const {
			return OwnerId;
		}

		int GetId() const {
			return Id;
		}

		void SetName(const std::string & name) {
			Name = name;
		}

		const std::string & GetName() const {
			return Name;
		}
	};

	struct EggAsset {
		std::string Name;
		std::string Type;
		std::string File;
	};

	class EggProject {
	private:
		std::string Name;
		std::vector<EggProjectFolder> ProjectFolders;
		std::vector<EggAsset> Assets;


	public:
		const std::vector<EggProjectFolder> & GetProjectFolders() const {
			return ProjectFolders;
		}

		const std::string & GetName() const {
			return Name;
		}

		void SetName(const std::string & name) {
			Name = name;
		}

	};

}
