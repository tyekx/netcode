#pragma once

#include "netcode_types.h"

namespace netcode {

	class BReport {
	protected:
		double Timeframe;
		double Uptime;
		uint32_t InMessageCount;
		uint32_t OutMessageCount;
		uint32_t ErrorMessageCount;
		uint32_t InBytes;
		uint32_t OutBytes;

	public:
		void IncrementInCounter();
		void IncrementOutCounter();
		void IncrementErrCounter();
		void AddInBytes(uint32_t bytes);
		void AddOutBytes(uint32_t bytes);

		double AverageBandwidthIn() const;
		double AverageBandwidthOut() const;
		double AverageInMessageCount() const;
		double AverageOutMessageCount() const;
		double AverageErrorCount() const;

		double GetTimeframe() const;
		double GetUptime() const;
		uint32_t GetInMessageCount() const;
		uint32_t GetOutMessageCount() const;
		uint32_t GetErrorMessageCount() const;
		uint32_t GetInBytes() const;
		uint32_t GetOutBytes() const;

		void SetTimeframe(double value);
		void SetUptime(double value);
		void SetInMessageCount(uint32_t value);
		void SetOutMessageCount(uint32_t value);
		void SetErrorMessageCount(uint32_t value);
		void SetInBytes(uint32_t value);
		void SetOutBytes(uint32_t value);

		bool Deserialize(const Json & json);
		Json::object Serialize() const;
		Json::object CreateStatistics() const;
		static const Json::shape & ShapeOf();

		BReport();
	};

}
