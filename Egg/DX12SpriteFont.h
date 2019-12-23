#pragma once

#include "Common.h"

#include "BinaryReader.h"
#include <DirectXMath.h>
#include "DX12Common.h"
#include "DX12Resource.h"
#include "DX12SpriteBatch.h"
#include <DirectXColors.h>
#include <DirectXTex/DirectXTex.h>

namespace Egg::Graphics::DX12 {

	template<typename T>
	inline T AlignUp(T size, size_t alignment)
	{
		if(alignment > 0)
		{
			assert(((alignment - 1) & alignment) == 0);
			T mask = static_cast<T>(alignment - 1);
			return (size + mask) & ~mask;
		}
		return size;
	}

	class SpriteFont
	{
		void Construct(ID3D12Device * device, Resource::IResourceUploader * upload, BinaryReader * reader, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorDest, D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptor);
	public:
		struct Glyph;

		SpriteFont(ID3D12Device * device, Resource::IResourceUploader * upload, _In_z_ wchar_t const * fileName, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorDest, D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptor);
		SpriteFont(ID3D12Device * device, Resource::IResourceUploader * upload, _In_reads_bytes_(dataSize) uint8_t * dataBlob, size_t dataSize, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorDest, D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptor);

		SpriteFont(SpriteFont && moveFrom) = default;
		SpriteFont & operator= (SpriteFont && moveFrom) = default;

		SpriteFont(SpriteFont const &) = delete;
		SpriteFont & operator= (SpriteFont const &) = delete;

		virtual ~SpriteFont() = default;

		// Wide-character / UTF-16LE
		void XM_CALLCONV DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color = DirectX::Colors::White, float rotation = 0, DirectX::XMFLOAT2 const & origin = Float2Zero, float scale = 1, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		void XM_CALLCONV DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color, float rotation, DirectX::XMFLOAT2 const & origin, DirectX::XMFLOAT2 const & scale, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		void XM_CALLCONV DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color = DirectX::Colors::White, float rotation = 0, DirectX::FXMVECTOR origin = DirectX::g_XMZero, float scale = 1, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		void XM_CALLCONV DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color, float rotation, DirectX::FXMVECTOR origin, DirectX::GXMVECTOR scale, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;

		DirectX::XMVECTOR XM_CALLCONV MeasureString(_In_z_ wchar_t const * text) const;

		RECT __cdecl MeasureDrawBounds(_In_z_ wchar_t const * text, DirectX::XMFLOAT2 const & position) const;
		RECT XM_CALLCONV MeasureDrawBounds(_In_z_ wchar_t const * text, DirectX::FXMVECTOR position) const;

		// UTF-8
		void XM_CALLCONV DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ char const * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color = DirectX::Colors::White, float rotation = 0, DirectX::XMFLOAT2 const & origin = Float2Zero, float scale = 1, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		void XM_CALLCONV DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ char const * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color, float rotation, DirectX::XMFLOAT2 const & origin, DirectX::XMFLOAT2 const & scale, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		void XM_CALLCONV DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ char const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color = DirectX::Colors::White, float rotation = 0, DirectX::FXMVECTOR origin = DirectX::g_XMZero, float scale = 1, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
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

		DirectX::ScratchImage imageData;

		com_ptr<ID3D12Resource> textureResource;
		D3D12_GPU_DESCRIPTOR_HANDLE texture;
		DirectX::XMUINT2 textureSize;
		std::vector<Glyph> glyphs;
		Glyph const * defaultGlyph;
		float lineSpacing;
		// cache members
		mutable size_t utfBufferSize;
		mutable std::unique_ptr<wchar_t[]> utfBuffer;

		template<typename TAction>
		void ForEachGlyph(wchar_t const * text, TAction action) const
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

		const wchar_t * ConvertUTF8(const char * text) const;

		void CreateTextureResource(ID3D12Device * device, Resource::IResourceUploader * upload, uint32_t width, uint32_t height, DXGI_FORMAT format, uint32_t stride, uint32_t rows, uint8_t * data);

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
