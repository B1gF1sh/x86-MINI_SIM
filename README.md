# x86 Mini CPU Simulator

A lightweight 16-bit CPU simulator with assembler and Qt-based IDE.  
Designed to help students learn assembly programming and computer architecture.

## ✨ Features
- 16-bit registers and flags (AX, BX, CX, DX, SP, BP, SI, DI, IP)
- Instruction memory & stack memory view
- Supported instructions: MOV, ADD, SUB, CMP, JMP, JZ, JNZ, JC, JNC, CALL, RET, PUSH, POP, AND, OR, XOR, NOT, INC, DEC...
- GUI built with Qt (Code editor, run/step/reset, memory viewer)
- Star dialog & About dialog

- ## 🚀 Windows Build
[⬇️ Download Windows Portable]([https://github.com/B1gF1sh/x86-MINI_SIM/releases/download/v1.0.0/x86_Simulator_GUI_Windows.zip](https://github.com/B1gF1sh/x86-MINI_SIM/releases/tag/v1.0.0))


## 🚀 Build (Linux)
```bash
qmake
make -j$(nproc)
./x86_Simulator_GUI

