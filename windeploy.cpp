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
using namespace std;

#include "MyQDialogs.h"
#include "MyQDifferent.h"
#include "MyQShortings.h"
#include "MyQFileDir.h"

bool CheckFile(const QString &file)
{
	for(auto &c:file)
	{
		if(!QFileInfo(file).isFile())
		{
			QMessageBox::critical(nullptr, "Ошибка", "Объект\n\n["+file+"]\n\nне является файлом!");
			return false;
		}

		if((c >= L'А' && c<=L'Я') || (c>=L'а' && c<=L'я'))
		{
			QMessageBox::critical(nullptr, "Ошибка", "Имя или путь файла\n\n["+file+"]\n\nсодержит кириллицу!");
			return false;
		}
	}

	return true;
}

Windeploy::Windeploy(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::Windeploy)
{
	ui->setupUi(this);

	setAcceptDrops(true);

	settingsFile = MyQDifferent::PathToExe()+"/files/settings.ini";
	QTimer::singleShot(0,this,[this]
	{
		QSettings settings(settingsFile, QSettings::IniFormat);
		if(settings.contains("deployKits"))
			deployKits = DeployKit::FromText(settings.value("deployKits").toString());

		KitsToTable();
	});
}

Windeploy::~Windeploy()
{
	QDir().mkpath(MyQDifferent::PathToExe()+"/files");

	QSettings settings(settingsFile, QSettings::IniFormat);

	settings.setValue("deployKits", DeployKit::ToText(deployKits));

	delete ui;
}

void Windeploy::KitsToTable()
{
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

		vector<QCheckBox*> chBoxesOfThisKit;
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
}

void Windeploy::dragEnterEvent(QDragEnterEvent* event)
{
	if(event->mimeData()->hasText() && event->mimeData()->text().right(4) == ".exe")
	{
		event->acceptProposedAction();
	}
}

void Windeploy::dropEvent(QDropEvent* event)
{
	QString droped = event->mimeData()->text();
	if(droped.mid(0,8) == "file:///") droped = droped.remove(0,8);
		else QMessageBox::warning(this,"Внимание", "Возможно неправильный формат файла. Проверьте и добавьте если нужно вручную!");
	droped.replace("/","\\");

	if(CheckFile(droped))
	{
		ui->editFile->clear();
		ui->editFile->setText(droped);
	}
}

void Windeploy::on_pushButtonDeploy_clicked()
{
	if(ui->editFile->text().isEmpty()) return;

	QString bat;
	//bat += "chcp 1251\n";

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

	QString path_dst = QFileInfo(ui->editFile->text()).path();
	for(auto &kitEl:checkedKitEls)
	{
		if(kitEl.type == KitElement::windeploy)
		{
			QString command = '"' + kitEl.fileInfo.filePath() + '"' + " " + '"' + ui->editFile->text() + '"' + "\n";
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

	//bat += "echo Восмотрите не было ли ошибок в выводе и закройте консоль.\n";
	//bat += "pause\n";

	QString fileDeployBat = MyQDifferent::PathToExe() + "/files/deploy.bat";

	ofstream out;
	out.open(fileDeployBat.toStdString());
	if (out.is_open())
	{
		out << bat.toStdString();
		out.close();

		system(("\"" + fileDeployBat + "\"").toStdString().c_str());
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
	QVBoxLayout *all  = new QVBoxLayout(dialog);
	QHBoxLayout *h1 = new QHBoxLayout;
	QHBoxLayout *h2 = new QHBoxLayout;
	all->addLayout(h1);
	all->addLayout(h2);

	QTextEdit *textEdit = new QTextEdit;
	textEdit->setText(DeployKit::ToText(deployKits));
	h2->addWidget(textEdit);

	auto btnWindep = new QPushButton(KeyWords::windployqtExe);
	auto btnAddFile = new QPushButton(KeyWords::additionalFile);
	auto btnAddTemplate = new QPushButton("both");
	auto btnAddMing73_32 = new QPushButton("template Ming73_32");

	h1->addWidget(btnWindep);
	h1->addWidget(btnAddFile);
	h1->addWidget(btnAddTemplate);
	h1->addWidget(btnAddMing73_32);
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
						  + KeyWords::windployqtExe + " " + "C:\\Qt\\Qt5.12.10\\5.12.10\\mingw73_32\\bin\\windeployqt.exe\n"
						  + KeyWords::additionalFile + " " + "C:\\Qt\\windeployqt\\Руками добавил для mingw73_32 ++\\libgcc_s_dw2-1.dll\n"
						  + KeyWords::additionalFile + " " + "C:\\Qt\\windeployqt\\Руками добавил для mingw73_32 ++\\libstdc++-6.dll\n"
						  + KeyWords::additionalFile + " " + "C:\\Qt\\windeployqt\\Руками добавил для mingw73_32 ++\\libwinpthread-1.dll\n"
						  + KeyWords::end + "\n\n");
	});

	dialog->resize(900,600);
	dialog->exec();

	deployKits = DeployKit::FromText(textEdit->toPlainText());
	KitsToTable();

	delete dialog;
}
