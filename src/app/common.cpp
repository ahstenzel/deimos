#include "common.hpp"

const QList<QPair<QString, AssetTypeDescriptor>> assetTypes = {
	{"Image", {
		"IMG_",
		{"png", "bmp", "tiff"}
	}},
	{"Text", {
		"TXT_",
		{"txt", "json", "ini", "xml", "hlsl"}
	}},
	{"Binary", {
		"BIN_",
		{"bin", "spv"}
	}},
};

const QStringList cipherMethods = {
	"none"
};

AssetTreeNode::AssetTreeNode() {}

AssetTreeNode::AssetTreeNode(const AssetInfo& assetInfo) :
	m_assetInfo(assetInfo) {}

AssetTreeNode::~AssetTreeNode() {
	for(auto child : m_children) {
		delete child;
	}
}

AssetTreeNode* AssetTreeNode::addChild() {
	AssetInfo defaultInfo;
	return addChild(defaultInfo);
}

AssetTreeNode* AssetTreeNode::addChild(const AssetInfo& assetInfo) {
	AssetTreeNode* child = new AssetTreeNode(assetInfo);
	return addChild(child);
}

AssetTreeNode* AssetTreeNode::addChild(AssetTreeNode* node) {
	m_children.push_back(node);
	return node;
}


void AssetTreeNode::removeChild(AssetTreeNode *child) {
	for(qsizetype i = 0; i < m_children.size(); ++i) {
		if (m_children[i] == child) {
			delete m_children[i];
			m_children.remove(i);
			return;
		}
	}
}

qsizetype AssetTreeNode::childCount() {
	return m_children.size();
}

QVector<AssetTreeNode *>::iterator AssetTreeNode::begin() {
	return m_children.begin();
}

QVector<AssetTreeNode *>::iterator AssetTreeNode::end() {
	return m_children.end();
}

QVector<AssetTreeNode *>::reverse_iterator AssetTreeNode::rbegin() {
	return m_children.rbegin();
}

QVector<AssetTreeNode *>::reverse_iterator AssetTreeNode::rend() {
	return m_children.rend();
}

bool AssetTreeNode::isGroup() {
	return m_children.size() > 0;
}

AssetInfo AssetTreeNode::assetInfo() {
	return m_assetInfo;
}

void AssetTreeNode::setAssetInfo(const AssetInfo &assetInfo) {
	m_assetInfo = assetInfo;
}
