#include <algorithm>
#include <vector>
#include "DX12SpriteFont.h"

static const char spriteFontMagic[] = "DXTKfont";

namespace Egg::Graphics::DX12 {

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

	void SpriteFont::CreateTextureResource(ID3D12Device * device, Resource::IResourceUploader * upload, uint32_t width, uint32_t height, DXGI_FORMAT format, uint32_t stride, uint32_t rows, uint8_t * data) {
		D3D12_RESOURCE_DESC desc = {};
		desc.Width = static_cast<UINT>(width);
		desc.Height = static_cast<UINT>(height);
		desc.MipLevels = 1;
		desc.DepthOrArraySize = 1;
		desc.Format = format;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

		DX_API("Failed to create committed resource")
		device->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(textureResource.ReleaseAndGetAddressOf()));

		textureResource->SetName(L"SpriteFont:Texture");

		D3D12_SUBRESOURCE_DATA subres = {};
		subres.pData = data;
		subres.RowPitch = ptrdiff_t(stride);
		subres.SlicePitch = ptrdiff_t(stride) * ptrdiff_t(rows);

		upload->Upload(desc, textureResource.Get(), data, stride * rows);
		upload->Transition(textureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	void SpriteFont::Construct(ID3D12Device * device, Resource::IResourceUploader * upload, BinaryReader * reader, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorDest, D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptor) {
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

		// Create the D3D texture object.
		CreateTextureResource(
			device, upload,
			textureWidth, textureHeight,
			textureFormat,
			textureStride, textureRows,
			textureData);

		const auto desc = textureResource->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = desc.Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		ASSERT(desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D, "Only texture2d is supported here");
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = (!desc.MipLevels) ? UINT(-1) : desc.MipLevels;

		device->CreateShaderResourceView(textureResource.Get(), &srvDesc, cpuDescriptorDest);

		// Save off the GPU descriptor pointer and size.
		texture = gpuDescriptor;
		textureSize = DirectX::XMUINT2(textureWidth, textureHeight);
	}

	SpriteFont::SpriteFont(ID3D12Device * device, Resource::IResourceUploader * upload, _In_z_ wchar_t const * fileName, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorDest, D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptor)
	{
		reader = std::make_unique<BinaryReader>(fileName);

		Construct(device, upload, reader.get(), cpuDescriptorDest, gpuDescriptor);
	}

	SpriteFont::SpriteFont(ID3D12Device * device, Resource::IResourceUploader * upload, _In_reads_bytes_(dataSize) uint8_t * dataBlob, size_t dataSize, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorDest, D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptor)
	{
		reader = std::make_unique<BinaryReader>(dataBlob, dataSize);
		Construct(device, upload, reader.get(), cpuDescriptorDest, gpuDescriptor);
	}

	void XM_CALLCONV SpriteFont::DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ const char * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color, float rotation, DirectX::XMFLOAT2 const & origin, float scale, SpriteEffects effects, float layerDepth) const
	{
		DrawString(spriteBatch, ConvertUTF8(text), XMLoadFloat2(&position), color, rotation, DirectX::XMLoadFloat2(&origin), DirectX::XMVectorReplicate(scale), effects, layerDepth);
	}


	void XM_CALLCONV SpriteFont::DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ char const * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color, float rotation, DirectX::XMFLOAT2 const & origin, DirectX::XMFLOAT2 const & scale, SpriteEffects effects, float layerDepth) const
	{
		DrawString(spriteBatch, ConvertUTF8(text), DirectX::XMLoadFloat2(&position), color, rotation, DirectX::XMLoadFloat2(&origin), DirectX::XMLoadFloat2(&scale), effects, layerDepth);
	}


	void XM_CALLCONV SpriteFont::DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ char const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color, float rotation, DirectX::FXMVECTOR origin, float scale, SpriteEffects effects, float layerDepth) const
	{
		DrawString(spriteBatch, ConvertUTF8(text), position, color, rotation, origin, DirectX::XMVectorReplicate(scale), effects, layerDepth);
	}


	void XM_CALLCONV SpriteFont::DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ char const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color, float rotation, DirectX::FXMVECTOR origin, DirectX::GXMVECTOR scale, SpriteEffects effects, float layerDepth) const
	{
		DrawString(spriteBatch, ConvertUTF8(text), position, color, rotation, origin, scale, effects, layerDepth);
	}

	void XM_CALLCONV SpriteFont::DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color, float rotation, DirectX::XMFLOAT2 const & origin, float scale, SpriteEffects effects, float layerDepth) const
	{
		DrawString(spriteBatch, text, DirectX::XMLoadFloat2(&position), color, rotation, DirectX::XMLoadFloat2(&origin), DirectX::XMVectorReplicate(scale), effects, layerDepth);
	}


	void XM_CALLCONV SpriteFont::DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color, float rotation, DirectX::XMFLOAT2 const & origin, DirectX::XMFLOAT2 const & scale, SpriteEffects effects, float layerDepth) const
	{
		DrawString(spriteBatch, text, DirectX::XMLoadFloat2(&position), color, rotation, DirectX::XMLoadFloat2(&origin), DirectX::XMLoadFloat2(&scale), effects, layerDepth);
	}


	void XM_CALLCONV SpriteFont::DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color, float rotation, DirectX::FXMVECTOR origin, float scale, SpriteEffects effects, float layerDepth) const
	{
		DrawString(spriteBatch, text, position, color, rotation, origin, DirectX::XMVectorReplicate(scale), effects, layerDepth);
	}

	void XM_CALLCONV SpriteFont::DrawString(_In_ SpriteBatch * spriteBatch, _In_z_ wchar_t const * text, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color, float rotation, DirectX::FXMVECTOR origin, DirectX::GXMVECTOR scale, SpriteEffects effects, float layerDepth) const
	{
		static_assert(SpriteEffects_FlipHorizontally == 1 &&
					  SpriteEffects_FlipVertically == 2, "If you change these enum values, the following tables must be updated to match");

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

		DirectX::XMVECTOR baseOffset = origin;

		if(effects)
		{
			baseOffset = XMVectorNegativeMultiplySubtract(
				MeasureString(text),
				axisIsMirroredTable[effects & 3],
				baseOffset);
		}

		ForEachGlyph(text, [&](Glyph const * glyph, float x, float y, float advance)
		{
			UNREFERENCED_PARAMETER(advance);

			DirectX::XMVECTOR offset = XMVectorMultiplyAdd(DirectX::XMVectorSet(x, y + glyph->YOffset, 0, 0), axisDirectionTable[effects & 3], baseOffset);

			if(effects)
			{
				DirectX::XMVECTOR glyphRect = DirectX::XMConvertVectorIntToFloat(DirectX::XMLoadInt4(reinterpret_cast<uint32_t const *>(&glyph->Subrect)), 0);

				glyphRect = DirectX::XMVectorSubtract(DirectX::XMVectorSwizzle<2, 3, 0, 1>(glyphRect), glyphRect);

				offset = XMVectorMultiplyAdd(glyphRect, axisIsMirroredTable[effects & 3], offset);
			}

			spriteBatch->Draw(texture, textureSize, position, &glyph->Subrect, color, rotation, offset, scale, effects, layerDepth);
		});
	}


	DirectX::XMVECTOR XM_CALLCONV SpriteFont::MeasureString(_In_z_ wchar_t const * text) const
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

	SpriteFont::Glyph const * SpriteFont::FindGlyph(wchar_t character) const
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

	D3D12_GPU_DESCRIPTOR_HANDLE SpriteFont::GetSpriteSheet() const
	{
		return texture;
	}

	DirectX::XMUINT2 SpriteFont::GetSpriteSheetSize() const
	{
		return textureSize;
	}


	DirectX::XMVECTOR XM_CALLCONV SpriteFont::MeasureString(_In_z_ char const * text) const
	{
		return MeasureString(ConvertUTF8(text));
	}

	RECT SpriteFont::MeasureDrawBounds(_In_z_ wchar_t const * text, DirectX::XMFLOAT2 const & position) const
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


	RECT XM_CALLCONV SpriteFont::MeasureDrawBounds(_In_z_ wchar_t const * text, DirectX::FXMVECTOR position) const
	{
		DirectX::XMFLOAT2 pos;
		DirectX::XMStoreFloat2(&pos, position);

		return MeasureDrawBounds(text, pos);
	}

	RECT SpriteFont::MeasureDrawBounds(_In_z_ char const * text, DirectX::XMFLOAT2 const & position) const
	{
		return MeasureDrawBounds(ConvertUTF8(text), position);
	}


	RECT XM_CALLCONV SpriteFont::MeasureDrawBounds(_In_z_ char const * text, DirectX::FXMVECTOR position) const
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
