#pragma once

#include <json11/json11.hpp>
#include "Utility.h"
#include "GameObject.h"

namespace Egg {

	struct ISceneObserver;

	class Scene {
		GameObject * Objects;
		unsigned int ObjectCount;
		unsigned int MaxCount;
		std::string Name;


	public:
		Scene(const Scene &) = delete;
		void operator=(const Scene &) = delete;

		Scene(unsigned int maxSize = 512) : Objects{ nullptr }, ObjectCount{ 0 }, MaxCount{ maxSize } {
			Objects = reinterpret_cast<GameObject *>(std::malloc(MaxCount * sizeof(GameObject)));
		}

		~Scene() {
			std::free(Objects);
		}

		const std::string & GetName() const {
			return Name;
		}

		void SetName(const std::string & name) {
			Name = name;
		}

		void Reset();

		GameObject * New();

		unsigned int GetObjectCount() const {
			return ObjectCount;
		}

		GameObject * operator[](unsigned int index) const {
			return Objects + index;
		}

		GameObject * operator[](unsigned int index) {
			return Objects + index;
		}

	};
}
