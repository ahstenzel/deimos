#include "projectfile.hpp"

void byteArrayPushInt8(QByteArray* array, uint8_t num) {
	array->push_back((char)num);
}

void byteArrayPushInt16(QByteArray* array, uint16_t num) {
	array->push_back((char)(num & 0xFF));
	array->push_back((char)((num >> 8) & 0xFF));
}

void byteArrayPushInt32(QByteArray* array, uint32_t num) {
	array->push_back((char)(num & 0xFF));
	array->push_back((char)((num >> 8) & 0xFF));
	array->push_back((char)((num >> 16) & 0xFF));
	array->push_back((char)((num >> 24) & 0xFF));
}

void byteArrayPushInt64(QByteArray* array, uint64_t num) {
	array->push_back((char)(num & 0xFF));
	array->push_back((char)((num >> 8) & 0xFF));
	array->push_back((char)((num >> 16) & 0xFF));
	array->push_back((char)((num >> 24) & 0xFF));
	array->push_back((char)((num >> 32) & 0xFF));
	array->push_back((char)((num >> 40) & 0xFF));
	array->push_back((char)((num >> 48) & 0xFF));
	array->push_back((char)((num >> 56) & 0xFF));
}

void byteArraySetInt8(QByteArray* array, qsizetype idx, uint8_t num) {
	(*array)[idx] = (char)(num);
}

void byteArraySetInt16(QByteArray* array, qsizetype idx, uint16_t num) {
	(*array)[idx] = (char)(num & 0xFF);
	(*array)[idx + 1] = (char)((num >> 8) & 0xFF);
}

void byteArraySetInt32(QByteArray* array, qsizetype idx, uint32_t num) {
	(*array)[idx] = (char)(num & 0xFF);
	(*array)[idx + 1] = (char)((num >> 8) & 0xFF);
	(*array)[idx + 2] = (char)((num >> 16) & 0xFF);
	(*array)[idx + 3] = (char)((num >> 24) & 0xFF);
}

void byteArraySetInt64(QByteArray* array, qsizetype idx, uint64_t num) {
	(*array)[idx] = (char)(num & 0xFF);
	(*array)[idx + 1] = (char)((num >> 8) & 0xFF);
	(*array)[idx + 2] = (char)((num >> 16) & 0xFF);
	(*array)[idx + 3] = (char)((num >> 24) & 0xFF);
	(*array)[idx + 4] = (char)((num >> 32) & 0xFF);
	(*array)[idx + 5] = (char)((num >> 40) & 0xFF);
	(*array)[idx + 6] = (char)((num >> 48) & 0xFF);
	(*array)[idx + 7] = (char)((num >> 56) & 0xFF);
}

void byteArrayPushStr(QByteArray *array, QString str, qsizetype len) {
	str.truncate(len);
	array->push_back(str.toStdString().data());
	for(qsizetype i = str.size(); i < len; ++i) {
		array->push_back(char(0));
	}
}

void byteArraySetStr(QByteArray *array, qsizetype idx, QString str, qsizetype len) {
	str.truncate(len);
	std::string stdStr = str.toStdString();

	array->push_back(str.toStdString().data());
	for(qsizetype i = str.size(); i < len; ++i) {
		array->push_back(char(0));
	}

	for(qsizetype i = 0; i < len; ++i) {
		if (i < stdStr.size()) {
			(*array)[idx + i] = stdStr[i];
		} else {
			(*array)[idx + i] = char(0);
		}
	}
}

QDomElement ProjectFile::addElement(QDomDocument& doc, QDomNode& node, const QString& tag, const QString& value) {
	QDomElement elem = doc.createElement(tag);
	node.appendChild(elem);
	if (!value.isEmpty()) {
		QDomText txt = doc.createTextNode(value);
		elem.appendChild(txt);
	}
	return elem;
}

bool ProjectFile::createFile(const QString& filename) {
	// Create new file
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) { return false; }

	// Populate with default structure
	QDomDocument doc("mrp");
	doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
	QDomElement root = addElement(doc, doc, "mrp");
	root.setAttribute("version", DEIMOS_VERSION_STR);
	addElement(doc, root, "compression", "false");
	addElement(doc, root, "cipher", cipherMethods[0]);
	file.write(doc.toByteArray());
	file.close();
	return true;
}

bool ProjectFile::readFile(ProjectFileInfo* info) {
	if (!info) { return false; }
	try {
		// Load file
		QFile file(info->m_filename);
		if (!file.exists()) {
			throw std::exception("File does not exist");
		}
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { 
			throw std::exception("Failed to open file");
		}
		QDomDocument doc("mrp");
		if (!doc.setContent(&file)) {
			file.close();
			throw std::exception("Failed to read file contents");
		}
		file.close();

		// Verify contents
		QDomElement root = doc.documentElement();
		if (root.attribute("version") != DEIMOS_VERSION_STR) {
			throw std::exception("Version does not match");
		}
		if (root.tagName() != "mrp" || doc.doctype().name() != "mrp") {
			throw std::exception("Document type is not a valid mrp file");
		}

		// Read contents
		QDomElement elemTop = root.firstChildElement();
		for(; !elemTop.isNull(); elemTop = elemTop.nextSiblingElement()) {
			// Read top level elements
			if (!elemTop.isNull()) {
				if (elemTop.tagName() == "compression") {
					info->m_useCompression = (elemTop.text() == "true");
				} else if (elemTop.tagName() == "cipher") {
					info->m_cipherMethod = elemTop.text();
				} else if (elemTop.tagName() == "asset") {
					// Iterate through file tree
					typedef QPair<AssetTreeNode*, QDomElement> AssetElementPair;
					QList<AssetElementPair> assetElementList;
					assetElementList.push_back(AssetElementPair::pair(nullptr, elemTop));
					while(!assetElementList.isEmpty()) {
						// Extract asset & element nodes
						AssetElementPair pair = assetElementList.front();
						assetElementList.pop_front();
						AssetTreeNode* assetCurrent = pair.first;
						QDomElement elemCurrent = pair.second;

						// Populate asset info
						AssetTreeNode* assetNew = new AssetTreeNode();
						AssetInfo assetInfo;
						assetInfo.title = elemCurrent.attribute("title");
						if (elemCurrent.attribute("group", "false") == "true") {
							// Group
							QDomElement elemChild = elemCurrent.firstChildElement();
							for(; !elemChild.isNull(); elemChild = elemChild.nextSiblingElement()) {
								assetElementList.push_back(AssetElementPair::pair(assetNew, elemChild));
							}
						} else {
							// Asset
							assetInfo.type = elemCurrent.attribute("type");
							assetInfo.filename = elemCurrent.text();
						}
						assetNew->setAssetInfo(assetInfo);

						// Add to asset tree
						if (!assetCurrent) {
							info->m_assetTree = assetNew;
						} else {
							assetCurrent->addChild(assetNew);
						}
					}
				} else {
					throw std::exception("Invalid token");
				}
			}
		}
	} catch (std::exception& e) {
		delete info->m_assetTree;
		info->m_assetTree = nullptr;
		QMessageBox::warning(nullptr, "Error", QString("Error loading project file (%0):\n%1").arg(info->m_filename).arg(e.what()));
		return false;
	}
	return true;
}

bool ProjectFile::writeFile(const ProjectFileInfo& info) {
	// Create new file
	QFile file(info.m_filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) { return false; }

	// Populate top level info
	QDomDocument doc("mrp");
	doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
	QDomElement elemTop = addElement(doc, doc, "mrp");
	elemTop.setAttribute("version", DEIMOS_VERSION_STR);
	addElement(doc, elemTop, "compression", info.m_useCompression ? "true" : "false");
	addElement(doc, elemTop, "cipher", info.m_cipherMethod);

	// Iterate through asset tree
	AssetTreeNode* assetRoot = info.m_assetTree;
	typedef QPair<AssetTreeNode*, QDomElement> AssetElementPair;
	QList<AssetElementPair> assetElementList;
	assetElementList.push_back(AssetElementPair::pair(assetRoot, elemTop));
	while(!assetElementList.isEmpty()) {
		// Extract asset & element nodes
		AssetElementPair pair = assetElementList.front();
		assetElementList.pop_front();
		AssetTreeNode* assetCurrent = pair.first;
		QDomElement elemCurrent = pair.second;
		AssetInfo assetInfo = assetCurrent->assetInfo();

		// Add to file tree
		QDomElement elemChild = addElement(doc, elemCurrent, "asset");
		elemChild.setAttribute("title", assetInfo.title);
		if (assetCurrent->isGroup()) {
			// Group
			elemChild.setAttribute("group", "true");
			for(auto child : *assetCurrent) {
				assetElementList.push_back(AssetElementPair::pair(child, elemChild));
			}
		} else {
			// Asset
			elemChild.setAttribute("type", assetInfo.type);
			QDomText elemChildTxt = doc.createTextNode(assetInfo.filename);
			elemChild.appendChild(elemChildTxt);
		}
	}

	file.write(doc.toByteArray());
	file.close();
	return true;
}

bool ProjectFile::exportFile(const ProjectFileInfo& info, const QString& exportFilename) {
	QByteArray byte;
	try {
		// ========================================== Write header
		// Signature
		byte.push_back("MARS");

		// Version
		byteArrayPushInt8(&byte, DEIMOS_VERSION_MAJOR);
		byteArrayPushInt8(&byte, DEIMOS_VERSION_MINOR);
		byteArrayPushInt8(&byte, DEIMOS_VERSION_PATCH);
		byteArrayPushInt8(&byte, 0);

		// Offsets
		uint64_t rootFileTableOffset = 24;
		byteArrayPushInt64(&byte, rootFileTableOffset);

		// ========================================== File table
		QVector<FileTable> fileTables;
		QVector<DataBlock> dataBlocks;
		QList<AssetTreeNode*> assetList;
		assetList.push_back(info.m_assetTree);
		uint64_t fileTreeIdx = 1;
		uint64_t dataBlockIdx = 0;
		while(!assetList.isEmpty()) {
			AssetTreeNode* assetCurrent = assetList.front();
			assetList.pop_front();

			// Add children
			FileTable table;
			for(auto child : *assetCurrent) {
				// Groups set their offset pointer to the index of their child in the array of file tables
				// Assets set their offset pointer to the index of their data in the array of data blocks
				AssetInfo childInfo = child->assetInfo();
				if (childInfo.type.isEmpty()) {
					// Group
					assetList.push_back(child);
					table.insert(childInfo.title.toStdString().data(), childInfo.title.size(), fileTreeIdx++, true);
				} else {
					// Asset
					dataBlocks.emplace_back(child);
					table.insert(childInfo.title.toStdString().data(), childInfo.title.size(), dataBlockIdx++, false);
				}
			}
			fileTables.push_back(std::move(table));
		}

		// Calculate offsets for each file table
		QVector<uint64_t> fileTableOffsets(fileTables.size());
		uint64_t fileTableAccum = rootFileTableOffset;
		for(qsizetype i = 0; i < fileTables.size(); ++i) {
			fileTableOffsets[i] = fileTableAccum;
			fileTableAccum += fileTables[i].byteSize();
		}

		// Calculate start of data blocks
		uint64_t firstDataBlockOffset = fileTableAccum;
		byteArrayPushInt64(&byte, firstDataBlockOffset);

		// Update offsets for children file tables
		for(qsizetype i = 0; i < fileTables.size(); ++i) {
			for(auto it = fileTables[i].begin(); it != fileTables[i].end(); ++it) {
				if (it.isGroup()) {
					uint64_t offset = fileTableOffsets[*it];
					*it = offset;
				}
			}
		}

		// ========================================== Data blocks
		QByteArray byteDataBlocks;

		// Iterate through all assets in all file tables
		uint64_t dataBlockAccum = firstDataBlockOffset;
		for (qsizetype i = 0; i < fileTables.size(); ++i) {
			for (auto it = fileTables[i].begin(); it != fileTables[i].end(); ++it) {
				if (!it.isGroup()) {
					// Convert asset to binary
					DataBlock block = dataBlocks[*it];
					QByteArray byteBlock = block.toBytes();

					// Update offset of block in file table
					*it = dataBlockAccum;
					dataBlockAccum += byteBlock.size();
					byteDataBlocks.push_back(byteBlock);
				}
			}
		}

		// ========================================== Final assembly
		// Write file tables
		for (qsizetype i = 0; i < fileTables.size(); ++i) {
			byte.push_back(fileTables[i].toBytes());
		}

		// Write data blocks
		byte.push_back(byteDataBlocks);

	} catch (std::exception& e) {
		QMessageBox::warning(nullptr, "Error", QString("Error exporting resource file (%0):\n%1").arg(exportFilename).arg(e.what()));
		return false;
	}

	// Write to file
	QFile file(exportFilename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) { return false; }
	file.write(byte);
	file.close();
	return true;
}

FileTable::FileTable() {}

FileTable::FileTable(size_t capacity) {
	resize(capacity);
}

FileTable::FileTable(FileTable&& other) noexcept {
	m_capacity = std::move(other.m_capacity);
	m_length = std::move(other.m_length);
	m_loadCount = std::move(other.m_loadCount);
	m_dirty = std::move(other.m_dirty);
	m_byteArray = std::move(other.m_byteArray);
	m_ctrl = new uint8_t[m_capacity];
	m_nodes = new FileNode[m_capacity];
	for (size_t i = 0; i < m_capacity; ++i) {
		m_ctrl[i] = std::move(other.m_ctrl[i]);
		m_nodes[i] = std::move(other.m_nodes[i]);
	}
}

FileTable::FileTable(const FileTable& other) {
	m_capacity = other.m_capacity;
	m_length = other.m_length;
	m_loadCount = other.m_loadCount;
	m_dirty = other.m_dirty;
	m_byteArray = other.m_byteArray;
	m_ctrl = new uint8_t[m_capacity];
	m_nodes = new FileNode[m_capacity];
	for (size_t i = 0; i < m_capacity; ++i) {
		m_ctrl[i] = other.m_ctrl[i];
		m_nodes[i] = other.m_nodes[i];
	}
}

FileTable::~FileTable() {
	delete[] m_ctrl;
	delete[] m_nodes;
}

void FileTable::insert(char* name, size_t len, FileElementType data, bool group) {
	// Error check
	if (!name) { return; }

	// Resize if needed
	if (m_capacity == 0) {
		resize(defaultCapacity);
	} else if (m_loadCount / (float)m_capacity >= 0.875f) {
		resize(m_capacity * 2);
	}

	// Hash the key
	FileHashType h = hash(name, len);
	size_t pos = h1(h) & (m_capacity - 1);

	// Linear probe to find an empty bucket
	while (1) {
		uint8_t* ctrl = &m_ctrl[pos];
		if (ctrlIsEmpty(*ctrl)) {
			// Deep copy hash key & data
			FileNode* node = &m_nodes[pos];
			strncpy_s(node->name, name, len);
			node->data = data;
			node->group = group;

			// Save lower bits of hash to the control block
			FileHashType low = h2(h);
			memcpy_s(ctrl, 1, &low, 1);
			break;
		} else {
			pos = (pos + 1) & (m_capacity - 1);
		}
	}
	m_length++;
	m_loadCount++;
	m_dirty = true;
}

size_t FileTable::size() const {
	return m_length;
}

size_t FileTable::byteSize() const {
	return 12 + (41 * m_capacity);
}

bool FileTable::isEmpty() const {
	return (m_length == 0);
}

QByteArray FileTable::toBytes() {
	if (m_dirty) {
		m_dirty = false;
		m_byteArray.clear();

		// Write header
		m_byteArray.push_back("MRFT");
		byteArrayPushInt32(&m_byteArray, uint32_t(m_length));
		byteArrayPushInt32(&m_byteArray, uint32_t(m_capacity));

		// Write control block
		for(size_t i = 0; i < m_capacity; ++i) {
			byteArrayPushInt8(&m_byteArray, m_ctrl[i]);
		}

		// Write nodes
		for(size_t i = 0; i < m_capacity; ++i) {
			FileNode node = m_nodes[i];
			byteArrayPushStr(&m_byteArray, node.name, 32);
			byteArrayPushInt64(&m_byteArray, node.data);
		}
	}
	return m_byteArray;
}

void swap(FileTable& lhs, FileTable& rhs) {
	std::swap(lhs.m_capacity, rhs.m_capacity);
	std::swap(lhs.m_length, rhs.m_length);
	std::swap(lhs.m_loadCount, rhs.m_loadCount);
	std::swap(lhs.m_dirty, rhs.m_dirty);
	std::swap(lhs.m_byteArray, rhs.m_byteArray);
	std::swap(lhs.m_ctrl, rhs.m_ctrl);
	std::swap(lhs.m_nodes, rhs.m_nodes);
}

FileTable& FileTable::operator=(FileTable& other) {
	swap(*this, other);
	return *this;
}

FileTable& FileTable::operator=(FileTable&& other) noexcept {
	swap(*this, other);
	return *this;
}

void FileTable::resize(size_t newCapacity) {
	// Create new buffers
	uint8_t* newCtrl = new uint8_t[newCapacity];
	FileNode* newNodes = new FileNode[newCapacity];
	if (!newCtrl || !newNodes) { return; }

	// Zero out new buffers
	memset(newCtrl, 0x80, sizeof(*newCtrl) * newCapacity);
	memset(newNodes, 0, sizeof(*newNodes) * newCapacity);

	// Overwrite old buffers
	uint8_t* oldCtrl = m_ctrl;
	FileNode* oldNodes = m_nodes;
	size_t oldCapacity = m_capacity;
	m_ctrl = newCtrl;
	m_nodes = newNodes;
	m_capacity = newCapacity;
	m_loadCount = 0;
	m_length = 0;

	// Move over old contents
	for (size_t i = 0; i < oldCapacity; ++i) {
		uint8_t ctrl = oldCtrl[i];
		if (!ctrlIsEmpty(ctrl)) {
			FileNode node = oldNodes[i];
			insert(&node.name[0], sizeof(node.name), node.data, node.group);
		}
	}
	delete[] oldCtrl;
	delete[] oldNodes;
}

FileTable::FileHashType FileTable::hash(char *name, size_t len) {
	uint8_t mask_size = sizeof(FileHashType) - 4;
	FileHashType hash = hashFNVPrime;
	FileHashType mask = 0xF << mask_size;
	for(size_t i = 0; i < len; ++i) {
		hash = (hash << 4) + (FileHashType)name[i];
		FileHashType g = hash & mask;
		if (g != 0) { hash ^= g >> mask_size; }
		hash &= ~g;
	}
	return hash;
}

inline uint8_t FileTable::h1(uint8_t h) {
	return h >> 7;
}

inline uint8_t FileTable::h2(uint8_t h) {
	return h & 0x7F;
}

inline bool FileTable::ctrlIsEmpty(uint8_t h) {
	return h & 0x80;
}

inline bool FileTable::ctrlIsDeleted(uint8_t h) {
	return h & 0xFE;
}

FileTable::iterator FileTable::begin() {
	iterator it(this, std::numeric_limits<size_t>::max());
	return ++it;
}

FileTable::iterator FileTable::end() {
	return iterator(this, m_capacity);
}

FileTable::iterator::iterator(FileTable *table, size_t idx) : 
	m_table(table), m_idx(idx) {}

FileTable::iterator &FileTable::iterator::operator++() {
	// Find next valid block
	if (m_idx == std::numeric_limits<size_t>::max()) { m_idx = 0; }
	else { ++m_idx; }
	while(m_idx < m_table->m_capacity) {
		uint8_t ctrl = m_table->m_ctrl[m_idx];
		if (!FileTable::ctrlIsEmpty(ctrl)) { break; }
		++m_idx;
	}
	return *this;
}

FileTable::iterator FileTable::iterator::operator++(int) {
	iterator tmp = *this;
	++(*this);
	return tmp;
}

FileTable::FileElementType& FileTable::iterator::operator*() {
	return m_table->m_nodes[m_idx].data;
}

FileTable::FileElementType *FileTable::iterator::operator->() {
	return &m_table->m_nodes[m_idx].data;
}

bool FileTable::iterator::isGroup() const {
	return m_table->m_nodes[m_idx].group;
}

DataBlock::DataBlock(AssetTreeNode *asset) :
	m_asset(asset) {}

AssetTreeNode* DataBlock::asset() {
	return m_asset;
}

void DataBlock::setAsset(AssetTreeNode* asset) {
	m_asset = asset;
	m_dirty = true;
}

QByteArray DataBlock::toBytes() {
	if (m_dirty && m_asset) {
		m_dirty = false;
		m_byteArray.clear();
		AssetInfo info = m_asset->assetInfo();

		// ========================================== Write header
		// Type code
		QString typeCode = "";
		for(auto type : assetTypes) {
			if (type.first == info.type) {
				AssetTypeDescriptor desc = type.second;
				typeCode = desc.code;
				byteArrayPushStr(&m_byteArray, typeCode, 4);
				break;
			}
		}
		if (typeCode.isEmpty()) {
			m_byteArray.clear();
			return m_byteArray;
		}

		// Get file contents
		QFile assetFile(info.filename);
		if (!assetFile.open(QIODevice::ReadOnly)) {
			m_byteArray.clear();
			return m_byteArray;
		}
		QByteArray fileBytes = assetFile.read(assetFile.size());
		assetFile.close();
		
		// Metadata
		byteArrayPushInt32(&m_byteArray, 0);	// CRC
		byteArrayPushInt64(&m_byteArray, 0);	// Next chunk
		byteArrayPushInt64(&m_byteArray, 0);	// Previous chunk
		byteArrayPushInt64(&m_byteArray, fileBytes.size());	// Packed size
		byteArrayPushInt64(&m_byteArray, fileBytes.size());	// Unpacked size
		byteArrayPushStr(&m_byteArray, info.title, 32);

		// ========================================== Write contents
		m_byteArray.push_back(fileBytes);
	}
	return m_byteArray;
}

qsizetype DataBlock::byteSize() {
	if (!m_asset) { return 0; }
	qsizetype size = 72;

	// Get file size
	AssetInfo info = m_asset->assetInfo();
	QFile assetFile(info.filename);
	size += assetFile.size();
	return size;
}
