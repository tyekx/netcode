#pragma once

#include <Netcode/Functions.h>
#include <Netcode/MovementController.h>
#include <NetcodeAssetLib/Model.h>

#include "GameObject.h"

#define CLIP_ARGS(id) id, model->animations[ id ].duration, model->animations[ id ].ticksPerSecond

void CreateYbotAnimationComponent(Netcode::Asset::Model * model, Animation * anim) {
	
	namespace func = Netcode::Function;
	using namespace Netcode::Animation;

	using CtrlType = Netcode::MovementController;

	// 0, 44-51

	BoneMask legsMask;
	BoneMask bodyMask;

	legsMask.set(0);
	for(int i = 44; i <= 51; ++i) {
		legsMask.set(i);
	}
	bodyMask = ~legsMask;

	anim->bones = model->bones;
	anim->clips = model->animations;
	anim->blender = std::make_shared<Netcode::Animation::Blender>();


	Netcode::Animation::IKEffector effector;
	effector.position = DirectX::XMFLOAT4{ 0.0f, 150.0f, -20.0f, 1.0f };
	effector.offset = DirectX::XMFLOAT4{ 0.0f, 0.0f, 40.0f, 0.0f };
	effector.parentId = 5;
	effector.chainLength = 5;

	anim->effectors = {
		effector
	};
	
	anim->blackboard = std::shared_ptr<Blackboard<CtrlType>>(new Blackboard<CtrlType>(
		{ // Group List
			{ // Legs Group
				{ // StateList
					State<CtrlType>("Idle", legsMask, StateBehaviour::LOOP, CLIP_ARGS(5)),
					State<CtrlType>("Forward", legsMask, StateBehaviour::LOOP, CLIP_ARGS(6)),
					State<CtrlType>("Backward", legsMask, StateBehaviour::LOOP, CLIP_ARGS(7)),
					State<CtrlType>("Left", legsMask, StateBehaviour::LOOP, CLIP_ARGS(12)),
					State<CtrlType>("Right", legsMask, StateBehaviour::LOOP, CLIP_ARGS(13)),
					State<CtrlType>("ForwardLeft", legsMask, StateBehaviour::LOOP, CLIP_ARGS(10)),
					State<CtrlType>("ForwardRight", legsMask, StateBehaviour::LOOP, CLIP_ARGS(11)),
					State<CtrlType>("BackwardLeft", legsMask, StateBehaviour::LOOP, CLIP_ARGS(8)),
					State<CtrlType>("BackwardRight", legsMask, StateBehaviour::LOOP, CLIP_ARGS(9)),
					State<CtrlType>("JumpStart", legsMask, StateBehaviour::ONCE, CLIP_ARGS(4)),
					State<CtrlType>("JumpLoop", legsMask, StateBehaviour::LOOP, CLIP_ARGS(3)),
					State<CtrlType>("JumpLand", legsMask, StateBehaviour::ONCE, CLIP_ARGS(2))
				},
				{
					TransitionInit<CtrlType>("Forward", "Idle", &CtrlType::IsIdle, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Forward", "Backward", &CtrlType::IsMovingBackward, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Forward", "Left", &CtrlType::IsMovingLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Forward", "Right", &CtrlType::IsMovingRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Forward", "ForwardLeft", &CtrlType::IsMovingForwardLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Forward", "ForwardRight", &CtrlType::IsMovingForwardRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Forward", "BackwardLeft", &CtrlType::IsMovingBackwardLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Forward", "BackwardRight", &CtrlType::IsMovingBackwardRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Forward", "JumpStart", &CtrlType::IsJumping, nullptr, &func::EaseIn, &func::ConstantZero, 0.2f),

					TransitionInit<CtrlType>("Backward", "Idle", &CtrlType::IsIdle, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Backward", "Forward", &CtrlType::IsMovingForward, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Backward", "Left", &CtrlType::IsMovingLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Backward", "Right", &CtrlType::IsMovingRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Backward", "ForwardLeft", &CtrlType::IsMovingForwardLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Backward", "ForwardRight", &CtrlType::IsMovingForwardRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Backward", "BackwardLeft", &CtrlType::IsMovingBackwardLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Backward", "BackwardRight",&CtrlType::IsMovingBackwardRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Backward", "JumpStart", &CtrlType::IsJumping, nullptr, &func::EaseIn, &func::ConstantZero, 0.2f),

					TransitionInit<CtrlType>("Left", "Idle", &CtrlType::IsIdle, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Left", "Forward", &CtrlType::IsMovingForward, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Left", "Backward", &CtrlType::IsMovingBackward, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Left", "Right", &CtrlType::IsMovingRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Left", "ForwardLeft", &CtrlType::IsMovingForwardLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Left", "ForwardRight", &CtrlType::IsMovingForwardRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Left", "BackwardLeft", &CtrlType::IsMovingBackwardLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Left", "BackwardRight", &CtrlType::IsMovingBackwardRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Left", "JumpStart", &CtrlType::IsJumping, nullptr, &func::EaseIn, &func::ConstantZero, 0.2f),

					TransitionInit<CtrlType>("Right", "Idle", &CtrlType::IsIdle, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Right", "Forward", &CtrlType::IsMovingForward, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Right", "Backward", &CtrlType::IsMovingBackward, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Right", "Left", &CtrlType::IsMovingLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Right", "ForwardLeft", &CtrlType::IsMovingForwardLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Right", "ForwardRight", &CtrlType::IsMovingForwardRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Right", "BackwardLeft", &CtrlType::IsMovingBackwardLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Right", "BackwardRight", &CtrlType::IsMovingBackwardRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Right", "JumpStart", &CtrlType::IsJumping, nullptr, &func::EaseIn, &func::ConstantZero, 0.2f),

					TransitionInit<CtrlType>("ForwardLeft", "Idle", &CtrlType::IsIdle, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("ForwardLeft", "Forward", &CtrlType::IsMovingForward, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("ForwardLeft", "Backward", &CtrlType::IsMovingBackward, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("ForwardLeft", "Left", &CtrlType::IsMovingLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("ForwardLeft", "Right", &CtrlType::IsMovingRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("ForwardLeft", "ForwardRight", &CtrlType::IsMovingForwardRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("ForwardLeft", "BackwardLeft", &CtrlType::IsMovingBackwardLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("ForwardLeft", "BackwardRight", &CtrlType::IsMovingBackwardRight,nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("ForwardLeft", "JumpStart", &CtrlType::IsJumping, nullptr, &func::EaseIn, &func::ConstantZero, 0.2f),

					TransitionInit<CtrlType>("ForwardRight", "Idle", &CtrlType::IsIdle, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("ForwardRight", "Forward", &CtrlType::IsMovingForward, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("ForwardRight", "Backward", &CtrlType::IsMovingBackward, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("ForwardRight", "Left", &CtrlType::IsMovingLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("ForwardRight", "Right", &CtrlType::IsMovingRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("ForwardRight", "ForwardLeft", &CtrlType::IsMovingForwardLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("ForwardRight", "BackwardLeft", &CtrlType::IsMovingBackwardLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("ForwardRight", "BackwardRight",&CtrlType::IsMovingBackwardRight,nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("ForwardRight", "JumpStart", &CtrlType::IsJumping, nullptr, &func::EaseIn, &func::ConstantZero, 0.2f),

					TransitionInit<CtrlType>("BackwardLeft", "Idle", &CtrlType::IsIdle, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("BackwardLeft", "Forward", &CtrlType::IsMovingForward, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("BackwardLeft", "Backward", &CtrlType::IsMovingBackward, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("BackwardLeft", "Left", &CtrlType::IsMovingLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("BackwardLeft", "Right", &CtrlType::IsMovingRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("BackwardLeft", "ForwardLeft", &CtrlType::IsMovingForwardLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("BackwardLeft", "ForwardRight", &CtrlType::IsMovingForwardRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("BackwardLeft", "BackwardRight",&CtrlType::IsMovingBackwardRight,nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("BackwardLeft", "JumpStart", &CtrlType::IsJumping, nullptr, &func::EaseIn, &func::ConstantZero, 0.2f),

					TransitionInit<CtrlType>("BackwardRight", "Idle", &CtrlType::IsIdle, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("BackwardRight", "Forward", &CtrlType::IsMovingForward, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("BackwardRight", "Backward", &CtrlType::IsMovingBackward, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("BackwardRight", "Left", &CtrlType::IsMovingLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("BackwardRight", "Right", &CtrlType::IsMovingRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("BackwardRight", "ForwardLeft", &CtrlType::IsMovingForwardLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("BackwardRight", "ForwardRight",&CtrlType::IsMovingForwardRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("BackwardRight", "BackwardLeft",&CtrlType::IsMovingBackwardLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("BackwardRight", "JumpStart", &CtrlType::IsJumping, nullptr, &func::EaseIn, &func::ConstantZero, 0.2f),

					TransitionInit<CtrlType>("Idle", "Forward", &CtrlType::IsMovingForward, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Idle", "Backward", &CtrlType::IsMovingBackward, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Idle", "Left", &CtrlType::IsMovingLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Idle", "Right", &CtrlType::IsMovingRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Idle", "ForwardLeft", &CtrlType::IsMovingForwardLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Idle", "ForwardRight", &CtrlType::IsMovingForwardRight, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Idle", "BackwardLeft", &CtrlType::IsMovingBackwardLeft, nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Idle", "BackwardRight", &CtrlType::IsMovingBackwardRight,nullptr, &func::EaseIn, &func::ConstantOne, 0.2f),
					TransitionInit<CtrlType>("Idle", "JumpStart", &CtrlType::IsJumping, nullptr, &func::EaseIn, &func::ConstantZero, 0.2f),

					TransitionInit<CtrlType>("JumpStart", "JumpLoop", &CtrlType::IsJumping, &StateBase::IsComplete, &func::EaseIn, &func::ConstantZero, 0.2f),
					TransitionInit<CtrlType>("JumpStart", "JumpLand", &CtrlType::IsOnGround, nullptr, &func::EaseIn, &func::ConstantZero, 0.2f),
					TransitionInit<CtrlType>("JumpLoop", "JumpLand", &CtrlType::IsOnGround, nullptr, &func::LerpIn, &func::ConstantZero, 0.1f),
					TransitionInit<CtrlType>("JumpLand", "Idle", nullptr, &StateBase::IsComplete, &func::LerpIn, &func::ConstantOne, 0.1f),
				}
			}/*,
			{ // Upper body group
				{
					State<CtrlType>("Idle", bodyMask, StateBehaviour::LOOP, CLIP_ARGS(5))
				},
				{

				}
			}*/
		}));
}