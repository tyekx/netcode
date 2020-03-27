#include "GameScene.h"

void GameSceneSimulationEventCallback::onConstraintBreak(physx::PxConstraintInfo * constraints, physx::PxU32 count)
{
}

void GameSceneSimulationEventCallback::onWake(physx::PxActor ** actors, physx::PxU32 count)
{
}

void GameSceneSimulationEventCallback::onSleep(physx::PxActor ** actors, physx::PxU32 count)
{
}

void GameSceneSimulationEventCallback::onContact(const physx::PxContactPairHeader & pairHeader, const physx::PxContactPair * pairs, physx::PxU32 nbPairs)
{
}

void GameSceneSimulationEventCallback::onTrigger(physx::PxTriggerPair * pairs, physx::PxU32 count)
{

}

void GameSceneSimulationEventCallback::onAdvance(const physx::PxRigidBody * const * bodyBuffer, const physx::PxTransform * poseBuffer, const physx::PxU32 count)
{
}
