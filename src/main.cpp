#include <QApplication>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <stdexcept>

#include "mainwindow.h"
#include "common/logger.h"

int main(int argc, char *argv[])
{
    try {
        bool headlessMode = false;
        for (int i = 1; i < argc; ++i) {
            if (QString(argv[i]) == "--headless") {
                headlessMode = true;
                break;
            }
        }

        QCoreApplication *app = nullptr;
        if (headlessMode) {
            app = new QCoreApplication(argc, argv);
        } else {
            app = new QApplication(argc, argv);
        }

        QString logPath = QCoreApplication::applicationDirPath() + "/cpss.log";
        Logger::init(logPath);
        Logger::info("CPSS v1.0 starting...");
        Logger::info("Mode: %s", headlessMode ? "headless" : "GUI");
        Logger::info("Log path: %s", logPath.toStdString().c_str());

        QCommandLineParser parser;
        parser.setApplicationDescription("CPSS - Combat Process Simulation Software");
        parser.addHelpOption();
        parser.addOption({{"H", "headless"}, "Run without GUI"});
        parser.addOption({{"c", "chart-dir"}, "Chart data directory", "path"});
        parser.process(*app);

        MainWindow *mainwin = nullptr;
        if (!headlessMode) {
            Logger::info("Creating MainWindow...");
            mainwin = new MainWindow();
            Logger::info("MainWindow created successfully");
            fprintf(stderr, "[DEBUG] About to setWindowTitle\n"); fflush(stderr);
            mainwin->setWindowTitle("CPSS - Combat Process Simulation");
            fprintf(stderr, "[DEBUG] About to call showMaximized()...\n"); fflush(stderr);
            Logger::info("About to call showMaximized()...");
            fprintf(stderr, "[DEBUG] Before resize\n"); fflush(stderr);
            mainwin->resize(800, 600);
            fprintf(stderr, "[DEBUG] After resize, before show\n"); fflush(stderr);
            mainwin->show();
            fprintf(stderr, "[DEBUG] After show\n"); fflush(stderr);
            Logger::info("Main window shown");
        }

        Logger::info("Entering main event loop");
        int ret = app->exec();

        delete mainwin;
        delete app;

        Logger::info("CPSS exited with code %d", ret);
        Logger::cleanup();

        return ret;
    } catch (const std::exception &e) {
        fprintf(stderr, "[FATAL] Exception: %s\n", e.what());
        Logger::error("Exception: %s", e.what());
        Logger::cleanup();
        return 1;
    } catch (...) {
        fprintf(stderr, "[FATAL] Unknown exception\n");
        Logger::error("Unknown exception");
        Logger::cleanup();
        return 1;
    }
}
