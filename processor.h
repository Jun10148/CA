#ifndef PROCESSOR_H
#define PROCESSOR_H

/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2024

   Class for processor

**************************************************************** */

#include "memory.h"

using namespace std;

class processor {

 private:
 bool is_verbose;
 bool is_stage2;

 memory* storage;

 vector<uint64_t> registers;
 uint64_t pc;
 uint64_t breakpoint;
 uint64_t instruction_count;
 int current_instruction[32];
 uint32_t curr_inst;
  // TODO: Add private members here *stage 2*
 unordered_map<uint64_t,uint64_t> csr;
 int priv;

 public:

  // Consructor
  processor(memory* main_memory, bool verbose, bool stage2);

  //load instruction in memory to array
  void load_instruction(uint64_t value, uint64_t pc);

  //find what type of instruction
  string instruction_type();

  //return signed decimal value
  uint64_t binary_return(int start, int length, bool is_signed);


  //do instruction
  void do_instruction( string type);

  // Display PC value
  void show_pc();

  // Set PC to new value
  void set_pc(uint64_t new_pc);

  // Display register value
  void show_reg(unsigned int reg_num);

  // Set register to new value
  void set_reg(unsigned int reg_num, uint64_t new_value);

  // Execute a number of instructions
  void execute(unsigned int num, bool breakpoint_check);

  // Clear breakpoint
  void clear_breakpoint();

  // Set breakpoint at an address
  void set_breakpoint(uint64_t address);
  

  // Show privilege level
  // Empty implementation for stage 1, required for stage 2
  void show_prv();

  // Set privilege level
  // Empty implementation for stage 1, required for stage 2
  void set_prv(unsigned int prv_num);

  // Display CSR value
  // Empty implementation for stage 1, required for stage 2
  void show_csr(unsigned int csr_num);

  // Set CSR to new value
  // Empty implementation for stage 1, required for stage 2
  void set_csr(unsigned int csr_num, uint64_t new_value);

  void raise_exception(int cause);
  void cause_interrupt(int cause);

  uint64_t get_instruction_count();

  // Used for Postgraduate assignment. Undergraduate assignment can return 0.
  uint64_t get_cycle_count();

  bool illegal_csr(uint64_t csr_num, uint64_t reg1);
  bool illegal_csr_imm(uint64_t csr_num);

};

#endif
