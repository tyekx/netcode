#include "Model.h"
#include "Mysql.h"

Model::Model(const std::string & table) : Table{ table } {}

void Model::Register(IField * field) {
	field->SetColumnIndex(columnIndexer++);
	Columns.push_back(field);
}

void Model::Load(mysqlx::Row & row) {
	uint32_t i = 0;
	for (IField * field : Columns) {
		field->Load(row, i++);
	}
}

void Model::Load(mysqlx::Row & row, std::vector<std::string> & columns) {
	for (IField * field : Columns) {
		uint32_t i = 0;
		for (std::string & colName : columns) {
			if (field->GetName() == colName) {
				field->Load(row, i);
				field->SetDirty(false);
				isNull = false;
				break;
			}
			i++;
		}
	}
}
