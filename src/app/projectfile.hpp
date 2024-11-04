#pragma once
/**
 * projectfile.hpp
 * 
 * Functionality for creating and parsing .mrp & .mrc files
 */
#include "common.hpp"

class ProjectFile {
public:
	static bool createFile(const QString& filename);

	static bool readFile(ProjectFileInfo* info);

	static bool writeFile(const ProjectFileInfo& info);

private:
	static QDomElement addElement(QDomDocument& doc, QDomNode& node, const QString& tag, const QString& value = "");
};