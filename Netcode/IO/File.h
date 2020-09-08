#pragma once

#include <NetcodeFoundation/ArrayView.hpp>

#include <memory>
#include <string>

namespace Netcode::IO {

	enum class FileOpenMode : uint32_t {
		READ_ONLY = 0x0001,
		WRITE_ONLY = 0x0002,
		CREATE_OR_OVERWRITE = 0x1000,
		OPEN = 0x2000
	};

	FileOpenMode operator|(FileOpenMode lhs, FileOpenMode rhs);
	FileOpenMode operator&(FileOpenMode lhs, FileOpenMode rhs);
	bool operator!=(FileOpenMode lhs, uint32_t value);

	class File {
		struct detail;
		std::unique_ptr<detail> impl;

	public:
		File(std::wstring_view parentDirectory, std::wstring relativePath);
		File(std::wstring fullPath);
		File & operator=(File &&) noexcept = default;
		File(File &&) noexcept = default;
		~File();

		File(const File &) = delete;
		File & operator=(const File &) = delete;

		void Open(FileOpenMode mode);

		size_t Read(MutableArrayView<uint8_t> buffer);

		size_t Write(ArrayView<uint8_t> buffer);

		void Close();

		size_t GetSize() const;

		const std::wstring & GetFullPath() const noexcept;

		std::wstring_view GetFullName() const noexcept;

		std::wstring_view GetName() const noexcept;

		std::wstring_view GetExtension() const noexcept;

		std::wstring_view GetParentDirectory() const noexcept;

		static bool Exists(const std::wstring & path) noexcept;
	};

	template<typename T>
	class FileReader {
		T & tRef;

	public:
		FileReader(T & tr) : tRef{ tr } { tr.Open(FileOpenMode::READ_ONLY | FileOpenMode::OPEN); }
		~FileReader() { tRef.Close(); }

		T * operator->() {
			return &tRef;
		}
	};

	template<typename T>
	class FileWriter {
		T & tRef;
	public:
		FileWriter(T & tr, FileOpenMode openMode = FileOpenMode::OPEN) : tRef{ tr } {
			tRef.Open(FileOpenMode::WRITE_ONLY);
		}
		~FileWriter() { tRef.Close(); }

		T * operator->() {
			return &tRef;
		}
	};

}
