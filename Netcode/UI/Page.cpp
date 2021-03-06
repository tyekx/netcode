#include "Page.h"
#include "Input.h"
#include <NetcodeFoundation/Exceptions.h>
#include <Netcode/Utility.h>
#include "../MathExt.h"
#include "../Input.h"
#include <physx/PxRigidDynamic.h>
#include <physx/PxScene.h>


namespace Netcode::UI {

	namespace Detail {

		class NullInput : public Input {
		public:
			using Input::Input;

			virtual void PropagateOnEnabled() override { }
			virtual void PropagateOnDisabled() override { }
			virtual void PropagateOnSizeChanged() override { }
			virtual void PropagateOnParentChanged() override { }
			virtual void PropagateOnPositionChanged() override { }
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
			virtual void PropagateOnDrag(DragEventArgs & args) override { }
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

		static Ref<NullInput> defaultInput;

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
		physx::PxPhysics & px) : Control{ eventAllocator },
		controlAllocator{ controlAllocator },
		eventAllocator{ eventAllocator },
		lastMousePosition{ Int2::Zero },
		windowSize{ UInt2::Zero },
		dummyMaterial{ nullptr },
		scene{ nullptr },
		raycastedControl{},
		draggedControl{},
		focusedInput{},
		clickToken{ 0 },
		moveToken{ 0 },
		scrollToken{ 0 },
		keyPressedToken{ 0 },
		charToken{ 0 } {

		InitPhysx(px);
		Sizing(SizingType::FIXED);
		HorizontalContentAlignment(HorizontalAnchor::LEFT);
		VerticalContentAlignment(VerticalAnchor::TOP);
		ZIndex(0.0f);

		if(Detail::defaultInput == nullptr) {
			Detail::defaultInput = std::make_shared<Detail::NullInput>(eventAllocator, nullptr);
			Detail::defaultInput->TabIndex(-1);
		}
	}

	void Page::InitPhysx(physx::PxPhysics & px) {
		physx::PxSceneDesc sceneDesc{ px.getTolerancesScale() };
		sceneDesc.gravity = physx::PxVec3{ 0.0f, 0.0f, 0.0f };
		sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;

		scene.Reset(px.createScene(sceneDesc));

		dummyMaterial.Reset(px.createMaterial(0.5f, 0.5f, 0.5f));

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

	void Page::HandleMouseLeaveEnter(const Int2 & windowPos, Control * lastRaycastedPtr, Control * currentRaycastedPtr)
	{
		if(lastRaycastedPtr != currentRaycastedPtr) {
			if(currentRaycastedPtr != nullptr) {
				RaycastEnter(currentRaycastedPtr);
				MouseEventArgs args{ windowPos, Key{}, KeyModifier::NONE };
				currentRaycastedPtr->PropagateOnMouseEnter(args);
			}

			if(lastRaycastedPtr != nullptr) {
				RaycastFade(lastRaycastedPtr);
				MouseEventArgs args{ windowPos, Key{}, KeyModifier::NONE };
				lastRaycastedPtr->PropagateOnMouseLeave(args);
			}
		}
	}

	Control * Page::HandleRaycastChanges(const Int2 & windowPos)
	{
		Control * ctrl = Raycast(windowPos);

		Ref<Control> lastRaycastedControl = raycastedControl.lock();

		HandleMouseLeaveEnter(windowPos, lastRaycastedControl.get(), ctrl);

		if(ctrl != nullptr) {
			raycastedControl = ctrl->weak_from_this();
		} else {
			raycastedControl.reset();
		}

		return ctrl;
	}

	void Page::Destruct() {
		Control::Destruct();

		dummyMaterial.Reset();

		scene.Reset();
	}

	void Page::Activate() {
		lastMousePosition = Netcode::Input::GetMousePosition();

		if(clickToken == 0) {
			clickToken = Netcode::Input::OnMouseInput->Subscribe([this](Key key, KeyModifiers modifier) -> void {

				Ref<Control> control = raycastedControl.lock();

				if(control == nullptr) {
					return;
				}

				MouseEventArgs args{ Netcode::Input::GetMousePosition(), key, modifier };

				if(key.IsRising()) {
					if(key.GetCode() == KeyCode::MOUSE_LEFT) {
						control->PropagateOnClick(args);
					}

					control->PropagateOnMouseKeyPressed(args);

					draggedControl = control;

					Control * handledBy = args.HandledBy();
					Detail::NullSafePtr<Input> input{ static_cast<Input *>(handledBy), Detail::defaultInput.get() };

					Ref<Input> currentlyFocusedInput = focusedInput.lock();
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
			moveToken = Netcode::Input::OnMouseMove->Subscribe([this](Int2 delta, KeyModifiers modifier) -> void {
				const Int2 mousePosition = Netcode::Input::GetMousePosition();


				if(Netcode::Input::GetKey(KeyCode::MOUSE_LEFT).IsPressed()) {
					Ref<Control> dragged = draggedControl.lock();

					Int2 mouseDelta = Int2{ mousePosition.x - lastMousePosition.x, mousePosition.y - lastMousePosition.y };
					if(dragged != nullptr) {
						DragEventArgs args{ mousePosition, KeyCode::UNDEFINED, modifier, mouseDelta };
						dragged->PropagateOnDrag(args);

						if(!args.Handled()) {
							draggedControl.reset();
						}

						if(args.Handled() && args.HandledBy() != dragged.get()) {
							draggedControl = args.HandledBy()->weak_from_this();
						}
					}
				}

				Control * ctrl = HandleRaycastChanges(mousePosition);

				if(ctrl != nullptr) {
					MouseEventArgs args{ mousePosition, KeyCode::UNDEFINED, modifier };
					ctrl->PropagateOnMouseMove(args);
				}

				lastMousePosition = mousePosition;
			});
		}

		if(scrollToken == 0) {
			scrollToken = Netcode::Input::OnScroll->Subscribe([this](int scrollVector, KeyModifiers modifier) -> void {
				Ref<Control> ctrl = raycastedControl.lock();

				if(ctrl != nullptr) {
					ScrollEventArgs scrollArgs{ Netcode::Input::GetMousePosition(), KeyCode::UNDEFINED, modifier, scrollVector };
					ctrl->PropagateOnMouseScroll(scrollArgs);

					if(scrollArgs.Handled()) {
						HandleRaycastChanges(Netcode::Input::GetMousePosition());
					}
				}
			});
		}

		if(keyPressedToken == 0) {
			keyPressedToken = Netcode::Input::OnKeyPressed->Subscribe([this](Key key, KeyModifiers modifier) -> void {
				if(!Netcode::Utility::IsWeakRefEmpty(focusedInput)) {
					Ref<Input> input = focusedInput.lock();

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
					Ref<Input> input = focusedInput.lock();

					if(input != nullptr) {
						CharInputEventArgs eventArgs{ value };
						input->PropagateOnCharInput(eventArgs);
					}
				}
			});
		}

		HandleRaycastChanges(Netcode::Input::GetMousePosition());
	}

	void Page::Deactivate() {
		if(clickToken != 0) {
			Netcode::Input::OnMouseInput->Erase(clickToken);
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

		if(charToken != 0) {
			Netcode::Input::OnCharInput->Erase(charToken);
			charToken = 0;
		}

		HandleRaycastChanges(Int2{ -50000, -50000 });
	}

	bool Page::IsActive() const
	{
		return clickToken != 0 || moveToken != 0 || scrollToken != 0 || keyPressedToken != 0 || charToken != 0;
	}

}
