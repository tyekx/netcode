#pragma once

#include <memory>
#include <type_traits>

template<typename T>
class NotNull {
	T * ptr;

	void AssignPointer(T * p) {
		if(p == nullptr) {
			throw std::exception{ "NotNull was assigned a nullptr" };
		}
	}

public:
	NotNull(NotNull<T> &&) = delete;
	NotNull<T> & operator=(NotNull<T> &&) = delete;

	NotNull(T * ptr) {
		AssignPointer(ptr);
	}

	~NotNull() = default;

	/*
	@TODO solve shared_ptr versions with lifetime handling
	*/
	NotNull(const std::weak_ptr<T> & weakPtr) {
		if(auto shared = weakPtr.lock()) {
			AssignPointer(shared.get());
		} else throw std::exception("NotNull got an expired weak ptr");
	}

	NotNull(const std::shared_ptr<T> & sharedPtr) {
		AssignPointer(sharedPtr.get());
	}

	NotNull(const std::unique_ptr<T> & uniquePtr) {
		AssignPointer(uniquePtr.get());
	}

	NotNull<T> & operator=(const NotNull<T> & n) {
		AssignPointer(n.ptr);
		return *this;
	}

	T * operator->() const noexcept {
		return ptr;
	}

	T * Get() const noexcept {
		return ptr;
	}

};

