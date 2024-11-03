#include "projectfile.hpp"

const QStringList cipherMethods = {
	"none"
};

AssetTreeNode::AssetTreeNode() {

}

AssetTreeNode::~AssetTreeNode() {

}

bool ProjectFile::createFile(const QString &filename) {
	// Create new file
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) { return false; }

	// Open XML streamer
	QXmlStreamWriter xml(&file);
	xml.setAutoFormatting(true);
	xml.writeStartDocument();
	xml.writeDTD("<!DOCTYPE mrp>");

	// Write blank file info
	xml.writeStartElement("mrp");
	xml.writeAttribute("version", DEIMOS_VERSION_STR);
	xml.writeTextElement("compression", "false");
	xml.writeTextElement("cipher", cipherMethods[0]);
	xml.writeEndElement();

	// Close file
	xml.writeEndDocument();
	file.close();
	return true;
}

ProjectFileInfo ProjectFile::loadFile(const QString &filename) {
	ProjectFileInfo info;
	info.m_assets = nullptr;
	info.m_cipherMethod = cipherMethods[0];
	info.m_useCompression = false;

	// Load file
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { return info; }
	QXmlStreamReader xml(&file);

	// Verify file
	if (xml.readNextStartElement()) {
		if (xml.name() == "mrp" && xml.attributes().value("version") == "1.0.0") {
			while(xml.readNextStartElement()) {
				if (xml.name() == "cipher") {
					info.m_cipherMethod = xml.readElementText();
				} else if (xml.name() == "compression") {
					info.m_useCompression = (xml.readElementText() == "true");
				} else {
					xml.skipCurrentElement();
				}
			}
		} else {
			xml.raiseError("File is not an MRP version 1.0.0 file.");
		}
	}

	file.close();
	if (xml.error()) { delete info.m_assets; }
	return info;
}

void ProjectFile::saveFile(const QString &filename, const ProjectFileInfo &assets) {
	
}