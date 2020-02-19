#pragma once

#include "GameObject.h"
#include <list>
#include <memory>
#include <Egg/BulkAllocator.hpp>

class GameObjectView {
protected:
	GameObject * reference;
public:

	bool IsValid() const {
		return reference != nullptr;
	}

	GameObject * Get() const {
		return reference;
	}
};
/*
base class
*/
class Scene {

	class OwnedGameObjectView : public GameObjectView {
	public:
		void Internal_UpdateReference(GameObject * obj) {
			reference = obj;
		}
	};

	/*
	This can not be bulk allocated because we want to keep our storage sorted
	*/
	struct SceneBlock {
		uint32_t freeSize;
		uint32_t maxSize;
		OwnedGameObjectView ** views;
		GameObject * objects;
	public:
		SceneBlock() = delete;
		SceneBlock(const SceneBlock &) = delete;
		SceneBlock & operator=(const SceneBlock &) = delete;

		~SceneBlock() noexcept {
			uint32_t size = maxSize - freeSize;

			for(uint32_t i = 0; i < size; ++i) {
				(objects + i)->~GameObject();
				views[i]->Internal_UpdateReference(nullptr);
				views[i] = nullptr;
			}

			freeSize = 0;
			maxSize = 0;

			std::free(views);
			std::free(objects);

		}

		SceneBlock(SceneBlock &&) noexcept = default;
		SceneBlock & operator=(SceneBlock && rhs) noexcept = default;

		SceneBlock(uint32_t capacity) : freeSize{ capacity }, maxSize{ capacity }, views{ nullptr }, objects{ nullptr } {
			views = static_cast<OwnedGameObjectView **>(std::malloc(capacity * sizeof(OwnedGameObjectView *)));
			objects = static_cast<GameObject *>(std::malloc(capacity * sizeof(GameObject)));
		}

		GameObject * operator[](uint32_t idx) {
			//@TODO: debug assert
			return objects + idx;
		}

		bool CanHost() const {
			return freeSize > 0;
		}

		ptrdiff_t GetRelativePosition(GameObject * gameObject) const {
			return gameObject - objects;
		}

		bool WithinBounds(ptrdiff_t diff) const {
			return diff >= 0 && diff < maxSize;
		}

		uint32_t Size() const {
			return maxSize - freeSize;
		}

		GameObject * Insert(OwnedGameObjectView* view) {
			if(!CanHost()) {
				return nullptr;
			}

			uint32_t idx = maxSize - freeSize;
			views[idx] = view;
			freeSize -= 1;

			GameObject * ptr = objects + idx;

			new (ptr) GameObject();

			view->Internal_UpdateReference(ptr);

			return ptr;
		}

		void Remove(GameObject * gameObject) {
			ptrdiff_t diff = GetRelativePosition(gameObject);
			
			if(WithinBounds(diff)) {
				if(freeSize < maxSize) {
					uint32_t lastValidPosition = maxSize - freeSize - 1;
					freeSize += 1;

					gameObject->~GameObject();
					views[diff]->Internal_UpdateReference(nullptr);
					views[diff] = nullptr;

					std::swap(views[diff], views[lastValidPosition]);
					Scene::Swap(gameObject, operator[](lastValidPosition));
				}
			} else {
				//@TODO: debug_assert
			}
		}
	};


	std::list<SceneBlock> storage;
	uint32_t numElements;
	Egg::BulkAllocator<OwnedGameObjectView> viewStorage;
protected:


	static void Swap(GameObject * lhs, GameObject * rhs) {
		std::swap(*lhs, *rhs);
	}

public:

	void Foreach(std::function<void(GameObject *)> callback) {
		for(SceneBlock & block : storage) {
			uint32_t size = block.Size();

			for(uint32_t i = 0; i < size; ++i) {
				callback(block[i]);
			}
		}
	}

	void Remove(GameObject * obj) {
		auto it = std::find_if(std::begin(storage), std::end(storage), [obj](const SceneBlock & block) -> bool {
			return block.WithinBounds(block.GetRelativePosition(obj));
		});

		if(it != std::end(storage)) {
			it->Remove(obj);
		}
	}

	GameObjectView* Insert() {
		auto it = std::find_if(std::begin(storage), std::end(storage), [](const SceneBlock& block) -> bool {
			return block.CanHost();
		});

		SceneBlock * block;

		if(it == std::end(storage)) {
			block = &storage.emplace_back(500);
		} else {
			block = it.operator->();
		}

		auto view = viewStorage.Allocate();

		block->Insert(view);

		return view;
	}

};
