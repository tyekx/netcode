#include "AnimationController.h"
#include "AnimationSet.h"

AnimationController::AnimationController(std::shared_ptr<AnimationSet> animationSet) : animationSet{ std::move(animationSet) } {

}

void AnimationController::Animate(const std::vector<Netcode::Animation::BlendItem> & blendPlan) {
	entryId = animationSet->Activate(blendPlan);
}
