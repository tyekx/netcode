#pragma once

#include <string>
#include <NetcodeFoundation/Json.h>

namespace Netcode {

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
        Property() noexcept : stored{ nullptr } {}
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

    };

	class Config {
        static void LoadReflectedValue(const std::wstring & path, std::wstring_view valueType, const JsonValue & value);

        static void LoadMembersRecursive(const std::wstring & prefix, const JsonValue & value);
		
	public:
		static void LoadJson(const JsonDocument & document);

        static Property & GetProperty(const std::wstring & key);

        static const Property & GetConstProperty(const std::wstring & key);

        static void SetProperty(const std::wstring & key, Property prop);

		template<typename T>
		static inline const T & Get(const std::wstring & key) {
            return GetConstProperty(key).Get<T>();
		}

		template<typename T>
		static inline void Set(const std::wstring & key, const T & value) {
            SetProperty(key, Property{ value });
		}
	};

}
