#pragma once

#include "BReport.h"

namespace netcode {

	class ClientReport : public BReport {
	protected:
		int UserId;

	public:
		int GetUserId() const;
		void SetUserId(int userId);

		ClientReport();

		bool Deserialize(const Json & json);
		Json::object Serialize() const;
		Json::object CreateStatistics() const;
		static const Json::shape & ShapeOf();
	};

}
