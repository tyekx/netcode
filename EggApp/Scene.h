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
		GameObject * end;
	public:

		SceneBlock(uint32_t capacity) : freeSize{ capacity }, maxSize{ capacity }, objects{ nullptr }, begin{ nullptr }, end{ nullptr } {
			objects = std::make_unique<GameObject[]>(capacity);
			begin = objects.get();
			end = begin + capacity;
		}

		SceneBlock(const SceneBlock &) = delete;
		SceneBlock & operator=(const SceneBlock &) = delete;

		~SceneBlock() noexcept = default;
		SceneBlock(SceneBlock &&) noexcept = default;
		SceneBlock & operator=(SceneBlock rhs) noexcept {
			std::swap(freeSize, rhs.freeSize);
			std::swap(maxSize, rhs.maxSize);
			std::swap(objects, rhs.objects);
			std::swap(begin, rhs.begin);
			std::swap(end, rhs.end);
			return *this;
		}

		GameObject * operator[](uint32_t idx) {
			//@TODO: debug assert
			return objects.get() + idx;
		}
	};

	std::list<SceneBlock> storage;
	uint32_t numElements;

protected:
	uint32_t IndexOf(GameObject * gameObject) {

	}

	void Swap(GameObject* lhs, GameObject* rhs) {
		std::swap(*lhs, *rhs);
	}

	void Insert() {

	}

public:

};
