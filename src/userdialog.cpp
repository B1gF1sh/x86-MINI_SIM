#include "userdialog.h"
#include <QTabWidget>
#include <QTextBrowser>
#include <QVBoxLayout>

UserGuideDialog::UserGuideDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("User Guide");
    resize(700, 550);

    tabs = new QTabWidget(this);

    // === Introduction ===
    addTab("Introduction",
           "<h2>Introduction</h2>"
           "<p>This application simulates a simple 16-bit CPU with registers, memory, and an assembler.</p>"
           "<p>It is designed to help students learn assembly programming and computer architecture concepts.</p>"
           );

    // === Getting Started ===
    addTab("Getting Started",
           "<h2>Getting Started</h2>"
           "<ol>"
           "<li>Write your assembly program in the editor on the left side.</li>"
           "<li>Click <b>Assemble</b> to compile the code into machine code and load it into memory.</li>"
           "<li>Click <b>Run</b> to execute the program until it halts.</li>"
           "<li>Click <b>Step</b> to execute one instruction at a time (useful for debugging).</li>"
           "<li>Click <b>Reset</b> to reset CPU registers and memory.</li>"
           "<li>Click <b>LoadFile</b> to open an <code>.asm</code> file from disk.</li>"
           "</ol>"
           );

    // === Editor ===
    addTab("Editor",
           "<h2>Editor</h2>"
           "<ul>"
           "<li>Line numbers are displayed on the left.</li>"
           "<li>Errors are highlighted in <span style='color:red'>red</span>.</li>"
           "<li>Comments start with <code>;</code>.</li>"
           "<li>The editor supports both writing new code and loading files from disk.</li>"
           "</ul>"
           );

    // === Registers & Flags ===
    addTab("Registers & Flags",
           "<h2>Registers</h2>"
           "<p><b>16-bit registers:</b> AX, BX, CX, DX, SP, BP, SI, DI, IP</p>"
           "<p><b>8-bit registers:</b> AL, AH, BL, BH, CL, CH, DL, DH</p>"
           "<h2>Flags</h2>"
           "<p>CF (Carry Flag), ZF (Zero Flag), SF (Sign Flag), OF (Overflow Flag).</p>"
           "<p>Flags are displayed at the bottom-right corner of the interface.</p>"
           );

    // === Memory ===
    addTab("Memory",
           "<h2>Memory</h2>"
           "<ul>"
           "<li><b>Instruction Memory</b>: Address range 0x0000 – 0x00FF (program is loaded here).</li>"
           "<li><b>Stack Memory</b>: Address range 0xFFFE – 0xFF00 (used for PUSH/POP operations).</li>"
           "</ul>"
           "<p><i>Coming Soon: Full 64KB memory viewer with paging support.</i></p>"
           );

    // === Instruction Set ===
    addTab("Instruction Set",
           "<h2>Instruction Set</h2>"
           "<h3>Program Control</h3>"
           "<p>HALT, NOP, JMP, CALL, RET, JZ, JNZ, JC, JNC, JS, JNS, JO, JNO</p>"
           "<h3>Data Transfer</h3>"
           "<p>MOV reg16, imm16<br>"
           "MOV reg16, reg16<br>"
           "MOV reg8, imm8<br>"
           "MOV reg8, reg8<br>"
           "MOV reg16, [imm16]<br>"
           "MOV [imm16], reg16<br>"
           "MOV reg16, [reg16]<br>"
           "MOV [reg16], reg16<br>"
           "MOV reg8, [imm16]<br>"
           "MOV [imm16], reg8<br>"
           "MOV reg8, [reg16]<br>"
           "MOV [reg16], reg8<br>"
           "MOV reg16, [reg+reg]<br>"
           "MOV [reg+reg], reg16</p>"
           "<p><i>Coming Soon: [REG+IMM], [REG+REG+IMM], [REG8] addressing modes.</i></p>"
           "<h3>Stack</h3>"
           "<p>PUSH reg16, POP reg16</p>"
           "<h3>Arithmetic / Logic</h3>"
           "<p>ADD, SUB, ADC, SBB, CMP, INC, DEC, NEG, NOT, AND, OR, XOR</p>"
           "<h3>Shift / Rotate</h3>"
           "<p>SHL, SHR, SAR, ROL, ROR, RCL, RCR (immediate or CL, 8-bit and 16-bit)</p>"
           "<p><i>Coming Soon: Memory operand arithmetic (e.g., ADD AX, [1234h]).</i></p>"
           );

    // === Errors & Reporting ===
    addTab("Errors & Reporting",
           "<h2>Error Handling</h2>"
           "<ul>"
           "<li>The parser shows error messages with the line number.</li>"
           "<li>The editor highlights the faulty line in red.</li>"
           "<li>The CPU stops execution when an unknown opcode is encountered.</li>"
           "</ul>"
           "<h2>Reporting Issues</h2>"
           "<p>Report bugs and feature requests here: "
           "<a href='https://github.com/B1gF1sh/x86_Simulator_GUI/issues'>GitHub Issues</a></p>"
           );

    // === Layout ===
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tabs);
    setLayout(layout);
}

void UserGuideDialog::addTab(const QString &title, const QString &htmlContent) {
    QTextBrowser *browser = new QTextBrowser;
    browser->setOpenExternalLinks(true);
    browser->setHtml(htmlContent);
    tabs->addTab(browser, title);
}
