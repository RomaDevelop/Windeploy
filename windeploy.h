#ifndef WINDEPLOY_H
#define WINDEPLOY_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class Windeploy; }
QT_END_NAMESPACE

class Windeploy : public QMainWindow
{
	Q_OBJECT

public:
	Windeploy(QWidget *parent = nullptr);
	~Windeploy();

protected:
	void dragEnterEvent( QDragEnterEvent* event );
	void dropEvent( QDropEvent* event );

private slots:
	void on_radioButtoComplectMingw73_32_clicked();

	void on_pushButtonDeploy_clicked();

	void on_pushButton_clicked();

	void on_pushButtonClear_clicked();

private:
	Ui::Windeploy *ui;
};
#endif // WINDEPLOY_H
