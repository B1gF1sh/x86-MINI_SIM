#include "mainwindow.h"
#include "cpu.h"
#include "parser.h"
#include "userdialog.h"


#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include "stardialog.h"
#include <QSettings>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    cpu = new CPU();
    parser = new Parser();

    setupUI();
}

MainWindow::~MainWindow()
{
    delete cpu;
    delete parser;
}

void MainWindow::setupUI()
{

    // Berk's INF
    // === MENU BAR ===
    QMenuBar *menuBarPtr = menuBar();

    // Help Menu
    QMenu *helpMenu = menuBarPtr->addMenu("Help");
    QAction *actUserGuide = helpMenu->addAction("User Guide");
    //QAction *actInstructionSet = helpMenu->addAction("Instruction Set");
    QAction *actReportIssue = helpMenu->addAction("Report Issue");

    connect(actUserGuide, &QAction::triggered, this, &MainWindow::showUserGuide);
    //connect(actInstructionSet, &QAction::triggered, this, &MainWindow::showInstructionSet);
    connect(actReportIssue, &QAction::triggered, this, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/B1gF1sh/x86_Simulator_GUI/issues"));
    });

    // About Menu
    QMenu *aboutMenu = menuBarPtr->addMenu("About");
    QAction *actAboutApp = aboutMenu->addAction("About App");
    QAction *actAboutMe = aboutMenu->addAction("About Me");

    connect(actAboutApp, &QAction::triggered, this, &MainWindow::showAboutApp);
    connect(actAboutMe, &QAction::triggered, this, &MainWindow::showAboutMe);



    // === STAR DIALOG sadece ilk açılışta göster ===
    QSettings settings("MyCompany", "x86_Simulator");
    bool shown = settings.value("starDialogShown", false).toBool();
    if (shown) {
        StarDialog dlg(this);
        dlg.exec();
        settings.setValue("starDialogShown", true);
    }

    // === TOOLBAR ===
    QToolBar *toolbar = addToolBar("Main Toolbar");
    actAssemble = toolbar->addAction("Assemble");
    actRun      = toolbar->addAction("Run");
    actStep     = toolbar->addAction("Step");
    actReset    = toolbar->addAction("Reset");
    actLoad     = toolbar->addAction("LoadFile");

    connect(actAssemble, &QAction::triggered, this, &MainWindow::on_actionAssemble_triggered);
    connect(actRun, &QAction::triggered, this, &MainWindow::on_actionRun_triggered);
    connect(actStep, &QAction::triggered, this, &MainWindow::on_actionStep_triggered);
    connect(actReset, &QAction::triggered, this, &MainWindow::on_actionReset_triggered);
    connect(actLoad, &QAction::triggered, this, &MainWindow::on_actionLoadFile_triggered);

    // === CODE EDITOR ===
    codeEditor = new CodeEditor;

    codeEditor->setPlaceholderText("Write your assembly code here...");
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    codeEditor->setFont(font);

    // === TERMINAL ===
    terminalOutput = new QPlainTextEdit;
    terminalOutput->setReadOnly(true);
    terminalOutput->setMaximumHeight(120);

    // === REGISTER TABLE ===
    QStringList regs = {"AX","BX","CX","DX","SP","BP","SI","DI","IP"};
    registerTable = new QTableWidget(regs.size(), 5);
    QStringList headers = {"Register", "Hex", "Dec", "Low", "High"};
    registerTable->setHorizontalHeaderLabels(headers);
    registerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (int i = 0; i < regs.size(); i++) {
        registerTable->setItem(i, 0, new QTableWidgetItem(regs[i]));
        for (int j = 1; j < 5; j++)
            registerTable->setItem(i, j, new QTableWidgetItem(""));
    }

    // === FLAGS ===
    cf = new QLabel("CF: ✘");
    zf = new QLabel("ZF: ✘");
    sf = new QLabel("SF: ✘");
    of = new QLabel("OF: ✘");

    QHBoxLayout *flagsLayout = new QHBoxLayout;
    flagsLayout->addWidget(cf);
    flagsLayout->addWidget(zf);
    flagsLayout->addWidget(sf);
    flagsLayout->addWidget(of);

    QGroupBox *flagsBox = new QGroupBox("Flags");
    flagsBox->setLayout(flagsLayout);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(registerTable);
    rightLayout->addWidget(flagsBox);

    QWidget *rightPanel = new QWidget;
    rightPanel->setLayout(rightLayout);

    // === CENTER SPLITTER ===
    QSplitter *centerSplitter = new QSplitter(Qt::Horizontal);
    centerSplitter->addWidget(codeEditor);
    centerSplitter->addWidget(rightPanel);
    centerSplitter->setStretchFactor(0, 3);
    centerSplitter->setStretchFactor(1, 2);

    // === MEMORY TABS ===
    memoryTabs = new QTabWidget;
    instructionMemoryTable = new QTableWidget;
    stackMemoryTable = new QTableWidget;
    memoryTabs->addTab(instructionMemoryTable, "Instruction Memory");
    memoryTabs->addTab(stackMemoryTable, "Stack Memory");

    // === MAIN LAYOUT ===
    QWidget *central = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(centerSplitter, 6);
    mainLayout->addWidget(memoryTabs, 3);
    mainLayout->addWidget(terminalOutput, 1);
    central->setLayout(mainLayout);

    setCentralWidget(central);
    resize(1280, 800);
    setWindowTitle("x86 Mini CPU Simulator IDE");
}

// === SLOTS ===

void MainWindow::on_actionAssemble_triggered()
{
    QString code = codeEditor->toPlainText();
    machine_code = parser->parse_from_string(code.toStdString());

    if (machine_code.empty() && !parser->get_last_error().empty()) {
        terminalOutput->appendPlainText(QString::fromStdString(parser->get_last_error()));
    } else {
        terminalOutput->appendPlainText("[Assemble] OK - Machine code generated");
        std::copy(machine_code.begin(), machine_code.end(), cpu->memory.begin());
        cpu->regs.IP = 0; // program counter reset
    }
    updateRegisters();
    updateFlags();
    updateMemoryView();
}

void MainWindow::on_actionRun_triggered()
{
    terminalOutput->appendPlainText("[Run] CPU started...");
    cpu->run();
    updateRegisters();
    updateFlags();
    updateMemoryView();
    terminalOutput->appendPlainText("[Run] CPU halted.");
}

void MainWindow::on_actionStep_triggered()
{
    terminalOutput->appendPlainText("[Step] Executing instruction...");
    cpu->step();
    updateRegisters();
    updateFlags();
    updateMemoryView();
}

void MainWindow::on_actionReset_triggered()
{
    delete cpu;
    cpu = new CPU();
    terminalOutput->appendPlainText("[Reset] CPU and memory reset.");
    updateRegisters();
    updateFlags();
    updateMemoryView();
}

void MainWindow::on_actionLoadFile_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open Assembly File", "", "ASM Files (*.asm);;All Files (*)");
    if (!filename.isEmpty()) {
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            codeEditor->setPlainText(file.readAll());
            terminalOutput->appendPlainText("[LoadFile] " + filename);
        } else {
            QMessageBox::warning(this, "Error", "Cannot open file!");
        }
    }
}

// === UPDATE HELPERS ===

void MainWindow::updateRegisters()
{
    auto setRow = [&](int row, uint16_t val) {
        registerTable->setItem(row, 1, new QTableWidgetItem(
                                           QString("0x%1").arg(val, 4, 16, QChar('0')).toUpper()));
        registerTable->setItem(row, 2, new QTableWidgetItem(QString::number(val)));

        if (row <= 3) { // AX,BX,CX,DX
            uint8_t low = val & 0xFF;
            uint8_t high = (val >> 8) & 0xFF;
            registerTable->setItem(row, 3, new QTableWidgetItem(
                                               QString("0x%1").arg(low, 2, 16, QChar('0')).toUpper()));
            registerTable->setItem(row, 4, new QTableWidgetItem(
                                               QString("0x%1").arg(high, 2, 16, QChar('0')).toUpper()));
        } else {
            registerTable->setItem(row, 3, new QTableWidgetItem("-"));
            registerTable->setItem(row, 4, new QTableWidgetItem("-"));
        }
    };

    setRow(0, cpu->regs.AX);
    setRow(1, cpu->regs.BX);
    setRow(2, cpu->regs.CX);
    setRow(3, cpu->regs.DX);
    setRow(4, cpu->regs.SP);
    setRow(5, cpu->regs.BP);
    setRow(6, cpu->regs.SI);
    setRow(7, cpu->regs.DI);
    setRow(8, cpu->regs.IP);
}

void MainWindow::updateFlags()
{
    cf->setText(QString("CF: %1").arg(cpu->flags.CF ? "✔" : "✘"));
    zf->setText(QString("ZF: %1").arg(cpu->flags.ZF ? "✔" : "✘"));
    sf->setText(QString("SF: %1").arg(cpu->flags.SF ? "✔" : "✘"));
    of->setText(QString("OF: %1").arg(cpu->flags.OF ? "✔" : "✘"));
}
void MainWindow::updateMemoryView()
{
    // === Instruction Memory (0x0000 - 0x00FF) ===
    int inst_rows = 0x0100; // 256 satır
    instructionMemoryTable->setRowCount(inst_rows);
    instructionMemoryTable->setColumnCount(2);
    instructionMemoryTable->setHorizontalHeaderLabels({"Addr", "Value"});

    for (int addr = 0; addr < inst_rows; addr++) {
        auto *item = new QTableWidgetItem(
            QString("%1").arg(cpu->memory[addr], 2, 16, QChar('0')).toUpper());

        // IP highlight → yeşil arkaplan + siyah yazı
        if (addr == cpu->regs.IP) {
            item->setBackground(Qt::green);
            item->setForeground(Qt::black);
        }

        instructionMemoryTable->setItem(addr, 0, new QTableWidgetItem(
                                                     QString("0x%1").arg(addr, 4, 16, QChar('0')).toUpper()));
        instructionMemoryTable->setItem(addr, 1, item);
    }

    // === Stack Memory (0xFFFE -> 0xFF00) ===
    int stack_start = 0xFFFE;
    int stack_end   = 0xFF00;
    int stack_rows  = stack_start - stack_end + 1;

    stackMemoryTable->setRowCount(stack_rows);
    stackMemoryTable->setColumnCount(2);
    stackMemoryTable->setHorizontalHeaderLabels({"Addr", "Value"});

    for (int addr = stack_start, row = 0; addr >= stack_end; addr--, row++) {
        auto *item = new QTableWidgetItem(
            QString("%1").arg(static_cast<int>(cpu->memory[addr]), 2, 16, QChar('0')).toUpper());

        // SP highlight → sarı arkaplan + siyah yazı
        if (addr == cpu->regs.SP || addr == cpu->regs.SP + 1) {
            item->setBackground(Qt::yellow);
            item->setForeground(Qt::black);
        }

        stackMemoryTable->setItem(row, 0, new QTableWidgetItem(
                                              QString("0x%1").arg(addr, 4, 16, QChar('0')).toUpper()));
        stackMemoryTable->setItem(row, 1, item);
    }
}


//void MainWindow::showInstructionSet() {
//    QMessageBox::information(this, "Instruction Set",
//                             "Desteklenen bazı komutlar:\n"
//                             "MOV, ADD, SUB, CMP, JMP, JZ, JNZ, JC, JNC...\n"
//                             "PUSH, POP, CALL, RET...\n"
//                             "AND, OR, XOR, NOT, NEG...\n"
//                             "INC, DEC, SHL, SHR, ROL, ROR...\n"
//                             "\n(Tam liste için kaynak koda bakınız)");
//}
void MainWindow::showAboutApp() {
    QMessageBox::about(this, "About App",
                       "x86 Mini CPU Simulator\n\n"
                       "A 16-bit CPU architecture simulator with assembler.\n\n"
                       "Designed to help students learn assembly programming and computer architecture concepts.\n\n"
                       "Version: 1.0.0");
}


void MainWindow::showAboutMe() {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("About Me");

    msgBox.setText(
        "Developer: Berk Ali Demir\n\n"
        "🌐 Website: <a href='https://berkali.eu'>https://berkali.eu</a><br>\n\n"
        "⭐ Give a Star: <a href='https://github.com/B1gF1sh/x86-MINI_SIM'>Click here to give a star</a><br>"
        );

    msgBox.setTextFormat(Qt::RichText);   // linkleri aktif et
    msgBox.setTextInteractionFlags(Qt::TextBrowserInteraction);
    msgBox.setStandardButtons(QMessageBox::Ok);

    msgBox.exec();
}

void MainWindow::showUserGuide() {
    UserGuideDialog dlg(this);
    dlg.exec();
}

