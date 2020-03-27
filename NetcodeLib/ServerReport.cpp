#include "ServerReport.h"
#include "Formatter.h"

Json::object netcode::ServerReport::CreateStatistics() const {
	Json::object obj = {
		{ "uptime", Json::object{ {"decimals", 2 }, { "dimension", "seconds" }, {"dimension_short", "s"}, {"value", GetUptime() } } },
		{ "simulation", Json::object{ {"decimals", 5 }, { "dimension", "seconds" }, {"dimension_short", "s"}, {"value", GetSimulationTime() } } },
		{ "report", Json::object{ {"decimals", 5 }, { "dimension", "seconds" }, {"dimension_short", "s"}, {"value", GetReportTime() } } },
		{ "timeframe", Json::object{ {"decimals", 5 }, { "dimension", "seconds" }, {"dimension_short", "s"}, {"value", GetTimeframe() } } },
		{ "players", Json::object{ {"decimals", 0 }, { "dimension", "number of" }, {"dimension_short", "n"}, {"value", (int)PlayerCount } } },
		{ "in_data", Json::object{ {"decimals", 0 }, { "dimension", "bytes" }, {"dimension_short", "b"}, {"value", (int)InBytes  } } },
		{ "out_data", Json::object{ {"decimals", 0 }, { "dimension", "bytes" }, {"dimension_short", "b"}, {"value", (int)OutBytes } } }
	};

	Json::array clientData;

	for(const ClientReport & i : Clients) {
		clientData.emplace_back(i.Serialize());
	}

	obj["clients"] = clientData;

	return obj;
}

const Json::shape & netcode::ServerReport::ShapeOf() {
	const static Json::shape shape = {
		{"simulation", Json::Type::NUMBER},
		{"report", Json::Type::NUMBER},
		{"players", Json::Type::NUMBER},
		{"tick_count", Json::Type::NUMBER},
		{"clients", Json::Type::ARRAY}
	};

	return shape;
}


bool netcode::ServerReport::Deserialize(const Json & json) {
	std::string shapeCheckError;

	Json::shape shape = ServerReport::ShapeOf();

	if(!json.has_shape(shape, shapeCheckError)) {
		return false;
	}

	for(const Json & j : json["clients"].array_items()) {
		if(!j.has_shape(ClientReport::ShapeOf(), shapeCheckError)) {
			return false;
		}
	}
	
	if(!BReport::Deserialize(json)) {
		return false;
	}

	Clients.clear();
	for(const Json & j : json["clients"].array_items()) {
		ClientReport cr;
		cr.Deserialize(j);
		Clients.push_back(cr);
	}

	SetSimulationTime(json["simulation"].number_value());
	SetReportTime(json["report"].number_value());
	SetPlayerCount((uint32_t)json["players"].int_value());
	SetTickCount((uint32_t)json["tick_count"].int_value());

	return true;
}

Json::object netcode::ServerReport::Serialize() const {
	Json::object obj = BReport::Serialize();

	Json::array arr;
	for(const netcode::ClientReport & crep : Clients) {
		arr.push_back(crep.Serialize());
	}

	obj["clients"] = arr;
	obj["simulation"] = GetSimulationTime();
	obj["report"] = GetReportTime();
	obj["players"] = (int)GetPlayerCount();
	obj["tick_count"] = (int)GetTickCount();

	return obj;
}

void netcode::ServerReport::ToString(std::ostream & os) const {
	os << "Server: " << std::endl;
	{
		ConsoleFormatter sFormatter{ os };
		sFormatter.BeginColumnDeclaration();
		sFormatter.AddNumberColumn("Uptime[s]", 2, 10);
		sFormatter.AddNumberColumn("Sim[s]", 5, 10);
		sFormatter.AddNumberColumn("Report[s]", 5, 10);
		sFormatter.AddNumberColumn("Timefr[s]", 5, 10);
		sFormatter.AddNumberColumn("P[n]", 0, 5);
		sFormatter.AddNumberColumn("In[B/s]", 3, 10);
		sFormatter.AddNumberColumn("Out[B/s]", 3, 10);
		sFormatter.EndColumnDeclaration();

		sFormatter.PrintHeader();

		sFormatter.BeginRow();
		sFormatter.PrintValue(Uptime);
		sFormatter.PrintValue(SimulationTime);
		sFormatter.PrintValue(ReportTime);
		sFormatter.PrintValue(Timeframe);
		sFormatter.PrintValue((int)PlayerCount);
		sFormatter.PrintValue(InBytes / Timeframe);
		sFormatter.PrintValue(OutBytes / Timeframe);
		sFormatter.EndRow();
		sFormatter.PrintSeparator();
	}

	if(Clients.size() == 0) {
		return;
	}

	os << "Clients: " << std::endl;
	{
		ConsoleFormatter cFormatter{ os };
		cFormatter.BeginColumnDeclaration();
		cFormatter.AddNumberColumn("ID", 0, 7);
		cFormatter.AddNumberColumn("Uptime", 2, 10);
		cFormatter.AddNumberColumn("In[n]", 2, 10);
		cFormatter.AddNumberColumn("In[B/s]", 2, 10);
		cFormatter.AddNumberColumn("Out[n]", 2, 10);
		cFormatter.AddNumberColumn("Out[B/s]", 2, 10);
		cFormatter.EndColumnDeclaration();

		cFormatter.PrintHeader();
		for(const ClientReport & i : Clients) {
			cFormatter.BeginRow();
			cFormatter.PrintValue(i.GetUserId());
			cFormatter.PrintValue(i.GetUptime());
			cFormatter.PrintValue(i.AverageInMessageCount());
			cFormatter.PrintValue(i.AverageBandwidthIn());
			cFormatter.PrintValue(i.AverageOutMessageCount());
			cFormatter.PrintValue(i.AverageBandwidthOut());
			cFormatter.EndRow();
		}
		cFormatter.PrintSeparator();
	}
}

double netcode::ServerReport::GetSimulationTime() const {
	return SimulationTime;
}

double netcode::ServerReport::GetReportTime() const {
	return ReportTime;
}

uint32_t netcode::ServerReport::GetPlayerCount() const {
	return PlayerCount;
}

uint32_t netcode::ServerReport::GetTickCount() const {
	return TickCount;
}

void netcode::ServerReport::SetSimulationTime(double value) {
	SimulationTime = value;
}

void netcode::ServerReport::SetReportTime(double value) {
	ReportTime = value;
}

void netcode::ServerReport::SetPlayerCount(uint32_t value) {
	PlayerCount = value;
}

void netcode::ServerReport::SetTickCount(uint32_t value) {
	TickCount = value;
}

netcode::ServerReport::ServerReport() : BReport(), SimulationTime{ 0.0 }, ReportTime{ 0.0 }, PlayerCount{ 0 }, TickCount{ 0 } {

}
