#pragma once
/**
 * projectfile.hpp
 * 
 * Functionality for creating and parsing .mrp & .mrc files
 */
#include "common.hpp"

void byteArrayPushInt8(QByteArray* array, uint8_t num);
void byteArrayPushInt16(QByteArray* array, uint16_t num);
void byteArrayPushInt32(QByteArray* array, uint32_t num);
void byteArrayPushInt64(QByteArray* array, uint64_t num);

void byteArraySetInt8(QByteArray* array, qsizetype idx, uint8_t num);
void byteArraySetInt16(QByteArray* array, qsizetype idx, uint16_t num);
void byteArraySetInt32(QByteArray* array, qsizetype idx, uint32_t num);
void byteArraySetInt64(QByteArray* array, qsizetype idx, uint64_t num);

void byteArrayPushStr(QByteArray* array, QString str, qsizetype maxLen);
void byteArrayPushStr(QByteArray* array, const char* str, qsizetype strLen, qsizetype maxLen);
void byteArraySetStr(QByteArray* array, qsizetype idx, QString str, qsizetype maxLen);
void byteArraySetStr(QByteArray* array, qsizetype idx, const char* str, qsizetype strLen, qsizetype maxLen);

/// @brief Class representing a file table in an MRC file.
class FileTable {
private:
	typedef uint32_t FileHashType;
	typedef uint64_t FileElementType;
	const static FileHashType hashFNVPrime = 16777619U;
	const static size_t defaultCapacity = 4;

public:
	FileTable();
	FileTable(size_t capacity);
	FileTable(FileTable&& other) noexcept;
	FileTable(const FileTable& other);
	~FileTable();

	void insert(char* name, size_t len, FileElementType data, bool group);
	size_t size() const;
	size_t byteSize() const;
	bool isEmpty() const;
	QByteArray toBytes();

	friend void swap(FileTable& lhs, FileTable& rhs);
	FileTable& operator=(FileTable& other);
	FileTable& operator=(FileTable&& other) noexcept;

private:
	void resize(size_t newCapacity);
	static FileHashType hash(char* name, size_t len);
	static inline uint8_t h1(uint8_t h);
	static inline uint8_t h2(uint8_t h);
	static inline bool ctrlIsEmpty(uint8_t h);

	/// @brief Struct representing a single file in a file table.
	struct FileNode {
		bool group = false;
		char name[32] = "\0";
		FileElementType data = 0;
	};

	uint8_t* m_ctrl = nullptr;
	FileNode* m_nodes = nullptr;
	size_t m_capacity = 0;
	size_t m_length = 0;
	bool m_dirty = true;
	QByteArray m_byteArray;

public:
	/// @brief Iterator for elements in a file table.
	class iterator {
	private:
		FileTable* m_table;
		size_t m_idx;
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = FileNode;
		using pointer = FileNode*;
		using reference = FileNode&;
		using iterator_category = std::forward_iterator_tag;

		iterator(FileTable* table, size_t idx);
		iterator& operator++();
		iterator operator++(int);
		reference operator*();
		pointer operator->();

		friend bool operator==(const iterator& lhs, const iterator& rhs) {
			return lhs.m_table == rhs.m_table && lhs.m_idx == rhs.m_idx;
		}

		friend bool operator!=(const iterator& lhs, const iterator& rhs) {
			return !(lhs == rhs);
		}
	};

	iterator begin();
	iterator end();
};

/// @brief Class representing a data block in an MRC file.
class DataBlock {
public:
	DataBlock(AssetTreeNode* asset = nullptr);

	AssetTreeNode* asset();
	void setAsset(AssetTreeNode* asset);
	QByteArray toBytes(bool useCompression, QString cipherMethod);

private:
	AssetTreeNode* m_asset = nullptr;
	bool m_dirty = true;
	QByteArray m_byteArray;
};

/// @brief Class for handing project file serialization/deserialization.
class ProjectFile {
public:
	/// @brief Create a new blank project file.
	/// @param filename Project file
	/// @return True if created successfully
	static bool createFile(const QString& filename);

	/// @brief Read the project file into the given info struct.
	/// @param info Destination info struct
	/// @return True if read successfully
	static bool readFile(ProjectFileInfo* info);

	/// @brief Write the project file in the given info struct to disk.
	/// @param info Info struct
	/// @return True if written successfully
	static bool writeFile(const ProjectFileInfo& info);

	/// @brief Convert the contents of the project file to a binary MRC file and write it to disk.
	/// @param info Info struct
	/// @param exportFilename Output file
	/// @return True if written successfully
	static bool exportFile(const ProjectFileInfo& info, const QString& exportFilename);

private:
	/// @brief Wrapper for adding a text element to a Qt document object.
	/// @param doc Root document object
	/// @param node Parent document node
	/// @param tag Tag name
	/// @param value Tag value (optional)
	/// @return Created element
	static QDomElement addElement(QDomDocument& doc, QDomNode& node, const QString& tag, const QString& value = "");
};