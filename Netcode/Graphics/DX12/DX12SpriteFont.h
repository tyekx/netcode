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

		Netcode::Vector2 NC_MATH_CALLCONV MeasureString_Impl(const wchar_t * text, int numChars = -1) const;
		Netcode::Vector2 NC_MATH_CALLCONV MeasureString_Impl(const char * text, int numChars = -1) const;
	public:
		SpriteFont(SpriteFont && moveFrom) = default;
		SpriteFont & operator= (SpriteFont && moveFrom) = default;

		SpriteFont(SpriteFont const &) = delete;
		SpriteFont & operator= (SpriteFont const &) = delete;

		SpriteFont(IResourceContext * resourceContext, IFrameContext * frameContext, const std::wstring & fileName);

		virtual ResourceViewsRef GetResourceView() const override;

		virtual void DrawString(SpriteBatchPtr spriteBatch, const wchar_t * text, const Float2 & position, const Float4 & color) const override;
		virtual void DrawString(SpriteBatchPtr spriteBatch, const wchar_t * text, const Float2 & position, const Float4 & color, float zIndex) const override;
		virtual void DrawString(SpriteBatchPtr spriteBatch, const wchar_t * text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ) const override;
		virtual void DrawString(SpriteBatchPtr spriteBatch, const wchar_t * text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ, float zIndex) const override;

		virtual void DrawString(SpriteBatchPtr spriteBatch, const char * text, const Float2 & position, const Float4 & color) const override;
		virtual void DrawString(SpriteBatchPtr spriteBatch, const char * text, const Float2 & position, const Float4 & color, float zIndex) const override;
		virtual void DrawString(SpriteBatchPtr spriteBatch, const char * text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ) const override;
		virtual void DrawString(SpriteBatchPtr spriteBatch, const char * text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ, float zIndex) const override;

		virtual void DrawString(SpriteBatchPtr spriteBatch, std::wstring_view text, const Float2 & position, const Float4 & color) const override;
		virtual void DrawString(SpriteBatchPtr spriteBatch, std::wstring_view text, const Float2 & position, const Float4 & color, float zIndex) const override;
		virtual void DrawString(SpriteBatchPtr spriteBatch, std::wstring_view text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ) const override;
		virtual void DrawString(SpriteBatchPtr spriteBatch, std::wstring_view text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ, float zIndex) const override;

		virtual void DrawString(SpriteBatchPtr spriteBatch, std::string_view text, const Float2 & position, const Float4 & color) const override;
		virtual void DrawString(SpriteBatchPtr spriteBatch, std::string_view text, const Float2 & position, const Float4 & color, float zIndex) const override;
		virtual void DrawString(SpriteBatchPtr spriteBatch, std::string_view text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ) const override;
		virtual void DrawString(SpriteBatchPtr spriteBatch, std::string_view text, const Float2 & position, const Float4 & color, const Float2 & rotationOrigin, float rotationZ, float zIndex) const override;

		void DrawString(SpriteBatchPtr spriteBatch, const wchar_t * text, int length, const Float2 & position, const Float4 & color, const Float2 & origin, float rotation, float layerDepth) const;

		Rect MeasureDrawBounds(const wchar_t * text, const Netcode::Float2 & position) const;
		Rect NC_MATH_CALLCONV MeasureDrawBounds(const wchar_t * text, DirectX::FXMVECTOR position) const;

		virtual Float2 MeasureString(const char * str) const override;
		virtual Float2 MeasureString(const wchar_t * str) const override;

		virtual Float2 MeasureString(std::string_view view) const override;
		virtual Float2 MeasureString(std::wstring_view view) const override;

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

		Rect MeasureDrawBounds(const char * text, const Netcode::Float2 & position) const;
		Rect NC_MATH_CALLCONV MeasureDrawBounds(const char * text, DirectX::FXMVECTOR position) const;

		// Spacing properties
		float GetLineSpacing() const;
		void SetLineSpacing(float spacing);

		// Font properties
		wchar_t GetDefaultCharacter() const;
		void SetDefaultCharacter(wchar_t character);

		bool ContainsCharacter(wchar_t character) const;

		const Glyph * FindGlyph(wchar_t character) const;
		Netcode::UInt2 GetSpriteSheetSize() const;

		template<typename TAction>
		void ForEachGlyph(const wchar_t * text, int numChars, TAction action) const
		{
			float x = 0;
			float y = 0;
			const uint32_t nChars = static_cast<uint32_t>(numChars);
			for(uint32_t i = 0; *text && (i < nChars); i++, text++)
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

		const wchar_t * ConvertUTF8(const char * text, int numChars = -1) const;
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
