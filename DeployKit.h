#ifndef DEPLOYKIT_H
#define DEPLOYKIT_H

#include <vector>

#include <QString>
#include <QStringList>

namespace KeyWords {
	const QString windployqtExe = "windeployqt.exe";
	const QString windployqtExeMarker = windployqtExe + ": ";

	const QString additionalFile = "additional file";
	const QString additionalFileMarker = additionalFile + ": ";
}

struct DeployKit
{
	QString windployFile;
	QStringList addFiles;

	static std::vector<DeployKit> FromText(QString text)
	{
		std::vector<DeployKit> ret;
		auto rows = text.split("\n", QString::SkipEmptyParts);

		for(auto &row:rows)
		{
			if(row.startsWith(KeyWords::windployqtExeMarker))
			{
				ret.push_back(DeployKit());
				ret.back().windployFile = row.right(row.size() - KeyWords::windployqtExeMarker.size());
			}
			else
			{
				if(!ret.empty())
				{
					if(row.startsWith(KeyWords::additionalFileMarker))
					{
						ret.back().addFiles += row.right(row.size() - KeyWords::additionalFileMarker.size());
					}
				}
			}
		}

		return ret;
	}
	static QString ToText(const std::vector<DeployKit> &kits)
	{
		QString ret;
		for(auto &kit:kits)
		{
			ret += kit.ToText();
			ret += "\n";
		}
		return ret;
	}
	QString ToText() const
	{
		QString ret = KeyWords::windployqtExeMarker + windployFile;
		ret += "\n";
		for(auto &addFile:addFiles)
		{
			ret += KeyWords::additionalFileMarker;
			ret += addFile;
			ret += "\n";
		}
		return ret;
	}
};

#endif // DEPLOYKIT_H
