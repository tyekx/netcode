#pragma once

#include "GameObject.h"
#include <list>
#include <memory>

/*
base class
*/
class Scene {

	struct SceneBlock {
		uint32_t freeSize;
		uint32_t maxSize;
		std::unique_ptr<GameObject[]> objects;
		GameObject * begin;
	public:
		SceneBlock() = delete;
		SceneBlock(const SceneBlock &) = delete;
		SceneBlock & operator=(const SceneBlock &) = delete;

		~SceneBlock() noexcept = default;
		SceneBlock(SceneBlock &&) noexcept = default;
		SceneBlock & operator=(SceneBlock && rhs) noexcept = default;

		SceneBlock(uint32_t capacity) : freeSize{ capacity }, maxSize{ capacity }, objects{ nullptr }, begin{ nullptr } {
			objects = std::make_unique<GameObject[]>(capacity);
			begin = objects.get();
		}

		GameObject * operator[](uint32_t idx) {
			//@TODO: debug assert
			return begin + idx;
		}

		bool CanHost() const {
			return freeSize > 0;
		}

		ptrdiff_t GetRelativePosition(GameObject * gameObject) const {
			return gameObject - begin;
		}

		bool WithinBounds(ptrdiff_t diff) const {
			return diff >= 0 && diff < maxSize;
		}

		GameObject * Insert() {
			if(!CanHost()) {
				return nullptr;
			}

			uint32_t idx = maxSize - freeSize;
			freeSize -= 1;

			return begin + idx;
		}

		void Remove(GameObject * gameObject) {
			ptrdiff_t diff = GetRelativePosition(gameObject);
			
			if(WithinBounds(diff)) {
				if(freeSize < maxSize) {
					uint32_t lastValidPosition = maxSize - freeSize - 1;
					freeSize += 1;

					Scene::Swap(gameObject, operator[](lastValidPosition));
				}
			} else {
				//@TODO: debug_assert
			}
		}
	};


	std::list<SceneBlock> storage;
	uint32_t numElements;

protected:

	static void Swap(GameObject * lhs, GameObject * rhs) {
		std::swap(*lhs, *rhs);
	}

public:

	GameObject* Insert() {
		auto it = std::find_if(std::begin(storage), std::end(storage), [](const SceneBlock& block) -> bool {
			return block.CanHost();
		});

		if(it == std::end(storage)) {
			storage.emplace_back(500);
		}

		return it->Insert();
	}

};
