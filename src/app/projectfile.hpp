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

void byteArrayPushStr(QByteArray* array, QString str, qsizetype len);
void byteArraySetStr(QByteArray* array, qsizetype idx, QString str, qsizetype len);

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
	static inline bool ctrlIsDeleted(uint8_t h);

	struct FileNode {
		bool group = false;
		char name[32] = "\0";
		FileElementType data = 0;
	};

	uint8_t* m_ctrl = nullptr;
	FileNode* m_nodes = nullptr;
	size_t m_capacity = 0;
	size_t m_length = 0;
	size_t m_loadCount = 0;
	bool m_dirty = true;
	QByteArray m_byteArray;

public:
	class iterator {
	private:
		FileTable* m_table;
		size_t m_idx;
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = FileElementType;
		using pointer = const size_t*;
		using reference = const size_t&;
		using iterator_category = std::forward_iterator_tag;

		iterator(FileTable* table, size_t idx);
		iterator& operator++();
		iterator operator++(int);
		FileElementType& operator*();
		FileElementType* operator->();

		friend bool operator==(const iterator& lhs, const iterator& rhs) {
			return lhs.m_table == rhs.m_table && lhs.m_idx == rhs.m_idx;
		}

		friend bool operator!=(const iterator& lhs, const iterator& rhs) {
			return !(lhs == rhs);
		}

		bool isGroup() const;
	};

	iterator begin();
	iterator end();
};

class DataBlock {
public:
	DataBlock(AssetTreeNode* asset = nullptr);

	AssetTreeNode* asset();
	void setAsset(AssetTreeNode* asset);
	QByteArray toBytes();
	qsizetype byteSize();

private:
	AssetTreeNode* m_asset = nullptr;
	bool m_dirty = true;
	QByteArray m_byteArray;
};

class ProjectFile {
public:
	static bool createFile(const QString& filename);
	static bool readFile(ProjectFileInfo* info);
	static bool writeFile(const ProjectFileInfo& info);
	static bool exportFile(const ProjectFileInfo& info, const QString& exportFilename);

private:
	static QDomElement addElement(QDomDocument& doc, QDomNode& node, const QString& tag, const QString& value = "");
};