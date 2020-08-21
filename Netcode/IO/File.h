#pragma once

#include <NetcodeFoundation/ArrayView.hpp>

#include <memory>
#include <string>

namespace Netcode::IO {

	enum class FileOpenMode : uint32_t {
		READ, WRITE, READ_BINARY, WRITE_BINARY
	};

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

		bool Open(FileOpenMode mode);

		size_t Read(MutableArrayView<uint8_t> buffer);

		size_t Write(ArrayView<uint8_t> buffer);

		void Close();

		size_t GetSize() const;

		const std::wstring & GetFullPath() const;

		std::wstring_view GetFullName() const;

		std::wstring_view GetName() const;

		std::wstring_view GetExtension() const;

		std::wstring_view GetParentDirectory() const;

		static bool Exists(const std::wstring & path);
	};

	template<typename T>
	class FileReader {
		T & tRef;

	public:
		FileReader(T & tr, FileOpenMode mode) : tRef{ tr } { tr.Open(mode); }
		~FileReader() { tRef.Close(); }

		T * operator->() {
			return &tRef;
		}
	};

	template<typename T>
	class FileWriter {
		T & tRef;
	public:
		FileWriter(T & tr) : tRef{ tr } {
			tRef.Open(FileOpenMode::WRITE);
		}
		~FileWriter() { tRef.Close(); }

		T * operator->() {
			return &tRef;
		}
	};

}
