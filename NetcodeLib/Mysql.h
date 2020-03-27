#pragma once

#include <iostream>
#include <sstream>
#include <memory>
#include <mysqlx/xdevapi.h>
#include "ModelInterfaces.h"

struct IMysqlEventHandler {
	virtual void ConnectSuccess() = 0;
	virtual void ConnectFail(const std::string & message) = 0;
};

struct Wrapper {
	mysqlx::Session Session;
	mysqlx::Schema Schema;

	Wrapper() : Session("127.0.0.1", 33060, "root", "password"), Schema(Session.getSchema("netcode")) {}
};

struct JoinData {
	std::string ForeignTable;
	std::string LocalColumn;
	std::string ForeignColumn;

	JoinData(const std::string & foreignTable, const std::string & localColumn, const std::string & foreignColumn);

	std::string ToString();
};

struct CountData {
	std::string ColumnName;
	std::string RenameTo;

	CountData(const std::string & cn, const std::string & rt);

	std::string ToString();
};

std::string Implode(const std::string & glue, std::vector<std::string> & arr);

std::string Implode(const std::string & glue, std::vector<JoinData> & data);

std::string Implode(const std::string & glue, std::vector<CountData> & data);

#define MAX_ROW_COUNT 4294967295U


template<typename U, typename V>
std::string DeduceColumnName(U V::* p) {
	V instance;
	return (instance.*p).GetName();
}

template<typename U, typename V>
std::string DeduceRawName(U V::*p) {
	V instance;
	return (instance.*p).GetRawName();
}

template<typename ... T>
struct SelectImpl;

template<>
struct SelectImpl<> {
	static void Invoke(std::vector<std::string> & projection) {

	}
};

template<typename U, typename ... T>
struct SelectImpl<U, T...> {
	static void Invoke(std::vector<std::string> & projection, U arg, T ... args) {
		projection.push_back(DeduceColumnName(arg));
		SelectImpl<T...>::Invoke(projection, args...);
	}
};


template<typename ... T>
struct FromImplHelper;

template<>
struct FromImplHelper<> {
	static void Invoke(std::vector<std::string> & tables, std::vector<std::string> & projectedColumns, bool starOperator) { }
};

template<typename MODEL_T, typename ... T>
struct FromImplHelper<MODEL_T, T...> {
	static void Invoke(std::vector<std::string> & tables, std::vector<std::string> & projectedColumns, bool starOperator) {
		MODEL_T model;

		if(starOperator) {
			std::vector<IField*> & cols = model.GetColumns();
			for(IField* field : cols) {
				if(field->HasOption(EColumnOptions::E_AGGREGATED)) {
					continue;
				}
				bool found = false;
				std::string name = field->GetName();
				for(std::string & u : projectedColumns) {
					if(u == name) {
						found = true;
						break;
					}
				}
				if(!found) {
					projectedColumns.push_back(name);
				}
			}
		}

		tables.push_back(model.GetTableName());

		FromImplHelper<T...>::Invoke(tables, projectedColumns, starOperator);
	}
};

template<typename ... T>
struct FromImpl {
	static void Invoke(std::vector<std::string> & tables, std::vector<std::string> & projectedColumns, bool starOperator) {
		FromImplHelper<T...>::Invoke(tables, projectedColumns, starOperator);
	}
};

template<typename ... T>
struct GroupByImpl;

template<>
struct GroupByImpl<> {
	static void Invoke(std::vector<std::string> & groupBy) { }
};

template<typename U, typename ... T>
struct GroupByImpl<U, T...> {
	static void Invoke(std::vector<std::string> & groupBy, U head, T ... tail) {
		groupBy.push_back(DeduceColumnName(head));
		GroupByImpl<T...>(groupBy, tail...);
	}
};

template<typename ... T>
struct CountImpl;

template<>
struct CountImpl<> {
	static void Invoke(std::vector<CountData> & counted) { }
};

template<typename U, typename ... T>
struct CountImpl<U, T...> {
	static void Invoke(std::vector<CountData> & abc, U u, T... tail) {
		std::string columnName = DeduceRawName(u);
		std::string projectedName = DeduceColumnName(u);
		abc.emplace_back(columnName, projectedName);
		CountImpl<T...>::Invoke(abc, tail...);
	}
};

template<typename ... T>
struct BindImpl;

template<>
struct BindImpl<> {
	static void Invoke(std::vector<mysqlx::Value> & boundValues) { }
};

template<typename U, typename ... T>
struct BindImpl<U, T...> {
	static void Invoke(std::vector<mysqlx::Value> & boundValues, const U & u, const T & ... tail) {
		boundValues.push_back(u);
		BindImpl<T...>::Invoke(boundValues, tail...);
	}
};

class Mysql {
public:
	Wrapper wrapper;

	class MysqlUpdateQuery {
		std::string Table;
		std::unique_ptr<mysqlx::SqlStatement> Statement;
		std::vector<std::string> Columns;
		std::vector<mysqlx::Value> BoundValues;
		IField & IdColumn;

		std::string GetQueryString();
	public:

		MysqlUpdateQuery(IField & idColumn, IModel & model);

		MysqlUpdateQuery & Debug();
		int Execute();
	};

	class MysqlRawUpdateQuery {
		std::string Table;
		std::unique_ptr<mysqlx::SqlStatement> Statement;
		std::string WhereClause;
		std::vector<std::string> Columns;
		std::vector<mysqlx::Value> BoundValues;
		std::vector<mysqlx::Value> BoundWhereClauseValues;
		std::string LimitClause;

		std::string GetQueryString();

	public:
		MysqlRawUpdateQuery(const std::string & table, std::vector<std::string> cols);

		int Execute();
		MysqlRawUpdateQuery & Debug();

		template<typename ... T>
		MysqlRawUpdateQuery & Set(const T & ... args) {
			BindImpl<T...>::Invoke(BoundValues, args...);
			return *this;
		}

		MysqlRawUpdateQuery & Where(const std::string & whereClause) {
			WhereClause = whereClause;
			return *this;
		}

		MysqlRawUpdateQuery & Safe() {
			LimitClause = " LIMIT 1";
			return *this;
		}

		template<typename ... T>
		MysqlRawUpdateQuery & Bind(const T & ... args) {
			BindImpl<T...>::Invoke(BoundWhereClauseValues, args...);
			return *this;
		}
	};

	class MysqlInsertQuery {
		Mysql * Owner;
		std::string Table;
		std::unique_ptr<mysqlx::SqlStatement> Statement;
		std::vector<std::string> Columns;
		std::vector<std::string> QuestionMarks;
		std::vector<mysqlx::Value> BoundValues;



		std::string GetQueryString();

	public:
		MysqlInsertQuery(Mysql * o, IModel & model);


        int Execute();

		MysqlInsertQuery & Debug();
	};

	class MysqlSelectQuery {
		Mysql * Owner;
		std::vector<std::string> ProjectedColumns;
		std::vector<CountData> CountedColumns;
		std::vector<std::string> Tables;
		std::vector<JoinData> Joins;
		std::string whereClause;
		std::vector<std::string> GroupByColumns;
		std::vector<mysqlx::Value> BoundValues;
		std::unique_ptr<mysqlx::SqlStatement> Statement;
		unsigned int skipN;
		unsigned int takeN;
		mysqlx::SqlResult result;
		bool StarOperator;

	private:
		std::string GetQueryString();
		mysqlx::SqlResult Execute();

	public:
		MysqlSelectQuery & Debug();

		MysqlSelectQuery(Mysql * owner, const std::vector<std::string> & columns);



		MysqlSelectQuery & Where(const std::string & whereClause);
		MysqlSelectQuery & Bind(const mysqlx::Value & value);

		MysqlSelectQuery & Skip(unsigned int value);
		MysqlSelectQuery & Take(unsigned int value);

		void FetchInto(IModel & model);

		template<typename ... T>
		MysqlSelectQuery & From() {
			FromImpl<T...>::Invoke(Tables, ProjectedColumns, ProjectedColumns.empty() && CountedColumns.empty());
			return *this;
		}

		template<typename ... T>
		MysqlSelectQuery & Count(T ... args) {
			CountImpl<T...>::Invoke(CountedColumns, args...);
			return *this;
		}

		template<typename ... T>
		MysqlSelectQuery & GroupBy(T ... args) {
			GroupByImpl<T...>::Invoke(GroupByColumns, args...);
			return *this;
		}

		template<typename LOCAL_MODEL, typename FOREIGN_MODEL, typename KEY_TYPE>
		MysqlSelectQuery & Join(KEY_TYPE LOCAL_MODEL::* localColumn, KEY_TYPE FOREIGN_MODEL::* foreignColumn) {
			LOCAL_MODEL localModelInstance;
			FOREIGN_MODEL foreginModelInstance;

			Joins.emplace_back(foreginModelInstance.GetTableName(), (localModelInstance.*localColumn).GetName(), (foreginModelInstance.*foreignColumn).GetName());

			return *this;
		}

		template<typename T>
		void FetchInto(std::vector<T> & modelVector) {
			mysqlx::RowResult results = Execute();
			for (CountData & d : CountedColumns) {
				ProjectedColumns.push_back(d.RenameTo);
			}
			modelVector.reserve(results.count());
			mysqlx::Row row;
			while ((row = results.fetchOne())) {
				modelVector.emplace_back();
				IModel & model = modelVector[modelVector.size() - 1];
				model.Load(row, ProjectedColumns);
			}

		}
	};


	bool connected;
public:
	Mysql() : wrapper{}, connected{ false } {}



	void Connect(IMysqlEventHandler * handler, const std::string & database) {
		std::string dbSelection = "use " + database;
		wrapper.Session.sql(dbSelection).execute();
		connected = true;
	}

	bool IsConnected() {
		return connected;
	}

	void BeginTransaction();
	void Commit();
	void Rollback();

	MysqlSelectQuery Select() {
		return MysqlSelectQuery{ this, {} };
	}

	MysqlInsertQuery Insert(IModel & model) {
		return MysqlInsertQuery(this, model);
	}

	template<typename ... T>
	MysqlSelectQuery Select(T ... args) {
		std::vector<std::string> projection;
		SelectImpl<T...>::Invoke(projection, args...);
		return MysqlSelectQuery{ this, projection };
	}

	MysqlSelectQuery Select(const std::vector<std::string> & projection) {
		return MysqlSelectQuery{ this, projection };
	}

	MysqlUpdateQuery Update(IField & idField, IModel & model) {
		return MysqlUpdateQuery(idField, model);
	}

	template<typename ... T>
	MysqlRawUpdateQuery Update(const std::string & table, T ... args) {
		std::vector<std::string> projection;
		SelectImpl<T...>::Invoke(projection, args...);
		return MysqlRawUpdateQuery(table, projection);
	}

	mysqlx::Table Table(const std::string & s) {
		return wrapper.Schema.getTable(s);
	}
};

extern Mysql Database;

