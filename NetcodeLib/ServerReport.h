#pragma once

#include "ClientReport.h"

namespace netcode {

	class ServerReport : public BReport {
	protected:
		double SimulationTime;
		double ReportTime;

		uint32_t PlayerCount;
		uint32_t TickCount;

	public:
		double GetSimulationTime() const;
		double GetReportTime() const;
		uint32_t GetPlayerCount() const;
		uint32_t GetTickCount() const;

		void SetSimulationTime(double value);
		void SetReportTime(double value);
		void SetPlayerCount(uint32_t value);
		void SetTickCount(uint32_t value);

		std::vector<ClientReport> Clients;

		ServerReport();

		void ToString(std::ostream & os) const;

		bool Deserialize(const Json & obj);
		Json::object Serialize() const;
		Json::object CreateStatistics() const;
		static const Json::shape & ShapeOf();
	};

}