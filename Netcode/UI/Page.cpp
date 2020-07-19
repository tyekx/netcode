#include "Page.h"
#include "Input.h"
#include <NetcodeFoundation/Exceptions.h>
#include "../MathExt.h"
#include "../Input.h"

namespace Netcode::UI {

	namespace Detail {

		class NullInput : public Input {
		public:
			using Input::Input;

			virtual void PropagateOnCharInput(CharInputEventArgs & args) override { }
			virtual void PropagateOnFocused(FocusChangedEventArgs & args) override { }
			virtual void PropagateOnBlurred(FocusChangedEventArgs & args) override { }
			virtual void PropagateOnMouseKeyPressed(MouseEventArgs & args) override { }
			virtual void PropagateOnMouseKeyReleased(MouseEventArgs & args) override { }
			virtual void PropagateOnClick(MouseEventArgs & args) override { }
			virtual void PropagateOnMouseEnter(MouseEventArgs & args) override { }
			virtual void PropagateOnMouseMove(MouseEventArgs & args) override { }
			virtual void PropagateOnMouseLeave(MouseEventArgs & args) override { }
			virtual void PropagateOnMouseScroll(ScrollEventArgs & args) override { }
			virtual void PropagateOnKeyPressed(KeyEventArgs & args) override { }
		};

		template<typename T>
		class NullSafePtr {
			T * main;
			T * secondary;
		public:
			NullSafePtr(T * main, T * secondary) : main{ main }, secondary{ secondary } {
				UndefinedBehaviourAssertion(secondary != nullptr);
			}

			T * Get() const {
				if(main) {
					return main;
				} else {
					return secondary;
				}
			}

			bool operator==(const NullSafePtr<T> & rhs) const {
				return Get() == rhs.Get();
			}

			bool operator!=(const NullSafePtr<T> & rhs) const {
				return !operator==(rhs);
			}

			T * operator->() {
				if(main) {
					return main;
				} else {
					return secondary;
				}
			}
		};

		static std::shared_ptr<NullInput> defaultInput;

	};

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

		if(Detail::defaultInput == nullptr) {
			Detail::defaultInput = std::make_shared<Detail::NullInput>(eventAllocator, nullptr);
			Detail::defaultInput->TabIndex(-1);
		}

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
			clickToken = Netcode::Input::OnMouseInput->Subscribe([this](Key key, KeyModifier modifier) -> void {
				const Int2 mousePosition = Netcode::Input::GetMousePosition();

				Control * raycastedCtrl = Raycast(mousePosition);

				Detail::NullSafePtr<Control> control{ raycastedCtrl, Detail::defaultInput.get() };

				MouseEventArgs args{ mousePosition, key, modifier };

				if(key.IsRising()) {
					/*
					if(key.GetCode() == KeyCode::MOUSE_LEFT) {
						control->PropagateOnClick(args);
					}*/

					control->PropagateOnMouseKeyPressed(args);

					Control * handledBy = args.HandledBy();
					Detail::NullSafePtr<Input> input{ static_cast<Input *>(handledBy), Detail::defaultInput.get() };

					std::shared_ptr<Input> currentlyFocusedInput = focusedInput.lock();
					Detail::NullSafePtr<Input> lastInput{ currentlyFocusedInput.get(), Detail::defaultInput.get() };

					if(input != lastInput) {
						FocusChangedEventArgs blurredArgs{ input->TabIndex() };
						lastInput->PropagateOnBlurred(blurredArgs);
					}


					FocusChangedEventArgs focusedArgs{ input->TabIndex() };
					input->PropagateOnFocused(focusedArgs);

					focusedInput = std::dynamic_pointer_cast<Input>(input->shared_from_this());
				} else {
					control->PropagateOnMouseKeyReleased(args);
				}
			});
		}

		if(moveToken == 0) {
			moveToken = Netcode::Input::OnMouseMove->Subscribe([this](Int2 delta, KeyModifier modifier) -> void {
				const Int2 mousePosition = Netcode::Input::GetMousePosition();

				Control * ctrl = Raycast(mousePosition);

				if(ctrl != nullptr) {
					MouseEventArgs args{ mousePosition, KeyCode::UNDEFINED, modifier };
					ctrl->PropagateOnMouseMove(args);
				}

				MouseEventArgs leaveArgs{ mousePosition, KeyCode::UNDEFINED, modifier };
				PropagateOnMouseLeave(leaveArgs);
			});
		}

		if(scrollToken == 0) {
			scrollToken = Netcode::Input::OnScroll->Subscribe([this](int scrollVector, KeyModifier modifier) -> void {
				const Int2 mousePosition = Netcode::Input::GetMousePosition();

				Control * ctrl = Raycast(mousePosition);

				if(ctrl != nullptr) {
					ScrollEventArgs scrollArgs{ mousePosition, KeyCode::UNDEFINED, modifier, scrollVector };
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
