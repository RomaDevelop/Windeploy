#include "windeploy.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Windeploy w(argc, argv);
	w.show();
	return a.exec();
}
