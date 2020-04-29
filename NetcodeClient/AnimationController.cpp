#include "AnimationController.h"
#include "AnimationSet.h"

AnimationController::AnimationController(std::shared_ptr<AnimationSet> animationSet) : animationSet{ std::move(animationSet) } {

}
