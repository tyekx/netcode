#pragma once

#include "Control.h"
#include <Netcode/PhysXWrapper.h>
#include <Netcode/Input.h>
#include <Netcode/MathExt.h>
#include <Netcode/Utility.h>

namespace UI {

	class Page : public Control {
	protected:
		Netcode::UInt2 screenSize;
		Netcode::PxPtr<physx::PxScene> scene;
		std::weak_ptr<UI::Control> hoveredControl;

		Netcode::PxPtr<physx::PxRigidDynamic> CreatePhysxActor() {
			physx::PxPhysics & physics = scene->getPhysics();

			Netcode::PxPtr<physx::PxRigidDynamic> actor = physics.createRigidDynamic(physx::PxTransform{ physx::PxIdentity });
			Netcode::PxPtr<physx::PxMaterial> dummyMat = physics.createMaterial(0.5f, 0.5f, 0.5f);

			actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
			scene->addActor(*actor);
			
			physx::PxRigidActorExt::createExclusiveShape(*actor, physx::PxBoxGeometry{ 0.1f, 0.1f, 0.1f }, *dummyMat, physx::PxShapeFlag::eTRIGGER_SHAPE | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eVISUALIZATION);

			return actor;
		}

		void FadeAway(std::shared_ptr<UI::Control> ctrl, MouseEventArgs & evtArgs) {
			if(ctrl != nullptr) {
				ctrl->OnMouseLeave(evtArgs);
			}
		}

	public:
		virtual ~Page() = default;

		void ScreenSize(const Netcode::UInt2 & ss);
		Netcode::UInt2 ScreenSize() const;

		Page(Netcode::Physics::PhysX & px);
		
		virtual void InitializeComponents() = 0;

		virtual void OnClick(MouseEventArgs & args) override;

		virtual void Update(float dt) override {
			Control::Update(dt);

			Netcode::Int2 v = Netcode::Input::GetMousePosition();
			MouseEventArgs evtArgs{ v };

			Netcode::Float3 mousePos = Netcode::Float3{ static_cast<float>(v.x), static_cast<float>(v.y), Control::MAX_DEPTH };

			physx::PxRaycastBuffer outRaycastResult;

			if(scene->raycast(Netcode::ToPxVec3(mousePos), physx::PxVec3{ 0.0f, 0.0f, -1.0f }, Control::MAX_DEPTH, outRaycastResult)) {
				physx::PxU32 numHits = outRaycastResult.getNbAnyHits();

				for(physx::PxU32 i = 0; i < numHits && !evtArgs.Handled(); ++i) {
					const physx::PxRaycastHit & hit = outRaycastResult.getAnyHit(i);

					Input * input = static_cast<Input *>(hit.actor->userData);
					if(input != nullptr) {
						auto ctrl = input->shared_from_this();

						
						if(!Netcode::Utility::IsWeakRefEmpty(hoveredControl)) {
							auto hCtrl = hoveredControl.lock();

							if(hCtrl != ctrl) {
								FadeAway(hCtrl, evtArgs);
								ctrl->OnMouseEnter(evtArgs);
							}

						} else {
							hoveredControl = ctrl;
							ctrl->OnMouseEnter(evtArgs);
						}

						ctrl->OnMouseMove(evtArgs);
					}
				}
			} else {
				if(!Netcode::Utility::IsWeakRefEmpty(hoveredControl)) {
					FadeAway(hoveredControl.lock(), evtArgs);
					hoveredControl = std::weak_ptr<UI::Control>{};
				}
			}
		}
	};

}
