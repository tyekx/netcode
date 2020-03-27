#pragma once

#include <iostream>
#include <vector>
#include "ModelInterfaces.h"

class Model : public IModel {
private:
	bool isNull;
	std::vector<std::string> InsertionColumns;
	std::vector<std::string> SelectionColumns;
	unsigned long columnIndexer;
protected:
	std::string Table;
	std::vector<IField*> Columns;
	std::vector<IField*> MergedColumns;
public:
	Model(const std::string & table);

	virtual void Register(IField * field) override;

	void Load(mysqlx::Row & row);

	void Load(mysqlx::Row & row, std::vector<std::string> & columns);

	virtual void SetNull(bool isnull = true) override {
		isNull = isnull;
	}

	virtual bool operator==(std::nullptr_t) const override {
		return isNull;
	}

	virtual bool operator!=(std::nullptr_t) const override {
		return isNull != false;
	}

	virtual bool operator!() const override {
		return isNull;
	}

	operator bool() const {
		return !isNull;
	}

	virtual std::vector<IField*> & GetColumns() override {
		return Columns;
	}

	virtual const std::string & GetTableName() const override {
		return Table;
	}

	void Merge(IModel & model) {
		for (IField * f : model.GetColumns()) {
			Register(f);
		}
	}
};
