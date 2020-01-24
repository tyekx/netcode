#pragma once


template<typename T, typename U>
class zip_iterator {
	T a;
	U b;

public:
	zip_iterator(T it1, U it2) : a{ it1 }, b{ it2 } {

	}

	zip_iterator & operator++() {
		++a;
		++b;
		return *this;
	}

	std::tuple<typename T::reference, typename U::reference> operator*() const {
		return std::tie(*a, *b);
	}

	bool operator!=(const zip_iterator & rhs) const {
		return a != rhs.a && b != rhs.b;
	}
};


template<typename T, typename U>
class zip_iterator_wrapper {
	T & a;
	U & b;
public:

	zip_iterator_wrapper(T & a, U & b) : a{ a }, b{ b } {

	}

	zip_iterator<typename T::iterator, typename U::iterator> begin() const {
		return zip_iterator<typename T::iterator, typename U::iterator>{ a.begin(), b.begin() };
	}

	zip_iterator<typename T::iterator, typename U::iterator> end() const {
		return zip_iterator<typename T::iterator, typename U::iterator>{ a.end(), b.end() };
	}
};

template<typename T, typename U>
class zip_const_iterator_wrapper {
	const T & a;
	const U & b;
public:

	zip_const_iterator_wrapper(const T & a, const U & b) : a{ a }, b{ b } {

	}

	zip_iterator<typename T::const_iterator, typename U::const_iterator> begin() const {
		return zip_iterator<typename T::const_iterator, typename U::const_iterator>{ a.cbegin(), b.cbegin() };
	}

	zip_iterator<typename T::const_iterator, typename U::const_iterator> end() const {
		return zip_iterator<typename T::const_iterator, typename U::const_iterator>{ a.cend(), b.cend() };
	}
};

// typename deduce helper
template<typename T, typename U>
zip_iterator_wrapper<T, U> Zip(T & a, U & b) {
	return zip_iterator_wrapper<T, U>{ a, b };
}

template<typename T, typename U>
zip_const_iterator_wrapper<T, U> ZipConst(const T & a, const U & b) {
	return zip_const_iterator_wrapper<T, U>{ a, b };
}
