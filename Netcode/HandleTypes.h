#pragma once

#include <NetcodeFoundation/Math.h>

#include "Graphics/ResourceEnums.h"
#include "Graphics/ResourceDesc.h"
#include "Common.h"

#if defined(NETCODE_OS_WINDOWS)
#include <DirectXTex.h>
#endif

#include <boost/beast/http.hpp>

#include <map>
#include <string>

/*
Note to future me:
The idea behind this pattern was to ditch handles, which could lead the system being in metastable state.
Having builders enforces an explicit Build call, or the destructor will clean up automatically, leaving a nullptr
which should be easy to debug.

The reasoning behind many interfaces approach was that these classes are just to initialize the pipeline and the
execution.

This also heavily favors the DirectX ecosystem, but thats an issue for possible future opengl ports
*/

namespace Netcode {

#if !defined(NETCODE_OS_WINDOWS)

	struct Image
	{
		size_t      width;
		size_t      height;
		DXGI_FORMAT format;
		size_t      rowPitch;
		size_t      slicePitch;
		uint8_t * pixels;
	};

	struct Rect {
		int32_t left;
		int32_t top;
		int32_t right;
		int32_t bottom;
	};

#else

	using Image = DirectX::Image;
	using Rect = RECT;

#endif

	enum class ShaderType : unsigned {
		VERTEX_SHADER = 0,
		PIXEL_SHADER = 1,
		GEOMETRY_SHADER = 2,
		HULL_SHADER = 3,
		DOMAIN_SHADER = 4,
		UNDEFINED_SHADER = 0xFFFFFFFF
	};

#undef DOMAIN
	enum class ShaderVisibility : unsigned
	{
		ALL = 0,
		VERTEX = 1,
		HULL = 2,
		DOMAIN = 3,
		GEOMETRY = 4,
		PIXEL = 5
	};

	enum class RangeType : unsigned
	{
		SRV = 0,
		UAV = (SRV + 1),
		CBV = (UAV + 1),
		SAMPLER = (CBV + 1)
	};

	enum class BlendOp : unsigned {
		ADD = 1,
		SUBTRACT = 2,
		REV_SUBTRACT = 3,
		MIN = 4,
		MAX = 5
	};

	enum class LogicOp : unsigned {
		CLEAR = 0,
		SET = (CLEAR + 1),
		COPY = (SET + 1),
		COPY_INVERTED = (COPY + 1),
		NOOP = (COPY_INVERTED + 1),
		INVERT = (NOOP + 1),
		AND = (INVERT + 1),
		NAND = (AND + 1),
		OR = (NAND + 1),
		NOR = (OR + 1),
		XOR = (NOR + 1),
		EQUIV = (XOR + 1),
		AND_REVERSE = (EQUIV + 1),
		AND_INVERTED = (AND_REVERSE + 1),
		OR_REVERSE = (AND_INVERTED + 1),
		OR_INVERTED = (OR_REVERSE + 1)
	};

	enum class BlendMode : unsigned
	{
		ZERO = 1,
		ONE = 2,
		SRC_COLOR = 3,
		INV_SRC_COLOR = 4,
		SRC_ALPHA = 5,
		INV_SRC_ALPHA = 6,
		DEST_ALPHA = 7,
		INV_DEST_ALPHA = 8,
		DEST_COLOR = 9,
		INV_DEST_COLOR = 10,
		SRC_ALPHA_SAT = 11,
		BLEND_FACTOR = 14,
		INV_BLEND_FACTOR = 15,
		SRC1_COLOR = 16,
		INV_SRC1_COLOR = 17,
		SRC1_ALPHA = 18,
		INV_SRC1_ALPHA = 19
	};

	struct RenderTargetBlendDesc
	{
		bool blendEnable;
		bool logicOpEnable;
		BlendMode srcBlend;
		BlendMode destBlend;
		BlendOp blendOp;
		BlendMode srcBlendAlpha;
		BlendMode destBlendAlpha;
		BlendOp blendOpAlpha;
		LogicOp logicOp;
		uint8_t renderTargetWriteMask;

		RenderTargetBlendDesc() : blendEnable{ false }, logicOpEnable{ false },
			srcBlend{ BlendMode::ONE }, destBlend{ BlendMode::ZERO }, blendOp{ BlendOp::ADD },
			srcBlendAlpha{ BlendMode::ONE }, destBlendAlpha{ BlendMode::ZERO }, blendOpAlpha{ BlendOp::ADD },
			logicOp{ LogicOp::NOOP }, renderTargetWriteMask{ 0x0f } { 

		}
	};

	struct BlendDesc {
		bool alphaToCoverageEnabled;
		bool independentAlphaEnabled;
		RenderTargetBlendDesc rtBlend[8];

		BlendDesc() : alphaToCoverageEnabled{ false }, independentAlphaEnabled{ false }, rtBlend{} { }
	};

	enum class CullMode : unsigned
	{
		NONE = 1,
		FRONT = 2,
		BACK = 3
	};

	enum class FillMode : unsigned {
		WIREFRAME = 2,
		SOLID = 3
	};

	struct RasterizerDesc
	{
		FillMode fillMode;
		CullMode cullMode;
		uint32_t depthBias;
		float depthBiasClamp;
		float slopeScaledDepthBias;
		bool frontCounterClockwise;
		bool depthClipEnable;
		bool multisampleEnable;
		bool antialiasedLineEnable;
		uint32_t forcedSampleCount;
		bool conservativeRaster;

		RasterizerDesc() :
			fillMode{ FillMode::SOLID },
			cullMode{ CullMode::BACK },
			depthBias{ 0 },
			depthBiasClamp{ 0.0f },
			slopeScaledDepthBias{ 0.0f },
			frontCounterClockwise{ false },
			depthClipEnable{ true },
			multisampleEnable{ false },
			antialiasedLineEnable{ false },
			forcedSampleCount{ 0 },
			conservativeRaster{ false } {
		}
	};

	enum class ComparisonFunc : unsigned
	{
		NEVER = 1,
		LESS = 2,
		EQUAL = 3,
		LESS_EQUAL = 4,
		GREATER = 5,
		NOT_EQUAL = 6,
		GREATER_EQUAL = 7,
		ALWAYS = 8
	};

	enum class StencilOp : unsigned
	{
		KEEP = 1,
		ZERO = 2,
		REPLACE = 3,
		INCR_SAT = 4,
		DECR_SAT = 5,
		INVERT = 6,
		INCR = 7,
		DECR = 8
	};

	enum class BackgroundType : unsigned {
		NONE = 0,
		SOLID = 1,
		TEXTURE = 2
	};

	enum class BorderType : unsigned {
		NONE = 0,
		SOLID = 1
	};

	struct StencilOpDesc {
		StencilOp stencilDepthFailOp;
		StencilOp stencilFailOp;
		StencilOp stencilPassOp;
		ComparisonFunc stencilFunc;

		StencilOpDesc() :
			stencilDepthFailOp{ StencilOp::KEEP },
			stencilFailOp{ StencilOp::KEEP },
			stencilPassOp{ StencilOp::KEEP },
			stencilFunc{ ComparisonFunc::ALWAYS } {

		}
	};

	struct DepthStencilDesc
	{
		ComparisonFunc depthFunc;
		StencilOpDesc frontFace;
		StencilOpDesc backFace;
		bool depthEnable;
		bool depthWriteMaskZero;
		bool stencilEnable;
		uint8_t stencilReadMask;
		uint8_t stencilWriteMask;

		DepthStencilDesc() :
			depthFunc{ ComparisonFunc::LESS },
			frontFace{},
			backFace{},
			depthEnable{ true },
			depthWriteMaskZero{ false },
			stencilEnable{ false },
			stencilReadMask{ 0xff },
			stencilWriteMask{ 0xff } {
		}
	};

	class GpuResource {
	public:
		virtual ~GpuResource() = default;
		virtual const Graphics::ResourceDesc & GetDesc() const = 0;
	};

	using GpuResourceWeakRef = std::weak_ptr<GpuResource>;
	using GpuResourceRef = std::shared_ptr<GpuResource>;

	class ShaderBytecode {
	public:
		virtual ~ShaderBytecode() = default;
		virtual uint8_t * GetBufferPointer() = 0;
		virtual size_t GetBufferSize() = 0;
		virtual const std::wstring & GetFileReference() const = 0;
	};

	using ShaderBytecodeRef = std::shared_ptr<ShaderBytecode>;
	using ShaderBytecodeWeakRef = std::weak_ptr<ShaderBytecode>;

	class ShaderBuilder {
	public:
		virtual ~ShaderBuilder() = default;

		virtual void SetShaderType(ShaderType shaderType) = 0;
		virtual void SetEntrypoint(const std::string & entryFunction) = 0;
		virtual void SetSource(const std::wstring & shaderPath) = 0;
		virtual void SetDefinitions(const std::map<std::string, std::string> & defines) = 0;

		virtual ShaderBytecodeRef LoadBytecode(const std::wstring & precompiledShaderPath) = 0;
		virtual ShaderBytecodeRef Build() = 0;
	};

	using ShaderBuilderRef = std::shared_ptr<ShaderBuilder>;
	using ShaderBuilderWeakRef = std::weak_ptr<ShaderBuilder>;

	class PipelineState {
	public:
		virtual ~PipelineState() = default;
		virtual void * GetImplDetail() const = 0;
	};

	using PipelineStateRef = std::shared_ptr<PipelineState>;
	using PipelineStateWeakRef = std::weak_ptr<PipelineState>;

	class InputLayout {
	public:
		virtual ~InputLayout() = default;
		
	};

	using InputLayoutRef = std::shared_ptr<InputLayout>;
	using InputLayoutWeakRef = std::weak_ptr<InputLayout>;

	class InputLayoutBuilder {
	public:
		virtual ~InputLayoutBuilder() = default;
		virtual void AddInputElement(const char * semanticName, DXGI_FORMAT format) = 0;
		virtual void AddInputElement(const char * semanticName, unsigned int semanticIndex, DXGI_FORMAT format) = 0;
		virtual void AddInputElement(const char * semanticName, DXGI_FORMAT format, unsigned int byteOffset) = 0;
		virtual void AddInputElement(const char * semanticName, unsigned int semanticIndex, DXGI_FORMAT format, unsigned int byteOffset) = 0;
		virtual InputLayoutRef Build() = 0;
	};

	using InputLayoutBuilderRef = std::shared_ptr<InputLayoutBuilder>;
	using InputLayoutBuilderWeakRef = std::weak_ptr<InputLayoutBuilder>;

	class StreamOutput {
	public:
		virtual ~StreamOutput() = default;
	};

	using StreamOutputRef = std::shared_ptr<StreamOutput>;
	using StreamOutputWeakRef = std::weak_ptr<StreamOutput>;

	class StreamOutputBuilder {
	public:
		virtual ~StreamOutputBuilder() = default;
		virtual void AddStride(uint32_t stride) = 0;
		virtual void SetRasterizedStream(uint32_t stream) = 0;
		virtual void AddStreamOutputEntry(const char * semanticName, uint8_t componentCount, uint8_t outputSlot, uint8_t startComponent, uint32_t stream) = 0;
		virtual void AddStreamOutputEntry(const char * semanticName, uint32_t semanticIndex, uint8_t componentCount, uint8_t outputSlot, uint8_t startComponent, uint32_t stream) = 0;
		virtual StreamOutputRef Build() = 0;
	};

	using StreamOutputBuilderRef = std::shared_ptr<StreamOutputBuilder>;
	using StreamOutputBuilderWeakRef = std::weak_ptr<StreamOutputBuilder>;

	class RootSignature {
	public:
		virtual ~RootSignature() = default;
		virtual void * GetImplDetail() const = 0;
	};

	using RootSignatureRef = std::shared_ptr<RootSignature>;
	using RootSignatureWeakRef = std::weak_ptr<RootSignature>;

	class RootSignatureBuilder {
	public:
		virtual ~RootSignatureBuilder() = default;
		virtual RootSignatureRef Build() = 0;
		virtual RootSignatureRef BuildFromShader(ShaderBytecodeRef rootSigContainingBytecode) = 0;
		virtual RootSignatureRef BuildEmpty() = 0;
	};

	using RootSignatureBuilderRef = std::shared_ptr<RootSignatureBuilder>;
	using RootSignatureBuilderWeakRef = std::weak_ptr<RootSignatureBuilder>;

	class GPipelineStateBuilder {
	public:
		virtual ~GPipelineStateBuilder() = default;
		virtual void SetRootSignature(RootSignatureRef rootSignature) = 0;
		virtual void SetDepthStencilState(const DepthStencilDesc & depthStencilState) = 0;
		virtual void SetRasterizerState(const RasterizerDesc & rasterizerState) = 0;
		virtual void SetBlendState(const BlendDesc & blendState) = 0;
		virtual void SetStreamOutput(StreamOutputRef streamOutput) = 0;
		virtual void SetInputLayout(InputLayoutRef inputLayout) = 0;
		virtual void SetVertexShader(ShaderBytecodeRef shader) = 0;
		virtual void SetPixelShader(ShaderBytecodeRef shader) = 0;
		virtual void SetGeometryShader(ShaderBytecodeRef shader) = 0;
		virtual void SetHullShader(ShaderBytecodeRef shader) = 0;
		virtual void SetDomainShader(ShaderBytecodeRef shader) = 0;
		virtual void SetNumRenderTargets(uint8_t numRenderTargets) = 0;
		virtual void SetDepthStencilFormat(DXGI_FORMAT format) = 0;
		virtual void SetRenderTargetFormat(uint8_t renderTargetIdx, DXGI_FORMAT format) = 0;
		virtual void SetRenderTargetFormats(std::initializer_list<DXGI_FORMAT> formats) = 0;
		virtual void SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType topType) = 0;
		virtual PipelineStateRef Build() = 0;
	};

	using GPipelineStateBuilderRef = std::shared_ptr<GPipelineStateBuilder>;
	using GPipelineStateBuilderWeakRef = std::weak_ptr<GPipelineStateBuilder>;

	class CPipelineStateBuilder {
	public:
		virtual ~CPipelineStateBuilder() = default;
		virtual void SetRootSignature(RootSignatureRef rootSig) = 0;
		virtual void SetComputeShader(ShaderBytecodeRef shader) = 0;
		virtual PipelineStateRef Build() = 0;
	};

	using CPipelineStateBuilderRef = std::shared_ptr<CPipelineStateBuilder>;

	class Texture {
	public:
		virtual ~Texture() = default;
		virtual Netcode::Graphics::ResourceDimension GetDimension() const = 0;
		virtual uint16_t GetMipLevelCount() const = 0;
		virtual const Image * GetImage(uint16_t mipIndex, uint16_t arrayIndex, uint32_t slice) = 0;
		virtual const Image * GetImages() = 0;
		virtual uint16_t GetImageCount() = 0;
	};

	using TextureRef = std::shared_ptr<Texture>;
	using TextureWeakRef = std::weak_ptr<Texture>;

	class TextureBuilder {
	public:
		virtual ~TextureBuilder() = default;
		virtual void LoadTexture2D(const std::wstring & mediaPath) = 0;
		virtual void LoadTexture3D(const std::wstring & mediaPath) = 0;
		virtual void LoadTextureCube(const std::wstring & mediaPath) = 0;

		virtual void LoadTexture2D(Netcode::ArrayView<uint8_t> data) = 0;
		virtual void LoadTexture3D(Netcode::ArrayView<uint8_t> data) = 0;
		virtual void LoadTextureCube(Netcode::ArrayView<uint8_t> data) = 0;

		virtual uint16_t GetCurrentMipLevelCount() = 0;
		virtual void GenerateMipLevels(uint16_t mipLevelCount) = 0;
		virtual TextureRef Build() = 0;
	};

	using TextureBuilderRef = std::shared_ptr<TextureBuilder>;

	class ResourceViews {
	public:
		virtual ~ResourceViews() = default;
		virtual void CreateSRV(uint32_t idx, GpuResourceRef resourceHandle) = 0;
		virtual void CreateSRV(uint32_t idx, GpuResourceRef resourceHandle, uint32_t firstElement, uint32_t numElements) = 0;
		virtual void CreateRTV(uint32_t idx, GpuResourceRef resourceHandle) = 0;
		virtual void CreateDSV(GpuResourceRef resourceHandle) = 0;
		virtual void CreateUAV(uint32_t idx, GpuResourceRef resourceHandle) = 0;
		virtual void CreateSampler(uint32_t idx, GpuResourceRef resourceHandle) = 0;
	};

	using ResourceViewsRef = std::shared_ptr<ResourceViews>;

	struct SpriteDesc {
		BackgroundType type;
		Netcode::ResourceViewsRef texture;
		Netcode::UInt2 textureSize;
		Netcode::Float4 color;
		Rect sourceRect;

		SpriteDesc() :
			type{ BackgroundType::NONE },
			texture{ nullptr },
			textureSize{ Netcode::UInt2::Zero },
			color{ Netcode::Float4::Zero },
			sourceRect{ 0, 0, 0, 0 } {

		}

		SpriteDesc(const Netcode::Float4 & color) :
			type{ BackgroundType::SOLID },
			texture{ nullptr },
			textureSize{ Netcode::UInt2::Zero },
			color{ color },
			sourceRect{ 0, 0, 0, 0 } {

		}

		SpriteDesc(const Netcode::ResourceViewsRef & texture, const Netcode::UInt2 & textureSize) :
			SpriteDesc(texture, textureSize, Netcode::Float4::One) { }

		SpriteDesc(const Netcode::ResourceViewsRef & texture, const Netcode::UInt2 & textureSize, const Netcode::Float4 & albedoColor) :
			SpriteDesc(texture, textureSize, Rect{ 0, 0, static_cast<int32_t>(textureSize.x), static_cast<int32_t>(textureSize.y) }, albedoColor) {

		}
		SpriteDesc(const Netcode::ResourceViewsRef & texture, const Netcode::UInt2 & textureSize, const Rect & sourceRect) :
			SpriteDesc(texture, textureSize, sourceRect, Netcode::Float4::One) {

		}

		SpriteDesc(const Netcode::ResourceViewsRef & texture, const Netcode::UInt2 & textureSize, const Rect & sourceRect, const Netcode::Float4 & albedoColor) :
			type{ BackgroundType::TEXTURE },
			texture{ texture },
			textureSize{ textureSize },
			color{ albedoColor },
			sourceRect{ sourceRect } {

		}
	};

	struct BorderDesc {
		BorderType type;
		float borderWidth;
		float borderRadius;
		Netcode::Float4 color;

		BorderDesc() : type{ BorderType::NONE }, borderWidth{ 0.0f }, borderRadius{ 0.0f }, color{ Netcode::Float4::Zero } { }
		
		BorderDesc(float width, float radius, const Netcode::Float4 & color) :
			type{ BorderType::SOLID },
			borderWidth{ width },
			borderRadius{ radius },
			color{ color } { }
	};

	class SpriteBatch {
	public:
		virtual ~SpriteBatch() = default;
		virtual void BeginRecord(void* renderContext, Float4x4 viewProj) = 0;

		virtual void SetScissorRect(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom) = 0;
		virtual void SetScissorRect(const Rect & rect) = 0;
		virtual void SetScissorRect() = 0;

		virtual void DrawSprite(const SpriteDesc & spriteDesc, const Float2 & position) = 0;
		virtual void DrawSprite(const SpriteDesc & spriteDesc, const Float2 & position, const Float2 & size) = 0;
		virtual void DrawSprite(const SpriteDesc & spriteDesc, const Float2 & position, const Float2 & size, const Float2 & rotationOrigin, float rotationZ) = 0;
		virtual void DrawSprite(const SpriteDesc & spriteDesc, const Float2 & position, const Float2 & size, const Float2 & rotationOrigin, float rotationZ, float layerDepth) = 0;

		virtual void DrawSprite(const SpriteDesc & spriteDesc, const BorderDesc & borderDesc, const Float2 & position) = 0;
		virtual void DrawSprite(const SpriteDesc & spriteDesc, const BorderDesc & borderDesc, const Float2 & position, const Float2 & size) = 0;
		virtual void DrawSprite(const SpriteDesc & spriteDesc, const BorderDesc & borderDesc, const Float2 & position, const Float2 & size, const Float2 & rotationOrigin, float rotationZ) = 0;
		virtual void DrawSprite(const SpriteDesc & spriteDesc, const BorderDesc & borderDesc, const Float2 & position, const Float2 & size, const Float2 & rotationOrigin, float rotationZ, float layerDepth) = 0;

		virtual void EndRecord() = 0;
	};

	using SpriteBatchRef = std::shared_ptr<SpriteBatch>;

	class SpriteBatchBuilder {
	public:
		virtual ~SpriteBatchBuilder() = default;

		virtual void SetPipelineState(PipelineStateRef pipelineState) = 0;
		virtual void SetRootSignature(RootSignatureRef rootSignature) = 0;
		virtual SpriteBatchRef Build() = 0;
	};

	using SpriteBatchBuilderRef = std::shared_ptr<SpriteBatchBuilder>;

	class SpriteFont {
	public:
		virtual ~SpriteFont() = default;
		virtual ResourceViewsRef GetResourceView() const = 0;
		virtual Float2 MeasureString(const char * str) const = 0;
		virtual Float2 MeasureString(const wchar_t * str) const = 0;

		virtual float GetHighestCharHeight() const = 0;
		virtual float GetWidestCharWidth() const = 0;
		virtual wchar_t GetHighestChar() const = 0;
		virtual wchar_t GetWidestChar() const = 0;
		virtual Float2 GetMaxSizedStringOf(uint32_t stringMaxLength) const = 0;

		virtual float GetWidestAlphaNumericCharWidth() const = 0;
		virtual float GetHighestAlphaNumericCharHeight() const = 0;
		virtual wchar_t GetWidestAlphaNumericChar() const = 0;
		virtual wchar_t GetHeighestAlphaNumericChar() const = 0;
		virtual Float2 GetMaxSizedAlphaNumericStringOf(uint32_t stringMaxLength) const = 0;

		virtual void DrawString(Netcode::SpriteBatchRef spriteBatch, const wchar_t * text, const Netcode::Float2 & position, const Netcode::Float4 & color) const = 0;
		virtual void DrawString(Netcode::SpriteBatchRef spriteBatch, const char * text, const Netcode::Float2 & position, const Netcode::Float4 & color) const = 0;
	};

	using SpriteFontRef = std::shared_ptr<SpriteFont>;

	class SpriteFontBuilder {
	public:
		virtual ~SpriteFontBuilder() = default;
		/* maybe later:
		virtual void PremultiplyAlpha() = 0;
		*/
		virtual void LoadFont(const std::wstring & mediaPath) = 0;
		virtual SpriteFontRef Build() = 0;
	};

	using SpriteFontBuilderRef = std::shared_ptr<SpriteFontBuilder>;

	class Fence {
	public:
		virtual ~Fence() = default;

		virtual void HostWait() = 0;
		virtual uint64_t GetValue() const = 0;
		virtual void Increment() = 0;
	};

	using FenceRef = std::shared_ptr<Fence>;

	struct User {
		int id;
		std::string hash;
		bool isBanned;
	};

	struct Server {
		int id;
		int maxPlayers;
		int activePlayers;
		std::string address;
		std::string owner;
	};

	using Response = boost::beast::http::response<boost::beast::http::string_body>;

}
