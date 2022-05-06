#ifndef trace_hpp
#define trace_hpp

#include <vector>
#include <cstdint>

enum InstructionType {
    INSTR_NOP,
    INSTR_ARITH,
    INSTR_LOGIC,
    INSTR_JUMP,
    INSTR_BRANCH,
    INSTR_LOAD,
    INSTR_STORE,
};

struct Instruction {
    unsigned char opcode;
    unsigned char rs1, rs2, rd;
    unsigned char funct3, funct7;
    int imm;
    Instruction();
};

struct Trace {
    struct _trace {
        uint64_t program_counter;
        uint32_t instruction;
        InstructionType instruction_type;
        uint64_t memory_address; //if memory instruction (load and store)
        uint64_t branch_target; //if jump and branch instruction
    };
    Trace(unsigned char *memory, uint64_t program_counter);

private:
    unsigned char *memory;
    uint64_t program_counter;
    uint64_t reg_file[32];
    std::vector<_trace> trace;

    uint32_t fetch();
    Instruction decode(uint32_t instr);
    void execute(const Instruction &instr);
    void generate();
};

#endif
