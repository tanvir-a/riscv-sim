#ifndef trace_hpp
#define trace_hpp

#include <vector>
#include <cstdint>

struct Trace {
    struct _trace {};
    std::vector<_trace> trace;
    void generate(unsigned char *memory, unsigned program_counter);
    uint64_t reg_file[32];

private:
    void fetch();
    void decode();
    void execute();
};

#endif
