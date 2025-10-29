#ifndef WINDEPLOY_H
#define WINDEPLOY_H

#include <QMainWindow>
#include <QRadioButton>
#include <QCheckBox>
#include <QFileInfo>
#include <QFileDialog>

#include "LaunchParams.h"
#include "ResourcesCreator.h"
#include "PlatformDependent.h"
#include "MyQExecute.h"
#include "AppDataWork.h"

#include "DeployKit.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Windeploy; }
QT_END_NAMESPACE

class Windeploy : public QMainWindow
{
	Q_OBJECT
	Ui::Windeploy *ui;
public:

	Windeploy(const QStringList &args, QWidget *parent = nullptr);
	~Windeploy();

	declare_struct_2_fields_move(MakeReleaseResult, bool, success, QString, pathToOutputExe);
	inline static MakeReleaseResult MakeRelease(const QString &programmName);

private:
	void WorkArgs(const QStringList &args);
	bool KitsToTable();

	std::vector<QRadioButton*> rBtns;
	std::vector<QCheckBox*> chBoxes;

	QString deployKitsStr;

	QString filesPath;
	QString settingsFile;

protected:
	void dragEnterEvent( QDragEnterEvent* event );
	void dropEvent( QDropEvent* event );

private slots:

	void on_pushButtonDeploy_clicked();

	void on_btnSelectFile_clicked();
	void on_pushButtonClear_clicked();

	void on_btnDeployKits_clicked();

};

Windeploy::MakeReleaseResult Windeploy::MakeRelease(const QString &programmName)
{
	MakeReleaseResult result;

#ifndef QT_NO_DEBUG
	QMbInfo("This finction works only at QT_NO_DEBUG mode"); return result;
#endif

	if(!LaunchParams::DevCompAndFromWorkFiles()) { QMbInfo("MakeRelease works only at developer PC"); return result; }

	QWidget w; w.move(100,100); w.show(); w.hide();
	// w требуется, потому что тупое окно диалога выводится посередине двух мониторов
	QString dirOutput = QFileDialog::getExistingDirectory(&w,"Выберите каталог где будет создан дистрибутив", "");

	if(dirOutput.isEmpty()) return result;
	dirOutput += "/release " + programmName + " " + QDateTime::currentDateTime().toString(DateTimeFormatForFileName);

	// если каталог выпуска уже существует - запрашиваем разрешение и  удаяем
	if(QFileInfo(dirOutput).isDir())
	{
		if(QMessageBox::question(0,"Dir already exists", "Dir " + dirOutput + " already exists, rewrite it")
				== QMessageBox::No)
			return result;
		if(!QDir(dirOutput).removeRecursively()) { QMbError("Can't removeRecursively dir "+dirOutput); return result; }
	}

	// создание подкаталогов
	result.pathToOutputExe = dirOutput + "/Files";
	QString exeFileOutput = result.pathToOutputExe + "/" + MyQDifferent::ExeNameNoPath();
	if(!QDir().mkdir(dirOutput)) { QMbError("Can't create dir "+dirOutput); return result; }
	if(!QDir().mkdir(result.pathToOutputExe)) { QMbError("Can't create dir "+result.pathToOutputExe); return result; }

	// копирование exe
	if(!QFile::copy(MyQDifferent::ExeNameWithPath(), exeFileOutput))
	{ QMbError("Can't copy " + MyQDifferent::ExeNameWithPath()); return result; }

	// копирование ресурсов
	QStringList dirsResourcesSrc;
	for(auto &resouresesInstance:BaseResources::existingResources)
		dirsResourcesSrc += resouresesInstance->ResourcesStoragePathOnCompile();
	QString resourcesCopyingRes;
	for(auto &dir:dirsResourcesSrc)
	{
		if(!MyQFileDir::CopyDirectory(dir, result.pathToOutputExe))
			resourcesCopyingRes += dir+"\n";
	}
	if(resourcesCopyingRes.isEmpty() == false)
	{
		resourcesCopyingRes.prepend("Can't copy resources directory(ies):\n\n");
		QMbError(resourcesCopyingRes);
		return result;
	}

	// copy link
	QString link = LaunchParams::SourcesPath() + "/ReleaseLink_"+MyQDifferent::ExeNameNoPath()+".lnk";
	if(QFileInfo(link).isFile())
	{
		if(!QFile::copy(link, dirOutput+"/Запуск.lnk")) QMbWarning("Can't copy link file " + link);
	}
	else QMbWarning("Link file "+link+" to copy not found");

	MyQExecute::OpenDir(dirOutput);

	// запуск Windeploy
	auto appDataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
	auto windeployExeFile = appDataPath + "/"+ADWN::RomaDevelop+"/"+ADWN::Windeploy+"/exe_path_name.txt";
	if(QFileInfo(windeployExeFile).isFile())
	{
		auto readRes = MyQFileDir::ReadFile2(windeployExeFile);
		if(!readRes.success) QMbError("error reading " + windeployExeFile);
		else
		{
			if(!QFileInfo(readRes.content).isFile()) QMbError("not found " + readRes.content);
			else
			{
				if(!MyQExecute::Execute(readRes.content, {exeFileOutput}))
					QMbError("error executing " + readRes.content);
			}
		}
	}
	else QMbWarning("not found " + windeployExeFile);

	result.success = true;
	return result;
}

//--------------------------------------------------------------------------------------------------------------------------

#endif // WINDEPLOY_H
