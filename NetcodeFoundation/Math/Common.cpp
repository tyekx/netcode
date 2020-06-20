#include "Common.h"

namespace Netcode {

	Float2 Float2::Zero{ 0.0f, 0.0f };
	Float2 Float2::One{ 1.0f, 1.0f };
	Float2 Float2::UnitX{ 1.0f, 0.0f };
	Float2 Float2::UnitY{ 0.0f, 1.0f };
	Float2 Float2::Eps{ C_Eps, C_Eps };

	Float3 Float3::Zero{ 0.0f, 0.0f, 0.0f };
	Float3 Float3::One{ 1.0f, 1.0f, 1.0f };
	Float3 Float3::UnitX{ 1.0f, 0.0f, 0.0f };
	Float3 Float3::UnitY{ 0.0f, 1.0f, 0.0f };
	Float3 Float3::UnitZ{ 0.0f, 0.0f, 1.0f };
	Float3 Float3::Eps{ C_Eps, C_Eps, C_Eps };

	Float4 Float4::Zero{ 0.0f, 0.0f, 0.0f, 0.0f };
	Float4 Float4::One{ 1.0f, 1.0f, 1.0f, 1.0f };
	Float4 Float4::UnitX{ 1.0f, 0.0f, 0.0f, 0.0f };
	Float4 Float4::UnitY{ 0.0f, 1.0f, 0.0f, 0.0f };
	Float4 Float4::UnitZ{ 0.0f, 0.0f, 1.0f, 0.0f };
	Float4 Float4::UnitW{ 0.0f, 0.0f, 0.0f, 1.0f };
	Float4 Float4::Eps{ C_Eps, C_Eps, C_Eps, C_Eps };

	Float4x4 Float4x4::Zero {
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f };
	Float4x4 Float4x4::Identity{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f };

	Int2 Int2::Zero{ 0, 0 };
	Int2 Int2::One{ 1, 1 };
	Int2 Int2::UnitX{ 1, 0 };
	Int2 Int2::UnitY{ 0, 1 };

	Int3 Int3::Zero{ 0, 0, 0 };
	Int3 Int3::One{ 1, 1, 1 };
	Int3 Int3::UnitX{ 1, 0, 0 };
	Int3 Int3::UnitY{ 0, 1, 0 };
	Int3 Int3::UnitZ{ 0, 0, 1 };

	Int4 Int4::Zero{ 0, 0, 0 ,0 };
	Int4 Int4::One{ 1, 1, 1, 1 };
	Int4 Int4::UnitX{ 1, 0, 0 ,0 };
	Int4 Int4::UnitY{ 0, 1, 0 ,0 };
	Int4 Int4::UnitZ{ 0, 0, 1 ,0 };
	Int4 Int4::UnitW{ 0, 0, 0 ,1 };

	UInt2 UInt2::Zero{ 0, 0 };
	UInt2 UInt2::One{ 1, 1 };
	UInt2 UInt2::UnitX{ 1, 0 };
	UInt2 UInt2::UnitY{ 0, 1 };

	UInt3 UInt3::Zero{ 0, 0, 0 };
	UInt3 UInt3::One{ 1, 1, 1 };
	UInt3 UInt3::UnitX{ 1, 0, 0 };
	UInt3 UInt3::UnitY{ 0, 1, 0 };
	UInt3 UInt3::UnitZ{ 0, 0, 1 };

	UInt4 UInt4::Zero{ 0, 0, 0 ,0 };
	UInt4 UInt4::One{ 1, 1, 1, 1 };
	UInt4 UInt4::UnitX{ 1, 0, 0 ,0 };
	UInt4 UInt4::UnitY{ 0, 1, 0 ,0 };
	UInt4 UInt4::UnitZ{ 0, 0, 1 ,0 };
	UInt4 UInt4::UnitW{ 0, 0, 0 ,1 };

}
