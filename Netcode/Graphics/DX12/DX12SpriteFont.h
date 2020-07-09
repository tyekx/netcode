#pragma once

#include "../../Common.h"
#include "../../IO/BinaryReader.h"
#include "../GraphicsContexts.h"

#include <DirectXMath.h>
#include "DX12Common.h"
#include "DX12SpriteBatch.h"
#include <DirectXColors.h>
#include <DirectXTex.h>
#include "DX12ResourceViews.h"
#include "DX12Texture.h"

namespace Netcode::Graphics::DX12 {
	
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
		Rect Subrect;
		float XOffset;
		float YOffset;
		float XAdvance;
	};

	struct GlyphLimits {
		Glyph highestGlyph;
		Glyph widestGlyph;
		Glyph highestAlphaNumericGlyph;
		Glyph widestAlphaNumericGlyph;
	};

	class SpriteFont : public Netcode::SpriteFont
	{
		GpuResourceRef textureResource;
		DX12ResourceViewsRef shaderResourceView;

		GlyphLimits limits;

		Netcode::UInt2 textureSize;
		std::vector<Glyph> glyphs;
		const Glyph * defaultGlyph;
		float lineSpacing;

		// cache members
		mutable size_t utfBufferSize;
		mutable std::unique_ptr<wchar_t[]> utfBuffer;

		void Construct(IO::BinaryReader * reader, IResourceContext * resourceContext, IFrameContext * frameContext);

		void UpdateLimits();

		DirectX::XMVECTOR MeasureString_Impl(const wchar_t * text) const;
		DirectX::XMVECTOR MeasureString_Impl(const char * text) const;
	public:
		SpriteFont(SpriteFont && moveFrom) = default;
		SpriteFont & operator= (SpriteFont && moveFrom) = default;

		SpriteFont(SpriteFont const &) = delete;
		SpriteFont & operator= (SpriteFont const &) = delete;

		SpriteFont(IResourceContext * resourceContext, IFrameContext * frameContext, const std::wstring & fileName);

		virtual ResourceViewsRef GetResourceView() const override;

		virtual void DrawString(Netcode::SpriteBatchRef spriteBatch, const wchar_t * text, const Netcode::Float2 & position, const Netcode::Float4 & color) const override;

		virtual void DrawString(Netcode::SpriteBatchRef spriteBatch, const char * text, const Netcode::Float2 & position, const Netcode::Float4 & color) const override;

		void DrawString(Netcode::SpriteBatchRef spriteBatch,
			const wchar_t * text,
			const Netcode::Float2 & position,
			const Netcode::Float4 & color,
			float rotation,
			const Netcode::Float2 & origin,
			float scale,
			float layerDepth) const;

		Rect __cdecl MeasureDrawBounds(const wchar_t * text, const Netcode::Float2 & position) const;
		Rect  MeasureDrawBounds(const wchar_t * text, DirectX::FXMVECTOR position) const;

		Netcode::Float2 MeasureString(const char * str) const override;
		Netcode::Float2 MeasureString(const wchar_t * str) const override;

		virtual float GetHighestCharHeight() const override;
		virtual float GetWidestCharWidth() const override;
		virtual wchar_t GetHighestChar() const override;
		virtual wchar_t GetWidestChar() const override;
		virtual Float2 GetMaxSizedStringOf(uint32_t stringMaxLength) const override;

		virtual float GetWidestAlphaNumericCharWidth() const override;
		virtual float GetHighestAlphaNumericCharHeight() const override;
		virtual wchar_t GetWidestAlphaNumericChar() const override;
		virtual wchar_t GetHeighestAlphaNumericChar() const override;
		virtual Float2 GetMaxSizedAlphaNumericStringOf(uint32_t stringMaxLength) const override;

		Rect __cdecl MeasureDrawBounds(const char * text, const Netcode::Float2 & position) const;
		Rect  MeasureDrawBounds(const char * text, DirectX::FXMVECTOR position) const;

		// Spacing properties
		float __cdecl GetLineSpacing() const;
		void __cdecl SetLineSpacing(float spacing);

		// Font properties
		wchar_t __cdecl GetDefaultCharacter() const;
		void __cdecl SetDefaultCharacter(wchar_t character);

		bool __cdecl ContainsCharacter(wchar_t character) const;

		const Glyph * __cdecl FindGlyph(wchar_t character) const;
		Netcode::UInt2 __cdecl GetSpriteSheetSize() const;

		template<typename TAction>
		void ForEachGlyph(const wchar_t * text, TAction action) const
		{
			float x = 0;
			float y = 0;

			for(; *text; text++)
			{
				wchar_t character = *text;

				switch(character)
				{
				case L'\r':
					// Skip carriage returns.
					continue;

				case L'\n':
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
	};

	using DX12SpriteFont = Netcode::Graphics::DX12::SpriteFont;
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
	
	using DX12SpriteFont = Netcode::Graphics::DX12::SpriteFont;
	using DX12SpriteFontRef = std::shared_ptr<DX12SpriteFont>;

};
