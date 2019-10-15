#pragma once



struct TransformComponent {
	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT4 Rotation;
	DirectX::XMFLOAT3 Scale;



	TransformComponent() : Position{ 0.0f, 0.0f, 0.0f, 1.0f }, Rotation{ 0.0f, 0.0f, 0.0f, 1.0f }, Scale{ 1.0f, 1.0f, 1.0f } {

	}


};
