/**
 * @file main.cpp
 * @brief CPSS (Combat Process Simulation System) 主程序入口
 * @details 该文件是战斗过程模拟系统的主程序入口，负责初始化应用程序、日志系统，
 *          创建主窗口并进入事件循环。支持两种运行模式：GUI模式和无头模式(headless)。
 * @date 2026-07-28
 */

#include <QApplication>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <stdexcept>

#include "mainwindow.h"
#include "common/logger.h"

/**
 * @brief 主程序入口函数
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 程序退出码
 */
int main(int argc, char *argv[])
{
    try {
        // 检查是否为无头模式
        bool headlessMode = false;
        for (int i = 1; i < argc; ++i) {
            if (QString(argv[i]) == "--headless") {
                headlessMode = true;
                break;
            }
        }

        // 根据运行模式创建应用程序实例
        QCoreApplication *app = nullptr;
        if (headlessMode) {
            app = new QCoreApplication(argc, argv);
        } else {
            app = new QApplication(argc, argv);
        }

        // 初始化日志系统
        QString logPath = QCoreApplication::applicationDirPath() + "/cpss.log";
        Logger::init(logPath);

        // 解析命令行参数
        QCommandLineParser parser;
        parser.setApplicationDescription("CPSS - Combat Process Simulation Software");
        parser.addHelpOption();
        parser.addOption({{"H", "headless"}, "Run without GUI"});
        parser.addOption({{"c", "chart-dir"}, "Chart data directory", "path"});
        parser.process(*app);

        // GUI模式下创建主窗口
        MainWindow *mainwin = nullptr;
        if (!headlessMode) {
            mainwin = new MainWindow();
            mainwin->setWindowTitle("CPSS - Combat Process Simulation");
            mainwin->resize(800, 600);
            mainwin->show();
        }

        // 进入主事件循环
        Logger::info("[main] about to exec()");
        int ret = app->exec();
        Logger::info("[main] exec() returned ret=%d", ret);

        // 清理资源
        Logger::info("[main] deleting mainwin");
        delete mainwin;
        Logger::info("[main] deleting app");
        delete app;
        Logger::info("[main] about to exit, process ID=%d", QCoreApplication::applicationPid());

        Logger::cleanup();

        return ret;
    } catch (const std::exception &e) {
        fprintf(stderr, "[FATAL] Exception: %s\n", e.what());
        Logger::cleanup();
        return 1;
    } catch (...) {
        fprintf(stderr, "[FATAL] Unknown exception\n");
        Logger::cleanup();
        return 1;
    }
}
