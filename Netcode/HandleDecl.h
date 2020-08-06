#pragma once

#include <NetcodeFoundation/Formats.h>
#include <cstdint>

namespace std {

	template<typename T>
	class allocator;

	template<typename T>
	struct default_delete;

	template<typename T>
	class shared_ptr;

	template<typename T>
	class weak_ptr;

	template<typename T, typename D>
	class unique_ptr;

	template<typename T>
	class initializer_list;

	template<typename ... T>
	class tuple;

	template<typename T>
	class function;

	template<typename T>
	class future;

	template<typename T, typename A>
	class vector;

}

namespace DirectX {

	struct Image;

}

template<typename T>
using Ptr = T *;

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using Weak = std::weak_ptr<T>;

template<typename T, typename D = std::default_delete<T>>
using Unique = std::unique_ptr<T, D>;

enum DXGI_FORMAT;

namespace Netcode {

	template<typename T>
	using Vector = std::vector<T, std::allocator<T>>;

	struct Rect {
		int32_t left;
		int32_t top;
		int32_t right;
		int32_t bottom;
	};

	template<typename T>
	class ArrayView;

	using Image = DirectX::Image;

	enum class ShaderType : uint32_t;
	enum class ShaderVisibility : uint32_t;
	enum class RangeType : uint32_t;
	enum class BlendOp : uint32_t;
	enum class LogicOp : uint32_t;
	enum class BlendMode : uint32_t;
	enum class CullMode : uint32_t;
	enum class FillMode : uint32_t;
	enum class ComparisonFunc : uint32_t;
	enum class StencilOp : uint32_t;
	enum class BackgroundType : uint32_t;
	enum class BorderType : uint32_t;

	struct Glyph;
	struct RenderTargetBlendDesc;
	struct BlendDesc;
	struct RasterizerDesc;
	struct StencilOpDesc;
	struct DepthStencilDesc;
	struct SpriteDesc;
	struct BorderDesc;

	class GpuResource;
	class ShaderBytecode;
	class ShaderBuilder;
	class PipelineState;
	class InputLayout;
	class InputLayoutBuilder;
	class StreamOutput;
	class StreamOutputBuilder;
	class RootSignature;
	class RootSignatureBuilder;
	class GPipelineStateBuilder;
	class CPipelineStateBuilder;
	class Texture;
	class TextureBuilder;
	class ResourceViews;
	class SpriteBatch;
	class SpriteBatchBuilder;
	class SpriteFont;
	class SpriteFontBuilder;
	class FrameGraphBuilder;
	class Fence;

	enum class KeyModifier : uint32_t;
	enum class KeyState : uint32_t;
	enum class KeyCode : uint32_t;
	class Key;

}

namespace Netcode::Network {

	class Cookie;
	class Response;
	class GameSession;

}

namespace Netcode::URI {

	class Texture;
	class Shader;
	class Model;

}

namespace Netcode::Graphics {

	enum class FrameGraphCullMode : uint32_t;
	enum class ResourceState : uint32_t;
	enum class ResourceType : uint32_t;
	enum class ResourceFlags : uint32_t;
	enum class ResourceDimension : uint32_t;
	enum class PrimitiveTopologyType : uint32_t;
	enum class PrimitiveTopology : uint32_t;

	class UploadBatch;

	struct ClearValue;
	struct ResourceDesc;

	class IDebugContext;
	class IResourceContext;
	class IFrameContext;

}
