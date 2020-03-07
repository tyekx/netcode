#pragma once

#include "Common.h"

#include "BinaryReader.h"
#include <DirectXMath.h>
#include "DX12Common.h"
#include "DX12SpriteBatch.h"
#include <DirectXColors.h>
#include <DirectXTex/DirectXTex.h>
#include "GraphicsContexts.h"
#include "DX12ResourceViews.h"
#include "DX12Texture.h"

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

	struct Glyph
	{
		uint32_t Character;
		RECT Subrect;
		float XOffset;
		float YOffset;
		float XAdvance;
	};

	class SpriteFont : public Egg::SpriteFont
	{
		uint64_t eggTexture;
		DX12ResourceViewsRef shaderResourceView;

		DirectX::ScratchImage imageData;
		DirectX::XMUINT2 textureSize;
		std::vector<Glyph> glyphs;
		const Glyph * defaultGlyph;
		float lineSpacing;

		// cache members
		mutable size_t utfBufferSize;
		mutable std::unique_ptr<wchar_t[]> utfBuffer;

		void Construct(BinaryReader * reader, IResourceContext * resourceContext, IFrameContext * frameContext);

	public:
		SpriteFont(SpriteFont && moveFrom) = default;
		SpriteFont & operator= (SpriteFont && moveFrom) = default;

		SpriteFont(SpriteFont const &) = delete;
		SpriteFont & operator= (SpriteFont const &) = delete;

		SpriteFont(IResourceContext * resourceContext, IFrameContext * frameContext, wchar_t const * fileName);

		virtual ResourceViewsRef GetResourceView() const override;

		virtual void DrawString(Egg::SpriteBatchRef spriteBatch, const wchar_t * text, const DirectX::XMFLOAT2 & position, const DirectX::XMFLOAT4 & color) const override;

		virtual void DrawString(Egg::SpriteBatchRef spriteBatch, const char * text, const DirectX::XMFLOAT2 & position, const DirectX::XMFLOAT4 & color) const override;

		/*
		// Wide-character / UTF-16LE
		void  DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::XMFLOAT2 const & position, DirectX::XMVECTOR color, float rotation = 0, DirectX::XMFLOAT2 const & origin = Float2Zero, float scale = 1, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		void  DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color, float rotation, DirectX::XMFLOAT2 const & origin, DirectX::XMFLOAT2 const & scale, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		void  DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color = DirectX::Colors::White, float rotation = 0, DirectX::FXMVECTOR origin = DirectX::g_XMZero, float scale = 1, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		void  DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color, float rotation, DirectX::FXMVECTOR origin, DirectX::GXMVECTOR scale, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;

		// UTF-8
		void  DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ char const * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color = DirectX::Colors::White, float rotation = 0, DirectX::XMFLOAT2 const & origin = Float2Zero, float scale = 1, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		void  DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ char const * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color, float rotation, DirectX::XMFLOAT2 const & origin, DirectX::XMFLOAT2 const & scale, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		void  DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ char const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color = DirectX::Colors::White, float rotation = 0, DirectX::FXMVECTOR origin = DirectX::g_XMZero, float scale = 1, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		void  DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ char const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color, float rotation, DirectX::FXMVECTOR origin, DirectX::GXMVECTOR scale, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0) const;
		*/

		void DrawString(Egg::SpriteBatchRef spriteBatch,
			const wchar_t * text,
			const DirectX::XMFLOAT2 & position,
			const DirectX::XMFLOAT4 & color,
			float rotation,
			const DirectX::XMFLOAT2 & origin,
			float scale,
			float layerDepth) const;

		RECT __cdecl MeasureDrawBounds(wchar_t const * text, DirectX::XMFLOAT2 const & position) const;
		RECT  MeasureDrawBounds(wchar_t const * text, DirectX::FXMVECTOR position) const;

		DirectX::XMVECTOR  MeasureString_Impl(wchar_t const * text) const;
		DirectX::XMVECTOR  MeasureString_Impl(char const * text) const;

		DirectX::XMFLOAT2 MeasureString(const char * str) const override;
		DirectX::XMFLOAT2 MeasureString(const wchar_t * str) const override;

		RECT __cdecl MeasureDrawBounds(char const * text, DirectX::XMFLOAT2 const & position) const;
		RECT  MeasureDrawBounds(char const * text, DirectX::FXMVECTOR position) const;

		// Spacing properties
		float __cdecl GetLineSpacing() const;
		void __cdecl SetLineSpacing(float spacing);

		// Font properties
		wchar_t __cdecl GetDefaultCharacter() const;
		void __cdecl SetDefaultCharacter(wchar_t character);

		bool __cdecl ContainsCharacter(wchar_t character) const;

		const Glyph * __cdecl FindGlyph(wchar_t character) const;
		DirectX::XMUINT2 __cdecl GetSpriteSheetSize() const;

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

		static const DirectX::XMFLOAT2 Float2Zero;
	};

	using DX12SpriteFont = Egg::Graphics::DX12::SpriteFont;
	using DX12SpriteFontRef = std::shared_ptr<DX12SpriteFont>;

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
	
	using DX12SpriteFont = Egg::Graphics::DX12::SpriteFont;
	using DX12SpriteFontRef = std::shared_ptr<DX12SpriteFont>;

};
