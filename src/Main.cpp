#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <thread>
#include <mutex>
#include <sstream>
#include "ReadElf.hpp"
#include "elfio/elfio_dump.hpp"
#include "argparse.hpp"
#include "Tracer.hpp"


using namespace std;


#ifndef DUMP_HEX_FORMAT
#define DUMP_HEX_FORMAT( width ) \
    setw( width ) << setfill( '0' ) << hex << right
#endif
#define MEMSIZE         0x0e000000      /* default size is 234MB   */


struct Args {
  argparse::ArgValue<string> filename;
  argparse::ArgValue<bool> trace_file;
  argparse::ArgValue<bool> dump_reg;
  argparse::ArgValue<bool> log;
  argparse::ArgValue<size_t> verbosity;
  argparse::ArgValue<string> msize;
};


int main(int argc, char **argv) {

  Args args;
  auto parser = argparse::ArgumentParser(argv[0], "Risc5 Simulator");

  parser.add_argument(args.filename, "filename")
    .help("Executable file");

  parser.add_argument(args.trace_file, "--trace")
    .help("Generate trace")
    .default_value("false")
    .action(argparse::Action::STORE_TRUE);

  parser.add_argument(args.dump_reg, "--dump_reg", "-d")
    .help("Dump the register file")
    .default_value("false")
    .action(argparse::Action::STORE_TRUE);

  parser.add_argument(args.log, "--dump_log", "-l")
    .help("Dump the log")
    .default_value("false")
    .action(argparse::Action::STORE_TRUE);

  parser.add_argument(args.msize, "--msize")
    .help("Set memory size");

  parser.parse_args(argc, argv);



  cout << "File name: " << args.filename << endl;
  if (args.trace_file) {
    cout << "Trace: Yes" << endl;
  } else {
    cout << "Trace: No" << endl;	  
  }

  if (args.dump_reg) {
    cout << "Dump register: ON\n";	  
  }



  {
    cout << "---------------------------\n";
    cout << "---- riscv Simulator    ---\n";
    cout << "---- Version: 0.0       ---\n";
    cout << "---------------------------\n";
  }


  unsigned char *memory = NULL;
  memory = (unsigned char*)malloc(MEMSIZE * sizeof(unsigned char));
  if (memory == NULL) {
      fprintf(stderr, "malloc: error\n");
      exit(1);
  }
  unsigned int program_counter = read_elf(memory, args.filename);

  Tracer tracer(memory, program_counter);
  auto trace = tracer.get_trace();

  free(memory);
  return 0;	
}
