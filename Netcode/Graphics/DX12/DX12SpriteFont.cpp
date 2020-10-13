#include "DX12SpriteFont.h"
#include "DX12Common.h"
#include "DX12SpriteBatch.h"
#include <vector>
#include <Netcode/Graphics/GraphicsContexts.h>
#include <Netcode/Graphics/ResourceEnums.h>
#include <Netcode/Graphics/SpriteDesc.h>
#include <Netcode/Graphics/UploadBatch.h>
#include <Netcode/Utility.h>
#include <Netcode/IO/File.h>
#include <Netcode/IO/Path.h>
#include <Netcode/IO/BinaryReader.h>
#include <DirectXTex.h>
#include <DirectXMath.h>

static const char spriteFontMagic[] = "DXTKfont";

namespace Netcode::Graphics::DX12 {

	const wchar_t * SpriteFontImpl::ConvertUTF8(const char * text, int numChars) const
	{
		if(!utfBuffer)
		{
			utfBufferSize = 1024;
			utfBuffer = std::make_unique<wchar_t[]>(utfBufferSize);
		}

		int result = MultiByteToWideChar(CP_UTF8, 0, text, numChars, utfBuffer.get(), static_cast<int>(utfBufferSize));
		if(!result && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
		{
			// Compute required buffer size
			result = MultiByteToWideChar(CP_UTF8, 0, text, numChars, nullptr, 0);
			utfBufferSize = Utility::Align<size_t, 1024>(result);
			utfBuffer = std::make_unique<wchar_t[]>(utfBufferSize);

			// Retry conversion
			result = MultiByteToWideChar(CP_UTF8, 0, text, numChars, utfBuffer.get(), static_cast<int>(utfBufferSize));
		}

		ASSERT(result, "ERROR: MultiByteToWideChar failed with error %u.\n", GetLastError());

		return utfBuffer.get();
	}


	void SpriteFontImpl::Construct(IO::BinaryReader * reader, IResourceContext * resourceContext, IFrameContext * frameContext) {
		for(char const * magic = spriteFontMagic; *magic; magic++)
		{
			ASSERT(reader->Read<uint8_t>() == *magic, "ERROR: SpriteFont provided with an invalid .spritefont file\r\n");
		}

		// Read the glyph data.
		auto glyphCount = reader->Read<uint32_t>();

		auto glyphData = reader->ReadArray<Glyph>(glyphCount);

		glyphs.assign(glyphData.Data(), glyphData.Data() + glyphCount);

		UpdateLimits();

		// Read font properties.
		lineSpacing = reader->Read<float>();

		SetDefaultCharacter(static_cast<wchar_t>(reader->Read<uint32_t>()));

		// Read the texture data.
		auto textureWidth = reader->Read<uint32_t>();
		auto textureHeight = reader->Read<uint32_t>();
		auto textureFormat = reader->Read<DXGI_FORMAT>();
		auto textureStride = reader->Read<uint32_t>();
		auto textureRows = reader->Read<uint32_t>();
		auto textureData = reader->ReadArray<uint8_t>(size_t(textureStride) * size_t(textureRows));
		textureSize = Netcode::UInt2(textureWidth, textureHeight);

		DirectX::ScratchImage imageData;

		DX_API("Failed to initialize texture2d")
			imageData.Initialize2D(textureFormat, textureWidth, textureHeight, 1, 1);

		memcpy(imageData.GetImage(0, 0, 0)->pixels, textureData.Data(), size_t(textureStride) * size_t(textureRows));

		textureResource = resourceContext->CreateTexture2D(textureWidth, textureHeight, imageData.GetMetadata().mipLevels, textureFormat, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST, ResourceFlags::NONE);
		shaderResourceView = resourceContext->CreateShaderResourceViews(1);
		shaderResourceView->CreateSRV(0, textureResource.get());

		resourceContext->SetDebugName(textureResource, L"SpriteFont:Texture");

		Ref<UploadBatch> batch = resourceContext->CreateUploadBatch();
		batch->Upload(textureResource, imageData.GetPixels(), imageData.GetPixelsSize());
		batch->Barrier(textureResource, ResourceState::COPY_DEST, ResourceState::PIXEL_SHADER_RESOURCE);
		frameContext->SyncUpload(std::move(batch));

		imageData.Release();
	}

	void SpriteFontImpl::UpdateLimits()
	{
		float widest = 0.0f;
		float highest = 0.0f;
		float widestAlphaNum = 0.0f;
		float highestAlphaNum = 0.0f;

		for(const Glyph & g : glyphs) {
			float w = static_cast<float>(g.Subrect.right - g.Subrect.left);
			float h = static_cast<float>(g.Subrect.bottom - g.Subrect.top);
			wchar_t c = static_cast<wchar_t>(g.Character);

			if(widest < w) {
				widest = w;
				limits.widestGlyph = g;
			}

			if(highest < h) {
				highest = h;
				limits.highestGlyph = g;
			}

			if( (c >= L'a' && c <= L'z') ||
				(c >= L'A' && c <= L'Z') ||
				(c >= L'0' && c <= L'9')) {
				
				if(widestAlphaNum < w) {
					widestAlphaNum = w;
					limits.widestAlphaNumericGlyph = g;
				}

				if(highestAlphaNum < h) {
					highestAlphaNum = h;
					limits.highestAlphaNumericGlyph = g;
				}
			}
 		}
	}

	SpriteFontImpl::SpriteFontImpl(IResourceContext * resourceContext, IFrameContext * frameContext, const std::wstring & fileName) {
		IO::File spriteFontFile{ IO::Path::MediaRoot(), fileName };
		IO::BinaryReader reader{ spriteFontFile };

		Construct(&reader, resourceContext, frameContext);
	}

	Ref<Netcode::ResourceViews> SpriteFontImpl::GetResourceView() const {
		return shaderResourceView;
	}

	void SpriteFontImpl::DrawString(Ptr<Netcode::SpriteBatch> spriteBatch, const wchar_t * text, const Float2 & position, const Float4 & color) const {
		DrawString(spriteBatch, text, -1, position, color, Float2::Zero, 0.0f, 0.0f);
	}

	void SpriteFontImpl::DrawString(Ptr<Netcode::SpriteBatch> spriteBatch, const wchar_t * text, const Float2 & position, const Float4 & color, float zIndex) const {
		DrawString(spriteBatch, text, -1, position, color, Float2::Zero, 0.0f, zIndex);
	}

	void SpriteFontImpl::DrawString(Ptr<Netcode::SpriteBatch> spriteBatch, const wchar_t * text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ) const {
		DrawString(spriteBatch, text, -1, position, color, rotationOrigin, rotationZ, 0.0f);
	}

	void SpriteFontImpl::DrawString(Ptr<Netcode::SpriteBatch> spriteBatch, const wchar_t * text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ, float zIndex) const {
		DrawString(spriteBatch, text, -1, position, color, rotationOrigin, rotationZ, zIndex);
	}

	void SpriteFontImpl::DrawString(Ptr<Netcode::SpriteBatch> spriteBatch, const char * text, const Float2 & position, const Float4 & color) const {
		DrawString(spriteBatch, ConvertUTF8(text), -1, position, color, Float2::Zero, 0.0f, 0.0f);
	}

	void SpriteFontImpl::DrawString(Ptr<Netcode::SpriteBatch> spriteBatch, const char * text, const Float2 & position, const Float4 & color, float zIndex) const
	{
		DrawString(spriteBatch, ConvertUTF8(text), -1, position, color, Float2::Zero, 0.0f, zIndex);
	}

	void SpriteFontImpl::DrawString(Ptr<Netcode::SpriteBatch> spriteBatch, const char * text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ) const
	{
		DrawString(spriteBatch, ConvertUTF8(text), -1, position, color, rotationOrigin, rotationZ, 0.0f);
	}

	void SpriteFontImpl::DrawString(Ptr<Netcode::SpriteBatch> spriteBatch, const char * text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ, float zIndex) const
	{
		DrawString(spriteBatch, ConvertUTF8(text), -1, position, color, rotationOrigin, rotationZ, zIndex);
	}

	void SpriteFontImpl::DrawString(Ptr<Netcode::SpriteBatch> spriteBatch, std::wstring_view text, const Float2 & position, const Float4 & color) const
	{
		int32_t textLen = static_cast<int32_t>(text.size());

		DrawString(spriteBatch, text.data(), textLen, position, color, Float2::Zero, 0.0f, 0.0f);
	}

	void SpriteFontImpl::DrawString(Ptr<Netcode::SpriteBatch> spriteBatch, std::wstring_view text, const Float2 & position, const Float4 & color, float zIndex) const
	{
		int32_t textLen = static_cast<int32_t>(text.size());

		DrawString(spriteBatch, text.data(), textLen, position, color, Float2::Zero, 0.0f, zIndex);
	}

	void SpriteFontImpl::DrawString(Ptr<Netcode::SpriteBatch> spriteBatch, std::wstring_view text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ) const
	{
		int32_t textLen = static_cast<int32_t>(text.size());

		DrawString(spriteBatch, text.data(), textLen, position, color, rotationOrigin, rotationZ, 0.0f);
	}

	void SpriteFontImpl::DrawString(Ptr<Netcode::SpriteBatch> spriteBatch, std::wstring_view text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ, float zIndex) const
	{
		int32_t textLen = static_cast<int32_t>(text.size());

		DrawString(spriteBatch, text.data(), textLen, position, color, rotationOrigin, rotationZ, zIndex);
	}

	void SpriteFontImpl::DrawString(Ptr<Netcode::SpriteBatch> spriteBatch, std::string_view text, const Float2 & position, const Float4 & color) const
	{
		int32_t textLen = static_cast<int32_t>(text.size());

		DrawString(spriteBatch, ConvertUTF8(text.data(), textLen), textLen, position, color, Float2::Zero, 0.0f, 0.0f);
	}

	void SpriteFontImpl::DrawString(Ptr<Netcode::SpriteBatch> spriteBatch, std::string_view text, const Float2 & position, const Float4 & color, float zIndex) const
	{
		int32_t textLen = static_cast<int32_t>(text.size());

		DrawString(spriteBatch, ConvertUTF8(text.data(), textLen), textLen, position, color, Float2::Zero, 0.0f, zIndex);
	}

	void SpriteFontImpl::DrawString(Ptr<Netcode::SpriteBatch> spriteBatch, std::string_view text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ) const
	{
		int32_t textLen = static_cast<int32_t>(text.size());

		DrawString(spriteBatch, ConvertUTF8(text.data(), textLen), textLen, position, color, rotationOrigin, rotationZ, 0.0f);
	}

	void SpriteFontImpl::DrawString(Ptr<Netcode::SpriteBatch> spriteBatch, std::string_view text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ, float zIndex) const
	{
		int32_t textLen = static_cast<int32_t>(text.size());

		DrawString(spriteBatch, ConvertUTF8(text.data(), textLen), textLen, position, color, rotationOrigin, rotationZ, zIndex);
	}

	void SpriteFontImpl::DrawString(Ptr<Netcode::SpriteBatch> spriteBatch, const wchar_t * text, int length, const Float2 & position, const Float4 & color, const Float2 & origin, float rotation, float layerDepth) const
	{
		SpriteEffects effects = SpriteEffects_None;

		// Lookup table indicates which way to move along each axis per SpriteEffects enum value.
		static DirectX::XMVECTORF32 axisDirectionTable[4] =
		{
			{ { { -1, -1, 0, 0 } } },
			{ { {  1, -1, 0, 0 } } },
			{ { { -1,  1, 0, 0 } } },
			{ { {  1,  1, 0, 0 } } },
		};

		// Lookup table indicates which axes are mirrored for each SpriteEffects enum value.
		static DirectX::XMVECTORF32 axisIsMirroredTable[4] =
		{
			{ { { 0, 0, 0, 0 } } },
			{ { { 1, 0, 0, 0 } } },
			{ { { 0, 1, 0, 0 } } },
			{ { { 1, 1, 0, 0 } } },
		};


		DirectX::XMVECTOR baseOffset = DirectX::XMLoadFloat2(&origin);

		if(effects)
		{
			baseOffset = XMVectorNegativeMultiplySubtract(
				MeasureString_Impl(text).v,
				axisIsMirroredTable[effects & 3],
				baseOffset);
		}

		ForEachGlyph(text, length, [&, color](Glyph const * glyph, float x, float y, float advance)
		{
			UNREFERENCED_PARAMETER(advance);

			DirectX::XMVECTOR offset = XMVectorMultiplyAdd(DirectX::XMVectorSet(x, y + glyph->YOffset, 0, 0), axisDirectionTable[effects & 3], baseOffset);

			if(effects)
			{
				DirectX::XMVECTOR glyphRect = DirectX::XMConvertVectorIntToFloat(DirectX::XMLoadInt4(reinterpret_cast<uint32_t const *>(&glyph->Subrect)), 0);

				glyphRect = DirectX::XMVectorSubtract(DirectX::XMVectorSwizzle<2, 3, 0, 1>(glyphRect), glyphRect);

				offset = XMVectorMultiplyAdd(glyphRect, axisIsMirroredTable[effects & 3], offset);
			}

			Netcode::Float2 offsetValue = Netcode::Vector2{ offset };
			Netcode::Float2 size {
				static_cast<float>(glyph->Subrect.right - glyph->Subrect.left),
				static_cast<float>(glyph->Subrect.bottom - glyph->Subrect.top)
			};

			spriteBatch->DrawSprite(SpriteDesc{ shaderResourceView.get(), textureSize, glyph->Subrect, color }, 
				position, size, offsetValue, rotation, layerDepth);
		});
	}


	Netcode::Vector2 NC_MATH_CALLCONV SpriteFontImpl::MeasureString_Impl(const wchar_t * text, int numChars) const
	{
		Vector2 result = Float2::Zero;

		ForEachGlyph(text, numChars, [&](Glyph const * glyph, float x, float y, float advance)
		{
			UNREFERENCED_PARAMETER(advance);

			auto w = static_cast<float>(glyph->Subrect.right - glyph->Subrect.left);
			auto h = static_cast<float>(glyph->Subrect.bottom - glyph->Subrect.top) + glyph->YOffset;

			h = std::max(h, lineSpacing);

			result = result.Max(Float2{ x + w, y + h });
		});

		return result;
	}


	void SpriteFontImpl::SetDefaultCharacter(wchar_t character)
	{
		defaultGlyph = nullptr;

		if(character)
		{
			defaultGlyph = DX12::SpriteFontImpl::FindGlyph(character);
		}
	}

	const Glyph * SpriteFontImpl::FindGlyph(wchar_t character) const
	{
		if(character < 32 || character > 127) {
			if(defaultGlyph)
			{
				return defaultGlyph;
			}

			ASSERT(false, "ERROR: SpriteFont encountered a character not in the font (%u, %C), and no default glyph was provided\n", character, character);
			return nullptr;
		} else {
			return glyphs.data() + (character - 32);
		}
	}

	UInt2 SpriteFontImpl::GetSpriteSheetSize() const
	{
		return textureSize;
	}


	Vector2 NC_MATH_CALLCONV SpriteFontImpl::MeasureString_Impl(const char * text, int numChars) const
	{
		return MeasureString_Impl(ConvertUTF8(text, numChars), numChars);
	}

	Float2 SpriteFontImpl::MeasureString(const char * str) const {
		return MeasureString_Impl(str);
	}

	Float2 SpriteFontImpl::MeasureString(const wchar_t * str) const {
		return MeasureString_Impl(str);
	}

	Float2 SpriteFontImpl::MeasureString(std::string_view view) const
	{
		const int32_t strLen = static_cast<int32_t>(view.size());

		return MeasureString_Impl(ConvertUTF8(view.data(), strLen), strLen);
	}

	Float2 SpriteFontImpl::MeasureString(std::wstring_view view) const
	{
		const int32_t strLen = static_cast<int32_t>(view.size());

		return MeasureString_Impl(view.data(), strLen);
	}

	float SpriteFontImpl::GetHighestCharHeight() const
	{
		return static_cast<float>(limits.highestGlyph.Subrect.bottom - limits.highestGlyph.Subrect.top);
	}

	float SpriteFontImpl::GetWidestCharWidth() const
	{
		return static_cast<float>(limits.widestGlyph.Subrect.right - limits.widestGlyph.Subrect.left);
	}

	wchar_t SpriteFontImpl::GetHighestChar() const
	{
		return static_cast<wchar_t>(limits.highestGlyph.Character);
	}

	wchar_t SpriteFontImpl::GetWidestChar() const
	{
		return static_cast<wchar_t>(limits.widestGlyph.Character);
	}

	Float2 SpriteFontImpl::GetMaxSizedStringOf(uint32_t stringMaxLength) const
	{
		if(stringMaxLength == 0) {
			return Float2::Zero;
		}

		return Float2{
			GetWidestCharWidth() * static_cast<float>(stringMaxLength),
			GetHighestCharHeight()
		};
	}

	float SpriteFontImpl::GetWidestAlphaNumericCharWidth() const
	{
		return static_cast<float>(limits.widestAlphaNumericGlyph.Subrect.right - limits.widestAlphaNumericGlyph.Subrect.left);
	}

	float SpriteFontImpl::GetHighestAlphaNumericCharHeight() const
	{
		return static_cast<float>(limits.highestAlphaNumericGlyph.Subrect.bottom - limits.highestAlphaNumericGlyph.Subrect.top);
	}

	wchar_t SpriteFontImpl::GetWidestAlphaNumericChar() const
	{
		return static_cast<wchar_t>(limits.widestAlphaNumericGlyph.Character);
	}

	wchar_t SpriteFontImpl::GetHeighestAlphaNumericChar() const
	{
		return static_cast<wchar_t>(limits.highestAlphaNumericGlyph.Character);
	}

	Float2 SpriteFontImpl::GetMaxSizedAlphaNumericStringOf(uint32_t stringMaxLength) const
	{
		if(stringMaxLength == 0) {
			return Float2::Zero;
		}

		return Float2 {
			GetWidestAlphaNumericChar() * static_cast<float>(stringMaxLength),
			GetHighestAlphaNumericCharHeight()
		};
	}

	Rect SpriteFontImpl::MeasureDrawBounds(const wchar_t * text, const Netcode::Float2 & position) const
	{
		Rect result = { LONG_MAX, LONG_MAX, 0, 0 };

		ForEachGlyph(text, -1, [&](Glyph const * glyph, float x, float y, float advance)
		{
			auto w = static_cast<float>(glyph->Subrect.right - glyph->Subrect.left);
			auto h = static_cast<float>(glyph->Subrect.bottom - glyph->Subrect.top);

			float minX = position.x + x;
			float minY = position.y + y + glyph->YOffset;

			float maxX = std::max(minX + advance, minX + w);
			float maxY = minY + h;

			if(minX < result.left)
				result.left = long(minX);

			if(minY < result.top)
				result.top = long(minY);

			if(result.right < maxX)
				result.right = long(maxX);

			if(result.bottom < maxY)
				result.bottom = long(maxY);
		});

		if(result.left == LONG_MAX)
		{
			result.left = 0;
			result.top = 0;
		}

		return result;
	}


	Rect SpriteFontImpl::MeasureDrawBounds(const wchar_t * text, DirectX::FXMVECTOR position) const
	{
		Netcode::Float2 pos;
		DirectX::XMStoreFloat2(&pos, position);

		return MeasureDrawBounds(text, pos);
	}

	Rect SpriteFontImpl::MeasureDrawBounds(const char * text, const Netcode::Float2 & position) const
	{
		return MeasureDrawBounds(ConvertUTF8(text), position);
	}


	Rect SpriteFontImpl::MeasureDrawBounds(const char * text, DirectX::FXMVECTOR position) const
	{
		Netcode::Float2 pos;
		XMStoreFloat2(&pos, position);

		return MeasureDrawBounds(ConvertUTF8(text), pos);
	}


	// Spacing properties
	float SpriteFontImpl::GetLineSpacing() const
	{
		return lineSpacing;
	}


	void SpriteFontImpl::SetLineSpacing(float spacing)
	{
		lineSpacing = spacing;
	}


	// Font properties
	wchar_t SpriteFontImpl::GetDefaultCharacter() const
	{
		return static_cast<wchar_t>(defaultGlyph ? defaultGlyph->Character : 0);
	}

	bool SpriteFontImpl::ContainsCharacter(wchar_t character) const
	{
		return std::binary_search(glyphs.begin(), glyphs.end(), character);
	}


}
