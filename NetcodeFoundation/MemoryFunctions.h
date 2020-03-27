#pragma once

namespace Netcode {

	void InterpretSkip(void ** ptr, unsigned int nBytes);

	template<typename T>
	T InterpretAs(void ** ptr) {
		void * src = *ptr;
		T data = *(reinterpret_cast<T *>(src));
		*ptr = reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(src) + sizeof(T));
		return data;
	}

	template<typename T>
	T * InterpretAsArray(void ** ptr, int len) {
		void * src = *ptr;
		T * data = (reinterpret_cast<T *>(src));
		*ptr = reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(src) + len * sizeof(T));
		return data;
	}

}
