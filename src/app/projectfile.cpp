#include "projectfile.hpp"

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
				} else if (elemTop.tagName() == "assets") {
					// Read asset tree
					typedef QPair<AssetTreeNode*, QDomElement> AssetElementPair;
					QList<AssetElementPair> assetElementList;
					info->m_assetTree = new AssetTreeNode();

					// Add top level elements
					QDomElement elemTopAssets = elemTop.firstChildElement();
					for(; !elemTopAssets.isNull(); elemTopAssets = elemTopAssets.nextSiblingElement()) {
						assetElementList.push_back(AssetElementPair::pair(info->m_assetTree, elemTopAssets));
					}

					// Iterate through children
					while(!assetElementList.isEmpty()) {
						// Extract asset & element nodes
						AssetElementPair pair = assetElementList.front();
						assetElementList.pop_front();
						AssetTreeNode* assetCurrent = pair.first;
						QDomElement elemCurrent = pair.second;

						// Add to asset tree
						if (elemCurrent.tagName() == "asset") {
							// Asset
							QDomElement elemAssetTitle = elemCurrent.firstChildElement("title");
							QDomElement elemAssetType = elemCurrent.firstChildElement("type");
							QDomElement elemAssetFilename = elemCurrent.firstChildElement("filename");
							if (elemAssetFilename.isNull() || elemAssetType.isNull() || elemAssetFilename.isNull()) {
								throw std::exception("Malformed asset tags");
							}
							AssetInfo assetInfo;
							assetInfo.title = elemAssetTitle.text();
							assetInfo.type = elemAssetType.text();
							assetInfo.filename = elemAssetFilename.text();
							assetCurrent->addChild(assetInfo);
						}
						if (elemCurrent.tagName() == "group") {
							// Group
							QDomElement elemGroupTitle = elemCurrent.firstChildElement("title");
							QDomElement elemGroupContents = elemCurrent.firstChildElement("contents");
							if (elemGroupTitle.isNull() || elemGroupContents.isNull()) {
								throw std::exception("Malformed group tags");
							}
							AssetInfo assetInfo;
							assetInfo.title = elemGroupTitle.text();
							AssetTreeNode* assetNew = assetCurrent->addChild(assetInfo);

							// Add children elements
							QDomElement elemChild = elemGroupContents.firstChildElement();
							for(; !elemChild.isNull(); elemChild = elemChild.nextSiblingElement()) {
								assetElementList.push_back(AssetElementPair::pair(assetNew, elemChild));
							}
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
	QDomElement elemTopAssets = addElement(doc, elemTop, "assets");

	// Write top level assets
	if (info.m_assetTree) {
		AssetTreeNode* assetRoot = info.m_assetTree;
		typedef QPair<AssetTreeNode*, QDomElement> AssetElementPair;
		QList<AssetElementPair> assetElementList;
		for(auto child : *assetRoot) {
			assetElementList.push_back(AssetElementPair::pair(child, elemTopAssets));
		}

		// Iterate through asset tree, writing children
		while(!assetElementList.isEmpty()) {
			// Extract asset & element nodes
			AssetElementPair pair = assetElementList.front();
			assetElementList.pop_front();
			AssetTreeNode* assetCurrent = pair.first;
			QDomElement elemCurrent = pair.second;
			AssetInfo assetInfo = assetCurrent->assetInfo();

			// Add to document
			if (assetCurrent->isGroup()) {
				// Group
				QDomElement elemGroup = addElement(doc, elemCurrent, "group");
				addElement(doc, elemGroup, "title", assetInfo.title);
				QDomElement elemGroupContents = addElement(doc, elemGroup, "contents");

				// Add children
				for(auto child : *assetCurrent) {
					assetElementList.push_back(AssetElementPair::pair(child, elemGroupContents));
				}
			} else {
				// Asset
				QDomElement elemAsset = addElement(doc, elemCurrent, "asset");
				addElement(doc, elemAsset, "title", assetInfo.title);
				addElement(doc, elemAsset, "type", assetInfo.type);
				addElement(doc, elemAsset, "filename", assetInfo.filename);
			}
		}
	}

	file.write(doc.toByteArray());
	file.close();
	return true;
}

/*
bool ProjectFile::saveFile(const QString &filename, const ProjectFileInfo &info) {
	// Create new file
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) { return false; }

	// Open XML streamer
	QXmlStreamWriter xml(&file);
	xml.setAutoFormatting(true);
	xml.writeStartDocument();
	xml.writeDTD("<!DOCTYPE mrp>");

	// Write blank file info
	xml.writeStartElement("mrp");
	xml.writeAttribute("version", DEIMOS_VERSION_STR);
	xml.writeTextElement("compression", (info.m_useCompression) ? "true" : "false");
	xml.writeTextElement("cipher", info.m_cipherMethod);

	// Write assets
	xml.writeStartElement("assets");
	if (info.m_assetTree) {
		for(auto child = info.m_assetTree->begin(); child != info.m_assetTree->end(); ++child) {
			traverseAssetTree(&xml, *child);
		}
	}
	xml.writeEndElement(); // assets
	xml.writeEndElement(); // mrp

	// Close file
	xml.writeEndDocument();
	file.close();
	return true;
}

void ProjectFile::traverseAssetTree(QXmlStreamWriter *xml, AssetTreeNode *node) {
	if (!node || !xml) { return; }

	AssetInfo info = node->assetInfo();
	xml->writeStartElement(info.title);
	if (node->childCount() > 0) {
		// Group
		xml->writeAttribute("group", "true");
		for(auto child = node->begin(); child != node->end(); ++child) {
			traverseAssetTree(xml, *child);
		}

		// End of group
		xml->writeEndElement();
		xml->writeStartElement("NULL");
		xml->writeAttribute("end", "true");
	} else {
		// Asset
		xml->writeAttribute("group", "false");
		xml->writeTextElement("type", info.type);
		xml->writeTextElement("filename", info.filename);
	}
	xml->writeEndElement(); // info.title
}
*/