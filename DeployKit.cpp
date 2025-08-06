#include "DeployKit.h"

#include <QMessageBox>

#include "MyQShortings.h"

KitElement KitElement::FromText(QString text)
{
	KitElement el;
	el.text = std::move(text);
	if(el.text.startsWith(KeyWords::windployqtExe))
	{
		el.type = KitElement::windeploy;
		el.fileInfo = QString(el.text).remove(0, KeyWords::windployqtExe.size() + 1);
	}
	else if(el.text.startsWith(KeyWords::additionalFile))
	{
		el.type = KitElement::additional;
		el.fileInfo = QString(el.text).remove(0, KeyWords::additionalFile.size() + 1);
	}

	return el;
}

std::vector<DeployKit> DeployKit::FromText(QString text, bool &ok)
{
	ok = false;
	std::vector<DeployKit> kitsVector;
	auto rows = text.split("\n", QString::SkipEmptyParts);

	DeployKit *currentKit = nullptr;
	bool namedKitNow = false;
	for(auto &row:rows)
	{
		if(row.startsWith(KeyWords::kit))
		{
			kitsVector.emplace_back();
			currentKit = &kitsVector.back();
			currentKit->name = QString(row).remove(0, KeyWords::kit.length() + 1);
			namedKitNow = true;
			continue;
		}
		else if(row.startsWith(KeyWords::end))
		{
			currentKit = nullptr;
			namedKitNow = false;
			continue;
		}

		if(!namedKitNow)
		{
			kitsVector.emplace_back();
			currentKit = &kitsVector.back();
			currentKit->name = KeyWords::unnamedKit;
		}

		if(!currentKit) { QMbc(nullptr, "Error loading kits", "Error loading kits: nullptr currentKit"); return {}; }

		KitElement kitElementOfRow = KitElement::FromText(row);
		if(kitElementOfRow.type == KitElement::undefined)
		{ QMbc(nullptr, "Error loading kits", "Error loading kits: undefined type kit element in row ["+row+"]"); return {}; }

		currentKit->elements.emplace_back(std::move(kitElementOfRow));
	}

	ok = true;
	return kitsVector;
}

QString DeployKit::ToText(const std::vector<DeployKit> & kits)
{
	QString ret;
	for(auto &kit:kits)
	{
		ret += kit.ToText();
		ret += "\n";
	}
	return ret;
}

QString DeployKit::ToText() const
{
	if(elements.empty()) { QMbc(nullptr, "Error", "elements.empty()"); return {}; }

	QString ret;
	for(auto &kitEl:elements)
	{
		ret += kitEl.text;
		ret += "\n";
	}

	if(name == KeyWords::unnamedKit)
	{
		if(elements.size() > 1) { QMbc(nullptr, "Error", "name == KeyWords::unnamedKit, but elements.size() > 1"); return {}; }
	}
	else
	{
		ret = KeyWords::kit + " " + name + "\n" + ret;
		ret = ret + KeyWords::end + "\n";
	}

	return ret;
}
