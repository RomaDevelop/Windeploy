#ifndef DEPLOYKIT_H
#define DEPLOYKIT_H

#include <vector>

#include <QFileInfo>
#include <QString>
#include <QStringList>

namespace KeyWords {
	const QString kit = "deploykit";
	const QString end = "end";

	const QString unnamedKit = "unnamedKit";

	const QString windployqtExe = "windeployqt.exe";

	const QString additionalFile = "additional_file";
}

struct KitElement
{
	enum { windeploy, additional, undefined };
	int type = undefined;
	QString text;
	QFileInfo fileInfo;

	static KitElement FromText(QString text);
};

struct DeployKit
{
	QString name;
	std::vector<KitElement> elements;

	static std::vector<DeployKit> FromText(QString text);
	static QString ToText(const std::vector<DeployKit> &kits);
	QString ToText() const;
};

#endif // DEPLOYKIT_H
