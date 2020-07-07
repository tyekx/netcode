#include "Page.h"

namespace UI {



	void Page::ScreenSize(const Netcode::UInt2 & ss) {
		screenSize = ss;
		OnScreenResized(screenSize);
	}

	Netcode::UInt2 Page::ScreenSize() const {
		return screenSize;
	}

	Page::Page(physx::PxPhysics * pxPhysics) {
		Sizing(SizingType::WINDOW);
		HorizontalContentAlignment(HorizontalAnchor::LEFT);
		VerticalContentAlignment(VerticalAnchor::TOP);
		Disable();

		scene.Reset(pxPhysics->createScene(physx::PxSceneDesc{ physx::PxTolerancesScale{} }));
	}

	void Page::OnClick(MouseEventArgs & args) {
		
	}

}
