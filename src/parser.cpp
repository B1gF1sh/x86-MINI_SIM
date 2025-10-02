#include "parser.h"
#include "cpu.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <cctype>

// STATIC MAPS AND HELPER STRUCTS/ENUMS REMAIN UNCHANGED AS THEY ARE WELL-DESIGNED
static const std::unordered_map<std::string, RegisterCode> RegisterMap = {
    {"AX", REG_AX}, {"BX", REG_BX}, {"CX", REG_CX}, {"DX", REG_DX}, {"MNK", REG_MNK}, {"SP", REG_SP}, {"BP", REG_BP}, {"SI", REG_SI}, {"DI", REG_DI}};

static const std::unordered_map<std::string, RegisterCode8bit> RegisterMap8 = {
    {"AL", REG_AL}, {"AH", REG_AH}, {"BL", REG_BL}, {"BH", REG_BH}, {"CL", REG_CL}, {"CH", REG_CH}, {"DL", REG_DL}, {"DH", REG_DH}, {"MNL", REG_MNL}, {"MNH", REG_MNH}};

enum OperandType
{
    TYPE_NONE,
    TYPE_REG16,
    TYPE_REG8,
    TYPE_MEM_FROM_REG,
    TYPE_MEM_FROM_IMM,
    TYPE_MEM_REG_REG,
    TYPE_IMMEDIATE,
    TYPE_LABEL
};

struct Operand
{
    OperandType type = TYPE_NONE;
    uint16_t value = 0;
    uint8_t reg_code = 0;
    uint8_t reg_code2 = 0; // [BX+SI]
    std::string str_val;
};

// HELPER FUNCTIONS (PARSING)
std::string trim(const std::string &str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first)
        return str;
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

std::string clean_line(std::string line)
{
    auto comment_pos = line.find(';');
    return trim((comment_pos != std::string::npos) ? line.substr(0, comment_pos) : line);
}

static bool try_parse_number(const std::string &s, uint16_t &out)
{
    try
    {
        if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        {
            out = static_cast<uint16_t>(std::stoul(s.substr(2), nullptr, 16));
            return true;
        }
        if (!s.empty() && (s.back() == 'h' || s.back() == 'H'))
        {
            out = static_cast<uint16_t>(std::stoul(s.substr(0, s.size() - 1), nullptr, 16));
            return true;
        }

        out = static_cast<uint16_t>(std::stoul(s, nullptr, 10));
        return true;
    }
    catch (...)
    {
        return false;
    }
}

Operand parse_operand(std::string op_str)
{
    Operand op;
    op_str = trim(op_str);

    if (op_str.empty())
    {
        op.type = TYPE_NONE;
        return op;
    }

    op.str_val = op_str;

    std::string op_upper = op_str;
    std::transform(op_upper.begin(), op_upper.end(), op_upper.begin(), ::toupper);

    if (RegisterMap.count(op_upper))
    {
        op.type = TYPE_REG16;
        op.reg_code = RegisterMap.at(op_upper);
        return op;
    }
    if (RegisterMap8.count(op_upper))
    {
        op.type = TYPE_REG8;
        op.reg_code = RegisterMap8.at(op_upper);
        return op;
    }
    if (op_str.front() == '[' && op_str.back() == ']')
    {
        std::string content = trim(op_str.substr(1, op_str.length() - 2));
        auto plus_pos = content.find('+');
        if (plus_pos != std::string::npos)
        {
            std::string reg1_str = trim(content.substr(0, plus_pos));
            std::string reg2_str = trim(content.substr(plus_pos + 1));
            std::transform(reg1_str.begin(), reg1_str.end(), reg1_str.begin(), ::toupper);
            std::transform(reg2_str.begin(), reg2_str.end(), reg2_str.begin(), ::toupper);

            if (RegisterMap.count(reg1_str) && RegisterMap.count(reg2_str))
            {
                op.type = TYPE_MEM_REG_REG;
                op.reg_code = RegisterMap.at(reg1_str);
                op.reg_code2 = RegisterMap.at(reg2_str);
                return op;
            }
        }
        std::string content_upper = content;
        std::transform(content_upper.begin(), content_upper.end(), content_upper.begin(), ::toupper);
        if (RegisterMap.count(content_upper))
        {
            op.type = TYPE_MEM_FROM_REG;
            op.reg_code = RegisterMap.at(content_upper);
            return op;
        }
        uint16_t IMM_Address;
        if (try_parse_number(content, IMM_Address))
        {
            op.value = IMM_Address;
            op.type = TYPE_MEM_FROM_IMM;
        }
        else
        {
            op.type = TYPE_NONE;
        }

        return op;
    }

    uint16_t val;

    if (try_parse_number(op_str, val))
    {
        op.value = val;
        op.type = TYPE_IMMEDIATE;
        return op;
    }
    else
    {
        op.type = TYPE_NONE;
    }

    op.type = TYPE_LABEL;
    return op;
}

// MAIN PARSING LOGIC, FIXED AND ROBUST VERSION
std::vector<uint8_t> Parser::parse_from_string(const std::string &code_string)
{
    label_map.clear();
    last_error = "";
    std::vector<uint8_t> machine_code;

    std::stringstream code_stream(code_string);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(code_stream, line))
    {
        lines.push_back(line);
    }

    // ===============================================================
    // == FIRST PASS: CALCULATE ADDRESS FOR LABELS
    // ===============================================================
    uint16_t current_address = 0;
    for (const auto &current_line : lines)
    {
        std::string cleaned_line = clean_line(current_line);
        if (cleaned_line.empty())
        {
            continue;
        }
        if (cleaned_line.back() == ':')
        {
            std::string label = cleaned_line.substr(0, cleaned_line.length() - 1);
            label_map[trim(label)] = current_address;
        }
        else
        {
            std::stringstream ss(cleaned_line);
            std::string command;
            ss >> command;
            std::transform(command.begin(), command.end(), command.begin(), ::toupper);

            std::string rest;
            std::getline(ss, rest);

            Operand op1, op2;
            auto comma_pos = rest.find(',');
            if (comma_pos != std::string::npos)
            {
                op1 = parse_operand(rest.substr(0, comma_pos));
                op2 = parse_operand(rest.substr(comma_pos + 1));
            }
            else
            {
                op1 = parse_operand(rest);
            }

            // INSTRUCTION SIZE
            if (op1.type == TYPE_NONE && op2.type == TYPE_NONE)
            {
                current_address += 1; // 0 OPERAND (HALT, RET, NOP)
            }
            else if (op2.type == TYPE_NONE)
            { // ONE OPERAND
                if (op1.type == TYPE_LABEL)
                    current_address += 3; // JMP, CALL, Jxx
                else
                    current_address += 2; // PUSH, POP, INC, DEC, NOT, NEG
            }
            else
            {
                // 16-bit reg + IMM16
                if (op1.type == TYPE_REG16 && op2.type == TYPE_IMMEDIATE)
                {
                    current_address += 4;
                }
                else if (op1.type == TYPE_REG8 && op2.type == TYPE_IMMEDIATE)
                {
                    current_address += 3;
                }
                else if ((op1.type == TYPE_REG16 && op2.type == TYPE_MEM_FROM_IMM) ||
                         (op1.type == TYPE_MEM_FROM_IMM && op2.type == TYPE_REG16))
                {
                    current_address += 4;
                }
                else if ((op1.type == TYPE_REG8 && op2.type == TYPE_MEM_FROM_IMM) || (op1.type == TYPE_MEM_FROM_IMM && op2.type == TYPE_REG8))
                {
                    current_address += 4;
                }
                else if (op1.type == TYPE_REG16 && op2.type == TYPE_REG16)
                {
                    current_address += 3;
                }
                else if (op1.type == TYPE_REG8 && op2.type == TYPE_REG8)
                {
                    current_address += 3;
                }
                else if (op1.type == TYPE_MEM_FROM_IMM && op2.type == TYPE_IMMEDIATE)
                {
                    if (op2.value <= 0xFF)
                    {
                        current_address += 4;
                    }
                    else
                    {
                        current_address += 5;
                    }
                }
                else
                {
                    current_address += 3;
                }
            }
        }
    }

    // ===============================================================
    // == SECOND PASS: Generate machine code
    // ===============================================================
    int line_number = 0;
    for (const auto &current_line : lines)
    {
        line_number++;
        std::string cleaned_line = clean_line(current_line);

        if (cleaned_line.empty() || cleaned_line.back() == ':')
            continue;

        std::stringstream ss(cleaned_line);
        std::string command_str;
        ss >> command_str;

        if (command_str.empty())
        {
            continue;
        }

        std::transform(command_str.begin(), command_str.end(), command_str.begin(), ::toupper);

        std::string rest;
        std::getline(ss, rest);

        Operand op1, op2;
        auto comma_pos = rest.find(',');
        if (comma_pos != std::string::npos)
        {
            op1 = parse_operand(rest.substr(0, comma_pos));
            op2 = parse_operand(rest.substr(comma_pos + 1));
        }
        else
        {
            op1 = parse_operand(rest);
        }

        auto generate_error = [&](const std::string &message)
        {
            last_error = "ERROR: " + message + " on line -> " + std::to_string(line_number);
            return std::vector<uint8_t>{};
        };

#define IS_REG16(op) ((op).type == TYPE_REG16)
#define IS_REG8(op) ((op).type == TYPE_REG8)
#define IS_IMM(op) ((op).type == TYPE_IMMEDIATE)
#define IS_MEM_IMM(op) ((op).type == TYPE_MEM_FROM_IMM)
#define IS_MEM_REG(op) ((op).type == TYPE_MEM_FROM_REG)
#define IS_MEM_REG_REG(op) ((op).type == TYPE_MEM_REG_REG)
#define IS_LABEL(op) ((op).type == TYPE_LABEL)
#define IS_CL(op) (IS_REG8(op) && (op).reg_code == REG_CL)

        // --- 0-Operand Instructions ---
        if (op1.type == TYPE_NONE)
        {
            if (command_str == "HALT")
                machine_code.push_back(OP_HALT);
            else if (command_str == "RET")
                machine_code.push_back(OP_RET);
            else if (command_str == "NOP")
                machine_code.push_back(OP_NOP);
            else
                return generate_error("Unknown or operandless command: " + command_str);
        }
        // --- 1-Operand Instructions ---
        else if (op2.type == TYPE_NONE)
        {
            // Branch Instructions
            if (command_str[0] == 'J' || command_str == "CALL")
            {
                if (!IS_LABEL(op1))
                    return generate_error("Jump/Call commands require a label");
                if (label_map.count(op1.str_val) == 0)
                    return generate_error("Unknown label: " + op1.str_val);

                uint16_t addr = label_map.at(op1.str_val);
                OpCode opc = OP_HALT;
                if (command_str == "JMP")
                    opc = OP_JMP;
                else if (command_str == "CALL")
                    opc = OP_CALL;
                else if (command_str == "JZ")
                    opc = OP_JZ;
                else if (command_str == "JNZ")
                    opc = OP_JNZ;
                else if (command_str == "JC")
                    opc = OP_JC;
                else if (command_str == "JNC")
                    opc = OP_JNC;
                else if (command_str == "JS")
                    opc = OP_JS;
                else if (command_str == "JNS")
                    opc = OP_JNS;
                else if (command_str == "JO")
                    opc = OP_JO;
                else if (command_str == "JNO")
                    opc = OP_JNO;
                else
                    return generate_error("Unknown jump instruction: " + command_str);

                machine_code.push_back(opc);
                machine_code.push_back(addr & 0xFF);
                machine_code.push_back((addr >> 8) & 0xFF);
            }
            // Other 1-Operand
            else
            {
                OpCode opc16 = OP_HALT, opc8 = OP_HALT;
                if (command_str == "PUSH")
                {
                    if (IS_REG16(op1))
                    {
                        opc16 = OP_PUSH_REG;
                    }
                    else
                    {
                        return generate_error("PUSH requires a 16-bit register");
                    }
                }
                else if (command_str == "POP")
                {
                    if (IS_REG16(op1))
                    {
                        opc16 = OP_POP_REG;
                    }
                    else
                    {
                        return generate_error("POP requires a 16-bit register");
                    }
                }
                else if (command_str == "INC")
                {
                    opc16 = OP_INC_REG;
                    opc8 = OP_INC_REG8;
                }
                else if (command_str == "DEC")
                {
                    opc16 = OP_DEC_REG;
                    opc8 = OP_DEC_REG8;
                }
                else if (command_str == "NEG")
                {
                    opc16 = OP_NEG_REG16;
                    opc8 = OP_NEG_REG8;
                }
                else if (command_str == "NOT")
                {
                    opc16 = OP_NOT_REG;
                    opc8 = OP_NOT_REG8;
                }
                else
                    return generate_error("Unknown 1-operand command: " + command_str);

                if (IS_REG16(op1))
                {
                    if (opc16 == OP_HALT)
                        return generate_error(command_str + " does not support 16-bit registers.");
                    machine_code.push_back(opc16);
                    machine_code.push_back(op1.reg_code);
                }
                else if (IS_REG8(op1))
                {
                    if (opc8 == OP_HALT)
                        return generate_error(command_str + " does not support 8-bit registers.");
                    machine_code.push_back(opc8);
                    machine_code.push_back(op1.reg_code);
                }
                else
                {
                    return generate_error(command_str + " requires a register operand.");
                }
            }
        }
        // --- 2-Operand Instructions ---
        else
        {
            // Shift/Rotate Instructions
            if (command_str == "SHL" || command_str == "SAL" || command_str == "SHR" || command_str == "SAR" || command_str == "ROL" || command_str == "ROR" || command_str == "RCL" || command_str == "RCR")
            {
                if (!IS_IMM(op2) && !IS_CL(op2))
                    return generate_error("Shift/Rotate requires an immediate value or CL as the second operand");

                OpCode opc16 = OP_HALT, opc8 = OP_HALT;

                if (IS_CL(op2))
                {
                    if (command_str == "SHL" || command_str == "SAL")
                    {
                        opc16 = OP_SHL_REG_CL;
                        opc8 = OP_SHL_REG8_CL;
                    }
                    else if (command_str == "SHR")
                    {
                        opc16 = OP_SHR_REG_CL;
                        opc8 = OP_SHR_REG8_CL;
                    }
                    else if (command_str == "SAR")
                    {
                        opc16 = OP_SAR_REG_CL;
                        opc8 = OP_SAR_REG8_CL;
                    }
                    else if (command_str == "ROL")
                    {
                        opc16 = OP_ROL_REG_CL;
                        opc8 = OP_ROL_REG8_CL;
                    }
                    else if (command_str == "ROR")
                    {
                        opc16 = OP_ROR_REG_CL;
                        opc8 = OP_ROR_REG8_CL;
                    }
                    else if (command_str == "RCL")
                    {
                        opc16 = OP_RCL_REG_CL;
                        opc8 = OP_RCL_REG8_CL;
                    }
                    else if (command_str == "RCR")
                    {
                        opc16 = OP_RCR_REG_CL;
                        opc8 = OP_RCR_REG8_CL;
                    }
                }
                else
                { // IS_IMM
                    if (command_str == "SHL" || command_str == "SAL")
                    {
                        opc16 = OP_SHL_REG_IMM;
                        opc8 = OP_SHL_REG8_IMM;
                    }
                    else if (command_str == "SHR")
                    {
                        opc16 = OP_SHR_REG_IMM;
                        opc8 = OP_SHR_REG8_IMM;
                    }
                    else if (command_str == "SAR")
                    {
                        opc16 = OP_SAR_REG_IMM;
                        opc8 = OP_SAR_REG8_IMM;
                    }
                    else if (command_str == "ROL")
                    {
                        opc16 = OP_ROL_REG_IMM;
                        opc8 = OP_ROL_REG8_IMM;
                    }
                    else if (command_str == "ROR")
                    {
                        opc16 = OP_ROR_REG_IMM;
                        opc8 = OP_ROR_REG8_IMM;
                    }
                    else if (command_str == "RCL")
                    {
                        opc16 = OP_RCL_REG_IMM;
                        opc8 = OP_RCL_REG8_IMM;
                    }
                    else if (command_str == "RCR")
                    {
                        opc16 = OP_RCR_REG_IMM;
                        opc8 = OP_RCR_REG8_IMM;
                    }
                }

                if (IS_REG16(op1))
                {
                    machine_code.push_back(opc16);
                    machine_code.push_back(op1.reg_code);
                }
                else if (IS_REG8(op1))
                {
                    machine_code.push_back(opc8);
                    machine_code.push_back(op1.reg_code);
                }
                else
                {
                    return generate_error("Shift/Rotate commands require a register as the first operand");
                }

                if (IS_IMM(op2))
                    machine_code.push_back(op2.value & 0xFF);
            }
            // General 2-Operand Instructions (MOV, ADD, SUB, etc.)
            else
            {
// Opcode lookup table
#define SET_OPCODES(name)             \
    op_r_r = OP_##name##_REG_REG;     \
    op_r_i = OP_##name##_REG_IMM;     \
    op_r8_r8 = OP_##name##_REG8_REG8; \
    op_r8_i = OP_##name##_REG8_IMM;

                OpCode op_r_r = OP_HALT, op_r_i = OP_HALT, op_r8_r8 = OP_HALT, op_r8_i = OP_HALT;
                if (command_str == "MOV")
                {
                    SET_OPCODES(MOV);
                }
                else if (command_str == "ADD")
                {
                    SET_OPCODES(ADD);
                }
                else if (command_str == "SUB")
                {
                    SET_OPCODES(SUB);
                }
                else if (command_str == "CMP")
                {
                    SET_OPCODES(CMP);
                }
                else if (command_str == "AND")
                {
                    SET_OPCODES(AND);
                }
                else if (command_str == "OR")
                {
                    SET_OPCODES(OR);
                }
                else if (command_str == "XOR")
                {
                    SET_OPCODES(XOR);
                }
                else if (command_str == "ADC")
                {
                    SET_OPCODES(ADC);
                }
                else if (command_str == "SBB")
                {
                    SET_OPCODES(SBB);
                }
                else if (command_str == "XCHG")
                {
                    op_r_r = OP_XCHG_REG_REG;
                    op_r8_r8 = OP_XCHG_REG8_REG8;
                }
                else
                    return generate_error("Unknown command: " + command_str);

                // Operand combination handling
                if (IS_REG16(op1) && IS_REG16(op2))
                {
                    machine_code.push_back(op_r_r);
                    machine_code.push_back(op1.reg_code);
                    machine_code.push_back(op2.reg_code);
                }
                else if (IS_REG8(op1) && IS_REG8(op2))
                {
                    machine_code.push_back(op_r8_r8);
                    machine_code.push_back(op1.reg_code);
                    machine_code.push_back(op2.reg_code);
                }
                else if (IS_REG16(op1) && IS_IMM(op2))
                {
                    machine_code.push_back(op_r_i);
                    machine_code.push_back(op1.reg_code);
                    machine_code.push_back(op2.value & 0xFF);
                    machine_code.push_back((op2.value >> 8) & 0xFF);
                }
                else if (IS_REG8(op1) && IS_IMM(op2))
                {
                    machine_code.push_back(op_r8_i);
                    machine_code.push_back(op1.reg_code);
                    machine_code.push_back(op2.value & 0xFF);
                }
                else if (IS_REG16(op1) && IS_MEM_IMM(op2))
                {
                    machine_code.push_back(OP_MOV_REG_FROM_MEM_IMM);
                    machine_code.push_back(op1.reg_code);
                    machine_code.push_back(op2.value & 0xFF);
                    machine_code.push_back((op2.value >> 8) & 0xFF);
                }
                else if (IS_MEM_IMM(op1) && IS_REG16(op2))
                {
                    machine_code.push_back(OP_MOV_MEM_IMM_FROM_REG);
                    machine_code.push_back(op2.reg_code);
                    machine_code.push_back(op1.value & 0xFF);
                    machine_code.push_back((op1.value >> 8) & 0xFF);
                }
                else if (IS_REG16(op1) && IS_MEM_REG(op2))
                {
                    machine_code.push_back(OP_MOV_REG_FROM_MEM_REG);
                    machine_code.push_back(op1.reg_code);
                    machine_code.push_back(op2.reg_code);
                }
                else if (IS_MEM_REG(op1) && IS_REG16(op2))
                {
                    machine_code.push_back(OP_MOV_MEM_REG_FROM_REG);
                    machine_code.push_back(op2.reg_code);
                    machine_code.push_back(op1.reg_code);
                }
                else if (IS_REG8(op1) && IS_MEM_IMM(op2))
                {
                    machine_code.push_back(OP_MOV_REG8_FROM_MEM_IMM);
                    machine_code.push_back(op1.reg_code);
                    machine_code.push_back(op2.value & 0xFF);
                    machine_code.push_back((op2.value >> 8) & 0xFF);
                }
                else if (IS_MEM_IMM(op1) && IS_REG8(op2))
                {
                    machine_code.push_back(OP_MOV_MEM_IMM_FROM_REG8);
                    machine_code.push_back(op2.reg_code);
                    machine_code.push_back(op1.value & 0xFF);
                    machine_code.push_back((op1.value >> 8) & 0xFF);
                }
                else if (IS_REG8(op1) && IS_MEM_REG(op2))
                {
                    machine_code.push_back(OP_MOV_REG8_FROM_MEM_REG);
                    machine_code.push_back(op1.reg_code);
                    machine_code.push_back(op2.reg_code);
                }
                else if (IS_MEM_REG(op1) && IS_REG8(op2))
                {
                    machine_code.push_back(OP_MOV_MEM_REG_FROM_REG8);
                    machine_code.push_back(op2.reg_code);
                    machine_code.push_back(op1.reg_code);
                }
                else if (IS_REG16(op1) && IS_MEM_REG_REG(op2))
                {
                    machine_code.push_back(OP_MOV_REG_FROM_MEM_REG_REG);
                    machine_code.push_back(op1.reg_code);
                    machine_code.push_back(op2.reg_code);
                    machine_code.push_back(op2.reg_code2);
                }
                else if (IS_MEM_REG_REG(op1) && IS_REG16(op2))
                {
                    machine_code.push_back(OP_MOV_MEM_REG_REG_FROM_REG);
                    machine_code.push_back(op2.reg_code);
                    machine_code.push_back(op1.reg_code);
                    machine_code.push_back(op1.reg_code2);
                }
                else if (IS_MEM_IMM(op1) && IS_IMM(op2))
                {
                    if (op2.value <= 0xFF)
                    {
                        machine_code.push_back(OP_MOV_MEM_IMM_FROM_IMM8);
                        machine_code.push_back(op1.value & 0xFF);
                        machine_code.push_back((op1.value >> 8) & 0xFF);
                        machine_code.push_back(op2.value & 0xFF);
                    }
                    else
                    {
                        machine_code.push_back(OP_MOV_MEM_IMM_FROM_IMM);
                        machine_code.push_back((op1.value & 0xFF));
                        machine_code.push_back((op1.value >> 8) & 0xFF);
                        machine_code.push_back(op2.value & 0xFF);
                        machine_code.push_back((op2.value >> 8) & 0xFF);
                    }
                }
                else
                {
                    return generate_error("Invalid operand combination for " + command_str);
                }
            }
        }
    }

    last_error = "";
    return machine_code;
}

std::vector<uint8_t> Parser::parse(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        last_error = "ERROR: Could not open file " + filename;
        return {};
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return parse_from_string(buffer.str());
}

std::string Parser::get_last_error()
{
    return last_error;
}
