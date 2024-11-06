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