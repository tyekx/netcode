#pragma once

#if !defined(NETCODE_CONSTRUCTORY_ALL)

#define NETCODE_CONSTRUCTORS_ALL(type) \
~type() = default; \
type() = default; \
type(type &&) noexcept = default; \
type & operator=(type &&) noexcept = default; \
type(const type & ) = default; \
type & operator=(const type &) = default

#endif

#if !defined(NETCODE_CONSTRUCTORS_NO_COPY)

#define NETCODE_CONSTRUCTORS_NO_COPY(type) \
~type() = default; \
type() = default; \
type(type &&) noexcept = default; \
type & operator=(type &&) noexcept = default; \
type(const type &) = delete; \
type & operator=(const type &) = delete

#endif

#if !defined(NETCODE_CONSTRUCTORS_NO_COPY_NO_MOVE)

#define NETCODE_CONSTRUCTORS_NO_COPY_NO_MOVE(type) \
~type() = default; \
type() = default; \
type(type &&) noexcept = delete; \
type & operator=(type &&) noexcept = delete; \
type(const type &) = delete; \
type & operator=(const type &) = delete

#endif

#if !defined(NETCODE_CONSTRUCTORS_DELETE_COPY)
#define NETCODE_CONSTRUCTORS_DELETE_COPY(type) \
type(const type &) = delete; \
type & operator=(const type &) = delete
#endif

#if !defined(NETCODE_CONSTRUCTORS_DELETE_MOVE)
#define NETCODE_CONSTRUCTORS_DELETE_MOVE(type) \
type(type &&) noexcept = delete; \
type & operator=(type &&) noexcept = delete
#endif


#if !defined(NETCODE_CONSTRUCTORS_DEFAULT_MOVE)
#define NETCODE_CONSTRUCTORS_DEFAULT_MOVE(type) \
type(type &&) noexcept = default; \
type & operator=(type &&) noexcept = default
#endif

#if !defined(NETCODE_CONSTRUCTORS_DEFAULT_COPY)
#define NETCODE_CONSTRUCTORS_DEFAULT_COPY(type) \
type(const type &) = default; \
type & operator=(const type &) = default
#endif
