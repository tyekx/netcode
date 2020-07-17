#include "Page.h"

namespace UI {



	void Page::WindowSize(const Netcode::UInt2 & ss) {
		if(windowSize.x != ss.x || windowSize.y != ss.y) {
			windowSize = ss;
			Size(Netcode::Float2{ static_cast<float>(ss.x), static_cast<float>(ss.y) });
			UpdateLayout();
		}
	}

	Netcode::UInt2 Page::WindowSize() const {
		return windowSize;
	}

	Page::Page(const Netcode::Memory::ObjectAllocator & controlAllocator,
		const Netcode::Memory::ObjectAllocator & eventAllocator,
		Netcode::Physics::PhysX & px) : Control{ eventAllocator },
		controlAllocator{ controlAllocator },
		eventAllocator{ eventAllocator },
		windowSize{ Netcode::UInt2::Zero },
		dummyMaterial{ nullptr },
		scene{ nullptr },
		hoveredControl{},
		clickToken{ 0 },
		moveToken{ 0 },
		scrollToken{ 0 } {

		InitPhysx(px);
		Sizing(SizingType::FIXED);
		HorizontalContentAlignment(HorizontalAnchor::LEFT);
		VerticalContentAlignment(VerticalAnchor::TOP);
	}

}
