#include <NetcodeFoundation/Platform.h>
#include <NetcodeFoundation/Exception/IOException.h>

#include "File.h"
#include "Path.h"
#include "../Logger.h"
#include "../Utility.h"

#include <cassert>
#include <algorithm>

#if defined(NETCODE_OS_WINDOWS)
#include <Windows.h>

#if defined(NETCODE_EDITOR_VARIANT)
#include <fileapifromapp.h>
#endif
#endif

namespace Netcode::IO {

	FileOpenMode operator|(FileOpenMode lhs, FileOpenMode rhs) {
		return static_cast<FileOpenMode>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
	}

	FileOpenMode operator&(FileOpenMode lhs, FileOpenMode rhs) {
		return static_cast<FileOpenMode>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
	}

	bool operator!=(FileOpenMode lhs, uint32_t value) {
		return static_cast<uint32_t>(lhs) != value;
	}

	struct File::detail {
	private:
		std::wstring path;
		std::wstring_view fullName;
		std::wstring_view name;
		std::wstring_view extension;
		std::wstring_view parentDir;
		HANDLE handle;

		static DWORD ConvertAccess(FileOpenMode mode) {
			if((mode & FileOpenMode::READ_ONLY) != 0) {
				return GENERIC_READ;
			}

			if((mode & FileOpenMode::WRITE_ONLY) != 0) {
				return GENERIC_WRITE;
			}

			return 0;
		}

		static DWORD ConvertMode(FileOpenMode mode) {
			if((mode & FileOpenMode::CREATE_OR_OVERWRITE) != 0) {
				return CREATE_ALWAYS;
			}

			if((mode & FileOpenMode::OPEN) != 0) {
				return OPEN_EXISTING;
			}

			return 0;
		}
	public:
		~detail() noexcept {
			Close();
		}

		void Open(FileOpenMode mode) {
			if(handle != INVALID_HANDLE_VALUE) {
				Close();
			}

			const DWORD apiAccess = ConvertAccess(mode);
			const DWORD openMode = ConvertMode(mode);
			const DWORD shareMode = ((mode & FileOpenMode::READ_ONLY) != 0) ? FILE_SHARE_READ : 0;

#if defined(NETCODE_EDITOR_VARIANT) 
			HANDLE f = CreateFileFromAppW(path.c_str(), apiAccess, shareMode, nullptr, openMode, FILE_ATTRIBUTE_NORMAL, nullptr);
#else
			HANDLE f = CreateFileW(path.c_str(), apiAccess, shareMode, nullptr, openMode, FILE_ATTRIBUTE_NORMAL, nullptr);
#endif

			if(f == INVALID_HANDLE_VALUE) {
				throw IOException{ "Failed to open file", Utility::ToNarrowString(path) };
			}

			handle = f;
		}

		size_t Read(MutableArrayView<uint8_t> buffer) {
			if(handle == INVALID_HANDLE_VALUE) {
				throw IOException{ "Trying to read from an invalid handle" };
			}

			if(buffer.Size() == 0) {
				throw IOException{ "Buffer is empty" };
			}

			DWORD readBytes = 0;
			BOOL readResult = ReadFile(handle, buffer.Data(), buffer.Size(), &readBytes, nullptr);

			if(!readResult) {
				throw IOException{ "Failed to read from file", Utility::ToNarrowString(path) };
			}

			return readBytes;
		}

		size_t Write(ArrayView<uint8_t> buffer) {
			if(handle == INVALID_HANDLE_VALUE) {
				throw IOException{ "Trying to write to an invalid handle" };
			}

			DWORD writtenBytes = 0;
			BOOL writeResult = WriteFile(handle, buffer.Data(), buffer.Size(), &writtenBytes, nullptr);

			if(!writeResult) {
				throw IOException{ "Failed to write to file", Utility::ToNarrowString(path) };
			}

			return writtenBytes;
		}

		void Close() noexcept {
			if(handle != INVALID_HANDLE_VALUE) {
				if(!CloseHandle(handle)) {
					Log::Error("Failed to close file: {0}", Utility::ToNarrowString(path));
				}

				handle = INVALID_HANDLE_VALUE;
			}
		}

		size_t GetSize() const {
			return GetFileSize(handle, nullptr);
		}

		const std::wstring & GetFullPath() const noexcept
		{
			return path;
		}

		std::wstring_view GetFullName() const noexcept
		{
			return fullName;
		}

		std::wstring_view GetName() const noexcept
		{
			return name;
		}

		std::wstring_view GetExtension() const noexcept
		{
			return extension;
		}

		std::wstring_view GetParentDirectory() const noexcept
		{
			return parentDir;
		}

		void ParsePath() {
			Path::FixFilePath(path);

			if(Path::IsRelative(path)) {
				path.insert(0, Path::WorkingDirectory());
			}

			wchar_t slash = Path::GetSlash();

			parentDir = Path::GetParentDirectory(path);

			size_t indexOfLastSlash = path.find_last_of(slash);

			if(indexOfLastSlash != std::wstring::npos) {
				fullName = std::wstring_view{ path.c_str() + indexOfLastSlash + 1, path.size() - indexOfLastSlash - 1 };
			}

			size_t indexOfLastDot = path.find_last_of(L'.');

			if(indexOfLastDot != std::wstring::npos) {
				extension = std::wstring_view{ path.c_str() + indexOfLastDot + 1, path.size() - indexOfLastDot - 1 };
				assert(indexOfLastSlash != std::wstring::npos && indexOfLastSlash < indexOfLastDot);
				name = std::wstring_view{ path.c_str() + indexOfLastSlash + 1, indexOfLastDot - indexOfLastSlash - 1 };
			} else {
				name = fullName;
			}
		}

		detail(std::wstring_view parentDirectory, std::wstring relativePath) : path{ }, fullName{}, name{}, extension{}, parentDir{}, handle{ INVALID_HANDLE_VALUE } {
			assert(Path::IsRelative(relativePath));

			std::wstring p;
			p.reserve(parentDirectory.size() + relativePath.size() + 1);
			p.assign(parentDirectory);
			Path::FixDirectoryPath(p);
			p.append(std::move(relativePath));

			std::swap(p, path);

			ParsePath();
		}

		detail(std::wstring wstr) : path{ std::move(wstr) }, fullName{}, name{}, extension{}, parentDir{}, handle{ INVALID_HANDLE_VALUE } {
			ParsePath();
		}
	};

	File::File(std::wstring_view parentDirectory, std::wstring relativePath) : impl{ nullptr }
	{
		impl.reset(new detail{ parentDirectory, std::move(relativePath) });
	}

	File::File(std::wstring fullPath) : impl{ nullptr }
	{
		impl.reset(new detail{ std::move(fullPath) });
	}

	File::~File()
	{
		impl.reset();
	}

	void File::Open(FileOpenMode mode)
	{
		impl->Open(mode);
	}

	size_t File::Read(MutableArrayView<uint8_t> buffer)
	{
		return impl->Read(buffer);
	}

	size_t File::Write(ArrayView<uint8_t> buffer)
	{
		return impl->Write(buffer);
	}

	void File::Close()
	{
		impl->Close();
	}

	size_t File::GetSize() const
	{
		return impl->GetSize();
	}

	const std::wstring & File::GetFullPath() const noexcept
	{
		return impl->GetFullPath();
	}

	std::wstring_view File::GetFullName() const noexcept
	{
		return impl->GetFullName();
	}

	std::wstring_view File::GetName() const noexcept
	{
		return impl->GetName();
	}

	std::wstring_view File::GetExtension() const noexcept
	{
		return impl->GetExtension();
	}

	std::wstring_view File::GetParentDirectory() const noexcept
	{
		return impl->GetParentDirectory();
	}

	bool File::Exists(const std::wstring & path) noexcept
	{
#if defined(NETCODE_OS_WINDOWS)
	#if defined(NETCODE_EDITOR_VARIANT)
		WIN32_FILE_ATTRIBUTE_DATA attrData;

		BOOL success = GetFileAttributesExFromAppW(path.c_str(), GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, &attrData);

		return success && (attrData.dwFileAttributes != INVALID_FILE_ATTRIBUTES) && !(attrData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	#else
		DWORD attribs = GetFileAttributesW(path.data());

		return (attribs != INVALID_FILE_ATTRIBUTES) &&
			!(attribs & FILE_ATTRIBUTE_DIRECTORY);
	#endif
#else

#endif
	}

}
