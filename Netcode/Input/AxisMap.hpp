#pragma once

#include "Key.h"
#include <Netcode/Common.h>
#include <vector>
#include <algorithm>


namespace Netcode {

	enum DefaultAxes {
		VERTICAL,
		HORIZONTAL,
		FIRE1,
		FIRE2,
		JUMP
	};

	template<typename AxisEnum = DefaultAxes>
	class AxisData {
	public:
		AxisEnum axisId;
		KeyCode positiveKey;
		KeyCode negativeKey;
		float value;

		AxisData() : axisId{ 0 }, positiveKey{ KeyCode::UNDEFINED }, negativeKey{ KeyCode::UNDEFINED }, value{ 0.0f } { }
		AxisData(AxisEnum axisId, KeyCode posKey, KeyCode negKey) :
			axisId{ axisId }, positiveKey{ posKey }, negativeKey{ negKey }, value{ 0.0f } { }
	};

	template<typename AxisEnum = DefaultAxes>
	class AxisMap : public AxisMapBase {

		using ContainerType = std::vector<AxisData<AxisEnum>>;

		ContainerType data;
	public:

		AxisMap(const std::initializer_list<typename ContainerType::value_type> & initList) : data{} {
			uint32_t maxId = 0;

			for(const auto & i : initList) {
				uint32_t axisId = static_cast<uint32_t>(i.axisId);
				maxId = std::max(axisId, maxId);
			}

			ASSERT(maxId <= 64, "AxisMap enum value is out of range. Valid range: [0, 64]");

			data.resize(maxId + 1);

			for(const auto & i : initList) {
				data[static_cast<uint32_t>(i.axisId)] = i;
			}
		}

		virtual float GetAxis(uint32_t axisId) override {
			ASSERT(axisId < static_cast<uint32_t>(data.size()), "Out of range");

			return data[axisId].value;
		}

		virtual void Update(ArrayView<Key> keys) override {
			for(AxisData<AxisEnum> & axisData : data) {
				Key posKey = keys[static_cast<uint32_t>(axisData.positiveKey)];
				Key negKey = keys[static_cast<uint32_t>(axisData.negativeKey)];

				if(posKey.IsPressed() && negKey.IsReleased()) {
					axisData.value = 1.0f;
				}

				if(posKey.IsReleased() && negKey.IsReleased()) {
					axisData.value = -1.0f;
				}

				// both is in the same state
				if(posKey.IsPressed() == negKey.IsPressed()) {
					axisData.value = 0.0f;
				}
			}
		}
	};

}
