#ifndef tracer_hpp
#define tracer_hpp

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

struct trace_info {
    uint64_t program_counter = 0;
    Instruction instruction;
    InstructionType instruction_type = INSTR_NOP;
    uint64_t memory_address = 0; //if memory instruction (load and store)
    uint64_t branch_target = 0;  //if jump and branch instruction
};

struct Tracer {
    Tracer(unsigned char *memory, uint64_t program_counter);
    std::vector<trace_info> get_trace();

private:
    unsigned char *memory;
    uint64_t program_counter;
    uint64_t old_program_counter;
    uint64_t reg_file[32];
    std::vector<trace_info> trace;

    uint32_t fetch();
    Instruction decode(uint32_t instr);
    void execute(const Instruction &instr);
    void generate();
};

#endif
