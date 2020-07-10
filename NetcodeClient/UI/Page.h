#pragma once

#include "Control.h"
#include <Netcode/PhysXWrapper.h>

namespace UI {

	class Page : public Control {
	protected:
		Netcode::UInt2 screenSize;
		Netcode::PxPtr<physx::PxScene> scene;

	public:
		virtual ~Page() = default;

		void ScreenSize(const Netcode::UInt2 & ss);
		Netcode::UInt2 ScreenSize() const;

		Page(Netcode::Physics::PhysX & px);
		
		virtual void InitializeComponents() = 0;

		virtual void OnClick(MouseEventArgs & args) override;
	};

}
