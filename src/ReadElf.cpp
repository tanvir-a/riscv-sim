#include <cstdio>
#include <cstdlib>
#include <ReadElf.hpp>
#include <elfio/elfio_dump.hpp>

#define DUMP_HEX_FORMAT( width ) \
    std::setw( width ) << std::setfill( '0' ) << std::hex << std::right

using namespace std;
using namespace ELFIO;


unsigned int read_elf(unsigned char *memory, string file_name) {
  elfio reader;

  if (!reader.load(file_name)) {
    cout << "Can't find ELF file " << file_name << endl;
    return 2;
  }

  const char *Text_p = NULL;
  const char *rodata_p = NULL;
  const char *Data_p = NULL;
  const char *sdata_p = NULL;

  unsigned int address_text = 0;
  for (size_t i = 0; i < reader.sections.size(); i++) {
    section* psec = reader.sections[i];
    if (psec->get_name() == ".text") {
      Text_p = psec->get_data();
      address_text = psec->get_address();
      cout << psec->get_name() << ": 0x" << DUMP_HEX_FORMAT(8) << psec->get_address() << dec << endl;
      memcpy(&memory[psec->get_address()], Text_p, psec->get_size());
      dump::section_data(cout, psec);
    } else if (psec->get_name() == ".sdata") {
      sdata_p = psec->get_data();
      cout << psec->get_name() << ": 0x" << DUMP_HEX_FORMAT(8) << psec->get_address() << dec << endl;
      memcpy(&memory[psec->get_address()], sdata_p, psec->get_size());
    } else if (psec->get_name() == ".data") {
      Data_p = psec->get_data();
      cout << psec->get_name() << ": 0x" << DUMP_HEX_FORMAT(8) << psec->get_address() << dec << endl;
      memcpy(&memory[psec->get_address()], Data_p, psec->get_size());
      //dump::section_data(cout, psec);
    } else if (psec->get_name() == ".bss") {
      cout << psec->get_name() << ": 0x" << DUMP_HEX_FORMAT(8) << psec->get_address() << dec << endl;
      //dump::section_data(cout, psec);
    } else if (psec->get_name() == ".rodata") {
      rodata_p = psec->get_data();
      cout << psec->get_name() << ": 0x" << DUMP_HEX_FORMAT(8) << psec->get_address() << dec << endl;
      memcpy(&memory[psec->get_address()], rodata_p, psec->get_size());
      //dump::section_data(cout, psec);
    } else if (psec->get_name() == ".startab") {
      cout << psec->get_name() << ": 0x" << DUMP_HEX_FORMAT(8) << psec->get_address() << dec << endl;
      //dump::section_data(cout, psec);
    }
  }

  Elf_Half sec_num = reader.sections.size();
  for (int i = 0; i < sec_num; i++) {
    section* psec = reader.sections[i];
    if (psec->get_name() == ".text") {
    }
  }


  return address_text;

}
