#include "Page.h"
#include "Input.h"
#include <NetcodeFoundation/Exceptions.h>
#include "../MathExt.h"
#include "../Input.h"

namespace Netcode::UI {

	void Page::WindowSize(const UInt2 & ss) {
		if(windowSize.x != ss.x || windowSize.y != ss.y) {
			windowSize = ss;
			Size(Netcode::Float2{ static_cast<float>(ss.x), static_cast<float>(ss.y) });
			UpdateLayout();
		}
	}

	UInt2 Page::WindowSize() const {
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
		scrollToken{ 0 },
		keyPressedToken{ 0 },
		charToken{ 0 } {

		InitPhysx(px);
		Sizing(SizingType::FIXED);
		HorizontalContentAlignment(HorizontalAnchor::LEFT);
		VerticalContentAlignment(VerticalAnchor::TOP);
	}

	void Page::InitPhysx(Physics::PhysX & px) {
		physx::PxSceneDesc sceneDesc{ px.physics->getTolerancesScale() };
		sceneDesc.gravity = physx::PxVec3{ 0.0f, 0.0f, 0.0f };
		sceneDesc.cpuDispatcher = px.dispatcher.Get();
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;

		scene.Reset(px.physics->createScene(sceneDesc));

		dummyMaterial.Reset(px.physics->createMaterial(0.5f, 0.5f, 0.5f));

		AssignActor(CreatePhysxActor());
	}

	PxPtr<physx::PxRigidDynamic> Page::CreatePhysxActor() {
		physx::PxPhysics & physics = scene->getPhysics();

		PxPtr<physx::PxRigidDynamic> actor = physics.createRigidDynamic(physx::PxTransform{ physx::PxIdentity });

		actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
		scene->addActor(*actor);

		physx::PxRigidActorExt::createExclusiveShape(*actor, physx::PxBoxGeometry{ 0.1f, 0.1f, 0.1f }, *dummyMaterial, physx::PxShapeFlag::eTRIGGER_SHAPE | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eVISUALIZATION);

		return actor;
	}

	Control * Page::Raycast(const Int2 & windowPos) {
		const Float3 rayStart = Float3{ static_cast<float>(windowPos.x), static_cast<float>(windowPos.y), Control::MAX_DEPTH };
		const Float3 rayDir = Float3{ 0.0f, 0.0f, -1.0f };

		physx::PxRaycastBuffer outRaycastResult;

		if(scene->raycast(ToPxVec3(rayStart), ToPxVec3(rayDir), Control::MAX_DEPTH, outRaycastResult)) {
			physx::PxU32 numHits = outRaycastResult.getNbAnyHits();

			Control * candidate = nullptr;

			for(physx::PxU32 i = 0; i < numHits; ++i) {
				const physx::PxRaycastHit & hit = outRaycastResult.getAnyHit(i);

				Control * input = static_cast<Control *>(hit.actor->userData);

				if(candidate == nullptr || candidate->ZIndex() < input->ZIndex()) {
					candidate = input;
				}
			}

			Netcode::UndefinedBehaviourAssertion(candidate != nullptr);

			return candidate;
		}

		return nullptr;
	}

	void Page::Destruct() {
		Control::Destruct();

		dummyMaterial.Reset();

		scene.Reset();
	}

	void Page::Activate() {
		if(clickToken == 0) {
			clickToken = Netcode::Input::OnMouseKeyPressed->Subscribe([this](Key key, KeyModifier modifier) -> void {
				if(key.IsRising() && key.GetCode() == Netcode::KeyCode::MOUSE_LEFT) {
					const Int2 mousePosition = Netcode::Input::GetMousePosition();

					Control * ctrl = Raycast(mousePosition);

					MouseEventArgs args{ mousePosition, modifier };
					if(ctrl != nullptr) {
						ctrl->PropagateOnClick(args);
					}

					Control * handledBy = args.HandledBy();

					if(handledBy != nullptr) {
						std::shared_ptr<Input> input = std::dynamic_pointer_cast<Input>(handledBy->shared_from_this());

						if(input != nullptr) {
							std::shared_ptr<Input> currentlyFocusedInput = focusedInput.lock();

							if(input != currentlyFocusedInput) {
								if(currentlyFocusedInput != nullptr) {
									FocusChangedEventArgs blurredArgs{ input->TabIndex() };
									currentlyFocusedInput->PropagateOnBlurred(blurredArgs);
								}

								focusedInput = input;

								FocusChangedEventArgs focusedArgs{ input->TabIndex() };
								input->PropagateOnFocused(focusedArgs);
							}
						} else {
							std::shared_ptr<Input> currentlyFocusedInput = focusedInput.lock();

							if(currentlyFocusedInput != nullptr) {
								FocusChangedEventArgs blurredArgs{ -1 };
								currentlyFocusedInput->PropagateOnBlurred(blurredArgs);
								focusedInput.reset();
							}
						}
					}
				}
			});
		}

		if(moveToken == 0) {
			moveToken = Netcode::Input::OnMouseMove->Subscribe([this](Int2 delta, KeyModifier modifier) -> void {
				const Int2 mousePosition = Netcode::Input::GetMousePosition();

				Control * ctrl = Raycast(mousePosition);

				if(ctrl != nullptr) {
					MouseEventArgs args{ mousePosition, modifier };
					ctrl->PropagateOnMouseMove(args);
				}

				MouseEventArgs leaveArgs{ mousePosition, modifier };
				PropagateOnMouseLeave(leaveArgs);
			});
		}

		if(scrollToken == 0) {
			scrollToken = Netcode::Input::OnScroll->Subscribe([this](int scrollVector, KeyModifier modifier) -> void {
				const Int2 mousePosition = Netcode::Input::GetMousePosition();

				Control * ctrl = Raycast(mousePosition);

				if(ctrl != nullptr) {
					ScrollEventArgs scrollArgs{ mousePosition, modifier, scrollVector };
					ctrl->PropagateOnClick(scrollArgs);
				}
			});
		}

		if(keyPressedToken == 0) {
			keyPressedToken = Netcode::Input::OnKeyPressed->Subscribe([this](Key key, KeyModifier modifier) -> void {
				if(!Netcode::Utility::IsWeakRefEmpty(focusedInput)) {
					std::shared_ptr<Input> input = focusedInput.lock();

					if(input != nullptr) {
						KeyEventArgs eventArgs{ key, modifier };
						input->PropagateOnKeyPressed(eventArgs);
					}
				}
			});
		}

		if(charToken == 0) {
			charToken = Netcode::Input::OnCharInput->Subscribe([this](wchar_t value) -> void {
				if(!Netcode::Utility::IsWeakRefEmpty(focusedInput)) {
					std::shared_ptr<Input> input = focusedInput.lock();

					if(input != nullptr) {
						CharInputEventArgs eventArgs{ value };
						input->PropagateOnCharInput(eventArgs);
					}
				}
			});
		}
	}

	void Page::Deactivate() {
		if(clickToken != 0) {
			Netcode::Input::OnKeyPressed->Erase(clickToken);
			clickToken = 0;
		}

		if(moveToken != 0) {
			Netcode::Input::OnMouseMove->Erase(moveToken);
			moveToken = 0;
		}

		if(scrollToken != 0) {
			Netcode::Input::OnScroll->Erase(scrollToken);
			scrollToken = 0;
		}

		if(keyPressedToken != 0) {
			Netcode::Input::OnKeyPressed->Erase(keyPressedToken);
			keyPressedToken = 0;
		}
	}

}
