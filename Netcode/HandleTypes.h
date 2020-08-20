#pragma once

#include "HandleDecl.h"

#include <NetcodeFoundation/Math.h>

#include <map>
#include <string>

#include "URI/Shader.h"

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

#else
	
	using Image = DirectX::Image;

	struct Glyph {
		uint32_t Character;
		Rect Subrect;
		float XOffset;
		float YOffset;
		float XAdvance;
	};

	static inline bool operator<(const Glyph & left, const Glyph & right)
	{
		return left.Character < right.Character;
	}

	static inline bool operator<(wchar_t left, const Glyph & right)
	{
		return left < right.Character;
	}

	static inline bool operator<(const Glyph & left, wchar_t right)
	{
		return left.Character < right;
	}

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

	class ShaderBytecode {
	public:
		virtual ~ShaderBytecode() = default;
		virtual uint8_t * GetBufferPointer() = 0;
		virtual size_t GetBufferSize() = 0;
		virtual const std::wstring & GetFileReference() const = 0;
	};

	class ShaderBuilder {
	public:
		virtual ~ShaderBuilder() = default;

		virtual void SetShaderType(ShaderType shaderType) = 0;
		virtual void SetEntrypoint(const std::string & entryFunction) = 0;
		virtual void SetSource(const URI::Shader & resourceIdentifier) = 0;
		virtual void SetDefinitions(const std::map<std::string, std::string> & defines) = 0;

		virtual Ref<ShaderBytecode> LoadBytecode(const URI::Shader & shaderUri) = 0;
		virtual Ref<ShaderBytecode> Build() = 0;
	};

	class PipelineState {
	public:
		virtual ~PipelineState() = default;
		virtual void * GetImplDetail() const = 0;
	};

	class InputLayout {
	public:
		virtual ~InputLayout() = default;
	};

	class InputLayoutBuilder {
	public:
		virtual ~InputLayoutBuilder() = default;
		virtual void AddInputElement(const char * semanticName, DXGI_FORMAT format) = 0;
		virtual void AddInputElement(const char * semanticName, unsigned int semanticIndex, DXGI_FORMAT format) = 0;
		virtual void AddInputElement(const char * semanticName, DXGI_FORMAT format, unsigned int byteOffset) = 0;
		virtual void AddInputElement(const char * semanticName, unsigned int semanticIndex, DXGI_FORMAT format, unsigned int byteOffset) = 0;
		virtual Ref<InputLayout> Build() = 0;
	};

	class StreamOutput {
	public:
		virtual ~StreamOutput() = default;
	};

	class StreamOutputBuilder {
	public:
		virtual ~StreamOutputBuilder() = default;
		virtual void AddStride(uint32_t stride) = 0;
		virtual void SetRasterizedStream(uint32_t stream) = 0;
		virtual void AddStreamOutputEntry(const char * semanticName, uint8_t componentCount, uint8_t outputSlot, uint8_t startComponent, uint32_t stream) = 0;
		virtual void AddStreamOutputEntry(const char * semanticName, uint32_t semanticIndex, uint8_t componentCount, uint8_t outputSlot, uint8_t startComponent, uint32_t stream) = 0;
		virtual Ref<StreamOutput> Build() = 0;
	};

	class RootSignature {
	public:
		virtual ~RootSignature() = default;
		virtual void * GetImplDetail() const = 0;
	};

	class RootSignatureBuilder {
	public:
		virtual ~RootSignatureBuilder() = default;
		virtual Ref<RootSignature> Build() = 0;
		virtual Ref<RootSignature> BuildFromShader(Ref<ShaderBytecode> rootSigContainingBytecode) = 0;
		virtual Ref<RootSignature> BuildEmpty() = 0;
	};

	class GPipelineStateBuilder {
	public:
		virtual ~GPipelineStateBuilder() = default;
		virtual void SetRootSignature(Ref<RootSignature> rootSignature) = 0;
		virtual void SetDepthStencilState(const DepthStencilDesc & depthStencilState) = 0;
		virtual void SetRasterizerState(const RasterizerDesc & rasterizerState) = 0;
		virtual void SetBlendState(const BlendDesc & blendState) = 0;
		virtual void SetStreamOutput(Ref<StreamOutput> streamOutput) = 0;
		virtual void SetInputLayout(Ref<InputLayout> inputLayout) = 0;
		virtual void SetVertexShader(Ref<ShaderBytecode> shader) = 0;
		virtual void SetPixelShader(Ref<ShaderBytecode> shader) = 0;
		virtual void SetGeometryShader(Ref<ShaderBytecode> shader) = 0;
		virtual void SetHullShader(Ref<ShaderBytecode> shader) = 0;
		virtual void SetDomainShader(Ref<ShaderBytecode> shader) = 0;
		virtual void SetNumRenderTargets(uint8_t numRenderTargets) = 0;
		virtual void SetDepthStencilFormat(DXGI_FORMAT format) = 0;
		virtual void SetRenderTargetFormat(uint8_t renderTargetIdx, DXGI_FORMAT format) = 0;
		virtual void SetRenderTargetFormats(std::initializer_list<DXGI_FORMAT> formats) = 0;
		virtual void SetPrimitiveTopologyType(Graphics::PrimitiveTopologyType topType) = 0;
		virtual Ref<PipelineState> Build() = 0;
	};

	class CPipelineStateBuilder {
	public:
		virtual ~CPipelineStateBuilder() = default;
		virtual void SetRootSignature(Ref<RootSignature> rootSig) = 0;
		virtual void SetComputeShader(Ref<ShaderBytecode> shader) = 0;
		virtual Ref<PipelineState> Build() = 0;
	};

	class Texture {
	public:
		virtual ~Texture() = default;
		virtual Graphics::ResourceDimension GetDimension() const = 0;
		virtual uint16_t GetMipLevelCount() const = 0;
		virtual const Image * GetImage(uint16_t mipIndex, uint16_t arrayIndex, uint32_t slice) = 0;
		virtual const Image * GetImages() = 0;
		virtual uint16_t GetImageCount() = 0;
	};

	class TextureBuilder {
	public:
		virtual ~TextureBuilder() = default;
		virtual void LoadTexture2D(const URI::Texture & mediaPath) = 0;
		virtual void LoadTexture3D(const URI::Texture & mediaPath) = 0;
		virtual void LoadTextureCube(const URI::Texture & mediaPath) = 0;

		virtual void LoadTexture2D(ArrayView<uint8_t> data) = 0;
		virtual void LoadTexture3D(ArrayView<uint8_t> data) = 0;
		virtual void LoadTextureCube(ArrayView<uint8_t> data) = 0;

		virtual void SetStateAfterUpload(Graphics::ResourceState state) = 0;
		virtual void SetMipLevels(uint16_t mipLevels) = 0;
		virtual Ref<GpuResource> Build() = 0;
	};

	class ResourceViews {
	public:
		virtual ~ResourceViews() = default;
		virtual void CreateSRV(uint32_t idx, Ptr<GpuResource> resourceHandle) = 0;
		virtual void CreateSRV(uint32_t idx, Ptr<GpuResource> resourceHandle, uint32_t firstElement, uint32_t numElements) = 0;
		virtual void CreateRTV(uint32_t idx, Ptr<GpuResource> resourceHandle) = 0;
		virtual void CreateDSV(Ptr<GpuResource> resourceHandle) = 0;
		virtual void CreateUAV(uint32_t idx, Ptr<GpuResource> resourceHandle) = 0;
		virtual void CreateSampler(uint32_t idx, Ptr<GpuResource> resourceHandle) = 0;

		virtual void ClearSRV(uint32_t idx, Graphics::ResourceDimension expectedResourceDimension) = 0;
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

	class SpriteBatchBuilder {
	public:
		virtual ~SpriteBatchBuilder() = default;

		virtual void SetPipelineState(Ref<PipelineState> pipelineState) = 0;
		virtual void SetRootSignature(Ref<RootSignature> rootSignature) = 0;
		virtual Ref<SpriteBatch> Build() = 0;
	};

	class SpriteFont {
	public:
		virtual ~SpriteFont() = default;
		virtual Ref<ResourceViews> GetResourceView() const = 0;
		virtual Float2 MeasureString(const char * str) const = 0;
		virtual Float2 MeasureString(const wchar_t * str) const = 0;
		virtual Float2 MeasureString(std::string_view view) const = 0;
		virtual Float2 MeasureString(std::wstring_view view) const = 0;

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

		virtual const Glyph * FindGlyph(wchar_t character) const = 0;

		virtual void DrawString(Ptr<SpriteBatch> spriteBatch, const wchar_t * text, const Float2 & position, const Float4 & color) const = 0;
		virtual void DrawString(Ptr<SpriteBatch> spriteBatch, const wchar_t * text, const Float2 & position, const Float4 & color, float zIndex) const = 0;
		virtual void DrawString(Ptr<SpriteBatch> spriteBatch, const wchar_t * text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ) const = 0;
		virtual void DrawString(Ptr<SpriteBatch> spriteBatch, const wchar_t * text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ, float zIndex) const = 0;

		virtual void DrawString(Ptr<SpriteBatch> spriteBatch, const char * text, const Float2 & position, const Float4 & color) const = 0;
		virtual void DrawString(Ptr<SpriteBatch> spriteBatch, const char * text, const Float2 & position, const Float4 & color, float zIndex) const = 0;
		virtual void DrawString(Ptr<SpriteBatch> spriteBatch, const char * text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ) const = 0;
		virtual void DrawString(Ptr<SpriteBatch> spriteBatch, const char * text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ, float zIndex) const = 0;

		virtual void DrawString(Ptr<SpriteBatch> spriteBatch, std::wstring_view text, const Float2 & position, const Float4 & color) const = 0;
		virtual void DrawString(Ptr<SpriteBatch> spriteBatch, std::wstring_view text, const Float2 & position, const Float4 & color, float zIndex) const = 0;
		virtual void DrawString(Ptr<SpriteBatch> spriteBatch, std::wstring_view text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ) const = 0;
		virtual void DrawString(Ptr<SpriteBatch> spriteBatch, std::wstring_view text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ, float zIndex) const = 0;

		virtual void DrawString(Ptr<SpriteBatch> spriteBatch, std::string_view text, const Float2 & position, const Float4 & color) const = 0;
		virtual void DrawString(Ptr<SpriteBatch> spriteBatch, std::string_view text, const Float2 & position, const Float4 & color, float zIndex) const = 0;
		virtual void DrawString(Ptr<SpriteBatch> spriteBatch, std::string_view text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ) const = 0;
		virtual void DrawString(Ptr<SpriteBatch> spriteBatch, std::string_view text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ, float zIndex) const = 0;
	};

	class SpriteFontBuilder {
	public:
		virtual ~SpriteFontBuilder() = default;
		virtual void LoadFont(const std::wstring & mediaPath) = 0;
		virtual Ref<SpriteFont> Build() = 0;
	};

	class Fence {
	public:
		virtual ~Fence() = default;

		virtual void HostWait() = 0;
		virtual uint64_t GetValue() const = 0;
		virtual void Increment() = 0;
	};

}
