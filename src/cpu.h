#pragma once
#include <cstdint>
#include <vector>

enum OpCode
{
    OP_HALT = 0x00,
    OP_MOV_REG_IMM = 0x01,
    OP_ADD_REG_IMM = 0x02,
    OP_MOV_REG_REG = 0x03,
    OP_ADD_REG_REG = 0x04,
    OP_PUSH_REG = 0x05,
    OP_POP_REG = 0x06,
    OP_SUB_REG_IMM = 0x07,
    OP_SUB_REG_REG = 0x08,
    OP_CMP_REG_IMM = 0x09,
    OP_JNZ = 0x0A,
    OP_JMP = 0x0B,
    OP_CMP_REG_REG = 0x0C,
    OP_AND_REG_IMM = 0x10,
    OP_OR_REG_IMM = 0x11,
    OP_XOR_REG_IMM = 0x12,
    OP_AND_REG_REG = 0x13,
    OP_OR_REG_REG = 0x14,
    OP_XOR_REG_REG = 0x15,
    OP_NOT_REG = 0x16,
    OP_NEG_REG = 0x17,
    OP_SHL_REG = 0x18,
    OP_SHR_REG = 0x19,
    OP_SAL_REG = 0x1A,
    OP_ROL_REG = 0x1B,
    OP_ROR_REG = 0x1C,
    OP_RCL_REG = 0x1D,
    OP_RCR_REG = 0x1E,
    OP_ADC_REG_IMM = 0x1F,
    OP_ADC_REG_REG = 0x20,
    OP_XCHG_REG_REG = 0x21,

    // 8-BIT ARITHMETIC LOGICAL
    OP_MOV_REG8_IMM = 0x22,
    OP_MOV_REG8_REG8 = 0x23,
    OP_ADD_REG8_IMM = 0x24,
    OP_ADD_REG8_REG8 = 0x25,
    OP_SUB_REG8_IMM = 0x26,
    OP_SUB_REG8_REG8 = 0x27,
    OP_CMP_REG8_IMM = 0x28,
    OP_CMP_REG8_REG8 = 0x29,
    OP_ADC_REG8_REG8 = 0x7D,
    OP_XCHG_REG8_REG8 = 0x7C,
    OP_SBB_REG8_REG8 = 0x7E,
    OP_NOT_REG8 = 0x7F,
    OP_AND_REG8_REG8 = 0x80,
    OP_OR_REG8_REG8 = 0x81,
    OP_XOR_REG8_REG8 = 0x82,
    OP_ADC_REG8_IMM = 0x83,
    OP_SBB_REG8_IMM = 0x84,
    OP_AND_REG8_IMM = 0x8C,
    OP_OR_REG8_IMM = 0x8D,
    OP_XOR_REG8_IMM = 0x8E,

    // 16-BIT DYNAMIC SHIFT ROTATE
    OP_SHL_REG_IMM = 0x2A,
    OP_SAL_REG_IMM = OP_SHL_REG_IMM,
    OP_SHR_REG_IMM = 0x2B,
    OP_SAR_REG_IMM = 0x2C,
    OP_ROL_REG_IMM = 0x2D,
    OP_ROR_REG_IMM = 0x2E,
    OP_RCL_REG_IMM = 0x2F,
    OP_RCR_REG_IMM = 0x30,

    // 8-BIT DYNAMIC SHIFT ROTATE
    OP_SHL_REG8_IMM = 0x31,
    OP_SAL_REG8_IMM = OP_SHL_REG8_IMM,
    OP_SHR_REG8_IMM = 0x32,
    OP_SAR_REG8_IMM = 0x33,
    OP_ROL_REG8_IMM = 0x34,
    OP_ROR_REG8_IMM = 0x35,
    OP_RCL_REG8_IMM = 0x36,
    OP_RCR_REG8_IMM = 0x37,
    OP_SHL_REG8_CL = 0x85,
    OP_SHR_REG8_CL = 0x86,
    OP_SAR_REG8_CL = 0x87,
    OP_ROL_REG8_CL = 0x88,
    OP_ROR_REG8_CL = 0x89,
    OP_RCL_REG8_CL = 0x8A,
    OP_RCR_REG8_CL = 0x8B,

    OP_MOV_REG_FROM_MEM_IMM = 0x38,
    OP_MOV_MEM_IMM_FROM_REG = 0x39,
    OP_MOV_REG_FROM_MEM_REG = 0x3A,
    OP_MOV_MEM_REG_FROM_REG = 0x3B,

    // 8-BIT MEMORY ADDRESSING MODES
    OP_MOV_REG8_FROM_MEM_IMM = 0x3C,
    OP_MOV_MEM_IMM_FROM_REG8 = 0x3D,
    OP_MOV_REG8_FROM_MEM_REG = 0x3E,
    OP_MOV_MEM_REG_FROM_REG8 = 0x3F,

    // 16-BIT ARITHMETIC WITH MEMORY OPERANDS
    OP_ADD_REG_FROM_MEM_IMM = 0x40,
    OP_ADD_REG_FROM_MEM_REG = 0x41,
    OP_SUB_REG_FROM_MEM_IMM = 0x42,
    OP_SUB_REG_FROM_MEM_REG = 0x43,
    OP_CMP_REG_FROM_MEM_IMM = 0x44,
    OP_CMP_REG_FROM_MEM_REG = 0x45,

    // INC, DEC...
    OP_INC_REG = 0x50,
    OP_DEC_REG = 0x51,
    OP_NEG_REG16 = 0x52,
    OP_INC_REG8 = 0x54,
    OP_DEC_REG8 = 0x55,
    OP_NEG_REG8 = 0x56,

    // ADVANCED ARITHMETIC
    OP_SBB_REG_REG = 0x5A,
    OP_SBB_REG_IMM = 0x5B,

    // FUNCTION CALL, BRANCH
    OP_CALL = 0x60,
    OP_RET = 0x61,

    // JUMPS
    OP_JZ = 0x62,
    OP_JC = 0x63,
    OP_JNC = 0x64,
    OP_JS = 0x65,
    OP_JNS = 0x66,
    OP_JO = 0x67,
    OP_JNO = 0x68,

    // SYSTEM
    OP_NOP = 0xFF,

    // 16-BIT COMPLEX ADDRESSING MODES MOV AX , [BX+SI]
    OP_MOV_REG_FROM_MEM_REG_REG = 0x70,
    OP_MOV_MEM_REG_REG_FROM_REG = 0x71,
    OP_ADD_REG_FROM_MEM_REG_REG = 0x72,
    OP_SUB_REG_FROM_MEM_REG_REG = 0x73,
    OP_CMP_REG_FROM_MEM_REG_REG = 0x74,

    // CL DYNAMIC SHIFT ROTATE
    OP_SHL_REG_CL = 0x75,
    OP_SAL_REG_CL = OP_SHL_REG_CL,
    OP_SHR_REG_CL = 0x76,
    OP_SAR_REG_CL = 0x77,
    OP_ROL_REG_CL = 0x78,
    OP_ROR_REG_CL = 0x79,
    OP_RCL_REG_CL = 0x7A,
    OP_RCR_REG_CL = 0x7B,

    // ADVANCED ADDRESSING MODES
    OP_MOV_MEM_IMM_FROM_IMM = 0x90,
    OP_MOV_MEM_IMM_FROM_IMM8 = 0x91,
    OP_MOV_REG_FROM_MEM_REG_IMM = 0x92,
    OP_MOV_MEM_REG_IMM_FROM_REG = 0x93,
    OP_MOV_REG8_FROM_MEM_REG_IMM = 0x94,
    OP_MOV_MEM_REG_IMM_FROM_REG8 = 0x95

};

enum RegisterCode
{
    REG_AX = 0x00,
    REG_BX = 0x01,
    REG_CX = 0x02,
    REG_DX = 0x03,
    REG_MNK = 0x04,
    REG_SP = 0x05,
    REG_SI = 0x06,
    REG_DI = 0x07,
    REG_BP = 0x08
};

enum RegisterCode8bit
{
    REG_AL = 0x00,
    REG_AH = 0x01,
    REG_BL = 0x02,
    REG_BH = 0x03,
    REG_CL = 0x04,
    REG_CH = 0x05,
    REG_DL = 0x06,
    REG_DH = 0x07,
    REG_MNL = 0x08,
    REG_MNH = 0x09
};

struct Flags
{
    bool CF = false; // CARRY FLAG
    bool ZF = false; // ZERO FLAG
    bool SF = false; // SIGN FLAG
    bool OF = false; // OVERFLOW FLAG
};

struct Registers
{
    // GENERAL PURPOSE REGISTERS (GPR)
    union
    {
        uint16_t AX;
        struct
        {
            uint8_t AL;
            uint8_t AH;
        };
    };
    union
    {
        uint16_t BX;
        struct
        {
            uint8_t BL;
            uint8_t BH;
        };
    };
    union
    {
        uint16_t CX;
        struct
        {
            uint8_t CL;
            uint8_t CH;
        };
    };
    union
    {
        uint16_t DX;
        struct
        {
            uint8_t DL;
            uint8_t DH;
        };
    };
    union
    {
        uint16_t MNK;
        struct
        {
            uint8_t MNL;
            uint8_t MNH;
        };
    };

    // SPESIFIC REGISTERS
    uint16_t SP; // STACK POINTER
    uint16_t BP; // BASE POINTER
    uint16_t SI; // SOURCE INDEX
    uint16_t DI; // DESTINATION INDEX
    uint16_t IP; // INSTRUCION POINTER
};

class CPU
{
private:
    uint16_t *get_register_ptr(uint8_t reg_code);
    uint8_t *get_register8_ptr(uint8_t reg_code);

    void write_mem16(uint16_t address, uint16_t value);
    uint16_t read_mem16(uint16_t address);

    void write_mem8(uint16_t address, uint8_t value);
    uint8_t read_mem8(uint16_t address);

    // Flag Calculator new auxiliary functions
    void update_flags_add(uint16_t dest_val, uint16_t src_val, uint32_t result);
    void update_flags_sub(uint16_t dest_val, uint16_t src_val, uint16_t result);
    void update_flags_logical(uint16_t result);
    void update_flags_inc(uint16_t val_before, uint16_t val_after);
    void update_flags_dec(uint16_t val_before, uint16_t val_after);

    void update_flags_add8(uint8_t dest_val, uint8_t src_val, uint16_t result);
    void update_flags_sub8(uint8_t dest_val, uint8_t src_val, uint8_t result);
    void update_flags_logical8(uint8_t result);
    void update_flags_inc8(uint8_t val_before, uint8_t val_after);
    void update_flags_dec8(uint8_t val_before, uint8_t val_after);

public:
    Registers regs;
    Flags flags;

    std::vector<uint8_t> memory;

    // CONSTRUCTOR
    CPU();

    // DESTRUCTOR
    ~CPU();

    void run();

    // DEBUG MODE
    bool step();
};
