#include "author_detector.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	author_detector w;
	w.show();
	return a.exec();
}
