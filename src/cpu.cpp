#include "cpu.h"
#include <iostream>
#include <iomanip>

CPU::CPU()
{
    memory.resize(65536, 0);

    // INITIALLY ALL REGISTERS SET BY ZERO
    regs.AX = 0;
    regs.BX = 0;
    regs.CX = 0;
    regs.DX = 0;
    regs.MNK = 0;
    regs.BP = 0;
    regs.SI = 0;
    regs.DI = 0;
    regs.IP = 0;
    flags.CF = 0;
    flags.ZF = 0;
    flags.SF = 0;
    flags.OF = 0;

    regs.SP = 0xFFFE;

    std::cout << "CPU initialized with 64KB of memory." << std::endl;
};

CPU::~CPU() {

};

uint16_t *CPU::get_register_ptr(uint8_t reg_code)
{
    switch (reg_code)
    {
    case REG_AX:
        return &regs.AX;
    case REG_BX:
        return &regs.BX;
    case REG_CX:
        return &regs.CX;
    case REG_DX:
        return &regs.DX;
    case REG_SP:
        return &regs.SP;
    case REG_BP:
        return &regs.BP;
    case REG_SI:
        return &regs.SI;
    case REG_DI:
        return &regs.DI;
    default:
        return nullptr;
    }
}

uint8_t *CPU::get_register8_ptr(uint8_t reg_code)
{
    switch (reg_code)
    {
    case REG_AL:
        return &regs.AL;
    case REG_AH:
        return &regs.AH;
    case REG_BL:
        return &regs.BL;
    case REG_BH:
        return &regs.BH;
    case REG_CL:
        return &regs.CL;
    case REG_CH:
        return &regs.CH;
    case REG_DL:
        return &regs.DL;
    case REG_DH:
        return &regs.DH;
    case REG_MNL:
        return &regs.MNL;
    case REG_MNH:
        return &regs.MNH;
    default:
        return nullptr;
    }
}

// AUTOMATING THE WRITING PROCESS
void CPU::write_mem16(uint16_t address, uint16_t value)
{
    memory[address] = value & 0xFF;
    memory[address + 1] = (value >> 8) & 0xFF;
}

void CPU::write_mem8(uint16_t address, uint8_t value)
{
    if (address < memory.size())
    {
        memory[address] = value;
    }
}

uint8_t CPU::read_mem8(uint16_t address)
{
    if (address < memory.size())
    {
        return memory[address];
    }
    return 0;
}

uint16_t CPU::read_mem16(uint16_t address)
{
    return (memory[address + 1] << 8) | memory[address];
}

void CPU::update_flags_sub(uint16_t dest_val, uint16_t src_val, uint16_t result)
{
    flags.ZF = (result == 0);
    flags.SF = (result & 0x8000) != 0;
    flags.CF = (dest_val < src_val);
    bool dest_sign = (dest_val & 0x8000) != 0;
    bool src_sign = (src_val & 0x8000) != 0;
    flags.OF = (dest_sign != src_sign) && (src_sign == flags.SF);
}

void CPU::update_flags_add(uint16_t dest_val, uint16_t src_val, uint32_t result)
{
    flags.ZF = ((result & 0xFFFF) == 0);
    flags.SF = (result & 0x8000) != 0;
    flags.CF = (result > 0xFFFF);
    bool dest_sign = (dest_val & 0x8000) != 0;
    bool src_sign = (src_val & 0x8000) != 0;
    flags.OF = (dest_sign == src_sign) && (dest_sign != flags.SF);
}

void CPU::update_flags_logical(uint16_t result)
{
    flags.CF = false;
    flags.OF = false;
    flags.ZF = (result == 0);
    flags.SF = (result & 0x8000) != 0;
}

void CPU::update_flags_inc(uint16_t val_before, uint16_t val_after)
{
    flags.ZF = (val_after == 0);
    flags.SF = (val_after & 0x8000) != 0;
    flags.OF = (val_before == 0x7FFF);
}

void CPU::update_flags_dec(uint16_t val_before, uint16_t val_after)
{
    flags.ZF = (val_after == 0);
    flags.SF = (val_after & 0x8000) != 0;
    flags.OF = (val_before == 0x8000); // OVERFLOW OCCURS IF AND ONLY IF 0X8000 TO 0X7FFFF
}
void CPU::update_flags_add8(uint8_t dest_val, uint8_t src_val, uint16_t result)
{
    flags.ZF = ((result & 0xFF) == 0);
    flags.SF = (result & 0x80) != 0;
    flags.CF = (result > 0xFF);
    bool dest_sign = (dest_val & 0x80) != 0;
    bool src_sign = (src_val & 0x80) != 0;
    flags.OF = (dest_sign == src_sign) && (dest_sign != flags.SF);
}
void CPU::update_flags_sub8(uint8_t dest_val, uint8_t src_val, uint8_t result)
{
    flags.ZF = (result == 0);
    flags.SF = (result & 0x80) != 0;
    flags.CF = (dest_val < src_val);
    bool dest_sign = (dest_val & 0x80) != 0;
    bool src_sign = (src_val & 0x80) != 0;
    flags.OF = (dest_sign != src_sign) && (src_sign == flags.SF);
}
void CPU::update_flags_logical8(uint8_t result)
{
    flags.CF = false;
    flags.OF = false;
    flags.ZF = (result == 0);
    flags.SF = (result & 0x80) != 0;
}
void CPU::update_flags_inc8(uint8_t val_before, uint8_t val_after)
{
    flags.ZF = (val_after == 0);
    flags.SF = (val_after & 0x80) != 0;
    flags.OF = (val_before == 0x7F);
}
void CPU::update_flags_dec8(uint8_t val_before, uint8_t val_after)
{
    flags.ZF = (val_after == 0);
    flags.SF = (val_after & 0x80) != 0;
    flags.OF = (val_before == 0x80);
}

bool CPU::step()
{
    uint8_t opcode = memory[regs.IP];

    switch (opcode)
    {
#define FETCH_REG16_PTR(offset) get_register_ptr(memory[regs.IP + offset])
#define FETCH_REG8_PTR(offset) get_register8_ptr(memory[regs.IP + offset])
#define FETCH_IMM16(offset) read_mem16(regs.IP + offset)
#define FETCH_IMM8(offset) memory[regs.IP + offset]

    // ===============================================================
    // == PART 1: PROGRAM CONTROLL BRANCH
    // ===============================================================
    case OP_HALT:
    {
        return false;
    }
    case OP_NOP:
    {
        regs.IP += 1;
        break;
    }
    case OP_JMP:
    {
        regs.IP = FETCH_IMM16(1);
        break;
    }
    case OP_CALL:
    {
        uint16_t target = FETCH_IMM16(1);
        uint16_t ret_addr = regs.IP + 3;
        regs.SP -= 2;
        write_mem16(regs.SP, ret_addr);
        regs.IP = target;
        break;
    }
    case OP_RET:
    {
        regs.IP = read_mem16(regs.SP);
        regs.SP += 2;
        break;
    }
    case OP_JZ:
    {
        regs.IP = flags.ZF ? FETCH_IMM16(1) : regs.IP + 3;
        break;
    }
    case OP_JNZ:
    {
        regs.IP = !flags.ZF ? FETCH_IMM16(1) : regs.IP + 3;
        break;
    }
    case OP_JC:
    {
        regs.IP = flags.CF ? FETCH_IMM16(1) : regs.IP + 3;
        break;
    }
    case OP_JNC:
    {
        regs.IP = !flags.CF ? FETCH_IMM16(1) : regs.IP + 3;
        break;
    }
    case OP_JS:
    {
        regs.IP = flags.SF ? FETCH_IMM16(1) : regs.IP + 3;
        break;
    }
    case OP_JNS:
    {
        regs.IP = !flags.SF ? FETCH_IMM16(1) : regs.IP + 3;
        break;
    }
    case OP_JO:
    {
        regs.IP = flags.OF ? FETCH_IMM16(1) : regs.IP + 3;
        break;
    }
    case OP_JNO:
    {
        regs.IP = !flags.OF ? FETCH_IMM16(1) : regs.IP + 3;
        break;
    }
        // ===============================================================
        // == PART 2: DATA TRANSFER
        // ===============================================================
    case OP_MOV_REG_IMM:
    {
        *FETCH_REG16_PTR(1) = FETCH_IMM16(2);
        regs.IP += 4;
        break;
    }
    case OP_MOV_REG_REG:
    {
        *FETCH_REG16_PTR(1) = *FETCH_REG16_PTR(2);
        regs.IP += 3;
        break;
    }
    case OP_MOV_REG8_IMM:
    {
        *FETCH_REG8_PTR(1) = FETCH_IMM8(2);
        regs.IP += 3;
        break;
    }
    case OP_MOV_REG8_REG8:
    {
        *FETCH_REG8_PTR(1) = *FETCH_REG8_PTR(2);
        regs.IP += 3;
        break;
    }
    case OP_MOV_REG_FROM_MEM_IMM:
    {
        *FETCH_REG16_PTR(1) = read_mem16(FETCH_IMM16(2));
        regs.IP += 4;
        break;
    }
    case OP_MOV_MEM_IMM_FROM_REG:
    {
        uint16_t target_address = FETCH_IMM16(2);
        uint16_t source_register = *FETCH_REG16_PTR(1);
        write_mem16(target_address, source_register);
        regs.IP += 4;
        break;
    }
    case OP_MOV_REG_FROM_MEM_REG:
    {
        *FETCH_REG16_PTR(1) = read_mem16(*FETCH_REG16_PTR(2));
        regs.IP += 3;
        break;
    }
    case OP_MOV_MEM_REG_FROM_REG:
    {
        write_mem16(*FETCH_REG16_PTR(2), *FETCH_REG16_PTR(1));
        regs.IP += 3;
        break;
    }
    case OP_MOV_REG8_FROM_MEM_IMM:
    {
        *FETCH_REG8_PTR(1) = memory[FETCH_IMM16(2)];
        regs.IP += 4;
        break;
    }
    case OP_MOV_MEM_IMM_FROM_REG8:
    {
        memory[FETCH_IMM16(2)] = *FETCH_REG8_PTR(1);
        regs.IP += 4;
        break;
    }
    case OP_MOV_REG8_FROM_MEM_REG:
    {
        *FETCH_REG8_PTR(1) = memory[*FETCH_REG16_PTR(2)];
        regs.IP += 3;
        break;
    }
    case OP_MOV_MEM_REG_FROM_REG8:
    {
        memory[*FETCH_REG16_PTR(2)] = *FETCH_REG8_PTR(1);
        regs.IP += 3;
        break;
    }
    case OP_MOV_REG_FROM_MEM_REG_REG:
    {
        uint16_t addr = *FETCH_REG16_PTR(2) + *FETCH_REG16_PTR(3);
        *FETCH_REG16_PTR(1) = read_mem16(addr);
        regs.IP += 4;
        break;
    }
    case OP_MOV_MEM_REG_REG_FROM_REG:
    {
        uint16_t addr = *FETCH_REG16_PTR(2) + *FETCH_REG16_PTR(3);
        write_mem16(addr, *FETCH_REG16_PTR(1));
        regs.IP += 4;
        break;
    }
    case OP_XCHG_REG_REG:
    {
        std::swap(*FETCH_REG16_PTR(1), *FETCH_REG16_PTR(2));
        regs.IP += 3;
        break;
    }
    case OP_XCHG_REG8_REG8:
    {
        std::swap(*FETCH_REG8_PTR(1), *FETCH_REG8_PTR(2));
        regs.IP += 3;
        break;
    }
        // ===============================================================
        // == PART 3: ADVANCED ADRESSING MODES
        // ===============================================================

    case OP_MOV_MEM_IMM_FROM_IMM:
    {
        uint16_t address = FETCH_IMM16(1);
        uint16_t value = FETCH_IMM16(3);
        write_mem16(address, value);
        regs.IP += 5;
        break;
    }
    case OP_MOV_MEM_IMM_FROM_IMM8:
    {
        uint16_t address = FETCH_IMM16(1);
        uint16_t value = FETCH_IMM8(3);
        write_mem8(address, value);
        regs.IP += 4;
        break;
    }
    // ===============================================================
    // == PART 4: STACK OPERATIONS
    // ===============================================================
    case OP_PUSH_REG:
    {
        regs.SP -= 2;
        write_mem16(regs.SP, *FETCH_REG16_PTR(1));
        regs.IP += 2;
        break;
    }
    case OP_POP_REG:
    {
        *FETCH_REG16_PTR(1) = read_mem16(regs.SP);
        regs.SP += 2;
        regs.IP += 2;
        break;
    }
    // ===============================================================
    // == PART 5: ARITHMETIC LOGICAL OPERATIONS
    // ===============================================================
    case OP_ADD_REG_REG:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint16_t *s = FETCH_REG16_PTR(2);
        uint16_t dv = *d;
        uint32_t res = (uint32_t)dv + *s;
        *d = res;
        update_flags_add(dv, *s, res);
        regs.IP += 3;
        break;
    }
    case OP_ADD_REG_IMM:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint16_t s = FETCH_IMM16(2);
        uint16_t dv = *d;
        uint32_t res = (uint32_t)dv + s;
        *d = res;
        update_flags_add(dv, s, res);
        regs.IP += 4;
        break;
    }
    case OP_SUB_REG_REG:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint16_t *s = FETCH_REG16_PTR(2);
        uint16_t dv = *d;
        *d -= *s;
        update_flags_sub(dv, *s, *d);
        regs.IP += 3;
        break;
    }
    case OP_SUB_REG_IMM:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint16_t s = FETCH_IMM16(2);
        uint16_t dv = *d;
        *d -= s;
        update_flags_sub(dv, s, *d);
        regs.IP += 4;
        break;
    }
    case OP_ADC_REG_REG:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint16_t *s = FETCH_REG16_PTR(2);
        uint16_t dv = *d;
        uint32_t res = (uint32_t)dv + *s + flags.CF;
        *d = res;
        update_flags_add(dv, *s, res);
        regs.IP += 3;
        break;
    }
    case OP_ADC_REG_IMM:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint16_t s = FETCH_IMM16(2);
        uint16_t dv = *d;
        uint32_t res = (uint32_t)dv + s + flags.CF;
        *d = res;
        update_flags_add(dv, s, res);
        regs.IP += 4;
        break;
    }
    case OP_SBB_REG_REG:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint16_t *s = FETCH_REG16_PTR(2);
        uint16_t dv = *d;
        uint16_t sv = *s;
        uint16_t cf = flags.CF;
        *d = dv - sv - cf;
        update_flags_sub(dv, sv + cf, *d);
        regs.IP += 3;
        break;
    }
    case OP_SBB_REG_IMM:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint16_t s = FETCH_IMM16(2);
        uint16_t dv = *d;
        uint16_t cf = flags.CF;
        *d = dv - s - cf;
        update_flags_sub(dv, s + cf, *d);
        regs.IP += 4;
        break;
    }
    case OP_CMP_REG_REG:
    {
        uint16_t d = *FETCH_REG16_PTR(1);
        uint16_t s = *FETCH_REG16_PTR(2);
        update_flags_sub(d, s, d - s);
        regs.IP += 3;
        break;
    }
    case OP_CMP_REG_IMM:
    {
        uint16_t d = *FETCH_REG16_PTR(1);
        uint16_t s = FETCH_IMM16(2);
        update_flags_sub(d, s, d - s);
        regs.IP += 4;
        break;
    }
    case OP_NEG_REG16:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint16_t val = *d;
        *d = -val;
        flags.CF = (val != 0);
        update_flags_sub(0, val, *d);
        regs.IP += 2;
        break;
    }
    case OP_NOT_REG:
    {
        *FETCH_REG16_PTR(1) = ~(*FETCH_REG16_PTR(1));
        regs.IP += 2;
        break;
    }
    case OP_INC_REG:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint16_t dv = *d;
        (*d)++;
        update_flags_inc(dv, *d);
        regs.IP += 2;
        break;
    }
    case OP_DEC_REG:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint16_t dv = *d;
        (*d)--;
        update_flags_dec(dv, *d);
        regs.IP += 2;
        break;
    }
    case OP_AND_REG_IMM:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint16_t s = FETCH_IMM16(2);
        *d &= s;
        update_flags_logical(*d);
        regs.IP += 4;
        break;
    }
    case OP_OR_REG_IMM:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint16_t s = FETCH_IMM16(2);
        *d |= s;
        update_flags_logical(*d);
        regs.IP += 4;
        break;
    }
    case OP_XOR_REG_IMM:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint16_t s = FETCH_IMM16(2);
        *d ^= s;
        update_flags_logical(*d);
        regs.IP += 4;
        break;
    }

    case OP_AND_REG_REG:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        *d &= *FETCH_REG16_PTR(2);
        update_flags_logical(*d);
        regs.IP += 3;
        break;
    }

    case OP_OR_REG_REG:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        *d |= *FETCH_REG16_PTR(2);
        update_flags_logical(*d);
        regs.IP += 3;
        break;
    }
    case OP_XOR_REG_REG:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        *d ^= *FETCH_REG16_PTR(2);
        update_flags_logical(*d);
        regs.IP += 3;
        break;
    }
    case OP_ADD_REG8_REG8:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t *s = FETCH_REG8_PTR(2);
        uint8_t dv = *d;
        uint16_t res = (uint16_t)dv + *s;
        *d = res;
        update_flags_add8(dv, *s, res);
        regs.IP += 3;
        break;
    }
    case OP_ADD_REG8_IMM:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t s = FETCH_IMM8(2);
        uint8_t dv = *d;
        uint16_t res = (uint16_t)dv + s;
        *d = res;
        update_flags_add8(dv, s, res);
        regs.IP += 3;
        break;
    }
    case OP_SUB_REG8_IMM:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t s = FETCH_IMM8(2);
        uint8_t dv = *d;
        *d -= s;
        update_flags_sub8(dv, s, *d);
        regs.IP += 3;
        break;
    }
    case OP_ADC_REG8_IMM:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t s = FETCH_IMM8(2);
        uint8_t dv = *d;
        uint16_t res = (uint16_t)dv + s + flags.CF;
        *d = res;
        update_flags_add8(dv, s, res);
        regs.IP += 3;
        break;
    }
    case OP_SBB_REG8_IMM:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t s = FETCH_IMM8(2);
        uint8_t dv = *d;
        uint8_t cf = flags.CF;
        *d = dv - s - cf;
        update_flags_sub8(dv, s + cf, *d);
        regs.IP += 3;
        break;
    }
    case OP_CMP_REG8_IMM:
    {
        uint8_t d = *FETCH_REG8_PTR(1);
        uint8_t s = FETCH_IMM8(2);
        update_flags_sub8(d, s, d - s);
        regs.IP += 3;
        break;
    }

    case OP_SUB_REG8_REG8:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t *s = FETCH_REG8_PTR(2);
        uint8_t dv = *d;
        *d -= *s;
        update_flags_sub8(dv, *s, *d);
        regs.IP += 3;
        break;
    }
    case OP_ADC_REG8_REG8:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t *s = FETCH_REG8_PTR(2);
        uint8_t dv = *d;
        uint16_t res = (uint16_t)dv + *s + flags.CF;
        *d = res;
        update_flags_add8(dv, *s, res);
        regs.IP += 3;
        break;
    }
    case OP_SBB_REG8_REG8:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t *s = FETCH_REG8_PTR(2);
        uint8_t dv = *d;
        uint8_t sv = *s;
        uint8_t cf = flags.CF;
        *d = dv - sv - cf;
        update_flags_sub8(dv, sv + cf, *d);
        regs.IP += 3;
        break;
    }
    case OP_CMP_REG8_REG8:
    {
        uint8_t d = *FETCH_REG8_PTR(1);
        uint8_t s = *FETCH_REG8_PTR(2);
        update_flags_sub8(d, s, d - s);
        regs.IP += 3;
        break;
    }
    case OP_NEG_REG8:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t val = *d;
        *d = -val;
        flags.CF = (val != 0);
        update_flags_sub8(0, val, *d);
        regs.IP += 2;
        break;
    }
    case OP_NOT_REG8:
    {
        *FETCH_REG8_PTR(1) = ~(*FETCH_REG8_PTR(1));
        regs.IP += 2;
        break;
    }
    case OP_INC_REG8:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t dv = *d;
        (*d)++;
        update_flags_inc8(dv, *d);
        regs.IP += 2;
        break;
    }
    case OP_DEC_REG8:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t dv = *d;
        (*d)--;
        update_flags_dec8(dv, *d);
        regs.IP += 2;
        break;
    }
    case OP_AND_REG8_IMM:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t s = FETCH_IMM8(2);
        *d &= s;
        update_flags_logical8(*d);
        regs.IP += 3;
        break;
    }
    case OP_OR_REG8_IMM:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t s = FETCH_IMM8(2);
        *d |= s;
        update_flags_logical8(*d);
        regs.IP += 3;
        break;
    }
    case OP_XOR_REG8_IMM:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t s = FETCH_IMM8(2);
        *d ^= s;
        update_flags_logical8(*d);
        regs.IP += 3;
        break;
    }

    case OP_AND_REG8_REG8:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        *d &= *FETCH_REG8_PTR(2);
        update_flags_logical8(*d);
        regs.IP += 3;
        break;
    }
    case OP_OR_REG8_REG8:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        *d |= *FETCH_REG8_PTR(2);
        update_flags_logical8(*d);
        regs.IP += 3;
        break;
    }
    case OP_XOR_REG8_REG8:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        *d ^= *FETCH_REG8_PTR(2);
        update_flags_logical8(*d);
        regs.IP += 3;
        break;
    }
        // ===============================================================
        // == PART 6: BIT SHIFTING AND ROTATE
        // ===============================================================

    case OP_SHL_REG_IMM:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint8_t count = FETCH_IMM8(2);
        for (int i = 0; i < count; ++i)
        {
            flags.CF = (*d & 0x8000);
            *d <<= 1;
        }
        update_flags_logical(*d);
        if (count == 1)
            flags.OF = (*d & 0x8000) != flags.CF;
        regs.IP += 3;
        break;
    }
    case OP_SHR_REG_IMM:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint8_t count = FETCH_IMM8(2);
        uint16_t val_before = *d;
        for (int i = 0; i < count; ++i)
        {
            flags.CF = (*d & 1);
            *d >>= 1;
        }
        update_flags_logical(*d);
        if (count == 1)
            flags.OF = (val_before & 0x8000);
        regs.IP += 3;
        break;
    }
    case OP_SAR_REG_IMM:
    {
        int16_t *d = (int16_t *)FETCH_REG16_PTR(1);
        uint8_t count = FETCH_IMM8(2);
        for (int i = 0; i < count; ++i)
        {
            flags.CF = (*d & 1);
            *d >>= 1;
        }
        update_flags_logical(*d);
        flags.OF = false;
        regs.IP += 3;
        break;
    }
    case OP_SHL_REG_CL:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint8_t count = regs.CL;
        for (int i = 0; i < count; ++i)
        {
            flags.CF = (*d & 0x8000);
            *d <<= 1;
        }
        update_flags_logical(*d);
        if (count == 1)
            flags.OF = (*d & 0x8000) != flags.CF;
        regs.IP += 2;
        break;
    }
    case OP_SHR_REG_CL:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint8_t count = regs.CL;
        uint16_t val_before = *d;
        for (int i = 0; i < count; ++i)
        {
            flags.CF = (*d & 1);
            *d >>= 1;
        }
        update_flags_logical(*d);
        if (count == 1)
            flags.OF = (val_before & 0x8000);
        regs.IP += 2;
        break;
    }
    case OP_SAR_REG_CL:
    {
        int16_t *d = (int16_t *)FETCH_REG16_PTR(1);
        uint8_t count = regs.CL;
        for (int i = 0; i < count; ++i)
        {
            flags.CF = (*d & 1);
            *d >>= 1;
        }
        update_flags_logical(*d);
        flags.OF = false;
        regs.IP += 2;
        break;
    }
    case OP_ROL_REG_CL:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint8_t count = regs.CL;
        for (int i = 0; i < count; ++i)
        {
            bool msb = (*d & 0x8000);
            *d = (*d << 1) | msb;
            flags.CF = msb;
        }
        if (count % 16 != 0 && count != 0)
        {
            if (count == 1)
                flags.OF = (*d & 0x8000) != flags.CF;
        }
        regs.IP += 2;
        break;
    }
    case OP_ROR_REG_CL:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint8_t count = regs.CL;
        for (int i = 0; i < count; ++i)
        {
            bool lsb = (*d & 1);
            *d = (*d >> 1) | (lsb << 15);
            flags.CF = lsb;
        }
        if (count % 16 != 0 && count != 0)
        {
            if (count == 1)
                flags.OF = (*d & 0x8000) != (*d & 0x4000);
        }
        regs.IP += 2;
        break;
    }
    case OP_RCL_REG_CL:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint8_t count = regs.CL;
        for (int i = 0; i < count; ++i)
        {
            bool msb = (*d & 0x8000);
            bool old_cf = flags.CF;
            *d = (*d << 1) | old_cf;
            flags.CF = msb;
        }
        if (count % 17 != 0 && count != 0)
        {
            if (count == 1)
                flags.OF = (*d & 0x8000) != flags.CF;
        }
        regs.IP += 2;
        break;
    }
    case OP_RCR_REG_CL:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint8_t count = regs.CL;
        for (int i = 0; i < count; ++i)
        {
            bool lsb = (*d & 1);
            bool old_cf = flags.CF;
            *d = (*d >> 1) | (old_cf << 15);
            flags.CF = lsb;
        }
        if (count % 17 != 0 && count != 0)
        {
            if (count == 1)
                flags.OF = (*d & 0x8000) != (*d & 0x4000);
        }
        regs.IP += 2;
        break;
    }
    case OP_ROL_REG_IMM:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint8_t count = FETCH_IMM8(2);
        for (int i = 0; i < count; ++i)
        {
            bool msb = (*d & 0x8000);
            *d = (*d << 1) | msb;
            flags.CF = msb;
        }
        if (count == 1)
            flags.OF = ((*d & 0x8000) != flags.CF);
        regs.IP += 3;
        break;
    }
    case OP_ROR_REG_IMM:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint8_t count = FETCH_IMM8(2);
        for (int i = 0; i < count; ++i)
        {
            bool lsb = (*d & 1);
            *d = (*d >> 1) | (lsb << 15);
            flags.CF = lsb;
        }
        if (count == 1)
            flags.OF = ((*d & 0x8000) != (*d & 0x4000));
        regs.IP += 3;
        break;
    }
    case OP_RCL_REG_IMM:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint8_t count = FETCH_IMM8(2);
        for (int i = 0; i < count; ++i)
        {
            bool msb = (*d & 0x8000);
            bool old_cf = flags.CF;
            *d = (*d << 1) | old_cf;
            flags.CF = msb;
        }
        if (count == 1)
            flags.OF = ((*d & 0x8000) != flags.CF);
        regs.IP += 3;
        break;
    }
    case OP_RCR_REG_IMM:
    {
        uint16_t *d = FETCH_REG16_PTR(1);
        uint8_t count = FETCH_IMM8(2);
        for (int i = 0; i < count; ++i)
        {
            bool lsb = (*d & 1);
            bool old_cf = flags.CF;
            *d = (*d >> 1) | (old_cf << 15);
            flags.CF = lsb;
        }
        if (count == 1)
            flags.OF = ((*d & 0x8000) != (*d & 0x4000));
        regs.IP += 3;
        break;
    }
    case OP_ROL_REG8_IMM:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t count = FETCH_IMM8(2) & 0x1F;
        for (int i = 0; i < count; i++)
        {
            bool msb = (*d & 0x80);
            *d = (*d << 1) | msb;
            flags.CF = msb;
        }
        if (count == 1)
            flags.OF = ((*d & 0x80) != flags.CF);
        regs.IP += 3;
        break;
    }
    case OP_ROR_REG8_IMM:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t count = FETCH_IMM8(2) & 0x1F;
        for (int i = 0; i < count; i++)
        {
            bool lsb = (*d & 1);
            *d = (*d >> 1) | (lsb << 7);
            flags.CF = lsb;
        }
        if (count == 1)
            flags.OF = ((*d & 0x80) != (*d & 0x40));
        regs.IP += 3;
        break;
    }
    case OP_RCL_REG8_IMM:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t count = FETCH_IMM8(2) & 0x1F;
        for (int i = 0; i < count; i++)
        {
            bool msb = (*d & 0x80);
            bool old_cf = flags.CF;
            *d = (*d << 1) | old_cf;
            flags.CF = msb;
        }
        if (count == 1)
            flags.OF = ((*d & 0x80) != flags.CF);
        regs.IP += 3;
        break;
    }
    case OP_RCR_REG8_IMM:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t count = FETCH_IMM8(2) & 0x1F;
        for (int i = 0; i < count; i++)
        {
            bool lsb = (*d & 1);
            bool old_cf = flags.CF;
            *d = (*d >> 1) | (old_cf << 7);
            flags.CF = lsb;
        }
        if (count == 1)
            flags.OF = ((*d & 0x80) != (*d & 0x40));
        regs.IP += 3;
        break;
    }
    case OP_SHL_REG8_IMM:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t count = FETCH_IMM8(2);
        for (int i = 0; i < count; ++i)
        {
            flags.CF = (*d & 0x80);
            *d <<= 1;
        }
        update_flags_logical8(*d);
        if (count == 1)
            flags.OF = ((*d & 0x80) != flags.CF);
        regs.IP += 3;
        break;
    }

    case OP_SHR_REG8_IMM:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t count = FETCH_IMM8(2);
        uint8_t val_before = *d;
        for (int i = 0; i < count; ++i)
        {
            flags.CF = (*d & 1);
            *d >>= 1;
        }
        update_flags_logical8(*d);
        if (count == 1)
            flags.OF = (val_before & 0x80);
        regs.IP += 3;
        break;
    }
    case OP_SAR_REG8_IMM:
    {
        int8_t *d = (int8_t *)FETCH_REG8_PTR(1);
        uint8_t count = FETCH_IMM8(2);
        for (int i = 0; i < count; ++i)
        {
            flags.CF = (*d & 1);
            *d >>= 1;
        }
        update_flags_logical8(*d);
        flags.OF = false;
        regs.IP += 3;
        break;
    }
    case OP_SHL_REG8_CL:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t count = regs.CL;
        for (int i = 0; i < count; ++i)
        {
            flags.CF = (*d & 0x80);
            *d <<= 1;
        }
        update_flags_logical8(*d);
        if (count == 1)
            flags.OF = ((*d & 0x80) != flags.CF);
        regs.IP += 2;
        break;
    }
    case OP_SHR_REG8_CL:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t count = regs.CL;
        uint8_t val_before = *d;
        for (int i = 0; i < count; ++i)
        {
            flags.CF = (*d & 1);
            *d >>= 1;
        }
        update_flags_logical8(*d);
        if (count == 1)
            flags.OF = (val_before & 0x80);
        regs.IP += 2;
        break;
    }
    case OP_SAR_REG8_CL:
    {
        int8_t *d = (int8_t *)FETCH_REG8_PTR(1);
        uint8_t count = regs.CL;
        for (int i = 0; i < count; ++i)
        {
            flags.CF = (*d & 1);
            *d >>= 1;
        }
        update_flags_logical8(*d);
        flags.OF = false;
        regs.IP += 2;
        break;
    }
    case OP_ROL_REG8_CL:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t count = regs.CL;
        for (int i = 0; i < count; ++i)
        {
            bool msb = (*d & 0x80);
            *d = (*d << 1) | msb;
            flags.CF = msb;
        }
        if (count % 8 != 0 && count != 0)
        {
            if (count == 1)
                flags.OF = ((*d & 0x80) != flags.CF);
        }
        regs.IP += 2;
        break;
    }
    case OP_ROR_REG8_CL:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t count = regs.CL;
        for (int i = 0; i < count; ++i)
        {
            bool lsb = (*d & 1);
            *d = (*d >> 1) | (lsb << 7);
            flags.CF = lsb;
        }
        if (count % 8 != 0 && count != 0)
        {
            if (count == 1)
                flags.OF = ((*d & 0x80) != (*d & 0x40));
        }
        regs.IP += 2;
        break;
    }
    case OP_RCL_REG8_CL:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t count = regs.CL;
        for (int i = 0; i < count; ++i)
        {
            bool msb = (*d & 0x80);
            bool old_cf = flags.CF;
            *d = (*d << 1) | old_cf;
            flags.CF = msb;
        }
        if (count % 9 != 0 && count != 0)
        {
            if (count == 1)
                flags.OF = ((*d & 0x80) != flags.CF);
        }
        regs.IP += 2;
        break;
    }
    case OP_RCR_REG8_CL:
    {
        uint8_t *d = FETCH_REG8_PTR(1);
        uint8_t count = regs.CL;
        for (int i = 0; i < count; ++i)
        {
            bool lsb = (*d & 1);
            bool old_cf = flags.CF;
            *d = (*d >> 1) | (old_cf << 7);
            flags.CF = lsb;
        }
        if (count % 9 != 0 && count != 0)
        {
            if (count == 1)
                flags.OF = ((*d & 0x80) != (*d & 0x40));
        }
        regs.IP += 2;
        break;
    }

    default:
    {
        std::cerr << "ERROR: Unknown OPCODE 0x" << std::hex << std::setw(2) << std::setfill('0')
                  << (int)opcode << " at address 0x" << std::setw(4) << (int)regs.IP << std::dec << std::endl;
        return false;
    }
    }
    return true;
}

void CPU::run()
{
    //bool running = true;
    while (step())
    {
    }
    std::cout << "CPU Halted." << std::endl;
}

//  case OP_MOV_REG_IMM:
//     {
//         // Length 4 byte
//         //  [OPCODE][Target Register Code][LOW BYTE][HIGH BYTE] <Commmand Format>
//         uint8_t reg_code = memory[regs.IP + 1];
//         uint16_t value = (memory[regs.IP + 3]) << 8 | memory[regs.IP + 2];

//         uint16_t *dest_ptr = get_register_ptr(reg_code);

//         if (dest_ptr)
//         {
//             *dest_ptr = value;
//         }
//         regs.IP += 4;
//         break;
//     }

//     case OP_MOV_REG_REG:
//     {
//         uint8_t dest_code = memory[regs.IP + 1];
//         uint8_t source_code = memory[regs.IP + 2];
//         uint16_t *dest_ptr = get_register_ptr(dest_code);
//         uint16_t *source_ptr = get_register_ptr(source_code);

//         if (dest_ptr && source_ptr)
//         {
//             *dest_ptr = *source_ptr;
//         }
//         regs.IP += 3;
//         break;
//     }

//     case OP_XCHG_REG_REG:
//     {
//         uint8_t dest_code = memory[regs.IP + 1];
//         uint8_t source_code = memory[regs.IP + 2];

//         uint16_t *dest_ptr = get_register_ptr(dest_code);
//         uint16_t *source_ptr = get_register_ptr(source_code);

//         if (dest_ptr && source_ptr)
//         {
//             std::swap(*dest_ptr, *source_ptr);
//         }
//         regs.IP += 3;
//         break;
//     }

//     case OP_ADD_REG_IMM:
//     {
//         uint8_t reg_code = memory[regs.IP + 1];
//         uint16_t value = (memory[regs.IP + 3]) << 8 | memory[regs.IP + 2];

//         uint16_t *dest_ptr = get_register_ptr(reg_code);

//         if (dest_ptr)
//         {
//             *dest_ptr += value;
//             flags.ZF = (*dest_ptr == 0);
//         }
//         regs.IP += 4;
//         break;
//     }

//     case OP_ADD_REG_REG:
//     {
//         uint8_t dest_code = memory[regs.IP + 1];
//         uint8_t source_code = memory[regs.IP + 2];
//         uint16_t *dest_ptr = get_register_ptr(dest_code);
//         uint16_t *source_ptr = get_register_ptr(source_code);

//         if (dest_ptr && source_ptr)
//         {
//             *dest_ptr += *source_ptr;
//             flags.ZF = (*dest_ptr == 0);
//         }
//         regs.IP += 3;
//         break;
//     }
//     case OP_SUB_REG_IMM:
//     {
//         uint8_t dest_code = memory[regs.IP + 1];
//         uint16_t value = (memory[regs.IP + 3] << 8) | memory[regs.IP + 2];

//         uint16_t *dest_ptr = get_register_ptr(dest_code);

//         if (dest_ptr)
//         {
//             *dest_ptr -= value;
//             flags.ZF = (*dest_ptr == 0);
//         }
//         regs.IP += 4;
//         break;
//     }

//     case OP_SUB_REG_REG:
//     {
//         uint8_t dest_code = memory[regs.IP + 1];
//         uint8_t source_code = memory[regs.IP + 2];
//         uint16_t *dest_ptr = get_register_ptr(dest_code);
//         uint16_t *source_ptr = get_register_ptr(source_code);

//         if (dest_ptr && source_ptr)
//         {
//             *dest_ptr -= *source_ptr;
//             flags.ZF = (*dest_ptr == 0);
//         }
//         regs.IP += 3;
//         break;
//     }

//     case OP_ADC_REG_REG:
//     {
//         uint8_t dest_code = memory[regs.IP + 1];
//         uint8_t source_code = memory[regs.IP + 2];

//         uint16_t *dest_ptr = get_register_ptr(dest_code);
//         uint16_t *source_ptr = get_register_ptr(source_code);

//         if (dest_ptr && source_ptr)
//         {
//             uint16_t carry_value = flags.CF ? 1 : 0;
//             *dest_ptr += *source_ptr + carry_value;
//             flags.ZF = (*dest_ptr == 0);
//         }
//         regs.IP += 3;
//         break;
//     }

//     case OP_NEG_REG:
//     {
//         uint8_t dest_code = memory[regs.IP + 1];
//         uint16_t *dest_ptr = get_register_ptr(dest_code);

//         if (dest_ptr)
//         {
//             *dest_ptr = -(*dest_ptr);
//             flags.ZF = (*dest_ptr == 0);
//         }
//         regs.IP += 2;
//         break;
//     }

//     case OP_AND_REG_REG:
//     {
//         uint16_t *dest_ptr = get_register_ptr(memory[regs.IP + 1]);
//         uint16_t *source_ptr = get_register_ptr(memory[regs.IP + 2]);

//         if (dest_ptr && source_ptr)
//         {
//             *dest_ptr &= *source_ptr;
//             flags.ZF = (*dest_ptr == 0);
//         }
//         regs.IP += 3;
//         break;
//     }

//     case OP_OR_REG_REG:
//     {
//         uint16_t *dest_ptr = get_register_ptr(memory[regs.IP + 1]);
//         uint16_t *source_ptr = get_register_ptr(memory[regs.IP + 2]);

//         if (dest_ptr && source_ptr)
//         {
//             *dest_ptr |= *source_ptr;
//             flags.ZF = (*dest_ptr == 0);
//         }
//         regs.IP += 3;
//         break;
//     }

//     case OP_XOR_REG_IMM:
//     {
//         uint8_t dest_code = memory[regs.IP + 1];
//         uint16_t value = ((memory[regs.IP + 3] << 8) | memory[regs.IP + 2]);

//         uint16_t *dest_ptr = get_register_ptr(dest_code);

//         if (dest_ptr)
//         {
//             *dest_ptr ^= value; // XOR ^=
//             flags.ZF = (*dest_ptr == 0);
//         }
//         regs.IP += 4;
//         break;
//     }

//     case OP_XOR_REG_REG:
//     {
//         uint8_t dest_code = memory[regs.IP + 1];
//         uint8_t source_code = memory[regs.IP + 2];
//         uint16_t *dest_ptr = get_register_ptr(dest_code);
//         uint16_t *source_ptr = get_register_ptr(source_code);

//         if (dest_ptr && source_ptr)
//         {
//             *dest_ptr ^= *source_ptr;
//             flags.ZF = (*dest_ptr == 0);
//         }
//         regs.IP += 3;
//         break;
//     }
//     case OP_NOT_REG:
//     {
//         uint8_t dest_code = memory[regs.IP + 1];
//         uint16_t *dest_ptr = get_register_ptr(dest_code);

//         if (dest_ptr)
//         {
//             *dest_ptr = ~(*dest_ptr); // ~ bitwise NOT
//         }
//         regs.IP += 2;
//         break;
//     }

//     case OP_SHL_REG:
//     {
//         uint8_t dest_code = memory[regs.IP + 1];
//         uint16_t *dest_ptr = get_register_ptr(dest_code);

//         if (dest_ptr)
//         {
//             flags.CF = (*dest_ptr & 0x8000) != 0; // left most bit = carry
//             *dest_ptr <<= 1;
//             flags.ZF = (*dest_ptr == 0);
//         }
//         regs.IP += 2;
//         break;
//     }

//     case OP_CMP_REG_IMM:
//     {
//         uint8_t dest_code = memory[regs.IP + 1];
//         uint16_t value = (memory[regs.IP + 3] << 8) | memory[regs.IP + 2];

//         uint16_t *dest_ptr = get_register_ptr(dest_code);

//         if (dest_ptr)
//         {
//             uint16_t result = *dest_ptr - value;
//             flags.ZF = (result == 0);
//         }
//         regs.IP += 4;
//         break;
//     }

//     case OP_CMP_REG_REG:
//     {
//         uint8_t dest_code = memory[regs.IP + 1];
//         uint8_t source_code = memory[regs.IP + 2];

//         uint16_t *dest_ptr = get_register_ptr(dest_code);
//         uint16_t *source_ptr = get_register_ptr(source_code);

//         if (dest_ptr && source_ptr)
//         {
//             uint16_t result = *dest_ptr - *source_ptr;
//             flags.ZF = (result == 0);
//         }
//         regs.IP += 3;
//         break;
//     }

//     case OP_JMP:
//     {
//         uint8_t low_byte = memory[regs.IP + 1];
//         uint8_t high_byte = memory[regs.IP + 2];

//         uint16_t target_adress = (high_byte << 8) | low_byte;

//         regs.IP = target_adress;
//         break;
//     }

//     case OP_JNZ:
//     {
//         if (!flags.ZF)
//         {
//             uint16_t target_adress = (memory[regs.IP + 1] << 8) | memory[regs.IP];
//             regs.IP = target_adress;
//         }
//         else
//         {
//             regs.IP += 3;
//         }
//         break;
//     }

//     case OP_PUSH_REG:
//     {
//         uint8_t dest_code = memory[regs.IP + 1];
//         uint16_t *source_ptr = get_register_ptr(dest_code);

//         if (source_ptr)
//         {
//             regs.SP -= 2;
//             uint16_t value = *source_ptr;
//             memory[regs.SP] = value & 0xFF;
//             memory[regs.SP + 1] = (value >> 8) & 0xFF;
//         }
//         regs.IP += 2;
//         break;
//     }
//     case OP_POP_REG:
//     {
//         uint8_t dest_code = memory[regs.IP + 1];
//         uint16_t *dest_ptr = get_register_ptr(dest_code);

//         if (dest_ptr)
//         {
//             uint16_t value = (memory[regs.SP + 1] << 8) | memory[regs.SP];

//             *dest_ptr = value;
//             regs.SP += 2;
//         }
//         regs.IP += 2;
//         break;
//     }

//     case OP_HALT:
//     {
//         std::cout << "EXECUTED: HALT" << std::endl;
//         return false;
//     }
//     default:
//     {
//         std::cerr << "ERROR: Unknown OPCODE 0x" << std::hex << std::setw(2) << std::setfill('0')
//                   << (int)opcode << " at address 0x" << std::setw(4) << (int)regs.IP << std::dec << std::endl;
//         return false;
//     }
//     }
//     return true;
