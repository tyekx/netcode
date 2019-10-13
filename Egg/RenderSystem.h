#pragma once

#include "Common.h"
#include "GameObject.h"

namespace Egg {
	
	class RenderSystem {
		ID3D12GraphicsCommandList * cl;
	public:
		constexpr static SignatureType Required() {
			return (0x1ULL << TupleIndexOf<ModelComponent, COMPONENTS_T>::value);
		}

		constexpr static SignatureType Incompatible() {
			return (0x0ULL);
		}

		void BindCommandList(ID3D12GraphicsCommandList * gcl) {
			cl = gcl;
		}

		void Run(GameObject * gameObject) {
			ModelComponent * mc = gameObject->GetComponent<ModelComponent>();
			
			
		}
	};

}
