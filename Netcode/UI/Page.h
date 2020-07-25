#pragma once

#include "Control.h"

namespace Netcode::UI {

	class Input;

	class Page : public Control {
	protected:
		Memory::ObjectAllocator controlAllocator;
		Memory::ObjectAllocator eventAllocator;

		Int2 lastMousePosition;
		UInt2 windowSize;
		PxPtr<physx::PxMaterial> dummyMaterial;
		PxPtr<physx::PxScene> scene;
		std::weak_ptr<Control> raycastedControl;
		std::weak_ptr<Control> draggedControl;
		std::weak_ptr<Input> focusedInput;

		EventToken clickToken;
		EventToken moveToken;
		EventToken scrollToken;
		EventToken keyPressedToken;
		EventToken charToken;

		void InitPhysx(Physics::PhysX & px);

		PxPtr<physx::PxRigidDynamic> CreatePhysxActor();

		Control * Raycast(const Int2 & windowPos);

		static void HandleMouseLeaveEnter(const Int2 & windowPos, Control * lastRaycastedPtr, Control * currentRaycastedPtr);

		Control * HandleRaycastChanges(const Int2 & windowPos);

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
