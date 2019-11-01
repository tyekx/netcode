#include "DebugPhysxActor.h"
#include "EggMath.h"

namespace Egg {

	void DebugPhysxActor::AfterPhysxUpdate() {
		physx::PxTransform transform = actorReference->getGlobalPose();
		DirectX::XMFLOAT4A quatSource{ transform.q.x, transform.q.y, transform.q.z, transform.q.w };

		DirectX::XMMATRIX translationMat = DirectX::XMMatrixTranslation(transform.p.x, transform.p.y, transform.p.z);
		DirectX::XMVECTOR quat = DirectX::XMLoadFloat4(&quatSource);
		DirectX::XMMATRIX rotationMat = DirectX::XMMatrixRotationQuaternion(quat);
		DirectX::XMMATRIX model = DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(rotationMat, translationMat));
		DirectX::XMStoreFloat4x4A(&perObject->Model, model);

		
		int shapeCount = (int)actorReference->getNbShapes();

		ASSERT(shapeCount < 16, "Too many shapes on a single actor");

		actorReference->getShapes(shapes.data(), 16);

		for(int i = 0; i < shapeCount; ++i) {
			const auto& pxTr = shapes[i]->getLocalPose();
			DirectX::XMFLOAT3A trValue = ToFloat3A(pxTr.p);
			DirectX::XMFLOAT4A rtValue = ToFloat4A(pxTr.q);
			DirectX::XMVECTOR rot = DirectX::XMLoadFloat4A(&rtValue);

			DirectX::XMMATRIX trMat = DirectX::XMMatrixTranslation(trValue.x, trValue.y, trValue.z);
			DirectX::XMMATRIX rtMat = DirectX::XMMatrixRotationQuaternion(rot);

			DirectX::XMMATRIX local = DirectX::XMMatrixMultiply(rtMat, trMat);
			DirectX::XMStoreFloat4x4A(&perShapeCb[i].local, DirectX::XMMatrixTranspose(local));
		}

		perShapeCb.Upload();
		perObject.Upload();
	}

	void DebugPhysxActor::Draw(ID3D12GraphicsCommandList * cl) {
	//	debugMaterial->BindConstantBuffer(cl, perObject);

		unsigned int geomId = 0;
		for(auto i : geometry) {
			debugMaterial->BindConstantBuffer(cl, DebugPhysxShapeCb::id, perShapeCb.AddressAt(geomId));
			++geomId;
			i.Draw(cl);
		}
	}

	DebugPhysxActor::DebugPhysxActor(ID3D12Device * device, Egg::Material * mat, physx::PxRigidActor * actorRef) : actorReference{ actorRef }, debugMaterial{ mat } {

		perObject.CreateResources(device);
		perShapeCb.CreateResources(device);

		physx::PxU32 numShapes = actorRef->getNbShapes();

		actorRef->getShapes(shapes.data(), numShapes);
		/*
		for(unsigned int i = 0; i < numShapes; ++i) {
			physx::PxGeometryType::Enum type = shapes[i]->getGeometryType();

			Egg::Mesh::Geometry::P geom;

			physx::PxTransform transform = shapes[i]->getLocalPose();

			DirectX::XMMATRIX translation = DirectX::XMMatrixTranslationFromVector(LoadPxVector3(transform.p));
			DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationQuaternion(LoadPxQuat(transform.q));

			DirectX::XMStoreFloat4x4A(&perShapeCb[i].local, DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(rotation, translation)));
			DirectX::XMStoreFloat4x4A(&perShapeCb[i].offset, DirectX::XMMatrixIdentity());

			if(type == physx::PxGeometryType::eCAPSULE) {
				physx::PxCapsuleGeometry capsule;
				shapes[i]->getCapsuleGeometry(capsule);
				geom = BasicGeometry::CreateCapsuleWireframe(device, capsule.halfHeight * 2.0f, capsule.radius, DirectX::XMFLOAT3{ 1.0f, 0.0f, 0.0f });
			}

			if(type == physx::PxGeometryType::ePLANE) {
				physx::PxPlaneGeometry plane;
				shapes[i]->getPlaneGeometry(plane);
				geom = BasicGeometry::CreatePlaneWireframe(device, 50, DirectX::XMFLOAT3{ 0.5f, 0.5f, 0.5f });
			}

			if(type == physx::PxGeometryType::eBOX) {
				physx::PxBoxGeometry box;
				shapes[i]->getBoxGeometry(box);
				geom = BasicGeometry::CreateBoxWireframe(device, ToFloat3(box.halfExtents), DirectX::XMFLOAT3{ 0.6f, 0.8f, 0.2f });
			}

			geometry.push_back(geom);
		}
		*/
		perShapeCb.Upload();
	}

}



