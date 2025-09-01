#ifndef WINDEPLOY_H
#define WINDEPLOY_H

#include <QMainWindow>
#include <QRadioButton>
#include <QCheckBox>

#include "DeployKit.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Windeploy; }
QT_END_NAMESPACE

class Windeploy : public QMainWindow
{
	Q_OBJECT
	Ui::Windeploy *ui;

public:

	Windeploy(int argc, char *argv[], QWidget *parent = nullptr);
	~Windeploy();

private:
	void WorkArgs(int argc, char *argv[]);
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
#endif // WINDEPLOY_H
