#pragma once
/**
 * projectfile.hpp
 * 
 * Functionality for creating and parsing .mrp & .mrc files
 */
#include "common.hpp"

extern const QStringList cipherMethods;

class AssetTreeNode {
public:
	AssetTreeNode();
	~AssetTreeNode();
private:
	std::vector<AssetTreeNode*> children;
};

struct ProjectFileInfo {
	AssetTreeNode* m_assets;
	QString m_cipherMethod;
	bool m_useCompression;
};

class ProjectFile {
public:
	static bool createFile(const QString& filename);

	static ProjectFileInfo loadFile(const QString& filename);

	static void saveFile(const QString& filename, const ProjectFileInfo& assets);
};