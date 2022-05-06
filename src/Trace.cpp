#include "Trace.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#define ONES(n) ((1 << (n)) - 1)

enum EncType {
    ENC_R, ENC_I, ENC_S, ENC_B, ENC_U, ENC_J, ENC_NONE,
};

Instruction::Instruction()
{
    memset(this, 0, sizeof *this);
}

Trace::Trace(unsigned char *memory, uint64_t program_counter)
    : memory(memory)
    , program_counter(program_counter)
{
    memset(reg_file, 0, sizeof reg_file);
}

uint32_t Trace::fetch()
{
    uint32_t instr = *(uint32_t *)(memory + program_counter);
    program_counter += 4;
    return instr;
}

Instruction Trace::decode(uint32_t instr)
{
    static EncType enctype_table[32] = {
        ENC_R,    // 00: LOAD
        ENC_NONE, // 01: LOAD-FP
        ENC_NONE, // 02: custom-0
        ENC_NONE, // 03: MISC-MEM
        ENC_I,    // 04: OP-IMM
        ENC_U,    // 05: AUIPC
        ENC_I,    // 06: OP-IMM-32
        ENC_NONE, // 07: > 32b
        ENC_S,    // 08: STORE
        ENC_NONE, // 09: STORE-FP
        ENC_NONE, // 0A: custom-1
        ENC_NONE, // 0B: AMO
        ENC_R,    // 0C: OP
        ENC_U,    // 0D: LUI
        ENC_R,    // 0E: OP-32
        ENC_NONE, // 0F: > 32b
        ENC_NONE, // 10: MADD
        ENC_NONE, // 11: MSUB
        ENC_NONE, // 12: NMSUB
        ENC_NONE, // 13: NMADD
        ENC_NONE, // 14: OP-FP
        ENC_NONE, // 15: reserved
        ENC_NONE, // 16: custom-2
        ENC_NONE, // 17: > 32b
        ENC_B,    // 18: BRANCH
        ENC_I,    // 19: JALR
        ENC_NONE, // 1A: reserved
        ENC_J,    // 1B: JAL
        ENC_NONE, // 1C: SYSTEM
        ENC_NONE, // 1D: reserved
        ENC_NONE, // 1E: custom-3
        ENC_NONE, // 1F: > 32b
    };

    Instruction out;
    out.opcode = instr & ONES(7);
    instr >>= 7;
    EncType enctype = enctype_table[out.opcode >> 2];
    uint32_t t1, t2, t3;
    switch (enctype) {
    case ENC_R:
        out.rd = instr & ONES(5);
        instr >>= 5;
        out.funct3 = instr & ONES(3);
        instr >>= 3;
        out.rs1 = instr & ONES(5);
        instr >>= 5;
        out.rs2 = instr & ONES(5);
        instr >>= 5;
        out.funct7 = instr;
        break;
    case ENC_I:
        out.rd = instr & ONES(5);
        instr >>= 5;
        out.funct3 = instr & ONES(3);
        instr >>= 3;
        out.rs1 = instr & ONES(5);
        instr >>= 5;
        out.imm = instr;
        if ((out.imm & (1 << 11)) != 0) {
            out.imm |= 0xfffff000;
        }
        break;
    case ENC_S:
        out.imm = instr & ONES(5);
        instr >>= 5;
        out.funct3 = instr & ONES(3);
        instr >>= 3;
        out.rs1 = instr & ONES(5);
        instr >>= 5;
        out.rs2 = instr & ONES(5);
        instr >>= 5;
        out.imm = (instr << 5) | out.imm;
        if ((out.imm & (1 << 11)) != 0) {
            out.imm |= 0xfffff000;
        }
        break;
    case ENC_B:
        out.imm = instr & ONES(5);
        instr >>= 5;
        out.funct3 = instr & ONES(3);
        instr >>= 3;
        out.rs1 = instr & ONES(5);
        instr >>= 5;
        out.rs2 = instr & ONES(5);
        instr >>= 5;
        out.imm = (instr << 5) | out.imm;
        t1 = out.imm & ONES(1);// get bit 11
        t2 = out.imm >> 11;    // get bit 12
        out.imm &= 0xffffe7fe; // clear bit 0, 11, 12
        out.imm |= (t1 << 11); // set bit 11
        out.imm |= (t2 << 12); // set bit 12
        if ((out.imm & (1 << 12)) != 0) {
            out.imm |= 0xffffe000;
        }
        break;
    case ENC_U:
        out.rd = instr & ONES(5);
        instr >>= 5;
        out.imm = instr << 12;
        break;
    case ENC_J:
        out.rd = instr & ONES(5);
        instr >>= 5;
        out.imm = instr;
        t1 = out.imm & ONES(8);
        t2 = (out.imm >> 8) & ONES(1);
        t3 = (out.imm >> 9) & ONES(10);
        out.imm <<= 1;
        out.imm &= 0xfff00000;
        out.imm |= (t1 << 12);
        out.imm |= (t2 << 11);
        out.imm |= (t3 << 1);
        if ((out.imm & (1 << 20)) != 0) {
            out.imm |= 0xffe00000;
        }
        break;
    case ENC_NONE:
        assert(0);
    }

    return out;
}

void Trace::execute(const Instruction &instr)
{
    uint64_t rs1u = reg_file[instr.rs1];
    int64_t  rs1  = (int64_t)reg_file[instr.rs1];
    uint64_t rs2u = reg_file[instr.rs2];
    int64_t  rs2  = (int64_t)reg_file[instr.rs2];
    uint64_t *rdu = &reg_file[instr.rd];
    int64_t  *rd  = (int64_t *)&reg_file[instr.rd];
    int      imm  = instr.imm;
    unsigned char opcode = instr.opcode;
    unsigned char funct3 = instr.funct3;
    unsigned char funct7 = instr.funct7;
    unsigned char shamt;

    switch (opcode >> 2) {
    case 0: // LOAD
        switch (funct3) {
        case 0: // LB
            *rd = *(char *)(memory + rs1u + imm);
            break;
        case 1: // LH
            *rd = *(int16_t *)(memory + rs1u + imm);
            break;
        case 2: // LW
            *rd = *(int32_t *)(memory + rs1u + imm);
            break;
        case 3: // LD
            *rd = *(int64_t *)(memory + rs1u + imm);
            break;
        case 4: // LBU
            *rdu = *(memory + rs1u + imm);
            break;
        case 5: // LHU
            *rdu = *(uint16_t *)(memory + rs1u + imm);
            break;
        case 6: // LWU
            *rdu = *(uint32_t *)(memory + rs1u + imm);
            break;
        default:
            assert(0);
        }
        break;
    case 4: // OP-IMM
        switch (funct3) {
        case 0: *rd = rs1 + imm; break;
        case 1: *rd = rs1 << imm; break;
        case 2: *rd = rs1 < imm ? 1 : 0; break;
        case 3: *rd = rs1u < (uint32_t)imm ? 1 : 0; break;
        case 4: *rd = rs1 ^ imm; break;
        case 5:
            shamt  = imm & ONES(6);
            *rd = (imm & (1 << 10)) == 0 ? rs1u >> shamt : rs1 >> shamt;
            break;
        case 6: *rd = rs1 | imm; break;
        case 7: *rd = rs1 & imm; break;
        }
        break;
    case 5: // AUIPC
        *rd = program_counter - 4 + imm;
        break;
    case 6: // OP-IMM-32
        switch (funct3) {
        case 0:
            *rd = (int)(rs1 + imm);
            break;
        case 1:
            *rd = (int)rs1 << imm;
            break;
        case 5:
            shamt = imm & ONES(5);
            *rd = (imm & (1 << 10)) == 0 ? (uint32_t)rs1u >> shamt : (int)rs1 >> shamt;
            break;
        default:
            assert(0);
        }
        break;
    case 8: // STORE
        switch (funct3) {
        case 0: // SB
            *(memory + rs1u + imm) = rs2;
            break;
        case 1: // SH
            *(int16_t *)(memory + rs1u + imm) = rs2;
            break;
        case 2: // SW
            *(int32_t *)(memory + rs1u + imm) = rs2;
            break;
        case 3: // SD
            *(int64_t *)(memory + rs1u + imm) = rs2;
            break;
        default:
            assert(0);
        }
        break;
    case 12: // OP
        switch (funct3) {
        case 0: *rd = funct7 == 0 ? rs1 + rs2 : rs1 - rs2; break;
        case 1: *rd = rs1 << (rs2 & ONES(6)); break;
        case 2: *rd = rs1 < rs2 ? 1 : 0; break;
        case 3: *rd = rs1u < rs2u ? 1 : 0; break;
        case 4: *rd = rs1 ^ rs2; break;
        case 5: *rd = funct7 == 0 ? rs1u >> (rs2 & ONES(6)) : rs1 >> (rs2 & ONES(6)); break;
        case 6: *rd = rs1 | rs2; break;
        case 7: *rd = rs1 & rs2; break;
        }
        break;
    case 13: // LUI
        *rd = imm;
        break;
    case 14: // OP-32
        switch (funct3) {
        case 0:
            *rd = funct7 == 0 ? (int)rs1 + (int)rs2 : (int)rs1 - (int)rs2;
            break;
        case 1:
            *rd = (int)rs1 << (rs2 & ONES(5));
            break;
        case 5:
            *rd = funct7 == 0 ? (uint32_t)rs1u >> (rs2 & ONES(5)) : (int)rs1 >> (rs2 & ONES(5));
            break;
        default:
            assert(0);
        }
        break;
    case 24: // BRANCH
        switch (funct3) {
        case 0: // BEQ
            program_counter = rs1 == rs2 ? program_counter - 4 + imm : program_counter;
            break;
        case 1: // BNE
            program_counter = rs1 != rs2 ? program_counter - 4 + imm : program_counter;
            break;
        case 4:
            program_counter = rs1 < rs2 ? program_counter - 4 + imm : program_counter;
            break;
        case 5:
            program_counter = rs1 >= rs2 ? program_counter - 4 + imm : program_counter;
            break;
        case 6:
            program_counter = rs1u < rs2u ? program_counter - 4 + imm : program_counter;
            break;
        case 7:
            program_counter = rs1u >= rs2u ? program_counter - 4 + imm : program_counter;
            break;
        default:
            assert(0);
        }
        break;
    case 25: // JALR
        *rdu = program_counter;
        program_counter = (rs1u + imm) & 0xfffffffffffffffe;
        break;
    case 27: // JAL
        *rdu = program_counter;
        program_counter = program_counter - 4 + imm;
        break;
    default:
        assert(0);
    }

    reg_file[0] = 0;
}

void Trace::generate()
{
    for (int i = 0; i < 10; i++) {
        uint32_t word = fetch();
        Instruction instr = decode(word);
        execute(instr);
    }
}
