#ifndef trace_hpp
#define trace_hpp

#include <vector>
#include <cstdint>

struct Trace {
    struct _trace {
      unsigned int program_counter;
      unsigned int instruction;
      unsigned int instruction_type; //0: nop, 1: arith, 2: logic, 3: jump
                                      //4: branch, 5: load, 6: store
      unsigned int memory_address; //if memory instruction (load and store)
      unsigned int branch_target; //if jump and branch instruction
    };
    std::vector<_trace> trace;
    void generate(unsigned char *memory, unsigned program_counter);
    uint64_t reg_file[32];

private:
    void fetch();
    void decode();
    void execute();
};

#endif
