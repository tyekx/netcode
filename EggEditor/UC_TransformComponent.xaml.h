//
// UC_TransformComponent.xaml.h
// Declaration of the UC_TransformComponent class
//

#pragma once

#include "UC_TransformComponent.g.h"
#include <Egg/TransformComponent.h>

namespace EggEditor
{
	public ref class VM_TransformComponent sealed {
		TransformComponent * tc;
	public:
		VM_TransformComponent() { }

		void SetComponent(UINT_PTR t) {
			tc = reinterpret_cast<TransformComponent *>(t);
		}

		property float PositionX {
			float get() {
				if(!tc) { return 0.0f; }
				return tc->Position.x;
			}
			void set(float v) { tc->Position.x = v; }
		}
		property float PositionY {
			float get() {
				if(!tc) { return 0.0f; } return tc->Position.y;
			}
			void set(float v) { tc->Position.y = v; }
		}
		property float PositionZ {
			float get() {
				if(!tc) { return 0.0f; }
				return  tc->Position.z;
			}
			void set(float v) { tc->Position.z = v; }
		}
		property float RotationX {
			float get() {
				if(!tc) { return 0.0f; }
				return tc->Rotation.x;
			}
			void set(float v) { tc->Rotation.x = v; }
		}
		property float RotationY {
			float get() {
				if(!tc) { return 0.0f; }
				return tc->Rotation.y;
			}
			void set(float v) { tc->Rotation.y = v; }
		}
		property float RotationZ {
			float get() {
				if(!tc) { return 0.0f; }
				return tc->Rotation.z;
			}
			void set(float v) { tc->Rotation.z = v; }
		}
		property float ScaleX {
			float get() {
				if(!tc) { return 0.0f; } return tc->Scale.x;
			}
			void set(float v) { tc->Scale.x = v; }
		}
		property float ScaleY {
			float get() {
				if(!tc) { return 0.0f; } return tc->Scale.y;
			}
			void set(float v) { tc->Scale.y = v; }
		}
		property float ScaleZ {
			float get() {
				if(!tc) { return 0.0f; } return tc->Scale.z;
			}
			void set(float v) { tc->Scale.z = v; }
		}
	};

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class UC_TransformComponent sealed
	{
		VM_TransformComponent ^ vmTc;
	public:
		property VM_TransformComponent ^ Component {
			VM_TransformComponent ^ get() {
				return this->vmTc;
			}
		}

		void SetComponent(UINT_PTR t) {
			vmTc = ref new VM_TransformComponent();
			vmTc->SetComponent(t);
		}

		UC_TransformComponent();
	};
}
