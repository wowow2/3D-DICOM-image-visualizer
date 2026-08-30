#include "MainWindow.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>

int main(int argc, char *argv[]) {
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("DicomViewer");
    QCoreApplication::setApplicationName("DicomViewer");
    MainWindow window;
    window.show();
    if (argc > 1) {
        window.loadPatientDirectory(QString::fromLocal8Bit(argv[1]));
    }
    return app.exec();
}