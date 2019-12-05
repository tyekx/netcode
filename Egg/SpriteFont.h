#pragma once

#include "Common.h"

#include "BinaryReader.h"
#include <DirectXMath.h>
/*
namespace Egg::Font {

	class SpriteFont
	{
	public:
		struct Glyph;

		SpriteFont(ID3D12Device * device, ResourceUploadBatch & upload, _In_z_ wchar_t const * fileName, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorDest, D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptor, bool forceSRGB = false);
		SpriteFont(ID3D12Device * device, ResourceUploadBatch & upload, _In_reads_bytes_(dataSize) uint8_t const * dataBlob, size_t dataSize, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorDest, D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptor, bool forceSRGB = false);
		SpriteFont(D3D12_GPU_DESCRIPTOR_HANDLE texture, DirectX::XMUINT2 textureSize, _In_reads_(glyphCount) Glyph const * glyphs, size_t glyphCount, float lineSpacing);

		SpriteFont(SpriteFont && moveFrom) noexcept;
		SpriteFont & operator= (SpriteFont && moveFrom) noexcept;

		SpriteFont(SpriteFont const &) = delete;
		SpriteFont & operator= (SpriteFont const &) = delete;

		virtual ~SpriteFont();

		// Wide-character / UTF-16LE
		void XM_CALLCONV DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color = Colors::White, float rotation = 0, DirectX::XMFLOAT2 const & origin = Float2Zero, float scale = 1, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		void XM_CALLCONV DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color, float rotation, DirectX::XMFLOAT2 const & origin, DirectX::XMFLOAT2 const & scale, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		void XM_CALLCONV DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color = Colors::White, float rotation = 0, DirectX::FXMVECTOR origin = DirectX::g_XMZero, float scale = 1, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		void XM_CALLCONV DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color, float rotation, DirectX::FXMVECTOR origin, DirectX::GXMVECTOR scale, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;

		DirectX::XMVECTOR XM_CALLCONV MeasureString(_In_z_ wchar_t const * text) const;

		RECT __cdecl MeasureDrawBounds(_In_z_ wchar_t const * text, DirectX::XMFLOAT2 const & position) const;
		RECT XM_CALLCONV MeasureDrawBounds(_In_z_ wchar_t const * text, DirectX::FXMVECTOR position) const;

		// UTF-8
		void XM_CALLCONV DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ char const * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color = Colors::White, float rotation = 0, DirectX::XMFLOAT2 const & origin = Float2Zero, float scale = 1, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		void XM_CALLCONV DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ char const * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color, float rotation, DirectX::XMFLOAT2 const & origin, XMFLOAT2 const & scale, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		void XM_CALLCONV DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ char const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color = Colors::White, float rotation = 0, DirectX::FXMVECTOR origin = DirectX::g_XMZero, float scale = 1, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		void XM_CALLCONV DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ char const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color, float rotation, DirectX::FXMVECTOR origin, DirectX::GXMVECTOR scale, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;

		DirectX::XMVECTOR XM_CALLCONV MeasureString(_In_z_ char const * text) const;

		RECT __cdecl MeasureDrawBounds(_In_z_ char const * text, DirectX::XMFLOAT2 const & position) const;
		RECT XM_CALLCONV MeasureDrawBounds(_In_z_ char const * text, DirectX::FXMVECTOR position) const;

		// Spacing properties
		float __cdecl GetLineSpacing() const;
		void __cdecl SetLineSpacing(float spacing);

		// Font properties
		wchar_t __cdecl GetDefaultCharacter() const;
		void __cdecl SetDefaultCharacter(wchar_t character);

		bool __cdecl ContainsCharacter(wchar_t character) const;

		// Custom layout/rendering
		Glyph const * __cdecl FindGlyph(wchar_t character) const;
		D3D12_GPU_DESCRIPTOR_HANDLE __cdecl GetSpriteSheet() const;
		DirectX::XMUINT2 __cdecl GetSpriteSheetSize() const;

		// Describes a single character glyph.
		struct Glyph
		{
			uint32_t Character;
			RECT Subrect;
			float XOffset;
			float YOffset;
			float XAdvance;
		};


	private:
		ComPtr<ID3D12Resource> textureResource;
		D3D12_GPU_DESCRIPTOR_HANDLE texture;
		XMUINT2 textureSize;
		std::vector<Glyph> glyphs;
		Glyph const * defaultGlyph;
		float lineSpacing;
		size_t utfBufferSize;
		std::unique_ptr<wchar_t[]> utfBuffer;

		template<typename TAction>
		void ForEachGlyph(_In_z_ wchar_t const * text, TAction action) const
		{
			float x = 0;
			float y = 0;

			for(; *text; text++)
			{
				wchar_t character = *text;

				switch(character)
				{
				case '\r':
					// Skip carriage returns.
					continue;

				case '\n':
					// New line.
					x = 0;
					y += lineSpacing;
					break;

				default:
					// Output this character.
					auto glyph = FindGlyph(character);

					x += glyph->XOffset;

					if(x < 0)
						x = 0;

					float advance = glyph->Subrect.right - glyph->Subrect.left + glyph->XAdvance;

					if(!iswspace(character)
					   || ((glyph->Subrect.right - glyph->Subrect.left) > 1)
					   || ((glyph->Subrect.bottom - glyph->Subrect.top) > 1))
					{
						action(glyph, x, y, advance);
					}

					x += advance;
					break;
				}
			}
		}

		const wchar_t * ConvertUTF8(_In_z_ const char * text)
		{
			if(!utfBuffer)
			{
				utfBufferSize = 1024;
				utfBuffer.reset(new wchar_t[1024]);
			}

			int result = MultiByteToWideChar(CP_UTF8, 0, text, -1, utfBuffer.get(), static_cast<int>(utfBufferSize));
			if(!result && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
			{
				// Compute required buffer size
				result = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
				utfBufferSize = AlignUp(static_cast<size_t>(result), 1024);
				utfBuffer.reset(new wchar_t[utfBufferSize]);

				// Retry conversion
				result = MultiByteToWideChar(CP_UTF8, 0, text, -1, utfBuffer.get(), static_cast<int>(utfBufferSize));
			}

			ASSERT(result, "ERROR: MultiByteToWideChar failed with error %u.\n", GetLastError());

			return utfBuffer.get();
		}

		void Read(
			ID3D12Device * device,
			BinaryReader * reader,
			D3D12_CPU_DESCRIPTOR_HANDLE cpuDesc,
			D3D12_GPU_DESCRIPTOR_HANDLE gpuDesc) 
		{
			defaultGlyph = (nullptr);
				utfBufferSize = (0);
			// Validate the header.
			for(char const * magic = spriteFontMagic; *magic; magic++)
			{
				ASSERT(reader->Read<uint8_t>() == *magic, "ERROR: SpriteFont provided with an invalid .spritefont file\n");
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

			// Create the D3D texture object.
			CreateTextureResource(
				device, upload,
				textureWidth, textureHeight,
				textureFormat,
				textureStride, textureRows,
				textureData);

			// Create the shader resource view
			/*CreateShaderResourceView(
				device, textureResource.Get(),
				cpuDesc, false);*

				// Save off the GPU descriptor pointer and size.
			texture = gpuDesc;
			textureSize = XMUINT2(textureWidth, textureHeight);
		}

		// Private implementation.
		class Impl;

		std::unique_ptr<Impl> pImpl;

		static const DirectX::XMFLOAT2 Float2Zero;

	};

	static inline bool operator< (SpriteFont::Glyph const & left, SpriteFont::Glyph const & right)
	{
		return left.Character < right.Character;
	}

	static inline bool operator< (wchar_t left, SpriteFont::Glyph const & right)
	{
		return left < right.Character;
	}

	static inline bool operator< (SpriteFont::Glyph const & left, wchar_t right)
	{
		return left.Character < right;
	}
}
*/