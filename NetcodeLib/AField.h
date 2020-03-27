#pragma once

#include <iostream>
#include "ModelInterfaces.h"

#define HAS_OPTION(input, testFor) ((input & testFor) > 0)

class AField : public IField {
public:
	bool AutoIncrement;
	bool isDirty;
	std::string ColumnName;
	uint32_t DefaultColumnIndex;
	uint32_t Options;

	AField(const std::string & name, IModel * model, uint32_t options = E_NONE);

	AField(AField &&) = delete;
	AField(const AField & f) = delete;

	virtual ~AField();

	virtual const std::string & GetName() const override;

	virtual void SetColumnIndex(uint32_t id) override;

	virtual void Bind(IModel * model) override;

	virtual unsigned long GetColumnIndex() const override;

	virtual void SetDirty(bool b) override;

	virtual bool IsDirty() const override;

	virtual bool HasOption(EColumnOptions option) const override;


	//virtual void Load(mysqlx::Row & row, unsigned long id) = 0;
	//virtual void Save(mysqlx::Row & row, unsigned long id) = 0;
};
