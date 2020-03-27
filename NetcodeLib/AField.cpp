#include "AField.h"

AField::AField(const std::string & name, IModel * collection, uint32_t options) :
	AutoIncrement{ HAS_OPTION(options, E_AUTO_INCREMENT) },
	isDirty{ false },
	ColumnName{  name },
	DefaultColumnIndex{ 0 },
	Options{ options } {
	if (!HAS_OPTION(options, E_IGNORE)) {
		Bind(collection);
	}
}


 void AField::Bind(IModel * model)  {
	model->Register(this);
}

 unsigned long AField::GetColumnIndex() const  {
	return DefaultColumnIndex;
}

 void AField::SetDirty(bool b)  {
	isDirty = b;
}

 bool AField::IsDirty() const  {
	return isDirty;
}

 bool AField::HasOption(EColumnOptions option) const {
	 return HAS_OPTION(Options, option);
 }

AField::~AField() {}

void AField::SetColumnIndex(uint32_t dfi) {
	DefaultColumnIndex = dfi;
}

const std::string & AField::GetName() const {
	return ColumnName;
}