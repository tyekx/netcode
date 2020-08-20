#pragma once

#include <Netcode/HandleDecl.h>
#include <Netcode/URI/Texture.h>
#include <NetcodeFoundation/Exceptions.h>
#include <NetcodeFoundation/ArrayView.hpp>
#include <NetcodeFoundation/Math.h>
#include <memory>
 

namespace Netcode {

	enum class MaterialType {
		BRDF,
		SKYBOX,
		EMISSIVE
	};

	enum class MaterialParamId {
		DIFFUSE_ALBEDO,
		FRESNEL_R0,
		ROUGHNESS,
		TEXTURE_TILES,
		TEXTURE_TILES_OFFSET,
		TEXTURE_FLAGS,
		DISPLACEMENT_SCALE,
		DISPLACEMENT_BIAS,

		SENTINEL_TEXTURE_PATHS_BEGIN = 0x0800,
		TEXTURE_DIFFUSE_PATH = 0x0800,
		TEXTURE_NORMAL_PATH,
		TEXTURE_AMBIENT_PATH,
		TEXTURE_SPECULAR_PATH,
		TEXTURE_ROUGHNESS_PATH,
		TEXTURE_DISPLACEMENT_PATH,
		TEXTURE_SKYBOX_PATH,

		SENTINEL_TEXTURE_PATHS_END = 0x1000,
		SENTINEL_NON_SERIALIZABLE = 0xE000,

		SENTINEL_TEXTURES = 0xE000,
		TEXTURE_DIFFUSE = 0xE000,
		TEXTURE_NORMAL,
		TEXTURE_AMBIENT,
		TEXTURE_SPECULAR,
		TEXTURE_ROUGHNESS,
		TEXTURE_DISPLACEMENT,
		TEXTURE_SKYBOX,

		USER_VIEW_1 = 0xF000,
		USER_VIEW_2,
		USER_VIEW_3,
		USER_VIEW_4,
		USER_VIEW_5,
		USER_VIEW_6,
		USER_VIEW_7,
		USER_VIEW_8,

		INVALID_PARAMETER = 0xFFFF
	};

	struct MaterialParam {
		uint32_t id;
		uint16_t offset;
		uint16_t size;

		MaterialParam() = default;
		MaterialParam(uint32_t id, uint16_t offset, uint16_t size) : 
			id{ id }, offset{ offset }, size{ size } {

		}

		static MaterialParam SafeCast(MaterialParamId id, size_t offset, size_t size);
	};

	class Material {
	protected:
		MaterialType type;
		std::string name;
		Ref<GpuResource> resources[8];
		Ref<ResourceViews> views[8];

	public:
		virtual ~Material() = default;
		Material(MaterialType type, const std::string & name) : type{ type }, name{ name }, resources{}, views{} { }

		Ref<GpuResource> GetResource(uint32_t resourceIndex) const;
		void SetResource(uint32_t resourceIndex, Ref<GpuResource> resource);

		Ref<ResourceViews> GetResourceView(uint32_t viewIndex) const;
		void SetResourceView(uint32_t viewIndex, Ref<ResourceViews> resource);

		MaterialType GetType() const {
			return type;
		}

		const std::string & GetName() const {
			return name;
		}

		void SetName(const std::string & value) {
			name = value;
		}
		
		virtual uint32_t GetParameterCount() const {
			return 0;
		}

		virtual MaterialParam GetParameterByIndex(uint32_t idx) const {
			return MaterialParam::SafeCast(MaterialParamId::INVALID_PARAMETER, 0, 0);
		}
		
		virtual const void * GetParameterPointerByIndex(uint32_t idx) const {
			return nullptr;
		}

		virtual void * GetParameterPointer(uint32_t id) {
			return nullptr;
		}

		virtual const void * GetParameterPointer(uint32_t id) const {
			return nullptr;
		}

		template<typename T, typename E>
		void SetParameter(E id, const T & rhs) {
			void * param = GetParameterPointer(static_cast<uint32_t>(id));

			if constexpr (std::is_copy_assignable<T>::value) {
				*reinterpret_cast<T *>(param) = rhs;
			} else {
				static_assert(std::is_pod<T>::value, "Memcpy works only for POD types");

				memcpy(param, &rhs, sizeof(T));
			}
		}

		template<typename T, typename E>
		inline const T & GetRequiredParameter(E id) const {
			const void * param = GetParameterPointer(static_cast<uint32_t>(id));

			Netcode::UndefinedBehaviourAssertion(param != nullptr);

			return *reinterpret_cast<const T *>(param);
		}

		template<typename T, typename E>
		inline T GetOptionalParameter(E id, T defValue) const {
			const void * param = GetParameterPointer(static_cast<uint32_t>(id));

			if(param == nullptr) {
				return defValue;
			}

			return *reinterpret_cast<const T *>(param);
		}
	};

	class BrdfMaterial : public Material {
	private:
		static MaterialParam brdfReflectionData[];

	protected:
		struct BrdfData {
			Netcode::Float4 diffuseAlbedo;
			Netcode::Float3 fresnelR0;
			float roughness;
			Netcode::Float2 textureTiles;
			Netcode::Float2 textureOffset;
			float displacementScale;
			float displacementBias;
			uint32_t textureFlags;
		};

		BrdfData brdfData;

		URI::Texture brdfPathData[6];

	public:
		using Material::Material;

		virtual uint32_t GetParameterCount() const override;

		virtual MaterialParam GetParameterByIndex(uint32_t idx) const override;

		virtual const void * GetParameterPointerByIndex(uint32_t idx) const override;

		virtual void * GetParameterPointer(uint32_t id) override;

		virtual const void * GetParameterPointer(uint32_t id) const override;
	};

}
