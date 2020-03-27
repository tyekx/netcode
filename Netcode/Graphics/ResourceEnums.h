#pragma once

#include <NetcodeFoundation/Formats.h>
#include <cstdint>

namespace Netcode::Graphics {

	enum class ResourceState : unsigned {
		COMMON = 0,
		VERTEX_AND_CONSTANT_BUFFER = 0x1,
		INDEX_BUFFER = 0x2,
		RENDER_TARGET = 0x4,
		UNORDERED_ACCESS = 0x8,
		DEPTH_WRITE = 0x10,
		DEPTH_READ = 0x20,
		NON_PIXEL_SHADER_RESOURCE = 0x40,
		PIXEL_SHADER_RESOURCE = 0x80,
		STREAM_OUT = 0x100,
		INDIRECT_ARGUMENT = 0x200,
		COPY_DEST = 0x400,
		COPY_SOURCE = 0x800,
		RESOLVE_DEST = 0x1000,
		RESOLVE_SOURCE = 0x2000,
		RAYTRACING_ACCELERATION_STRUCTURE = 0x400000,
		SHADING_RATE_SOURCE = 0x1000000,
		ANY_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
		PRESENT = 0,
		PREDICATION = 0x200
	};

	ResourceState operator|(ResourceState lhs, ResourceState rhs);

	enum class ResourceType : unsigned {
		TRANSIENT_DEFAULT,
		TRANSIENT_UPLOAD,
		TRANSIENT_READBACK,
		PERMANENT_DEFAULT,
		PERMANENT_UPLOAD,
		PERMANENT_READBACK
	};

	enum class ResourceFlags : unsigned {
		NONE = 0,
		ALLOW_RENDER_TARGET = 1,
		ALLOW_DEPTH_STENCIL = 2,
		ALLOW_UNORDERED_ACCESS = 4,
		DENY_SHADER_RESOURCE = 8
	};

	enum class ResourceDimension : unsigned {
		UNKNOWN = 0,
		BUFFER = 1,
		TEXTURE1D = 2,
		TEXTURE2D = 3,
		TEXTURE3D = 4
	};

	enum class PrimitiveTopologyType : unsigned
	{
		UNDEFINED = 0,
		POINT = 1,
		LINE = 2,
		TRIANGLE = 3,
		PATCH = 4
	};

	enum class PrimitiveTopology : unsigned {
		UNDEFINED = 0,
		POINTLIST = 1,
		LINELIST = 2,
		LINESTRIP = 3,
		TRIANGLELIST = 4,
		TRIANGLESTRIP = 5,
		LINELIST_ADJ = 10,
		LINESTRIP_ADJ = 11,
		TRIANGLELIST_ADJ = 12,
		TRIANGLESTRIP_ADJ = 13,
		CONTROL_POINT_PATCHLIST_1 = 33,
		CONTROL_POINT_PATCHLIST_2 = 34,
		CONTROL_POINT_PATCHLIST_3 = 35,
		CONTROL_POINT_PATCHLIST_4 = 36,
		CONTROL_POINT_PATCHLIST_5 = 37,
		CONTROL_POINT_PATCHLIST_6 = 38,
		CONTROL_POINT_PATCHLIST_7 = 39,
		CONTROL_POINT_PATCHLIST_8 = 40,
		CONTROL_POINT_PATCHLIST_9 = 41,
		CONTROL_POINT_PATCHLIST_10 = 42,
		CONTROL_POINT_PATCHLIST_11 = 43,
		CONTROL_POINT_PATCHLIST_12 = 44,
		CONTROL_POINT_PATCHLIST_13 = 45,
		CONTROL_POINT_PATCHLIST_14 = 46,
		CONTROL_POINT_PATCHLIST_15 = 47,
		CONTROL_POINT_PATCHLIST_16 = 48,
		CONTROL_POINT_PATCHLIST_17 = 49,
		CONTROL_POINT_PATCHLIST_18 = 50,
		CONTROL_POINT_PATCHLIST_19 = 51,
		CONTROL_POINT_PATCHLIST_20 = 52,
		CONTROL_POINT_PATCHLIST_21 = 53,
		CONTROL_POINT_PATCHLIST_22 = 54,
		CONTROL_POINT_PATCHLIST_23 = 55,
		CONTROL_POINT_PATCHLIST_24 = 56,
		CONTROL_POINT_PATCHLIST_25 = 57,
		CONTROL_POINT_PATCHLIST_26 = 58,
		CONTROL_POINT_PATCHLIST_27 = 59,
		CONTROL_POINT_PATCHLIST_28 = 60,
		CONTROL_POINT_PATCHLIST_29 = 61,
		CONTROL_POINT_PATCHLIST_30 = 62,
		CONTROL_POINT_PATCHLIST_31 = 63,
		CONTROL_POINT_PATCHLIST_32 = 64,
	};
}
