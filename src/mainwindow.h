#pragma once

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QTabWidget>
#include <QLabel>
#include <QToolBar>
#include <QAction>
#include "codeeditor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CPU;
class Parser;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionAssemble_triggered();
    void on_actionRun_triggered();
    void on_actionStep_triggered();
    void on_actionReset_triggered();
    void on_actionLoadFile_triggered();
    // Berk's Custom INF storing HERééééééé:)
    void showUserGuide();
    //void showInstructionSet();
    void showAboutApp();
    void showAboutMe();
    // Update




private:
    // UI bileşenleri
    CodeEditor *codeEditor;
    QPlainTextEdit *terminalOutput;
    QTableWidget   *registerTable;
    QTabWidget     *memoryTabs;
    QTableWidget   *instructionMemoryTable;
    QTableWidget   *stackMemoryTable;

    QLabel *cf;
    QLabel *zf;
    QLabel *sf;
    QLabel *of;

    QAction *actAssemble;
    QAction *actRun;
    QAction *actStep;
    QAction *actReset;
    QAction *actLoad;

    // CPU & Parser
    CPU *cpu;
    Parser *parser;
    std::vector<uint8_t> machine_code;

    // Helpers
    void setupUI();
    void updateRegisters();
    void updateFlags();
    void updateMemoryView();
};
