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

	std::tuple<typename T::value_type &, typename U::value_type &> operator*() const {
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

// typename deduce helper
template<typename T, typename U>
zip_iterator_wrapper<T, U> Zip(T & a, U & b) {
	return zip_iterator_wrapper<T, U>{ a, b };
}
