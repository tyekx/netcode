#pragma once

#include "Animation.h"
#include "Input.h"
#include "Asset/Bone.h"
#include <algorithm>
#include "ConstantBuffer.hpp"
#include "AnimationComponent.h"


namespace Egg {

	class AnimationController {

		struct AnimationBuffer {
			DirectX::XMFLOAT3A position;
			DirectX::XMFLOAT4A rotation;
			DirectX::XMFLOAT3A scale;
		};

		struct AnimationClip {
			int animId;
			float weight;
			float timeline;
			float animSpeed;
			Animation * animationRef;
		};

		Egg::Asset::Bone * bones;
		unsigned int bonesLength;
		unsigned int animationsLength;
		Animation * animations;
		unsigned int playingAnimationCount;
		AnimationClip currentAnimations[10];
		AnimationBuffer buffer[10][128];
		int InAir;
		

		void AddAnimationIfNotExists(unsigned int animId, float dt) {
			for(unsigned int i = 0; i < playingAnimationCount; ++i) {
				if(currentAnimations[i].animId == animId) {
					currentAnimations[i].weight += currentAnimations[i].animSpeed * 5.0f * dt;
					currentAnimations[i].weight = std::min(currentAnimations[i].weight, 1.0f);
					return;
				}
			}

			currentAnimations[playingAnimationCount].animId = animId;
			currentAnimations[playingAnimationCount].animationRef = animations + animId;
			currentAnimations[playingAnimationCount].animSpeed = 1.0f;
			if(playingAnimationCount == 0) {
				currentAnimations[playingAnimationCount].weight = 1.0f;
			} else {
				currentAnimations[playingAnimationCount].weight = 5.0f * dt;
			}
			currentAnimations[playingAnimationCount].timeline = 0.0f;

			playingAnimationCount += 1;
		}

		void CleanCurrentAnimations() {
			for(unsigned int i = 0; i < playingAnimationCount; ++i) {
				if(currentAnimations[i].weight <= 0.0f) {
					currentAnimations[i] = currentAnimations[playingAnimationCount - 1];
					playingAnimationCount -= 1;
					i = 0;
				}
			}
		}

		void StopAnimation(int animId) {
			for(unsigned int i = 0; i < playingAnimationCount; ++i) {
				if(currentAnimations[i].animId == animId) {
					currentAnimations[i].animSpeed = 0.0f;
				}
			}
		}

		void ReduceAllExcept(int id, float dt) {
			for(unsigned int i = 0; i < playingAnimationCount; ++i) {
				AnimationClip * anim = currentAnimations + i;

				if(anim->animId == id) {
					continue;
				}

				anim->weight -= 5.0f * dt;
				anim->weight = std::max(anim->weight, 0.0f);
			}
		}

		bool IsFinished(int id, float dt) {
			for(unsigned int i = 0; i < playingAnimationCount; ++i) {
				AnimationClip * anim = currentAnimations + i;
				if(anim->animId == id) {
					return (anim->timeline + 60.0f * dt) >= (float)anim->animationRef->duration;
				}
			}
			return true;
		}

	public:
		AnimationController(Animation * anims , unsigned int animsLength, Egg::Asset::Bone* skeleton, unsigned int skeletonLength) {
			animations = anims;
			animationsLength = animsLength;
			bones = skeleton;
			bonesLength = skeletonLength;
			InAir = 2;
		}

		void StartJump() {
			// riflejump: 5
			InAir = 3;
		}

		void EndJump() {
			InAir = 1;
		}

		/*
		* Will write dstTransforms, size expected to match the constant buffer type's max bone count
		*/
		void Animate(ConstantBuffer<BoneDataCb> & boneData, float dt) {
			float h = Input::GetAxis("Horizontal");
			float v = Input::GetAxis("Vertical");


			/*
			4: RifleIdle
			5: RifleJump
			6: RifleRun
			7: RifleRunBackwards
			8: BackwardsLeft
			9: BackwardRight
			10: ForwardLeft
			11: ForwardRight
			12: StrafeLeft
			13: StrafeRight
			*/

			if(InAir > 0) {

				if(InAir == 3) {
					AddAnimationIfNotExists(5, dt);
					ReduceAllExcept(5, dt);
					if(IsFinished(5, dt)) {
						StopAnimation(5);
						InAir = 2;
					}
				}

				if(InAir == 2) {
					AddAnimationIfNotExists(3, dt);
					ReduceAllExcept(3, dt);
				}

				if(InAir == 1) {
					AddAnimationIfNotExists(2, 0.2f);
					ReduceAllExcept(2, dt);
					InAir = 0;

				}
			} else if(v > 0.0f && h > 0.0f) {
				// forwardleft
				AddAnimationIfNotExists(10, dt);
				ReduceAllExcept(10, dt);
			} else if(v > 0.0f && h < 0.0f) {
				// forwardright
				AddAnimationIfNotExists(11, dt);
				ReduceAllExcept(11, dt);
			} else if(v < 0.0f && h > 0.0f) {
				// backwardleft
				AddAnimationIfNotExists(8, dt);
				ReduceAllExcept(8, dt);
			} else if(v < 0.0f && h < 0.0f) {
				// backwardright
				AddAnimationIfNotExists(9, dt);
				ReduceAllExcept(9, dt);
			} else if(v > 0.0f) {
				// forward
				AddAnimationIfNotExists(6, dt);
				ReduceAllExcept(6, dt);
			} else if(v < 0.0f) {
				// backward
				AddAnimationIfNotExists(7, dt);
				ReduceAllExcept(7, dt);
			} else if(h > 0.0f) {
				// strafeleft
				AddAnimationIfNotExists(12, dt);
				ReduceAllExcept(12, dt);
			} else if(h < 0.0f) {
				// straferight
				AddAnimationIfNotExists(13, dt);
				ReduceAllExcept(13, dt);
			} else {
				// idle
				AddAnimationIfNotExists(4, dt);
				ReduceAllExcept(4, dt);
			}

			/*
			if(Input::GetAxis("Vertical") > 0.0f && Input::GetAxis("Horizontal"))
			if(Input::GetAxis("Vertical") > 0.0f) {
				ReduceAnimationWeights({ 13, 12, 7, 4 }, dt);
				AddAnimationIfNotExists(6, dt); // RifleRun
			} else if(Input::GetAxis("Vertical") < 0.0f) {
				ReduceAnimationWeights({ 13, 12, 6, 4 }, dt);
				AddAnimationIfNotExists(7, dt);
			} else if(Input::GetAxis("Horizontal") > 0.0f) {
				ReduceAnimationWeights({ 13, 7, 6, 4 }, dt);
				AddAnimationIfNotExists(12, dt);
			} else if(Input::GetAxis("Horizontal") < 0.0f) {
				ReduceAnimationWeights({ 12, 7, 6, 4 }, dt);
				AddAnimationIfNotExists(13, dt);
			} else {
				ReduceAnimationWeights({ 13, 12, 7, 6 }, dt);
				AddAnimationIfNotExists(4, dt);
			}*/

			CleanCurrentAnimations();

			DirectX::XMFLOAT4 unitW{ 0.0f, 0.0f, 0.0f, 1.0f };
			DirectX::XMVECTOR rotationSource = DirectX::XMLoadFloat4(&unitW);
			float sumWeights = 0.0f;
			int bonesLength = 0;
			

			for(unsigned int i = 0; i < playingAnimationCount; ++i) {
				currentAnimations[i].timeline += currentAnimations[i].animSpeed * dt * 60.0f;

				Animation * anim = currentAnimations[i].animationRef;
				float aT = currentAnimations[i].timeline;
				sumWeights += currentAnimations[i].weight;
				aT = fmodf(aT, (float)anim->duration);

				float timeSinceLastTick = 0.0f;
				float timeGap = 0.0f;
				unsigned int keysId = 0;
				for(unsigned int i = 1; i < anim->keysLength; ++i) {
					if(aT >= (float)(anim->times[i - 1]) && (float)(anim->times[i]) >= aT) {
						keysId = i;
						timeSinceLastTick = aT - (float)(anim->times[i - 1]);
						timeGap = (float)((anim->times[i]) - (anim->times[i - 1]));
						break;
					}
				}

				float lerpArg = timeSinceLastTick / timeGap;
				Egg::Asset::AnimationKey * keysPrev = anim->keys + ((keysId - 1) * anim->bonesLength);
				Egg::Asset::AnimationKey * keysNext = anim->keys + (keysId * anim->bonesLength);

				bonesLength = anim->bonesLength;

				for(unsigned int j = 0; j < anim->bonesLength; ++j) {
					DirectX::XMVECTOR posA = DirectX::XMLoadFloat3(&keysPrev[j].position);
					DirectX::XMVECTOR posB = DirectX::XMLoadFloat3(&keysNext[j].position);
					DirectX::XMVECTOR T = DirectX::XMVectorLerp(posA, posB, lerpArg);

					DirectX::XMVECTOR quatA = DirectX::XMLoadFloat4(&keysPrev[j].rotation);
					DirectX::XMVECTOR quatB = DirectX::XMLoadFloat4(&keysNext[j].rotation);
					DirectX::XMVECTOR R = DirectX::XMQuaternionSlerp(quatA, quatB, lerpArg);


					DirectX::XMVECTOR scaleA = DirectX::XMLoadFloat3(&keysPrev[j].scale);
					DirectX::XMVECTOR scaleB = DirectX::XMLoadFloat3(&keysNext[j].scale);
					DirectX::XMVECTOR S = DirectX::XMVectorLerp(scaleA, scaleB, lerpArg);

					DirectX::XMStoreFloat3A(&buffer[i][j].position, T);
					DirectX::XMStoreFloat4A(&buffer[i][j].rotation, R);
					DirectX::XMStoreFloat3A(&buffer[i][j].scale, S);
				}
			}

			DirectX::XMFLOAT4X4A identity;
			DirectX::XMMATRIX idM = DirectX::XMMatrixIdentity();
			DirectX::XMStoreFloat4x4A(&identity, idM);
			DirectX::XMFLOAT3A zeroValue{ 0,0,0 };
			DirectX::XMVECTOR zero = DirectX::XMLoadFloat3A(&zeroValue);

			for(int i = 0; i < bonesLength; ++i) {

				

				DirectX::XMVECTOR S = DirectX::XMLoadFloat3A(&zeroValue);
				DirectX::XMVECTOR R = DirectX::XMLoadFloat4(&unitW);
				DirectX::XMVECTOR T = DirectX::XMLoadFloat3A(&zeroValue);

				for(unsigned int j = 0; j < playingAnimationCount; ++j) {
					float lambda = currentAnimations[j].weight / sumWeights;

					DirectX::XMVECTOR loadedT = DirectX::XMLoadFloat3A(&buffer[j][i].position);
					DirectX::XMVECTOR loadedR = DirectX::XMLoadFloat4A(&buffer[j][i].rotation);
					DirectX::XMVECTOR loadedS = DirectX::XMLoadFloat3A(&buffer[j][i].scale);

					T = DirectX::XMVectorAdd(T, DirectX::XMVectorLerp(zero, loadedT, lambda));

					R = DirectX::XMQuaternionMultiply(R, DirectX::XMQuaternionSlerp(rotationSource, loadedR, lambda));

					S = DirectX::XMVectorAdd(S, DirectX::XMVectorLerp(zero, loadedS, lambda));

					

				}

				DirectX::XMStoreFloat4x4A(&(boneData->ToRootTransform[i]), DirectX::XMMatrixAffineTransformation(S, rotationSource, R, T));
			}

			for(int i = 1; i < bonesLength; ++i) {
				DirectX::XMMATRIX local = DirectX::XMLoadFloat4x4A(&(boneData->ToRootTransform[i]));
				DirectX::XMMATRIX root = DirectX::XMLoadFloat4x4A(&(boneData->ToRootTransform[bones[i].parentId]));
				DirectX::XMStoreFloat4x4A(&(boneData->ToRootTransform[i]), DirectX::XMMatrixMultiply(local, root));
			}

			for(int i = 0; i < bonesLength; ++i) {
				DirectX::XMMATRIX root = DirectX::XMLoadFloat4x4A(&(boneData->ToRootTransform[i]));
				DirectX::XMMATRIX offset = DirectX::XMLoadFloat4x4(&bones[i].transform);
				DirectX::XMStoreFloat4x4A(&(boneData->BindTransform[i]), DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(offset, root)));
			}
		}


	};

}
