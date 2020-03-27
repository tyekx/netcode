#pragma once
#include <type_traits>
#include "AField.h"

template<typename T>
class Field : public AField {
	T Value;
public:
	Field(const std::string & columnName, IModel * collection, uint32_t options = E_NONE) : AField{ collection->GetTableName() + "." + columnName, collection, options }, Value{} { }

	Field(const std::string & columnName, IModel * collection, const T & defaultValue, uint32_t options = E_NONE) : AField{ collection->GetTableName() + "." + columnName, collection, options }, Value{ defaultValue } { }

	const T & operator*() const {
		return Value;
	}

	Field(Field &&) = delete;
	Field(const Field &) = delete;

	Field & operator=(const Field & field) {
		Value = field.Value;
		return *this;
	}

	Field & operator=(const T & value) {
		Value = value;
		SetDirty(true);
		return *this;
	}

	virtual void Load(mysqlx::Row & row, unsigned int id) override {
		Value = row.get(id).get<T>();
	}


	virtual void Print(std::ostream & os) override {
		os << Value;
	}

	operator T() const {
		return Value;
	}

	bool operator==(const T & t) {
		return Value == t;
	}

	virtual mysqlx::Value GetValue() override {
		return mysqlx::Value(Value);
	}
};

template<typename T>
class AggregatedField : public AField {
	T Value;
	std::string RawName;

	std::string TransformName(const std::string & s) {
		std::string cp = s;
		for (char & c : cp) {
			if (c == '.') {
				c = '_';
			}
		}
		return cp;
	}
public:

	AggregatedField(IField & aggregateOf, IModel * model) : AField{ TransformName(aggregateOf.GetName()) + "_nc_agg", model, E_AGGREGATED }, Value{}, RawName{ aggregateOf.GetName() } { }

	const std::string & GetRawName() const  {
		return RawName;
	}

	virtual void Load(mysqlx::Row & row, unsigned int id) override {
		Value = row.get(id).get<T>();
	}

	virtual void Print(std::ostream & os) override {
		os << Value;
	}

	operator T() const {
		return Value;
	}

	bool operator==(const T & t) {
		return Value == t;
	}

	virtual mysqlx::Value GetValue() override {
		throw mysqlx::Error("Aggregated field cant be modified");
	}
};
