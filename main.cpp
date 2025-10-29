#include "windeploy.h"

#include <QApplication>

#include "MyQDifferent.h"

int main(int argc, char *argv[])
{
	QApplication a(argc,argv);
	Windeploy w(MyQDifferent::ArgsToStrList(argc,argv));
	w.show();
	return a.exec();
}
