#pragma once

#include <mysqlx/xdevapi.h>


enum EColumnOptions {
	E_NONE = 0, E_AUTO_INCREMENT = 1, E_PRIMARY_KEY = 2, E_FOREIGN_KEY = 4, E_IGNORE = 8, E_AGGREGATED = 16
};

struct IModel;

struct IField {
	virtual void Bind(IModel * model) = 0;

	virtual void SetColumnIndex(uint32_t id) = 0;
	virtual unsigned long GetColumnIndex() const = 0;

	virtual void SetDirty(bool b) = 0;
	virtual bool IsDirty() const = 0;

	virtual const std::string & GetName() const = 0;

	virtual void Load(mysqlx::Row & row, unsigned int id) = 0;

	virtual mysqlx::Value GetValue() = 0;

	virtual void Print(std::ostream & os) = 0;

	virtual bool HasOption(EColumnOptions option) const = 0;
};

struct IModel {
	virtual void Register(IField * field) = 0;

	virtual void Load(mysqlx::Row & row, std::vector<std::string> & columns) = 0;

	virtual const std::string & GetTableName() const = 0;

	virtual std::vector<IField*> & GetColumns() = 0;

	virtual void Save() = 0;

	virtual void SetNull(bool isnull = true) = 0;

	virtual bool operator==(std::nullptr_t) const = 0;

	virtual bool operator!=(std::nullptr_t) const = 0;

	virtual bool operator!() const = 0;
};

std::ostream & operator<<(std::ostream & os, IField & field);

