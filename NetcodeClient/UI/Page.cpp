#include "Page.h"

namespace UI {



	void Page::ScreenSize(const Netcode::UInt2 & ss) {
		screenSize = ss;
		OnScreenResized(screenSize);
	}

	Netcode::UInt2 Page::ScreenSize() const {
		return screenSize;
	}

	Page::Page(Netcode::Physics::PhysX & px) {
		Sizing(SizingType::WINDOW);
		HorizontalContentAlignment(HorizontalAnchor::LEFT);
		VerticalContentAlignment(VerticalAnchor::TOP);
		Disable();

		physx::PxSceneDesc sceneDesc{ px.physics->getTolerancesScale() };
		sceneDesc.gravity = physx::PxVec3{ 0.0f, 0.0f, 0.0f };
		sceneDesc.cpuDispatcher = px.dispatcher.Get();
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;

		scene.Reset(px.physics->createScene(sceneDesc));
	}

	void Page::OnClick(MouseEventArgs & args) {
		
	}

}
