#include <algorithm>
#include <vector>
#include "DX12SpriteFont.h"

static const char spriteFontMagic[] = "DXTKfont";

namespace Netcode::Graphics::DX12 {
	const DirectX::XMFLOAT2 SpriteFont::Float2Zero{ 0.0f, 0.0f };

	const wchar_t * SpriteFont::ConvertUTF8(const char * text) const
	{
		if(!utfBuffer)
		{
			utfBufferSize = 1024;
			utfBuffer = std::make_unique<wchar_t[]>(utfBufferSize);
		}

		int result = MultiByteToWideChar(CP_UTF8, 0, text, -1, utfBuffer.get(), static_cast<int>(utfBufferSize));
		if(!result && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
		{
			// Compute required buffer size
			result = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
			utfBufferSize = AlignUp(static_cast<size_t>(result), 1024);
			utfBuffer = std::make_unique<wchar_t[]>(utfBufferSize);

			// Retry conversion
			result = MultiByteToWideChar(CP_UTF8, 0, text, -1, utfBuffer.get(), static_cast<int>(utfBufferSize));
		}

		ASSERT(result, "ERROR: MultiByteToWideChar failed with error %u.\n", GetLastError());

		return utfBuffer.get();
	}


	void SpriteFont::Construct(BinaryReader * reader, IResourceContext * resourceContext, IFrameContext * frameContext) {
		for(char const * magic = spriteFontMagic; *magic; magic++)
		{
			ASSERT(reader->Read<uint8_t>() == *magic, "ERROR: SpriteFont provided with an invalid .spritefont file\r\n");
		}

		// Read the glyph data.
		auto glyphCount = reader->Read<uint32_t>();

		auto glyphData = reader->ReadArray<Glyph>(glyphCount);

		glyphs.assign(glyphData, glyphData + glyphCount);

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
		textureSize = DirectX::XMUINT2(textureWidth, textureHeight);

		DX_API("Failed to initialize texture2d")
			imageData.Initialize2D(textureFormat, textureWidth, textureHeight, 1, 1);

		memcpy(imageData.GetImage(0, 0, 0)->pixels, textureData, imageData.GetImage(0, 0, 0)->slicePitch);

		eggTexture = resourceContext->CreateTexture2D(textureWidth, textureHeight, textureFormat, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST, ResourceFlags::NONE);
		shaderResourceView = std::dynamic_pointer_cast<DX12ResourceViews>(resourceContext->CreateShaderResourceViews(1));
		shaderResourceView->CreateSRV(0, eggTexture);

		resourceContext->SetDebugName(eggTexture, L"SpriteFont:Texture");

		UploadBatch batch;
		batch.Upload(eggTexture, imageData.GetPixels(), imageData.GetPixelsSize());
		batch.ResourceBarrier(eggTexture, ResourceState::COPY_DEST, ResourceState::PIXEL_SHADER_RESOURCE);
		frameContext->SyncUpload(batch);

		imageData.Release();
	}

	SpriteFont::SpriteFont(IResourceContext * resourceContext, IFrameContext * frameContext, wchar_t const * fileName) {
		BinaryReader reader(fileName);

		Construct(&reader, resourceContext, frameContext);
	}

	ResourceViewsRef SpriteFont::GetResourceView() const {
		return shaderResourceView;
	}

	void SpriteFont::DrawString(Netcode::SpriteBatchRef spriteBatch, const wchar_t * text, const DirectX::XMFLOAT2 & position, const DirectX::XMFLOAT4 & color) const {
		DrawString(spriteBatch, text, position, color, 0, DirectX::XMFLOAT2{ 0,0 }, 1.0f, 0);
	}

	void SpriteFont::DrawString(Netcode::SpriteBatchRef spriteBatch, const char * text, const DirectX::XMFLOAT2 & position, const DirectX::XMFLOAT4 & color) const {
		DrawString(spriteBatch, ConvertUTF8(text), position, color, 0, DirectX::XMFLOAT2{ 0,0 }, 1.0f, 0);
	}
	/*
	void SpriteFont::DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ const char * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color, float rotation, DirectX::XMFLOAT2 const & origin, float scale, SpriteEffects effects, float layerDepth) const
	{
		DrawString(spriteBatch, ConvertUTF8(text), XMLoadFloat2(&position), color, rotation, DirectX::XMLoadFloat2(&origin), DirectX::XMVectorReplicate(scale), effects, layerDepth);
	}


	void SpriteFont::DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ char const * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color, float rotation, DirectX::XMFLOAT2 const & origin, DirectX::XMFLOAT2 const & scale, SpriteEffects effects, float layerDepth) const
	{
		DrawString(spriteBatch, ConvertUTF8(text), DirectX::XMLoadFloat2(&position), color, rotation, DirectX::XMLoadFloat2(&origin), DirectX::XMLoadFloat2(&scale), effects, layerDepth);
	}


	void SpriteFont::DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ char const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color, float rotation, DirectX::FXMVECTOR origin, float scale, SpriteEffects effects, float layerDepth) const
	{
		DrawString(spriteBatch, ConvertUTF8(text), position, color, rotation, origin, DirectX::XMVectorReplicate(scale), effects, layerDepth);
	}


	void SpriteFont::DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ char const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color, float rotation, DirectX::FXMVECTOR origin, DirectX::GXMVECTOR scale, SpriteEffects effects, float layerDepth) const
	{
		DrawString(spriteBatch, ConvertUTF8(text), position, color, rotation, origin, scale, effects, layerDepth);
	}

	void SpriteFont::DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::XMFLOAT2 const & position, DirectX::XMVECTOR color, float rotation, DirectX::XMFLOAT2 const & origin, float scale, SpriteEffects effects, float layerDepth) const
	{
		DirectX::XMVECTOR posV = DirectX::XMLoadFloat2(&position);
		DirectX::XMVECTOR originV = DirectX::XMLoadFloat2(&origin);
		DirectX::XMVECTOR scaleV = DirectX::XMVectorReplicate(scale);
		DrawString(spriteBatch, text, posV, color, rotation, originV, scaleV, effects, layerDepth);
	}


	void SpriteFont::DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color, float rotation, DirectX::XMFLOAT2 const & origin, DirectX::XMFLOAT2 const & scale, SpriteEffects effects, float layerDepth) const
	{
		DrawString(spriteBatch, text, DirectX::XMLoadFloat2(&position), color, rotation, DirectX::XMLoadFloat2(&origin), DirectX::XMLoadFloat2(&scale), effects, layerDepth);
	}


	void SpriteFont::DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color, float rotation, DirectX::FXMVECTOR origin, float scale, SpriteEffects effects, float layerDepth) const
	{
		DrawString(spriteBatch, text, position, color, rotation, origin, DirectX::XMVectorReplicate(scale), effects, layerDepth);
	}*/

	void SpriteFont::DrawString(Netcode::SpriteBatchRef spriteBatch,
		const wchar_t * text,
		const DirectX::XMFLOAT2 & position,
		const DirectX::XMFLOAT4 & color,
		float rotation,
		const DirectX::XMFLOAT2 & origin,
		float scale,
		float layerDepth) const
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
				MeasureString_Impl(text),
				axisIsMirroredTable[effects & 3],
				baseOffset);
		}

		ForEachGlyph(text, [&, color](Glyph const * glyph, float x, float y, float advance)
		{
			UNREFERENCED_PARAMETER(advance);

			DirectX::XMVECTOR offset = XMVectorMultiplyAdd(DirectX::XMVectorSet(x, y + glyph->YOffset, 0, 0), axisDirectionTable[effects & 3], baseOffset);

			if(effects)
			{
				DirectX::XMVECTOR glyphRect = DirectX::XMConvertVectorIntToFloat(DirectX::XMLoadInt4(reinterpret_cast<uint32_t const *>(&glyph->Subrect)), 0);

				glyphRect = DirectX::XMVectorSubtract(DirectX::XMVectorSwizzle<2, 3, 0, 1>(glyphRect), glyphRect);

				offset = XMVectorMultiplyAdd(glyphRect, axisIsMirroredTable[effects & 3], offset);
			}

			DirectX::XMFLOAT2 offsetValue;
			DirectX::XMStoreFloat2(&offsetValue, offset);

			spriteBatch->DrawSprite(shaderResourceView, textureSize, position, &glyph->Subrect, color, rotation, offsetValue, scale, layerDepth);
		});
	}


	DirectX::XMVECTOR SpriteFont::MeasureString_Impl(wchar_t const * text) const
	{
		DirectX::XMVECTOR result = DirectX::XMVectorZero();

		ForEachGlyph(text, [&](Glyph const * glyph, float x, float y, float advance)
		{
			UNREFERENCED_PARAMETER(advance);

			auto w = static_cast<float>(glyph->Subrect.right - glyph->Subrect.left);
			auto h = static_cast<float>(glyph->Subrect.bottom - glyph->Subrect.top) + glyph->YOffset;

			h = std::max(h, lineSpacing);

			result = DirectX::XMVectorMax(result, DirectX::XMVectorSet(x + w, y + h, 0, 0));
		});

		return result;
	}


	void __cdecl SpriteFont::SetDefaultCharacter(wchar_t character)
	{
		defaultGlyph = nullptr;

		if(character)
		{
			defaultGlyph = FindGlyph(character);
		}
	}

	const Glyph * SpriteFont::FindGlyph(wchar_t character) const
	{
		auto glyph = std::lower_bound(glyphs.begin(), glyphs.end(), character);

		if(glyph != glyphs.end() && glyph->Character == character)
		{
			return &(*glyph);
		}

		if(defaultGlyph)
		{
			return defaultGlyph;
		}

		ASSERT(false, "ERROR: SpriteFont encountered a character not in the font (%u, %C), and no default glyph was provided\n", character, character);
		return nullptr;
	}

	DirectX::XMUINT2 SpriteFont::GetSpriteSheetSize() const
	{
		return textureSize;
	}


	DirectX::XMVECTOR SpriteFont::MeasureString_Impl(char const * text) const
	{
		return MeasureString_Impl(ConvertUTF8(text));
	}

	DirectX::XMFLOAT2 SpriteFont::MeasureString(const char * str) const {
		DirectX::XMFLOAT2 res;
		DirectX::XMVECTOR v = MeasureString_Impl(str);
		DirectX::XMStoreFloat2(&res, v);
		return res;
	}

	DirectX::XMFLOAT2 SpriteFont::MeasureString(const wchar_t * str) const {
		DirectX::XMFLOAT2 res;
		DirectX::XMVECTOR v = MeasureString_Impl(str);
		DirectX::XMStoreFloat2(&res, v);
		return res;
	}

	RECT SpriteFont::MeasureDrawBounds(wchar_t const * text, DirectX::XMFLOAT2 const & position) const
	{
		RECT result = { LONG_MAX, LONG_MAX, 0, 0 };

		ForEachGlyph(text, [&](Glyph const * glyph, float x, float y, float advance)
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


	RECT SpriteFont::MeasureDrawBounds(wchar_t const * text, DirectX::FXMVECTOR position) const
	{
		DirectX::XMFLOAT2 pos;
		DirectX::XMStoreFloat2(&pos, position);

		return MeasureDrawBounds(text, pos);
	}

	RECT SpriteFont::MeasureDrawBounds(char const * text, DirectX::XMFLOAT2 const & position) const
	{
		return MeasureDrawBounds(ConvertUTF8(text), position);
	}


	RECT SpriteFont::MeasureDrawBounds(char const * text, DirectX::FXMVECTOR position) const
	{
		DirectX::XMFLOAT2 pos;
		XMStoreFloat2(&pos, position);

		return MeasureDrawBounds(ConvertUTF8(text), pos);
	}


	// Spacing properties
	float SpriteFont::GetLineSpacing() const
	{
		return lineSpacing;
	}


	void SpriteFont::SetLineSpacing(float spacing)
	{
		lineSpacing = spacing;
	}


	// Font properties
	wchar_t SpriteFont::GetDefaultCharacter() const
	{
		return static_cast<wchar_t>(defaultGlyph ? defaultGlyph->Character : 0);
	}

	bool SpriteFont::ContainsCharacter(wchar_t character) const
	{
		return std::binary_search(glyphs.begin(), glyphs.end(), character);
	}


}
