#pragma once

#include "AField.h"

template<typename T>
class NullableField : public AField {
	T * Value;

public:
	NullableField(const std::string & columnName, IModel * collection) : AField(collection->GetTableName() + "." + columnName, collection), Value{ nullptr } {
	}

	NullableField(const std::string & columnName, IModel * collection, const T & defaultValue) : AField(collection->GetTableName() + "." + columnName, collection), Value{ new T{ defaultValue } } {}

	~NullableField() {
		if (Value != nullptr) {
			delete Value;
		}
		Value = nullptr;
	}

	virtual bool IsNull() const {
		return Value == nullptr;
	}

	const T & operator*() const {
		return *Value;
	}

	NullableField & operator=(const NullableField & n) {
		delete Value;
		Value = nullptr;
		if(n.IsNull()) {
			return *this;
		}
		Value = new T((const T &)(*n.Value));
		return *this;
	}

	NullableField & operator=(std::nullptr_t nullp) {
		delete Value;
		Value = nullptr;
		SetDirty(true);
		return *this;
	}

	NullableField & operator=(const T & value) {
		delete Value;
		Value = new T{ value };
		SetDirty(true);
		return *this;
	}

	virtual void Load(mysqlx::Row & row, unsigned int columnId) override {
		mysqlx::Value & v = row.get(columnId);
		if (v.isNull()) {
			if (Value != nullptr) {
				delete Value;
			}
			Value = nullptr;
		} else {
			Value = new T{ v.get<T>() };
		}
	}

	operator T() const {
		return *Value;
	}

	bool operator==(std::nullptr_t) {
		return IsNull();
	}

	bool operator==(const T & op) {
		if(IsNull()) {
			return false;
		} else {
			return *Value == op;
		}
	}

	bool operator!=(std::nullptr_t) {
		return !IsNull();
	}

	bool operator!=(const T & op) {
		if(IsNull()) {
			return true;
		} else {
			return *Value != op;
		}
	}

	virtual void Print(std::ostream & os) {
		if(IsNull()) {
			os << "<null>";
		} else {
			os << *Value;
		}
	}

	virtual mysqlx::Value GetValue() override {
		if(IsNull()) {
			return mysqlx::Value(nullptr);
		} else {
			return mysqlx::Value(*Value);
		}
	}
};
