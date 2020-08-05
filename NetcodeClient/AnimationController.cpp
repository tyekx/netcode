#include "AnimationController.h"
#include "AnimationSet.h"

AnimationController::AnimationController(Ref<AnimationSet> animationSet) : animationSet{ std::move(animationSet) } {

}

void AnimationController::Update(const std::vector<Netcode::Animation::BlendItem> & blendPlan) {
	entryId = animationSet->Activate(blendPlan);
}
