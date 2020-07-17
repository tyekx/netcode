#pragma once

#include "Control.h"
#include <Netcode/PhysXWrapper.h>
#include <Netcode/Input.h>
#include <Netcode/MathExt.h>
#include <Netcode/Utility.h>

namespace UI {

	class Page : public Control {
	protected:
		Netcode::Memory::ObjectAllocator controlAllocator;
		Netcode::Memory::ObjectAllocator eventAllocator;
		
		Netcode::UInt2 windowSize;
		Netcode::PxPtr<physx::PxMaterial> dummyMaterial;
		Netcode::PxPtr<physx::PxScene> scene;
		std::weak_ptr<UI::Control> hoveredControl;

		Netcode::EventToken clickToken;
		Netcode::EventToken moveToken;
		Netcode::EventToken scrollToken;

		void InitPhysx(Netcode::Physics::PhysX & px) {
			physx::PxSceneDesc sceneDesc{ px.physics->getTolerancesScale() };
			sceneDesc.gravity = physx::PxVec3{ 0.0f, 0.0f, 0.0f };
			sceneDesc.cpuDispatcher = px.dispatcher.Get();
			sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;

			scene.Reset(px.physics->createScene(sceneDesc));

			dummyMaterial.Reset(px.physics->createMaterial(0.5f, 0.5f, 0.5f));

			AssignActor(CreatePhysxActor());
		}

		Netcode::PxPtr<physx::PxRigidDynamic> CreatePhysxActor() {
			physx::PxPhysics & physics = scene->getPhysics();

			Netcode::PxPtr<physx::PxRigidDynamic> actor = physics.createRigidDynamic(physx::PxTransform{ physx::PxIdentity });

			actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
			scene->addActor(*actor);
			
			physx::PxRigidActorExt::createExclusiveShape(*actor, physx::PxBoxGeometry{ 0.1f, 0.1f, 0.1f }, *dummyMaterial, physx::PxShapeFlag::eTRIGGER_SHAPE | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eVISUALIZATION);

			return actor;
		}

		Control * Raycast(const Netcode::Int2 & windowPos) {
			const Netcode::Float3 rayStart = Netcode::Float3{ static_cast<float>(windowPos.x), static_cast<float>(windowPos.y), Control::MAX_DEPTH };
			const Netcode::Float3 rayDir = Netcode::Float3{ 0.0f, 0.0f, -1.0f };

			physx::PxRaycastBuffer outRaycastResult;

			if(scene->raycast(Netcode::ToPxVec3(rayStart), Netcode::ToPxVec3(rayDir), Control::MAX_DEPTH, outRaycastResult)) {
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

	public:
		virtual ~Page() = default;

		void WindowSize(const Netcode::UInt2 & ss);
		Netcode::UInt2 WindowSize() const;

		Page(const Netcode::Memory::ObjectAllocator & controlAllocator,
			 const Netcode::Memory::ObjectAllocator & eventAllocator,
			 Netcode::Physics::PhysX & px);
		
		virtual void InitializeComponents() = 0;

		virtual void Destruct() override {
			Control::Destruct();

			dummyMaterial.Reset();

			scene.Reset();
		}

		virtual void Activate() {
			if(clickToken == 0) {
				clickToken = Netcode::Input::OnMouseKeyPressed->Subscribe([this](Netcode::Key key, Netcode::KeyModifier modifier) -> void {
					if(key.IsRising() && key.GetCode() == Netcode::KeyCode::MOUSE_LEFT) {
						const Netcode::Int2 mousePosition = Netcode::Input::GetMousePosition();

						Control * ctrl = Raycast(mousePosition);

						if(ctrl != nullptr) {
							MouseEventArgs args{ mousePosition };
							ctrl->PropagateOnClick(args);
						}
					}
				});
			}

			if(moveToken == 0) {
				moveToken = Netcode::Input::OnMouseMove->Subscribe([this](Netcode::Int2 delta, Netcode::KeyModifier modifier) -> void {
					const Netcode::Int2 mousePosition = Netcode::Input::GetMousePosition();

					Control * ctrl = Raycast(mousePosition);

					if(ctrl != nullptr) {
						MouseEventArgs args{ mousePosition };
						ctrl->PropagateOnMouseMove(args);
					}

					Log::Debug("modifier: {0}", static_cast<int32_t>(modifier));

					MouseEventArgs leaveArgs{ mousePosition };
					PropagateOnMouseLeave(leaveArgs);
				});
			}

			if(scrollToken == 0) {
				scrollToken = Netcode::Input::OnScroll->Subscribe([this](int scrollVector, Netcode::KeyModifier modifier) -> void {
					const Netcode::Int2 mousePosition = Netcode::Input::GetMousePosition();

					Control * ctrl = Raycast(mousePosition);

					if(ctrl != nullptr) {
						ScrollEventArgs scrollArgs{ mousePosition, scrollVector };
						ctrl->PropagateOnClick(scrollArgs);
					}
				});
			}
		}

		virtual void Deactivate() {
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
		}
	};

	class PageManager {
		Netcode::Memory::ObjectAllocator controlAllocator;
		Netcode::Memory::ObjectAllocator eventHandlerAllocator;
		std::vector<std::shared_ptr<Page>> pages;
		std::shared_ptr<Page> activePage;
		Netcode::UInt2 windowSize;
	public:
		PageManager() : controlAllocator{ }, eventHandlerAllocator{ }, pages{  }, activePage{ nullptr } {
			pages.reserve(16);
		}

		template<typename PageType>
		std::shared_ptr<PageType> CreatePage(Netcode::Physics::PhysX & px) {
			return controlAllocator.MakeShared<PageType>(controlAllocator, eventHandlerAllocator, px);
		}

		void AddPage(std::shared_ptr<Page> page) {
			pages.emplace_back(std::move(page));
		}

		void Update(float dt) {
			if(activePage != nullptr) {
				activePage->Update(dt);
			}
		}

		void WindowResized(const Netcode::UInt2 & newSize) {
			if(activePage != nullptr) {
				activePage->WindowSize(newSize);
			}
			windowSize = newSize;
		}

		void Deactivate() {
			if(activePage != nullptr) {
				activePage->Deactivate();
			}
		}

		void Activate(uint32_t value) {
			Netcode::UndefinedBehaviourAssertion(static_cast<uint32_t>(pages.size()) > value);
			Deactivate();
			activePage = pages[value];
			activePage->Activate();
			activePage->WindowSize(windowSize);
		}

		void Destruct() {
			for(auto & i : pages) {
				i->Destruct();
			}
			pages.clear();
		}
	};

}
