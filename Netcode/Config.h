#pragma once

#include <string>
#include <NetcodeFoundation/Json.h>
#include <NetcodeFoundation/ErrorCode.h>

namespace Netcode {

	enum class ConfigErrc {
		SUCCESS,
		BAD_ARGUMENTS,
		CONFIG_NOT_FOUND,
		SHADER_ROOT_NOT_FOUND,
		MEDIA_ROOT_NOT_FOUND,
		INVALID_NETWORK_MODE,
		APPDATA_INACCESSIBLE
	};

    class Property {
    private:
        class PropertyBase {
        public:
            virtual ~PropertyBase() = default;
            virtual PropertyBase * Clone() const = 0;
        };

        PropertyBase * stored;

        template <typename T>
        class PropertyContainer : public PropertyBase {
        public:
            T data;
        	
            PropertyContainer(T&& data) : data { std::move(data) } { }
        	
            PropertyContainer(const T & data) : data{ data } {
	            
            }

            PropertyContainer * Clone() const {
                return new PropertyContainer<T>(*this);
            }
        };

    public:
        constexpr Property() noexcept : stored{ nullptr } {}
    	Property(Property&& rhs) noexcept : stored { std::move(rhs.stored) } { }
        Property(const Property & rhs) : stored{ nullptr } {
        	if(rhs.stored) {
                stored = rhs.stored->Clone();
        	}
        }
        Property & operator=(Property rhs) noexcept { std::swap(stored, rhs.stored); return *this; }

    	template<typename T>
    	Property(const T & rhs) : stored { new PropertyContainer<T>{ rhs } } { }
    	
        ~Property() { delete stored; }

        template<typename T>
    	Property & operator=(T && rhs) {
            PropertyBase * nextData = new PropertyContainer<T>(std::move(rhs));
            delete stored;
            stored = nextData;
            return *this;
        }
    	
        template <typename T>
        Property & operator=(const T & rhs) {
            PropertyBase * nextData = new PropertyContainer<T>(rhs);
            delete stored;
            stored = nextData;
            return *this;
        }

        template <typename T>
        T & Get() {
            return dynamic_cast<PropertyContainer<T> &>(*stored).data;
        }
    	
        template <typename T>
        T const & Get() const {
            return dynamic_cast<PropertyContainer<T> &>(*stored).data;
        }

        constexpr bool operator==(std::nullptr_t) const {
            return stored == nullptr;
        }

        constexpr bool operator!=(std::nullptr_t) const {
            return stored != nullptr;
        }
    };

	class Config {
        static void LoadReflectedValue(const std::wstring & path, std::wstring_view valueType, const JsonValue & value);

        static void LoadMembersRecursive(const std::wstring & prefix, const JsonValue & value);
		
	public:
		static void LoadJson(const JsonDocument & document);

        static Property & GetProperty(const std::wstring & key);

        static const Property & GetConstProperty(const std::wstring & key);

        static const Property & GetConstOptionalProperty(const std::wstring & key) noexcept;
		
        static void SetProperty(const std::wstring & key, Property prop);

		template<typename T>
		static inline const T & Get(const std::wstring & key) {
            return GetConstProperty(key).Get<T>();
		}

		template<typename T>
		static const T & GetOptional(const std::wstring & key, const T & fallbackValue) {
            const Property & prop = GetConstOptionalProperty(key);

			if(prop == nullptr) {
                return fallbackValue;
			}

			// might throw an invalid cast exception
            return prop.Get<T>();
		}

		template<typename T>
		static inline void Set(const std::wstring & key, const T & value) {
            SetProperty(key, Property{ value });
		}
	};


    class ConfigErrorCategory : public std::error_category {
    public:
        [[nodiscard]]
        const char * name() const noexcept override
        {
            return "Netcode.Config";
        }

        [[nodiscard]]
        std::string message(int v) const override
        {
            switch(static_cast<ConfigErrc>(v)) {
                case ConfigErrc::SUCCESS: return "Success";
                case ConfigErrc::BAD_ARGUMENTS: return "Bad commandline arguments";
                case ConfigErrc::CONFIG_NOT_FOUND: return "Config file not found";
                case ConfigErrc::MEDIA_ROOT_NOT_FOUND: return "Media folder not found";
                case ConfigErrc::SHADER_ROOT_NOT_FOUND: return "Shader folder not found";
                case ConfigErrc::INVALID_NETWORK_MODE: return "Invalid network mode";
                case ConfigErrc::APPDATA_INACCESSIBLE: return "AppData inaccessible";
                default: return "Unknown configuration error";
            }
        }
    };

    inline ErrorCode make_error_code(ConfigErrc e) {
        static ConfigErrorCategory cat;
        return ErrorCode{ static_cast<int>(e), cat };
    }
}

namespace std {
    template<>
    struct is_error_code_enum<Netcode::ConfigErrc> {
        static const bool value = true;
    };
}
