#include "BReport.h"

double netcode::BReport::AverageBandwidthIn() const {
	return (double)InBytes / Timeframe;
}

double netcode::BReport::AverageBandwidthOut() const {
	return (double)OutBytes / Timeframe;
}

double netcode::BReport::AverageInMessageCount() const {
	return (double)InMessageCount / Timeframe;
}

double netcode::BReport::AverageOutMessageCount() const {
	return (double)OutMessageCount / Timeframe;
}

double netcode::BReport::AverageErrorCount() const {
	return (double)ErrorMessageCount / Timeframe;
}

void netcode::BReport::SetTimeframe(double value) {
	Timeframe = value;
}

double netcode::BReport::GetTimeframe() const {
	return Timeframe;
}

void netcode::BReport::SetUptime(double value) {
	Uptime = value;
}

double netcode::BReport::GetUptime() const {
	return Uptime;
}

void netcode::BReport::IncrementInCounter() {
	++InMessageCount;
}

void netcode::BReport::IncrementOutCounter() {
	++OutMessageCount;
}

void netcode::BReport::IncrementErrCounter() {
	++ErrorMessageCount;
}

void netcode::BReport::AddInBytes(uint32_t bytes) {
	InBytes += bytes;
}

void netcode::BReport::AddOutBytes(uint32_t bytes) {
	OutBytes += bytes;
}

uint32_t netcode::BReport::GetInMessageCount() const {
	return InMessageCount;
}

uint32_t netcode::BReport::GetOutMessageCount() const {
	return OutMessageCount;
}

uint32_t netcode::BReport::GetErrorMessageCount() const {
	return ErrorMessageCount;
}

uint32_t netcode::BReport::GetInBytes() const {
	return InBytes;
}

uint32_t netcode::BReport::GetOutBytes() const {
	return OutBytes;
}

void netcode::BReport::SetInMessageCount(uint32_t value) {
	InMessageCount = value;
}

void netcode::BReport::SetOutMessageCount(uint32_t value) {
	OutMessageCount = value;
}

void netcode::BReport::SetErrorMessageCount(uint32_t value) {
	ErrorMessageCount = value;
}

void netcode::BReport::SetInBytes(uint32_t value) {
	InBytes = value;
}

void netcode::BReport::SetOutBytes(uint32_t value) {
	OutBytes = value;
}

netcode::BReport::BReport() : Timeframe{ 0.0 }, Uptime{ 0.0 }, InMessageCount{ 0 }, OutMessageCount{ 0 }, ErrorMessageCount{ 0 }, InBytes{ 0 }, OutBytes{ 0 } {

}


const Json::shape & netcode::BReport::ShapeOf() {
	const static Json::shape shape = {
		{"timeframe", Json::Type::NUMBER },
		{"uptime", Json::Type::NUMBER },
		{"in_count", Json::Type::NUMBER },
		{"out_count", Json::Type::NUMBER },
		{"err_count", Json::Type::NUMBER },
		{"in_bytes", Json::Type::NUMBER },
		{"out_bytes", Json::Type::NUMBER }
	};

	return shape;
}

bool netcode::BReport::Deserialize(const Json & json) {
	std::string shapeCheckErr;

	if(!json.has_shape(BReport::ShapeOf(), shapeCheckErr)) {
		return false;
	}

	SetTimeframe(json["timeframe"].number_value());
	SetUptime(json["uptime"].number_value());
	SetInMessageCount((uint32_t)json["in_count"].int_value());
	SetOutMessageCount((uint32_t)json["out_count"].int_value());
	SetErrorMessageCount((uint32_t)json["err_count"].int_value());
	SetInBytes((uint32_t)json["in_bytes"].int_value());
	SetOutBytes((uint32_t)json["out_bytes"].int_value());

	return true;
}

Json::object netcode::BReport::Serialize() const {
	Json::object obj = {
		{"timeframe", Timeframe },
		{"uptime", Uptime },
		{"in_count", (int)InMessageCount},
		{"out_count", (int)OutMessageCount},
		{"err_count", (int)ErrorMessageCount},
		{"in_bytes", (int)InBytes},
		{"out_bytes", (int)OutBytes}
	};
	return obj;
}

Json::object netcode::BReport::CreateStatistics() const {
	Json::object obj = {
		{"uptime", Json::object{ { "decimals", 2 }, {"dimension","seconds"}, {"dimension_short","s"}, {"value", GetUptime()} } },
		{"in_updates", Json::object{ { "decimals", 2 }, {"dimension","number of"}, {"dimension_short","n"}, {"value", AverageInMessageCount() } } },
		{"in_data", Json::object{ { "decimals", 3 }, {"dimension","bytes per second"}, {"dimension_short","B/s"}, {"value", AverageBandwidthIn() } } },
		{"out_updates", Json::object{ { "decimals", 2 }, {"dimension","number of"}, {"dimension_short","n"}, {"value", AverageOutMessageCount() } } },
		{"out_data", Json::object{ { "decimals", 3 }, {"dimension","bytes per second"}, {"dimension_short","B/s"}, {"value", AverageBandwidthOut() } } },
		{"errors", Json::object{ { "decimals", 2 }, {"dimension","number of"}, {"dimension_short","n"}, {"value", AverageErrorCount() } } }
	};
	return obj;
}
