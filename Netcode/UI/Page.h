#pragma once

#include "Control.h"

namespace Netcode::UI {


	class Page : public Control {
	protected:
		Memory::ObjectAllocator controlAllocator;
		Memory::ObjectAllocator eventAllocator;

		UInt2 windowSize;
		PxPtr<physx::PxMaterial> dummyMaterial;
		PxPtr<physx::PxScene> scene;
		std::weak_ptr<UI::Control> hoveredControl;

		EventToken clickToken;
		EventToken moveToken;
		EventToken scrollToken;

		void InitPhysx(Physics::PhysX & px);

		PxPtr<physx::PxRigidDynamic> CreatePhysxActor();

		Control * Raycast(const Int2 & windowPos);

	public:
		virtual ~Page() = default;

		void WindowSize(const UInt2 & ss);
		UInt2 WindowSize() const;

		Page(const Memory::ObjectAllocator & controlAllocator,
			const Memory::ObjectAllocator & eventAllocator,
			Physics::PhysX & px);

		virtual void InitializeComponents() = 0;

		virtual void Destruct() override;

		virtual void Activate();

		virtual void Deactivate();
	};

}
