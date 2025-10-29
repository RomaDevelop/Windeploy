#include "windeploy.h"
#include "ui_windeploy.h"

#include <QMessageBox>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QSettings>
#include <QTimer>
#include <QRadioButton>

#include <fstream>

#include "MyQDialogs.h"
#include "MyQDifferent.h"
#include "MyQShortings.h"
#include "MyQFileDir.h"
#include "MyQExecute.h"
#include "AppDataWork.h"

bool CheckKirillic(const QString &fileOrDir)
{
	for(auto &c:fileOrDir)
	{
		if((c >= L'А' && c<=L'Я') || (c>=L'а' && c<=L'я'))
		{
			QMessageBox::critical(nullptr, "Ошибка", "Имя или путь файла\n\n["+fileOrDir+"]\n\nсодержит кириллицу!");
			return false;
		}
	}

	return true;
}

bool CheckFile(const QString &file)
{
	if(!CheckKirillic(file)) return false;

	auto fi = QFileInfo(file);
	if(!fi.isFile() || fi.suffix() != "exe")
	{
		QMessageBox::critical(nullptr, "Ошибка", "Объект\n\n["+file+"]\n\nне является исполняемым файлом!");
		return false;
	}

	return true;
}

Windeploy::Windeploy(const QStringList &args, QWidget *parent):
	QMainWindow(parent),
	ui(new Ui::Windeploy)
{
	ui->setupUi(this);

	setAcceptDrops(true);

	filesPath = MyQDifferent::PathToExe()+"/files";
	if(!QDir().mkpath(filesPath)) QMbError("mkpath error for "+filesPath);

	WorkArgs(args);

	AppDataWork::MakeFolderAndLinkInAppData(ADWN::RomaDevelop, ADWN::Windeploy);

	settingsFile = filesPath+"/settings.ini";
	QTimer::singleShot(0,this,[this]
	{
		QSettings settings(settingsFile, QSettings::IniFormat);
		if(settings.contains("deployKits"))
			deployKitsStr = settings.value("deployKits").toString();

		KitsToTable();
	});
}

Windeploy::~Windeploy()
{
	QDir().mkpath(MyQDifferent::PathToExe()+"/files");

	QSettings settings(settingsFile, QSettings::IniFormat);

	settings.setValue("deployKits", deployKitsStr);

	delete ui;
}

void Windeploy::WorkArgs(const QStringList &args)
{
	if(args.size() == 1) ;
	else if(args.size() > 2) QMbError("wrong argc ("+QSn(args.size())+")");
	else if(args.size() == 2)
	{
		QString arg = args[1];
		if(CheckFile(arg))
			ui->editFile->setText(arg);
		else QMbError("Некорректный аргумент ("+arg+")");
	}
}

bool Windeploy::KitsToTable()
{
	bool ok;
	std::vector<DeployKit> deployKits = DeployKit::FromText(deployKitsStr, ok);
	if(!ok) return false;

	ui->tableWidget->horizontalHeader()->hide();

	rBtns.clear();
	chBoxes.clear();

	ui->tableWidget->clear();
	ui->tableWidget->setColumnCount(1);
	ui->tableWidget->setRowCount(deployKits.size());
	for(uint i=0; i<deployKits.size(); i++ )
	{
		auto &kit = deployKits[i];

		auto w = new QWidget;

		auto vlo = new QVBoxLayout(w);
		auto hlo = new QHBoxLayout;
		vlo->setContentsMargins(0,0,0,0);
		hlo->setContentsMargins(0,0,0,0);

		auto rBtn = new QRadioButton(kit.name);
		rBtns.push_back(rBtn);
		hlo->addSpacing(10);
		if(kit.name != KeyWords::unnamedKit)
			hlo->addWidget(rBtn);
		vlo->addLayout(hlo);

		std::vector<QCheckBox*> chBoxesOfThisKit;
		for(auto &kitElement:kit.elements)
		{
			chBoxes.push_back(new QCheckBox(kitElement.text));
			chBoxesOfThisKit.push_back(chBoxes.back());
			auto hlo = new QHBoxLayout;
			hlo->setContentsMargins(0,0,0,0);
			hlo->addSpacing(30);
			hlo->addWidget(chBoxes.back());

			vlo->addLayout(hlo);
		}

		ui->tableWidget->setCellWidget(i,0, w);

		connect(rBtn, &QRadioButton::clicked, [chBoxesOfThisKit, this, rBtn](bool checked){
			for(auto &rBtnInList:rBtns)
				if(rBtn != rBtnInList) rBtnInList->setChecked(false);
			for(auto &chBox:chBoxesOfThisKit)
				chBox->setChecked(checked);
		});
	}

	ui->tableWidget->resizeRowsToContents();
	ui->tableWidget->resizeColumnsToContents();

	return true;
}

void Windeploy::dragEnterEvent(QDragEnterEvent* event)
{
	if(event->mimeData()->hasText())
	{
		event->acceptProposedAction();
	}
}

void Windeploy::dropEvent(QDropEvent* event)
{
	QString file = event->mimeData()->text();
	if(file.mid(0,8) != "file:///")
	{
		QMessageBox::warning(this,"Внимание", "Возможно неправильный формат файла. Проверьте и добавьте если нужно вручную!");
		return;
	}

	file = file.remove(0,8);
	file.replace("/","\\");

	if(CheckKirillic(file))
	{
		QFileInfo fi(file);
		if(fi.isDir())
		{
			auto fiList = MyQFileDir::GetAllFilesIncludeSubcats(file, {"exe"});
			if(fiList.size() == 0) { QMbError("В директории \n\n["+file+"]\n\n отсутсвуют исполняемые файлы"); return; }
			else if(fiList.size() == 1) file = fiList.first().filePath();
			else
			{
				if(0) CodeMarkers::to_do("из-за того что ListDialog вызывается прямо в dropEvent место откуда вызвали зависает"
										 "можно создать слот и там самым сделать развязку");

				auto filesList = MyQFileDir::FileInfoListToStrList(fiList);
				auto res = MyQDialogs::ListDialog("Choose executable file", filesList);
				file = res.choosedText;
				if(file.isEmpty()) { QMbError("Не выбраны файлы"); return; }
			}
		}

		if(CheckFile(file))
		{
			ui->editFile->clear();
			ui->editFile->setText(file);
		}
	}
}

void Windeploy::on_pushButtonDeploy_clicked()
{
	if(ui->editFile->text().isEmpty()) return;

	QString kitName;
	std::vector<KitElement> checkedKitEls;
	for(auto &rbtn:rBtns)
	{
		if(rbtn->isChecked())
		{
			if(kitName.isEmpty())
				kitName = rbtn->text();
			else
			{
				QMessageBox::critical(this,"Error", "More one deploy kit selected!");
				return;
			}
		}
	}

	for(auto &chBox:chBoxes)
	{
		if(chBox->isChecked())
		{
			checkedKitEls.emplace_back(KitElement::FromText(chBox->text()));
		}
	}

	int countWindeploy = 0;
	for(auto &kitEl:checkedKitEls)
	{
		if(!kitEl.fileInfo.isFile())
		{
			QMbc(this, "File not found", "File " + kitEl.fileInfo.filePath() + " not found");
			return;
		}

		if(kitEl.type == KitElement::windeploy) countWindeploy++;
	}

	if(countWindeploy > 1) { QMbc(this, "File not found", "countWindeploy > 1"); return; }

	QString bat;
	bat += "@echo off\n";

	QString path_dst = QFileInfo(ui->editFile->text()).path();
	for(auto &kitEl:checkedKitEls)
	{
		if(kitEl.type == KitElement::windeploy)
		{
			QString command = '"' + kitEl.fileInfo.filePath() + '"' + " " + '"' + ui->editFile->text() + '"' + "\n";
			bat +=	"echo launching\n"
					"echo " + kitEl.fileInfo.filePath() + "\n"
					"echo with param\n"
					"echo " + ui->editFile->text() + "\n"
					"echo.\n"
					"echo Result:\n"
					"echo.\n";
			bat += command;
		}
		else if(kitEl.type == KitElement::additional)
		{
			QString file_dst = path_dst + "/" + kitEl.fileInfo.fileName();
			if(!MyQFileDir::CopyFileWithReplace(kitEl.fileInfo.filePath(), file_dst))
			{
				QMbw(this, "Error", "Error copying " + kitEl.fileInfo.filePath());
			}
		}
	}

	bat += "echo.\n";
	bat += "echo.\n";
	bat += "echo Finished\n";
	bat += "pause\n";

	QString fileDeployBat = filesPath+"/deploy.bat";

	std::ofstream out;
	out.open(fileDeployBat.toStdString());
	if (out.is_open())
	{
		out << bat.toStdString();
		out.close();

		QDesktopServices::openUrl(QUrl::fromLocalFile(fileDeployBat));
		//MyQExecute::Execute(fileDeployBat);
		//system(("\"" + fileDeployBat + "\"").toStdString().c_str());
	}
	else QMessageBox::information(this,"Ошибка записи команд","Не удалось записать файл команд вызова windeployqt");
}

void Windeploy::on_btnSelectFile_clicked()
{
	auto file = QFileDialog::getOpenFileName(this,"Select file","","Exe (*.exe)");
	if(file.isEmpty() || !CheckFile(file))
		return;

	ui->editFile->setText(file);
}

void Windeploy::on_pushButtonClear_clicked()
{
	ui->editFile->clear();
}

void Windeploy::on_btnDeployKits_clicked()
{
	QDialog *dialog = new QDialog;
	dialog->setWindowTitle("Deploy kits");
	dialog->setWindowFlag(Qt::WindowCloseButtonHint, false);
	QVBoxLayout *vloMain  = new QVBoxLayout(dialog);
	QHBoxLayout *h1 = new QHBoxLayout;
	QHBoxLayout *h2 = new QHBoxLayout;
	QHBoxLayout *hlo3 = new QHBoxLayout;
	vloMain->addLayout(h1);
	vloMain->addLayout(h2);
	vloMain->addLayout(hlo3);

	QTextEdit *textEdit = new QTextEdit;
	textEdit->setText(deployKitsStr);
	h2->addWidget(textEdit);

	auto btnWindep = new QPushButton(KeyWords::windployqtExe);
	auto btnAddFile = new QPushButton(KeyWords::additionalFile);
	auto btnAddTemplate = new QPushButton("both");
	auto btnAddMing73_32 = new QPushButton("Ming73_32");
	auto btnAddMingw81_32 = new QPushButton("Mingw81_32");
	auto btnAddMingw81_64 = new QPushButton("Mingw81_64");

	h1->addWidget(btnWindep);
	h1->addWidget(btnAddFile);
	h1->addWidget(btnAddTemplate);
	h1->addWidget(btnAddMing73_32);
	h1->addWidget(btnAddMingw81_32);
	h1->addWidget(btnAddMingw81_64);
	h1->addStretch();

	connect(btnWindep,&QPushButton::clicked,[textEdit](){
		auto cursor = textEdit->textCursor();
		cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
		cursor.insertText(KeyWords::windployqtExe + " file");
	});
	connect(btnAddFile,&QPushButton::clicked,[textEdit](){
		auto cursor = textEdit->textCursor();
		cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
		cursor.insertText(KeyWords::additionalFile + " file");
	});
	connect(btnAddTemplate,&QPushButton::clicked,[textEdit](){
		auto cursor = textEdit->textCursor();
		cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
		cursor.insertText(KeyWords::kit + " Qt 5.12.10 mingw73_32\n"
						  + KeyWords::windployqtExe + " " + "file\n" + KeyWords::additionalFile + " file\n"
						  + KeyWords::end + "file\n\n");
	});
	connect(btnAddMing73_32,&QPushButton::clicked,[textEdit](){
		auto cursor = textEdit->textCursor();
		cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
		cursor.insertText(KeyWords::kit + " Qt 5.12.10 mingw73_32\n"
						  + KeyWords::windployqtExe + " " +		"C:\\Qt\\Qt5.12.10\\5.12.10\\mingw73_32\\bin\\windeployqt.exe\n"
						  + KeyWords::additionalFile + " " +	"C:\\Qt\\Qt5.12.10\\5.12.10\\mingw73_32\\bin\\libgcc_s_dw2-1.dll\n"
						  + KeyWords::additionalFile + " " +	"C:\\Qt\\Qt5.12.10\\5.12.10\\mingw73_32\\bin\\libstdc++-6.dll\n"
						  + KeyWords::additionalFile + " " +	"C:\\Qt\\Qt5.12.10\\5.12.10\\mingw73_32\\bin\\libwinpthread-1.dll\n"
						  + KeyWords::end + "\n\n");
	});

	connect(btnAddMingw81_32,&QPushButton::clicked,[textEdit](){
		auto cursor = textEdit->textCursor();
		cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
		cursor.insertText(KeyWords::kit + " Qt 5.12.10 mingw81_32\n"
						  + KeyWords::windployqtExe + " " +		"C:\\Qt\\Qt5.12.10\\5.12.10\\mingw81_32\\bin\\windeployqt.exe\n"
						  + KeyWords::additionalFile + " " +	"C:\\Qt\\Qt5.12.10\\5.12.10\\mingw81_32\\bin\\libgcc_s_dw2-1.dll\n"
						  + KeyWords::additionalFile + " " +	"C:\\Qt\\Qt5.12.10\\5.12.10\\mingw81_32\\bin\\libstdc++-6.dll\n"
						  + KeyWords::additionalFile + " " +	"C:\\Qt\\Qt5.12.10\\5.12.10\\mingw81_32\\bin\\libwinpthread-1.dll\n"
						  + KeyWords::end + "\n\n");
	});

	connect(btnAddMingw81_64,&QPushButton::clicked,[textEdit](){
		auto cursor = textEdit->textCursor();
		cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
		cursor.insertText(KeyWords::kit + " Qt 5.12.10 mingw81_64\n"
						  + KeyWords::windployqtExe + " " +		"C:\\Qt\\Qt5.12.10\\5.12.10\\mingw81_64\\bin\\windeployqt.exe\n"
						  + KeyWords::additionalFile + " " +	"C:\\Qt\\Qt5.12.10\\5.12.10\\mingw81_64\\bin\\libgcc_s_seh-1.dll\n"
						  + KeyWords::additionalFile + " " +	"C:\\Qt\\Qt5.12.10\\5.12.10\\mingw81_64\\bin\\libstdc++-6.dll\n"
						  + KeyWords::additionalFile + " " +	"C:\\Qt\\Qt5.12.10\\5.12.10\\mingw81_64\\bin\\libwinpthread-1.dll\n"
						  + KeyWords::end + "\n\n");
	});

	auto btnSave = new QPushButton("Save");
	auto btnAbort = new QPushButton("Abort");

	hlo3->addStretch();
	hlo3->addWidget(btnSave);
	hlo3->addWidget(btnAbort);

	connect(btnSave,&QPushButton::clicked,[this, dialog, textEdit](){
		deployKitsStr = textEdit->toPlainText();
		if(KitsToTable())
			dialog->close();
	});
	connect(btnAbort,&QPushButton::clicked,[this, dialog](){
		if(KitsToTable())
			dialog->close();
	});

	dialog->resize(900,600);
	dialog->exec();

	delete dialog;
}
