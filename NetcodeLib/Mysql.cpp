#include "Mysql.h"

Mysql Database{};

JoinData::JoinData(const std::string & foreignTable, const std::string & localColumn, const std::string & foreignColumn) :
	ForeignTable{ foreignTable }, LocalColumn{ localColumn }, ForeignColumn{ foreignColumn } {}

std::string JoinData::ToString() {
	std::ostringstream oss;
	oss << "INNER JOIN " << ForeignTable << " ON " << LocalColumn << " = " << ForeignColumn;
	return oss.str();
}

std::string Implode(const std::string & glue, std::vector<std::string> & arr) {
	std::string r = "";
	for (std::string & s : arr) {
		if (r.size() == 0) {
			r = s;
		} else {
			r += glue + s;
		}
	}
	return r;
}

std::string Implode(const std::string & glue, std::vector<JoinData> & data) {
	std::vector<std::string> v;
	for (JoinData & jd : data) {
		v.push_back(jd.ToString());
	}
	return Implode(glue, v);
}

std::string Implode(const std::string & glue, std::vector<CountData> & data) {
	std::vector<std::string> v;
	for (CountData & jd : data) {
		v.push_back(jd.ToString());
	}
	return Implode(glue, v);
}

CountData::CountData(const std::string & cn, const std::string & rt) : ColumnName{ cn }, RenameTo{ rt } {}

std::string CountData::ToString() {
	std::ostringstream oss;
	oss << "COUNT(" << ColumnName << ") AS " << RenameTo;
	return oss.str();
}


Mysql::MysqlSelectQuery & Mysql::MysqlSelectQuery::Skip(unsigned int value) {
	skipN = value;
	return *this;
}

Mysql::MysqlSelectQuery & Mysql::MysqlSelectQuery::Take(unsigned int value) {
	takeN = value;
	return *this;
}

mysqlx::SqlResult Mysql::MysqlSelectQuery::Execute() {
	Statement.reset(new mysqlx::SqlStatement(Database.wrapper.Session.sql(GetQueryString())));
	for (mysqlx::Value & value : BoundValues) {
		Statement->bind(value);
	}
	return Statement->execute();
}

Mysql::MysqlSelectQuery::MysqlSelectQuery(Mysql * owner, const std::vector<std::string> & columns) : ProjectedColumns{ columns }, Statement{ nullptr },
skipN{ 0 }, takeN{ MAX_ROW_COUNT } {
	StarOperator = ProjectedColumns.size() == 0;

	}

Mysql::MysqlSelectQuery & Mysql::MysqlSelectQuery::Where(const std::string & wc) {
	whereClause = wc;
	return *this;
}

Mysql::MysqlSelectQuery & Mysql::MysqlSelectQuery::Bind(const mysqlx::Value & value) {
	BoundValues.emplace_back(value);
	return *this;
}

void Mysql::MysqlSelectQuery::FetchInto(IModel & model) {
	result = Execute();
	for(CountData & c : CountedColumns) {
		ProjectedColumns.push_back(c.RenameTo);
	}

	mysqlx::Row row = result.fetchOne();

	if (row) {
		model.Load(row, ProjectedColumns);
	} else {
		model.SetNull();
	}
}

Mysql::MysqlSelectQuery & Mysql::MysqlSelectQuery::Debug() {
	std::cout << "STR: '" << GetQueryString() << "'" << std::endl;
	return *this;
}

std::string Mysql::MysqlSelectQuery::GetQueryString() {
	std::ostringstream oss;
	oss << "SELECT " << Implode(", ", ProjectedColumns);
	if(CountedColumns.size() > 0) {
		if(ProjectedColumns.size() > 0) {
			oss << ", ";
		}
		oss << Implode(", ", CountedColumns);
	}
	oss << " FROM " << Implode(", ", Tables);

	if(Joins.size() > 0) {
		oss << " " << Implode(" ", Joins);
	}

	if(whereClause.size() > 0) {
		oss << " WHERE " << whereClause;
	}

	if(GroupByColumns.size() > 0) {
		oss << " GROUP BY " << Implode(", ", GroupByColumns);
	}

	if(skipN != 0 && takeN != MAX_ROW_COUNT) {
		oss << " LIMIT " << skipN << ", " << takeN;
	}

	return oss.str();
}


std::string Mysql::MysqlInsertQuery::GetQueryString() {
	return "INSERT INTO " + Table + " (" + Implode(", ", Columns) + ") VALUES (" + Implode(", ", QuestionMarks) + ")";
}

Mysql::MysqlInsertQuery::MysqlInsertQuery(Mysql * o, IModel & model) : Owner{o}{
    Table = model.GetTableName();
    std::vector<IField*> & v = model.GetColumns();
    for(IField * field : v) {
        if(field->IsDirty()) {
            Columns.push_back(field->GetName());
            BoundValues.push_back(field->GetValue());
            QuestionMarks.emplace_back("?");
        }
    }
}


int Mysql::MysqlInsertQuery::Execute() {
	Statement.reset(new mysqlx::SqlStatement(Database.wrapper.Session.sql(GetQueryString())));
	for (mysqlx::Value & value : BoundValues) {
		Statement->bind(value);
	}
	mysqlx::SqlResult r = Statement->execute();

	int affected = (int)r.getAffectedItemsCount();
	if(affected == 0) {
		return 0;
	}
	return (int)r.getAutoIncrementValue();
}

Mysql::MysqlInsertQuery & Mysql::MysqlInsertQuery::Debug() {
	std::cerr << GetQueryString() << std::endl;
	return *this;
}


std::string Mysql::MysqlUpdateQuery::GetQueryString() {
	return "UPDATE " + Table + " SET " + Implode(", ", Columns) + " WHERE " + IdColumn.GetName() + " = ?";
}

Mysql::MysqlUpdateQuery::MysqlUpdateQuery(IField & idColumn, IModel & model) : Table{ model.GetTableName() }, Statement{ nullptr }, Columns{}, BoundValues{}, IdColumn{ idColumn } {
	std::vector<IField*> & fields = model.GetColumns();
	for(IField * f : fields) {
		if(f->IsDirty()) {
			Columns.push_back(f->GetName() + " = ?");
			BoundValues.push_back(f->GetValue());
		}
	}
}

Mysql::MysqlUpdateQuery & Mysql::MysqlUpdateQuery::Debug() {
	std::cerr << GetQueryString() << std::endl;
	return *this;
}

int Mysql::MysqlUpdateQuery::Execute() {
	std::string query = GetQueryString();
	Statement.reset(new mysqlx::SqlStatement(Database.wrapper.Session.sql(query)));
	for(mysqlx::Value & value : BoundValues) {
		Statement->bind(value);
	}
	Statement->bind(IdColumn.GetValue());
	mysqlx::SqlResult result = Statement->execute();

	return (int)result.getAffectedItemsCount();
}


void Mysql::BeginTransaction() {
	Database.wrapper.Session.startTransaction();
}

void Mysql::Rollback() {
	Database.wrapper.Session.rollback();
}

void Mysql::Commit() {
	Database.wrapper.Session.commit();
}


std::string Mysql::MysqlRawUpdateQuery::GetQueryString() {
	std::string query = "UPDATE " + Table + " SET " + Implode(", ", Columns);
	if(!WhereClause.empty()) {
		query += " WHERE " + WhereClause + LimitClause;
	}
	return query;
}
Mysql::MysqlRawUpdateQuery::MysqlRawUpdateQuery(const std::string & table, std::vector<std::string> cols) : Table{ table }, Columns { cols } {
	for(std::string & col : Columns) {
		col += " = ?";
	}
}

int Mysql::MysqlRawUpdateQuery::Execute() {
	std::string query = GetQueryString();
	Statement.reset(new mysqlx::SqlStatement(Database.wrapper.Session.sql(query)));
	for(mysqlx::Value & value : BoundValues) {
		Statement->bind(value);
	}

	for(mysqlx::Value & value : BoundWhereClauseValues) {
		Statement->bind(value);
	}

	mysqlx::SqlResult result = Statement->execute();

	return (int)result.getAffectedItemsCount();
}

Mysql::MysqlRawUpdateQuery & Mysql::MysqlRawUpdateQuery::Debug() {
	std::cerr << GetQueryString() << std::endl;
	return *this;
}
