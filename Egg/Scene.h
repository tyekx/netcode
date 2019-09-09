#pragma once

#include <json11/json11.hpp>
#include "Utility.h"
#include "GameObject.h"
#include "SceneObserver.h"

namespace Egg {

	struct ISceneObserver;

	class Scene {
		GameObject * Objects;
		unsigned int ObjectCount;
		unsigned int MaxCount;
		std::string Name;

		EGG_DEBUG_ENGINE_DEFINE_OBSERVER(Egg::ISceneObserver)

	public:
		Scene(const Scene &) = delete;
		void operator=(const Scene &) = delete;

		Scene(unsigned int maxSize = 100) : Objects{ nullptr }, ObjectCount{ 0 }, MaxCount{ maxSize } EGG_DEBUG_ENGINE_CONSTRUCT_OBSERVER {
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

		GameObject * Next();

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
