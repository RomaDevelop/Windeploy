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

#include <fstream>
using namespace std;

Windeploy::Windeploy(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::Windeploy)
{
	ui->setupUi(this);

	setAcceptDrops(true);
	//this->setAcceptDrops(true);
	//ui->listWidget->setAcceptDrops(true);

}

Windeploy::~Windeploy()
{
	delete ui;
}

void Windeploy::dragEnterEvent( QDragEnterEvent* event )
{
	if(event->mimeData()->hasText() &&
			event->mimeData()->text().right(4) == ".exe")
		event->acceptProposedAction();
}

void Windeploy::dropEvent( QDropEvent* event )
{
	QString droped = event->mimeData()->text();
	if(droped.mid(0,8) == "file:///") droped = droped.remove(0,8);
		else QMessageBox::warning(this,"Внимание", "Возможно неправильный формат файла. Проверьте и дубавьте если нужно вручную!");
	droped.replace("/","\\");
	ui->listWidget->addItem(droped);
	//event->acceptProposedAction();  было в примере, но у меня это лишнее, строки выше делают всё что нужно
}

void Windeploy::on_radioButtoComplectMingw73_32_clicked()
{
	if(ui->radioButtoComplectMingw73_32->isChecked())
	{
		ui->checkBox_libgcc_s_dw2_1_dll->setChecked(true);
		ui->checkBox_libstdcpp_6_dll->setChecked(true);
		ui->checkBox_libwinpthread_1_dll->setChecked(true);
	}
	else
	{
		ui->checkBox_libgcc_s_dw2_1_dll->setChecked(false);
		ui->checkBox_libstdcpp_6_dll->setChecked(false);
		ui->checkBox_libwinpthread_1_dll->setChecked(false);
	}
}

void Windeploy::on_pushButtonDeploy_clicked()
{
	QString bat;
	//bat += "chcp 1251\n";
	for(int i=0; i<ui->listWidget->count(); i++)
	{
		if(ui->radioButtoComplectMingw73_32->isChecked())
		{

			QString command = "\"C:\\Qt\\Qt5.12.10\\5.12.10\\mingw73_32\\bin\\windeployqt.exe\" \"" +
					ui->listWidget->item(i)->text() + "\"\n";
			bat += command;

			if(ui->checkBox_libgcc_s_dw2_1_dll->isChecked())
			{
				QString path_dst = QFileInfo(ui->listWidget->item(i)->text()).path();
				QString file_src = "C:\\Qt\\windeployqt\\Руками добавил для mingw73_32 ++\\libgcc_s_dw2-1.dll";
				QString fileName = QFileInfo(file_src).fileName();
				QString file_dst = QFileInfo(ui->listWidget->item(i)->text()).path() + "/" + fileName;
				QFile::copy(file_src, file_dst);
			}
			if(ui->checkBox_libstdcpp_6_dll->isChecked())
			{
				QString path_dst = QFileInfo(ui->listWidget->item(i)->text()).path();
				QString file_src = "C:\\Qt\\windeployqt\\Руками добавил для mingw73_32 ++\\libstdc++-6.dll";
				QString fileName = QFileInfo(file_src).fileName();
				QString file_dst = QFileInfo(ui->listWidget->item(i)->text()).path() + "/" + fileName;
				QFile::copy(file_src, file_dst);
			}
			if(ui->checkBox_libwinpthread_1_dll->isChecked())
			{
				QString path_dst = QFileInfo(ui->listWidget->item(i)->text()).path();
				QString file_src = "C:\\Qt\\windeployqt\\Руками добавил для mingw73_32 ++\\libwinpthread-1.dll";
				QString fileName = QFileInfo(file_src).fileName();
				QString file_dst = QFileInfo(ui->listWidget->item(i)->text()).path() + "/" + fileName;
				QFile::copy(file_src, file_dst);
			}

		}
	}
	//bat += "echo Восмотрите не было ли ошибок в выводе и закройте консоль.\n";
	//bat += "pause\n";

	QString path = QApplication::applicationDirPath() + "\\deploy.bat";

	ofstream out;
	out.open(path.toStdString());
	if (out.is_open())
	{
		out << bat.toStdString();
		out.close();

		system(path.toStdString().c_str());
	}
	else QMessageBox::information(this,"Ошибка записи команд","Не удалось записать файл команд вызова windeployqt");
}

void Windeploy::on_pushButton_clicked()
{
	ui->listWidget->addItem(ui->lineEdit->text());
}

void Windeploy::on_pushButtonClear_clicked()
{
	ui->listWidget->clear();
}
