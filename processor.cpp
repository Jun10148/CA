/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2024

   Class members for processor

**************************************************************** */

#include "processor.h"

#include <stdlib.h>

#include <climits>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>

#include "memory.h"

using namespace std;

// Consructor
processor::processor(memory* main_memory, bool verbose, bool stage2) {
  storage = main_memory;
  is_verbose = verbose;
  is_stage2 = stage2;
  priv = 3;

  registers = vector<uint64_t>(32);
  pc = 0;
  breakpoint = 0xffff00ffff;
  instruction_count = 0;

  csr[0xf11] = 0;                   // mvendorid
  csr[0xf12] = 0;                   // marchid
  csr[0xf13] = 0x2024020000000000;  // mimpid
  csr[0xf14] = 0;                   // mhartid
  csr[0x300] = 0x0000000200000000;  // mstatus
  csr[0x301] = 0x8000000000100100;  // misa
  csr[0x304] = 0;                   // mie
  csr[0x305] = 0;                   // mtvec
  csr[0x340] = 0;                   // mscratch
  csr[0x341] = 0;                   // mepc
  csr[0x342] = 0;                   // mcause
  csr[0x343] = 0;                   // mtval
  csr[0x344] = 0;                   // mip
  if (verbose) {
    cout << "Processor created" << endl;
  }
  return;
}

void processor::load_instruction(uint64_t value, uint64_t pc) {
  curr_inst = value;
  if (pc % 8 == 4) {
    value = value >> 32;
  }
  for (int i = 31; i >= 0; i--) {  // Assuming uint64_t is 64 bits
    current_instruction[31 - i] = ((value >> i) & 1) ? 1 : 0;
  }  // line shifts to desired bit, uses & to isolate, 1 or 0 depending on t/f

  if (is_verbose) {
    for (int i = 0; i < 32; i++) {
      cout << current_instruction[i];
    }
    cout << endl;
  }
}

// find what type of instruction
string processor::instruction_type() {
  string opcode;
  string funct3;
  string funct7;

  for (int i = 25; i < 32; i++) {
    opcode.push_back(current_instruction[i] + '0');
  }

  for (int i = 17; i < 20; i++) {
    funct3.push_back(current_instruction[i] + '0');
  }

  for (int i = 0; i < 7; i++) {
    funct7.push_back(current_instruction[i] + '0');
  }

  if (is_verbose) {
    cout << "opcode: " << opcode << " funct3: " << funct3;
    cout << " funct7: " << funct7 << endl;
  }

  if (opcode == "0110111") {
    return "LUI";
  } else if (opcode == "0010111") {
    return "AUIPC";
  } else if (opcode == "1101111") {
    return "JAL";
  } else if (opcode == "1100111" && funct3 == "000") {
    return "JALR";
  } else if (opcode == "1100011" && funct3 == "000") {
    return "BEQ";
  } else if (opcode == "1100011" && funct3 == "001") {
    return "BNE";
  } else if (opcode == "1100011" && funct3 == "100") {
    return "BLT";
  } else if (opcode == "1100011" && funct3 == "101") {
    return "BGE";
  } else if (opcode == "1100011" && funct3 == "110") {
    return "BLTU";
  } else if (opcode == "1100011" && funct3 == "111") {
    return "BGEU";
  } else if (opcode == "0000011" && funct3 == "000") {
    return "LB";
  } else if (opcode == "0000011" && funct3 == "001") {
    return "LH";
  } else if (opcode == "0000011" && funct3 == "010") {
    return "LW";
  } else if (opcode == "0000011" && funct3 == "100") {
    return "LBU";
  } else if (opcode == "0000011" && funct3 == "101") {
    return "LHU";
  } else if (opcode == "0100011" && funct3 == "000") {
    return "SB";
  } else if (opcode == "0100011" && funct3 == "001") {
    return "SH";
  } else if (opcode == "0100011" && funct3 == "010") {
    return "SW";
  } else if (opcode == "0010011" && funct3 == "000") {
    return "ADDI";
  } else if (opcode == "0010011" && funct3 == "010") {
    return "SLTI";
  } else if (opcode == "0010011" && funct3 == "011") {
    return "SLTIU";
  } else if (opcode == "0010011" && funct3 == "100") {
    return "XORI";
  } else if (opcode == "0010011" && funct3 == "110") {
    return "ORI";
  } else if (opcode == "0010011" && funct3 == "111") {
    return "ANDI";
  } else if (opcode == "0010011" && funct3 == "001" && funct7 == "0000000") {
    return "SLLI";
  } else if (opcode == "0010011" && funct3 == "101" && funct7 == "0000000") {
    return "SRLI";
  } else if (opcode == "0010011" && funct3 == "101" && funct7 == "0100000") {
    return "SRAI";
  } else if (opcode == "0110011" && funct3 == "000" && funct7 == "0000000") {
    return "ADD";
  } else if (opcode == "0110011" && funct3 == "000" && funct7 == "0100000") {
    return "SUB";
  } else if (opcode == "0110011" && funct3 == "001" && funct7 == "0000000") {
    return "SLL";
  } else if (opcode == "0110011" && funct3 == "010" && funct7 == "0000000") {
    return "SLT";
  } else if (opcode == "0110011" && funct3 == "011" && funct7 == "0000000") {
    return "SLTU";
  } else if (opcode == "0110011" && funct3 == "100" && funct7 == "0000000") {
    return "XOR";
  } else if (opcode == "0110011" && funct3 == "101" && funct7 == "0000000") {
    return "SRL";
  } else if (opcode == "0110011" && funct3 == "101" && funct7 == "0100000") {
    return "SRA";
  } else if (opcode == "0110011" && funct3 == "110" && funct7 == "0000000") {
    return "OR";
  } else if (opcode == "0110011" && funct3 == "111" && funct7 == "0000000") {
    return "AND";
  } else if (opcode == "0001111") {
    return "FENCE";
  }  // 64I instructions now
  else if (opcode == "0000011" && funct3 == "110") {
    return "LWU";
  } else if (opcode == "0000011" && funct3 == "011") {
    return "LD";
  } else if (opcode == "0100011" && funct3 == "011") {
    return "SD";
  } else if (opcode == "0010011" && funct3 == "001") {
    return "SLLI";
  } else if (opcode == "0010011" && funct3 == "101" &&
             current_instruction[1] == 0) {
    return "SRLI";
  } else if (opcode == "0010011" && funct3 == "101" &&
             current_instruction[1] == 1) {
    return "SRAI";
  } else if (opcode == "0011011" && funct3 == "000") {
    return "ADDIW";
  } else if (opcode == "0011011" && funct3 == "001" && funct7 == "0000000") {
    return "SLLIW";
  } else if (opcode == "0011011" && funct3 == "101" && funct7 == "0000000") {
    return "SRLIW";
  } else if (opcode == "0011011" && funct3 == "101" && funct7 == "0100000") {
    return "SRAIW";
  } else if (opcode == "0111011" && funct3 == "000" && funct7 == "0000000") {
    return "ADDW";
  } else if (opcode == "0111011" && funct3 == "000" && funct7 == "0100000") {
    return "SUBW";
  } else if (opcode == "0111011" && funct3 == "001" && funct7 == "0000000") {
    return "SLLW";
  } else if (opcode == "0111011" && funct3 == "101" && funct7 == "0000000") {
    return "SRLW";
  } else if (opcode == "0111011" && funct3 == "101" && funct7 == "0100000") {
    return "SRAW";
  }
  // ZICSR extension instructions
  string funct12;
  for (int i = 0; i < 12; i++) {
    funct12.push_back(current_instruction[i] + '0');
  }
  if (opcode == "1110011" && funct3 == "001") {
    return "CSRRW";
  } else if (opcode == "1110011" && funct3 == "010") {
    return "CSRRS";
  } else if (opcode == "1110011" && funct3 == "011") {
    return "CSRRC";
  } else if (opcode == "1110011" && funct3 == "101") {
    return "CSRRWI";
  } else if (opcode == "1110011" && funct3 == "110") {
    return "CSRRSI";
  } else if (opcode == "1110011" && funct3 == "111") {
    return "CSRRCI";
  } else if (opcode == "1110011" && funct12 == "000000000000") {
    return "ECALL";
  } else if (opcode == "1110011" && funct12 == "000000000001") {
    return "EBREAK";
  } else if (opcode == "1110011" && funct12 == "001100000010") {
    return "MRET";
  }
  return "unknown command";
}

// return signed decimal value
uint64_t processor::binary_return(int start, int length, bool is_signed) {
  // bool negative = current_instruction[start];
  uint64_t return_int = 0;
  int power = length - 1;
  if (is_signed == true && current_instruction[start] == 1) {
    for (int i = start; i < start + length; i++) {
      if (current_instruction[i] == 0) {
        return_int = return_int + pow(2, power);
      }
      power--;
    }
    return_int = -1 * return_int - 1;
  } else {
    for (int i = start; i < start + length; i++) {
      if (current_instruction[i] == 1) {
        return_int = return_int + pow(2, power);
      }
      power--;
    }
  }
  return return_int;
}

// do instruction
void processor::do_instruction(string type) {
  if (is_verbose) {
    cout << type << endl;
  }
  if(type == "unknown command"){
    raise_exception(2);
  }
  if (type == "LUI") {
    uint64_t immediate = binary_return(0, 20, 1);
    immediate = immediate << 12;
    uint64_t destination_reg = binary_return(20, 5, 0);
    set_reg(destination_reg, immediate);
  } else if (type == "AUIPC") {
    uint64_t immediate = binary_return(0, 20, 1);  // check if int or uint
    immediate = immediate << 12;
    uint64_t destination_reg = binary_return(20, 5, 0);
    immediate = immediate + pc;
    set_reg(destination_reg, immediate);
  } else if (type == "JAL") {
    uint64_t immediate1_10 = binary_return(1, 10, 0) << 1;
    uint64_t immediate11 = binary_return(11, 1, 0) << 11;
    uint64_t immediate19_12 = binary_return(12, 8, 0) << 12;
    uint64_t immediate = immediate1_10 + immediate11 + immediate19_12;
    if (current_instruction[0] == 1) {
      immediate = immediate + 0xfffffffffff00000;
    }
    // cout << dec << "immediate: " << (int)immediate << endl;
    uint64_t destination_reg = binary_return(20, 5, 0);
    set_reg(destination_reg, pc + 4);
    set_pc(pc + immediate - 4);
  } else if (type == "JALR") {
    uint64_t immediate = binary_return(0, 12, 1);
    // cout << "IMM: " << immediate << endl;
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t newpc = immediate + registers[register_1] - 4;
    newpc = newpc - (newpc % 2);
    set_reg(destination_reg, pc + 4);
    // cout << hex<<"REg: " << registers[register_1] << endl;
    set_pc(newpc);
  } else if (type == "BEQ") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    if (registers[register_1] == registers[register_2]) {
      uint64_t immediate11 = current_instruction[24] << 11;
      uint64_t immediate10_5 = binary_return(1, 6, 0) << 5;
      uint64_t immediate4_1 = binary_return(20, 4, 0) << 1;
      uint64_t combined_immediate = immediate4_1 + immediate10_5 + immediate11;
      if (current_instruction[0] == 1) {
        combined_immediate = combined_immediate + 0xfffffffffffff000;
      }
      // cout  << dec << "immediate: " << combined_immediate << endl;
      pc = pc + combined_immediate - 4;
    }
  } else if (type == "BNE") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    if (registers[register_1] != registers[register_2]) {
      uint64_t immediate11 = current_instruction[24] << 11;
      uint64_t immediate10_5 = binary_return(1, 6, 0) << 5;
      uint64_t immediate4_1 = binary_return(20, 4, 0) << 1;
      uint64_t combined_immediate = immediate4_1 + immediate10_5 + immediate11;
      if (current_instruction[0] == 1) {
        combined_immediate = combined_immediate + 0xfffffffffffff000;
      }
      pc = pc + combined_immediate - 4;
    }
  } else if (type == "BLT") {  // uns
    // cout << "BLT" << endl;
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    if ((int)registers[register_1] < (int)registers[register_2]) {
      uint64_t immediate11 = current_instruction[24] << 11;
      uint64_t immediate10_5 = binary_return(1, 6, 0) << 5;
      uint64_t immediate4_1 = binary_return(20, 4, 0) << 1;
      uint64_t combined_immediate = immediate4_1 + immediate10_5 + immediate11;
      if (current_instruction[0] == 1) {
        combined_immediate = combined_immediate + 0xfffffffffffff000;
      }
      //  cout  << dec << "immediate: " << combined_immediate << endl;
      pc = pc + combined_immediate - 4;
    }
  } else if (type == "BGE") {  // uns
    // cout << "BGE" << endl;
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    if ((int)registers[register_1] >= (int)registers[register_2]) {
      uint64_t immediate11 = current_instruction[24] << 11;
      uint64_t immediate10_5 = binary_return(1, 6, 0) << 5;
      uint64_t immediate4_1 = binary_return(20, 4, 0) << 1;
      uint64_t combined_immediate = immediate4_1 + immediate10_5 + immediate11;
      if (current_instruction[0] == 1) {
        combined_immediate = combined_immediate + 0xfffffffffffff000;
      }
      // cout  << dec << "immediate: " << combined_immediate << endl;
      pc = pc + combined_immediate - 4;
    }
  } else if (type == "BLTU") {
    // cout << "BLTU" << endl;
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    if (registers[register_1] < registers[register_2]) {
      uint64_t immediate11 = current_instruction[24] << 11;
      uint64_t immediate10_5 = binary_return(1, 6, 0) << 5;
      uint64_t immediate4_1 = binary_return(20, 4, 0) << 1;
      uint64_t combined_immediate = immediate4_1 + immediate10_5 + immediate11;
      if (current_instruction[0] == 1) {
        combined_immediate = combined_immediate + 0xfffffffffffff000;
      }
      //  cout  << dec << "immediate: " << combined_immediate << endl;
      pc = pc + combined_immediate - 4;
    }
  } else if (type == "BGEU") {
    // cout << "BGEU" << endl;
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    if (registers[register_1] >= registers[register_2]) {
      uint64_t immediate11 = current_instruction[24] << 11;
      uint64_t immediate10_5 = binary_return(1, 6, 0) << 5;
      uint64_t immediate4_1 = binary_return(20, 4, 0) << 1;
      uint64_t combined_immediate = immediate4_1 + immediate10_5 + immediate11;
      if (current_instruction[0] == 1) {
        combined_immediate = combined_immediate + 0xfffffffffffff000;
      }
      // cout  << dec << "immediate: " << combined_immediate << endl;
      pc = pc + combined_immediate - 4;
    }
  } else if (type == "LB") {
    uint64_t immediate = binary_return(0, 12, 1);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    int64_t addr = registers[register_1] + immediate;
    int offset = addr % 8;
    uint64_t buffer = storage->read_doubleword(addr);
    buffer = (buffer >> offset * 8) & 0xFF;
    if ((buffer >> 7) == 1) {
      buffer = buffer + 0xffffffffffffff00;
    }
    // cout << "buf: " << buffer << endl;
    set_reg(destination_reg, buffer);
  } else if (type == "LH") {
    uint64_t immediate = binary_return(0, 12, 1);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    int64_t addr = registers[register_1] + immediate;
    int offset = addr % 8;
    uint64_t buffer = storage->read_doubleword(addr);
    buffer = (buffer >> offset * 8) & 0xFFFF;
    if ((buffer >> 15) == 1) {
      buffer = buffer + 0xffffffffffff0000;
    }
    if (addr % 2 == 0) {
      // cout << "buf: " << buffer << endl;
      set_reg(destination_reg, buffer);
    } else {
      raise_exception(4);
    }
  } else if (type == "LW") {
    uint64_t immediate = binary_return(0, 12, 1);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    int64_t addr = registers[register_1] + immediate;
    int offset = addr % 8;
    uint64_t buffer = storage->read_doubleword(addr);
    buffer = (buffer >> offset * 8) & 0xFFFFFFFF;
    if ((buffer >> 31) == 1) {
      buffer = buffer + 0xffffffff00000000;
    }
    if (addr % 4 == 0) {
      // cout << "buf: " << buffer << endl;
      set_reg(destination_reg, buffer);
    } else {
      raise_exception(4);
    }
  } else if (type == "LBU") {
    uint64_t immediate = binary_return(0, 12, 1);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    int64_t addr = registers[register_1] + immediate;
    int offset = addr % 8;
    uint64_t buffer = storage->read_doubleword(addr);
    buffer = (buffer >> offset * 8) & 0xFF;
    // cout << "buf: " << buffer << endl;
    set_reg(destination_reg, buffer);
  } else if (type == "LHU") {
    uint64_t immediate = binary_return(0, 12, 1);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    int64_t addr = registers[register_1] + immediate;
    int offset = addr % 8;
    uint64_t buffer = storage->read_doubleword(addr);
    buffer = (buffer >> offset * 8) & 0xFFFF;
    // cout << "buf: " << buffer << endl;
    if (addr % 2 == 0) {
      // cout << "buf: " << buffer << endl;
      set_reg(destination_reg, buffer);
    } else {
      raise_exception(4);
    }
  } else if (type == "SB") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t immediate5_11 = binary_return(0, 7, 0) << 5;
    uint64_t immediate4_0 = binary_return(20, 5, 0);
    uint64_t combined_immediate = immediate4_0 + immediate5_11;
    if (current_instruction[0] == 1) {
      combined_immediate = combined_immediate + 0xfffffffffffff000;
    }
    // cout << dec << "IMM: " << (int)combined_immediate << endl;
    uint64_t addr = combined_immediate + registers[register_1];
    // cout << dec << "addr: " << (int)addr << endl;
    int offset = addr % 8;
    uint64_t buff = registers[register_2] << offset * 8;
    uint64_t mask = 0xFFULL << (offset * 8);
    // cout << hex << "MASK: " << mask << endl;
    storage->write_doubleword(addr, buff, mask);
  } else if (type == "SH") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t immediate5_11 = binary_return(0, 7, 0) << 5;
    uint64_t immediate4_0 = binary_return(20, 5, 0);
    uint64_t combined_immediate = immediate4_0 + immediate5_11;
    if (current_instruction[0] == 1) {
      combined_immediate = combined_immediate + 0xfffffffffffff000;
    }
    // cout << dec << "IMM: " << (int)combined_immediate << endl;
    uint64_t addr = combined_immediate + registers[register_1];
    // cout << dec << "addr: " << (int)addr << endl;
    int offset = addr % 8;
    uint64_t buff = registers[register_2] << offset * 8;
    uint64_t mask = 0xFFFFULL << (offset * 8);
    // cout << hex << "MASK: " << mask << endl;
    if (addr % 2 == 0) {
      storage->write_doubleword(addr, buff, mask);
    } else {
      raise_exception(6);
    }

  } else if (type == "SW") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t immediate5_11 = binary_return(0, 7, 0) << 5;
    uint64_t immediate4_0 = binary_return(20, 5, 0);
    uint64_t combined_immediate = immediate4_0 + immediate5_11;
    if (current_instruction[0] == 1) {
      combined_immediate = combined_immediate + 0xfffffffffffff000;
    }
    // cout << dec << "IMM: " << (int)combined_immediate << endl;
    uint64_t addr = combined_immediate + registers[register_1];
    // cout << dec << "addr: " << (int)addr << endl;
    int offset = addr % 8;
    uint64_t buff = registers[register_2] << offset * 8;
    uint64_t mask = 0xFFFFFFFFULL << (offset * 8);
    // cout << hex << "MASK: " << mask << endl;
    if (addr % 4 == 0) {
      storage->write_doubleword(addr, buff, mask);
    } else {
      raise_exception(6);
    }
  } else if (type == "ADDI") {
    uint64_t immediate = binary_return(0, 12, 1);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    // cout << "IMM: " << immediate << endl;
    immediate = immediate + registers[register_1];
    set_reg(destination_reg, immediate);
  } else if (type == "SLTI") {
    uint64_t immediate = binary_return(0, 12, 1);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    // cout << dec << "reg1: " << (int)registers[register_1] << " imm: " <<
    // (int)immediate << endl;
    if ((int)registers[register_1] < (int)immediate) {
      set_reg(destination_reg, 1);
    } else {
      set_reg(destination_reg, 0);
    }
  } else if (type == "SLTIU") {
    uint64_t immediate = binary_return(0, 12, 1);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    // cout << dec << "reg1: " << (int)registers[register_1] << " imm: " <<
    // (int)immediate << endl;
    if (registers[register_1] < immediate) {
      set_reg(destination_reg, 1);
    } else {
      set_reg(destination_reg, 0);
    }
  } else if (type == "XORI") {
    uint64_t immediate = binary_return(0, 12, 1);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    // cout << dec << "reg1: " << (int)registers[register_1] << " imm: " <<
    // (int)immediate << endl;
    uint64_t buff = registers[register_1] ^ (int)immediate;
    set_reg(destination_reg, buff);
  } else if (type == "ORI") {
    uint64_t immediate = binary_return(0, 12, 1);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    // cout << dec << "reg1: " << (int)registers[register_1] << " imm: " <<
    // (int)immediate << endl;
    uint64_t buff = registers[register_1] | (int)immediate;
    set_reg(destination_reg, buff);
  } else if (type == "ANDI") {
    uint64_t immediate = binary_return(0, 12, 1);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    // cout << dec << "reg1: " << (int)registers[register_1] << " imm: " <<
    // (int)immediate << endl;
    uint64_t buff = registers[register_1] & (int)immediate;
    set_reg(destination_reg, buff);
  } else if (type == "SLLI") {  // 64I version with 6 shamt bits
    uint64_t shamt = binary_return(6, 6, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    set_reg(destination_reg, registers[register_1] << shamt);
  } else if (type == "SRLI") {  // 64I version with 6 shamt bits
    uint64_t shamt = binary_return(6, 6, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    set_reg(destination_reg, registers[register_1] >> shamt);
  } else if (type == "SRAI") {  // 64I version with 6 shamt bits
    uint64_t shamt = binary_return(6, 6, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    // cout << hex <<"REG: " << (int)registers[register_1] << endl;
    // cout << "SHAMT: " << shamt << endl;
    set_reg(destination_reg, (long int)registers[register_1] >> shamt);
  } else if (type == "ADD") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t buffer = registers[register_1] + registers[register_2];
    set_reg(destination_reg, buffer);
  } else if (type == "SUB") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t buffer = registers[register_1] - registers[register_2];
    set_reg(destination_reg, buffer);
  } else if (type == "SLL") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t shamt = registers[register_2] & 0x3f;
    uint64_t buff = registers[register_1] << shamt;
    set_reg(destination_reg, buff);

  } else if (type == "SLT") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    if ((int)registers[register_1] < (int)registers[register_2]) {
      set_reg(destination_reg, 1);
    } else {
      set_reg(destination_reg, 0);
    }
  } else if (type == "SLTU") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    if (registers[register_1] < registers[register_2]) {
      set_reg(destination_reg, 1);
    } else {
      set_reg(destination_reg, 0);
    }
  } else if (type == "XOR") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t buff = registers[register_1] ^ registers[register_2];
    set_reg(destination_reg, buff);
  } else if (type == "SRL") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t shamt = registers[register_2] & 0x3f;
    uint64_t buff = registers[register_1] >> shamt;
    set_reg(destination_reg, buff);
  } else if (type == "SRA") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t shamt = registers[register_2] & 0x3f;
    uint64_t buff = (long int)registers[register_1] >> shamt;
    set_reg(destination_reg, buff);
  } else if (type == "OR") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t buff = registers[register_1] | registers[register_2];
    set_reg(destination_reg, buff);
  } else if (type == "AND") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t buff = registers[register_1] & registers[register_2];
    set_reg(destination_reg, buff);
  } else if (type == "FENCE") {
    if (is_verbose) {
      cout << "FENCE was called" << endl;
    }
  } else if (type == "LWU") {
    uint64_t immediate = binary_return(0, 12, 1);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    int64_t addr = registers[register_1] + immediate;
    int offset = addr % 8;
    uint64_t buffer = storage->read_doubleword(addr);
    buffer = (buffer >> offset * 8) & 0xFFFFFFFF;
    // cout << "buf: " << buffer << endl;
    if (addr % 4 == 0) {
      // cout << "buf: " << buffer << endl;
      set_reg(destination_reg, buffer);
    } else {
      raise_exception(4);
    }
  } else if (type == "LD") {
    uint64_t immediate = binary_return(0, 12, 1);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    int64_t addr = registers[register_1] + immediate;
    uint64_t buffer = storage->read_doubleword(addr);
    if (addr % 8 == 0) {
      // cout << "buf: " << buffer << endl;
      set_reg(destination_reg, buffer);
    } else {
      raise_exception(4);
    }
  } else if (type == "SD") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t immediate5_11 = binary_return(0, 7, 0) << 5;
    uint64_t immediate4_0 = binary_return(20, 5, 0);
    uint64_t combined_immediate = immediate4_0 + immediate5_11;
    if (current_instruction[0] == 1) {
      combined_immediate = combined_immediate + 0xfffffffffffff000;
    }
    // cout << dec << "IMM: " << (int)combined_immediate << endl;
    uint64_t addr = combined_immediate + registers[register_1];
    // cout << dec << "addr: " << (int)addr << endl;
    uint64_t buff = registers[register_2];
    // cout << hex << "MASK: " << mask << endl;
    if (addr % 8 == 0) {
      storage->write_doubleword(addr, buff, 0xffffffffffffffff);
    } else {
      raise_exception(6);
    }
  } else if (type == "ADDIW") {
    uint64_t immediate = binary_return(0, 12, 1);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    // cout << hex <<"imm: " << immediate << " register: " <<
    // registers[register_1] <<  endl;
    int result = immediate + registers[register_1];
    //  cout << hex <<"res: " << result << endl;
    set_reg(destination_reg, result);
  }
  // I think i can change SLLIW and SRLIW using cast to int32_t like SRAIW, will
  // look
  else if (type == "SLLIW") {  // 32bit version with 5 shamt bits
    uint64_t shamt = binary_return(7, 5, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t buff = (registers[register_1] & 0x00000000ffffffff) << shamt;
    // cout << "BUFF: " << buff << endl;
    if ((buff & 0xffffffff) >> 31 == 1) {
      buff = 0xffffffff00000000 | buff;
    } else {
      buff = buff & 0x00000000ffffffff;
    }
    set_reg(destination_reg, buff);
  } else if (type == "SRLIW") {  // 32bit version with 5 shamt bits
    uint64_t shamt = binary_return(7, 5, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t buff = (registers[register_1] & 0x00000000ffffffff) >> shamt;
    // cout << "BUFF: " << buff << endl;
    if ((buff & 0xffffffff) >> 31 == 1) {
      buff = 0xffffffff00000000 | buff;
    } else {
      buff = buff & 0x00000000ffffffff;
    }
    set_reg(destination_reg, buff);
  } else if (type == "SRAIW") {  // 32bit version with 5 shamt bits
    uint64_t shamt = binary_return(7, 5, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t buff = (int32_t)registers[register_1] >> shamt;
    // cout << "BUFF: " << buff << endl;
    set_reg(destination_reg, buff);
  } else if (type == "ADDW") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t buffer =
        (int32_t)registers[register_1] + (int32_t)registers[register_2];
    set_reg(destination_reg, buffer);
  } else if (type == "SUBW") {
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t buffer =
        (int32_t)registers[register_1] - (int32_t)registers[register_2];
    set_reg(destination_reg, buffer);
  } else if (type == "SLLW") {  // 32bit version with 5 shamt bits
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t shamt = registers[register_2] & 0x1f;
    uint64_t buff = (registers[register_1] & 0x00000000ffffffff) << shamt;
    // cout << "BUFF: " << buff << endl;
    if ((buff & 0xffffffff) >> 31 == 1) {
      buff = 0xffffffff00000000 | buff;
    } else {
      buff = buff & 0x00000000ffffffff;
    }
    set_reg(destination_reg, buff);
  } else if (type == "SRLW") {  // 32bit version with 5 shamt bits
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t shamt = registers[register_2] & 0x1f;
    uint64_t buff = (registers[register_1] & 0x00000000ffffffff) >> shamt;
    // cout << "BUFF: " << buff << endl;
    if ((buff & 0xffffffff) >> 31 == 1) {
      buff = 0xffffffff00000000 | buff;
    } else {
      buff = buff & 0x00000000ffffffff;
    }
    set_reg(destination_reg, buff);
  } else if (type == "SRAW") {  // 32bit version with 5 shamt bits
    uint64_t register_2 = binary_return(7, 5, 0);
    uint64_t shamt = registers[register_2] & 0x1f;
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t buff = (int32_t)registers[register_1] >> shamt;
    // cout << "BUFF: " << buff << endl;
    set_reg(destination_reg, buff);
  }
  // ZICSR EXTENSION ISA
  if (type == "CSRRW") {
    uint64_t csr_num = binary_return(0, 12, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    if (illegal_csr(csr_num, register_1)) {
      uint64_t buffer = registers[register_1];
      set_reg(destination_reg, csr[csr_num]);
      if (csr_num != 0xf11 && csr_num != 0xf12 && csr_num != 0xf13 &&
          csr_num != 0xf14) {
        if (csr_num == 0x344) {  // Mxxx cannot be written through csr inst
          buffer = buffer & 0x111;
        }
        set_csr(csr_num, buffer);
      }
    } else {
      raise_exception(2);
    }
  } else if (type == "CSRRS") {
    uint64_t csr_num = binary_return(0, 12, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    if (illegal_csr(csr_num, register_1)) {
      uint64_t buffer = registers[register_1] | csr[csr_num];
      set_reg(destination_reg, csr[csr_num]);
      if (csr_num != 0xf11 && csr_num != 0xf12 && csr_num != 0xf13 &&
          csr_num != 0xf14) {
        if (csr_num == 0x344) {  // Mxxx cannot be written through csr inst
          buffer = buffer & 0x111;
        }
        set_csr(csr_num, buffer);
      }
    } else {
      raise_exception(2);
    }
  } else if (type == "CSRRC") {
    uint64_t csr_num = binary_return(0, 12, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t register_1 = binary_return(12, 5, 0);
    if (illegal_csr(csr_num, register_1)) {
      uint64_t buffer = (~registers[register_1]) & csr[csr_num];
      set_reg(destination_reg, csr[csr_num]);
      if (csr_num != 0xf11 && csr_num != 0xf12 && csr_num != 0xf13 &&
          csr_num != 0xf14) {
        if (csr_num == 0x344) {  // Mxxx cannot be written through csr inst
          buffer = buffer & 0x111;
        }
        set_csr(csr_num, buffer);
      }
    } else {
      raise_exception(2);
    }
  } else if (type == "CSRRWI") {
    uint64_t csr_num = binary_return(0, 12, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t immediate = binary_return(12, 5, 0);
    // cout << hex <<"IMM: " << immediate << endl;
    if (illegal_csr_imm(csr_num)) {
      set_reg(destination_reg, csr[csr_num]);
      if (csr_num != 0xf11 && csr_num != 0xf12 && csr_num != 0xf13 &&
          csr_num != 0xf14) {
        if (csr_num == 0x344) {  // Mxxx cannot be written through csr inst
          immediate = immediate & 0x111;
        }
        set_csr(csr_num, immediate);
      }
    } else {
      raise_exception(2);
    }
  } else if (type == "CSRRSI") {
    uint64_t csr_num = binary_return(0, 12, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t immediate = binary_return(12, 5, 0);
    uint64_t buffer = immediate | csr[csr_num];
    if (illegal_csr_imm(csr_num)) {
      set_reg(destination_reg, csr[csr_num]);
      if (csr_num != 0xf11 && csr_num != 0xf12 && csr_num != 0xf13 &&
          csr_num != 0xf14) {
        if (csr_num == 0x344) {  // Mxxx cannot be written through csr inst
          buffer = buffer & 0x111;
        }
        set_csr(csr_num, buffer);
      }
    } else {
      raise_exception(2);
    }
  } else if (type == "CSRRCI") {
    uint64_t csr_num = binary_return(0, 12, 0);
    uint64_t destination_reg = binary_return(20, 5, 0);
    uint64_t immediate = binary_return(12, 5, 0);
    uint64_t buffer = (~immediate) & csr[csr_num];
    if (illegal_csr_imm(csr_num)) {
      set_reg(destination_reg, csr[csr_num]);
      if (csr_num != 0xf11 && csr_num != 0xf12 && csr_num != 0xf13 &&
          csr_num != 0xf14) {
        if (csr_num == 0x344) {  // Mxxx cannot be written through csr inst
          buffer = buffer & 0x111;
        }
        set_csr(csr_num, buffer);
      }
    } else {
      raise_exception(2);
    }
  } else if (type == "ECALL") {
    if (priv == 0) {
      raise_exception(8);
    } else if (priv == 3) {
      raise_exception(11);
    } else {
      cout << "ecall error" << endl;
    }
  } else if (type == "EBREAK") {
    // mepc = pc
    set_csr(0x341, pc);

    // mtvec
    if (csr[0x305] & 0x1) {  // vectored (set pc to BASE+4×cause.)
      uint64_t base = (csr[0x305] & 0xfffffffffffffffc);
      pc = base + (4 * (csr[0x342]) & 0x8000000000000000) - 4;
    } else {  // unvectored (All traps set pc to BASE.)
      pc = (csr[0x305] & 0xfffffffffffffffc) - 4;
    }

    // mcause
    set_csr(0x342, 3);

    // mstatus
    // set mpp (in mstatus)
    if (priv == 0) {                                    // user mode
      set_csr(0x300, csr[0x300] & 0xffffffffffffe7ff);  // set mpp
    } else if (priv == 3) {                             // machine mode
      set_csr(0x300, csr[0x300] | 0x1800);
    }

    // set mpie (in mstatus)
    if (csr[0x300] & 0x8) {  // if mpie was 1 before
      set_csr(0x300, csr[0x300] | 0x0000000000000080);
    } else {  // if mpie was 0
      set_csr(0x300, csr[0x300] & 0xfffffffffffffff7);
    }

    // set mie (in mstatus)
    set_csr(0x300, csr[0x300] & 0xfffffffffffffff7);

    priv = 3;
    instruction_count--;  // for some reason, calling an exception = error
  } else if (type == "MRET") {
    if (priv == 0) {  // if mret during user priv, exception
      raise_exception(2);
    } else {
      set_pc(csr[0x341] - 4);  // return pc

      priv = (csr[0x300] >> 11) & 0x3;  // set priv to mpp

      uint64_t buff = (csr[0x300] & 0x80) >> 4;  // extract MPIE
      set_csr(0x300, (csr[0x300] & 0xffffffffffffe777) | (0x80 | buff));
    }
  }
}

void processor::raise_exception(int cause) {
  uint64_t og_pc = pc;
  set_csr(0x341, pc);     // set mepc to pc
  set_csr(0x342, cause);  // set mcause to cause
  pc = (csr[0x305] & 0xfffffffffffffffc) - 4;

  uint64_t buff = (csr[0x300] & 0x8);
  uint64_t buff2 = csr[0x300] & 0xffffffffffffe777;
  set_csr(0x300, buff2 | (priv << 11 | buff << 4));

  if (cause == 0) {  // misaligned instruction
    set_csr(0x343, og_pc);
    instruction_count++;
    pc = pc + 4;
  }

  if (cause == 2) {  // illegal instruction
    set_csr(0x343, curr_inst);
  }

  if (cause == 4) {  // misaligned load
    uint64_t immediate = binary_return(0, 12, 1);
    uint64_t register_1 = binary_return(12, 5, 0);
    int64_t addr = registers[register_1] + immediate;
    set_csr(0x343, addr);
  }

  if (cause == 6) {  // misaligned store
    uint64_t register_1 = binary_return(12, 5, 0);
    uint64_t immediate5_11 = binary_return(0, 7, 0) << 5;
    uint64_t immediate4_0 = binary_return(20, 5, 0);
    uint64_t combined_immediate = immediate4_0 + immediate5_11;
    if (current_instruction[0] == 1) {
      combined_immediate = combined_immediate + 0xfffffffffffff000;
    }
    uint64_t addr = combined_immediate + registers[register_1];
    set_csr(0x343, addr);
  }

  if (cause == 8 || cause == 11) {
    priv = 3;
    set_csr(0x343, 0);
  }
  instruction_count--;
}
// Display PC value
void processor::show_pc() {
  cout << setw(16) << setfill('0') << hex << pc << endl;
  return;
}

// Set PC to new value
void processor::set_pc(uint64_t new_pc) {
  pc = new_pc;
  return;
}

// Display register value
void processor::show_reg(unsigned int reg_num) {
  cout << setw(16) << setfill('0') << hex << registers[reg_num] << endl;
  return;
}

// Set register to new value
void processor::set_reg(unsigned int reg_num, uint64_t new_value) {
  if (reg_num != 0) {
    registers[reg_num] = new_value;
  }
  return;
}

// Execute a number of instructions
void processor::execute(unsigned int num, bool breakpoint_check) {
  for (unsigned int i = 0; i < num; i++) {
    if (breakpoint_check && (pc == breakpoint)) {
      cout << "Breakpoint reached at ";
      cout << setw(16) << setfill('0') << hex << breakpoint << endl;
      break;
    }
    // interrupt catcher
    if ((csr[0x300] & 0x8) || (priv == 0)) {
      // 0x344 = mip, 0x304 = mie
      if ((csr[0x344] & 0x800) && (csr[0x304] & 0x800)) {  // meip, meie
        cause_interrupt(11);  // machine external interrupt
      } else if ((csr[0x344] & 0x8) && (csr[0x304] & 0x8)) {  // msip, msie
        cause_interrupt(3);  // machine software interrupt
      } else if ((csr[0x344] & 0x80) && (csr[0x304] & 0x80)) {  // mtip, mtie
        cause_interrupt(7);  // machine timer interrupt
      } else if ((csr[0x344] & 0x100) && (csr[0x304] & 0x100)) {  // ueip, ueie
        cause_interrupt(8);  // user external interrupt
      } else if ((csr[0x344] & 0x1) && (csr[0x304] & 0x1)) {  // usip, usie
        cause_interrupt(0);  // user software interrupt
      } else if ((csr[0x344] & 0x10) && (csr[0x304] & 0x10)) {  // utip, utie
        cause_interrupt(4);  // user timer interrupt
      }
    }
    if (pc % 4 != 0) {
      raise_exception(0);
      // cout << "Error: misaligned pc" << endl;
      continue;
    }
    load_instruction(storage->read_doubleword(pc), pc);
   // cout << setw(16) << setfill('0') << hex << storage->read_doubleword(pc)
   //      << endl;
    string type = instruction_type();
   // cout << "Type: " << type << endl;
   // cout << "pc og: ";
   // show_pc();
    do_instruction(type);
   // cout << "pc new: ";
  //  show_pc();
    instruction_count++;
    pc = pc + 4;
  }
}

// Clear breakpoint
void processor::clear_breakpoint() { breakpoint = ULLONG_MAX; }

// Set breakpoint at an address
void processor::set_breakpoint(uint64_t address) { breakpoint = address; }

// TODO stage2
// Show privilege level
// Empty implementation for stage 1, required for stage 2
void processor::show_prv() {
  if (priv == 0) {
    cout << "0 (user)" << endl;
  } else if (priv == 3) {
    cout << "3 (machine)" << endl;
  } else {
    cout << "ERROR: priv is not 0 or 3" << endl;
  }
  return;
}

// Set privilege level
// Empty implementation for stage 1, required for stage 2
void processor::set_prv(unsigned int prv_num) {
  if (prv_num == 0) {
    priv = 0;
  } else if (prv_num == 3) {
    priv = 3;
  } else {
    cout << "ERROR: prv_num is not 0 or 3" << endl;
  }
  return;
}

// Display CSR value
// Empty implementation for stage 1, required for stage 2
void processor::show_csr(unsigned int csr_num) {
  if (csr_num == 0xf11 || csr_num == 0xf12 || csr_num == 0xf13 ||
      csr_num == 0xf14 || csr_num == 0x300 || csr_num == 0x301 ||
      csr_num == 0x304 || csr_num == 0x305 || csr_num == 0x340 ||
      csr_num == 0x341 || csr_num == 0x342 || csr_num == 0x343 ||
      csr_num == 0x344) {
    cout << setw(16) << setfill('0') << hex << csr[csr_num] << endl;
  } else {
    cout << "Illegal CSR number" << endl;
  }
  return;
}

// Set CSR to new value
// Empty implementation for stage 1, required for stage 2
void processor::set_csr(unsigned int csr_num, uint64_t new_value) {
  if (csr_num == 0xf11 || csr_num == 0xf12 || csr_num == 0xf13 ||
      csr_num == 0xf14) {
    // mvendorid, marchid, mimpid, mhartid are fixed registers
    cout << "Illegal write to read-only CSR" << endl;
  } else if (csr_num == 0x300) {                 // mstatus
    new_value = new_value & 0x0000000000001888;  // masked assignment
    new_value = new_value + 0x0000000200000000;  // add uxl (set to 2)
    csr[csr_num] = new_value;
  } else if (csr_num == 0x301) {  // misa
    if (is_verbose) {
      cout << "the csr is writable but fixed" << endl;
    }
  } else if (csr_num == 0x304) {                 // mie
    new_value = new_value & 0x0000000000000999;  // for implemented bits
    csr[csr_num] = new_value;
  } else if (csr_num == 0x305) {  // mtvec
    if ((new_value & 0x1) == 1) {
      new_value = new_value & 0xffffffffffffff01;  // clear8 bits, leaving MODE
    } else {
      new_value = new_value & 0xfffffffffffffffc;  // c to not mask MODE
    }
    csr[csr_num] = new_value;
  } else if (csr_num == 0x340) {  // mscratch
    csr[csr_num] = new_value;
  } else if (csr_num == 0x341) {                 // mepc
    new_value = new_value & 0xfffffffffffffffc;  // remove last 2 bits (pc % 4)
    csr[csr_num] = new_value;
  } else if (csr_num == 0x342) {                 // mcause
    new_value = new_value & 0x800000000000000f;  // 8=interrupt bit, f=exception
    csr[csr_num] = new_value;
  } else if (csr_num == 0x343) {  // mtval
    csr[csr_num] = new_value;     // write depends on interrupt / exception
  } else if (csr_num == 0x344) {  // mip
    new_value = new_value & 0x0000000000000999;  // for implemented bits
    csr[csr_num] = new_value;
  } else {
    if (is_verbose) {
      cout << "csr not implemented" << endl;
    }
  }

  return;
}

bool processor::illegal_csr(uint64_t csr_num, uint64_t reg1) {
  bool valid_csr = false;
  if (csr_num == 0xf11 || csr_num == 0xf12 || csr_num == 0xf13 ||
      csr_num == 0xf14 || csr_num == 0x300 || csr_num == 0x301 ||
      csr_num == 0x304 || csr_num == 0x305 || csr_num == 0x340 ||
      csr_num == 0x341 || csr_num == 0x342 || csr_num == 0x343 ||
      csr_num == 0x344) {
    valid_csr = true;
  }
  if (priv == 0 || valid_csr == false || (csr_num == 0xf11 && reg1 != 0) ||
      (csr_num == 0xf12 && reg1 != 0) || (csr_num == 0xf13 && reg1 != 0) ||
      (csr_num == 0xf14 && reg1 != 0)) {
    return false;
  }
  return true;
}
bool processor::illegal_csr_imm(uint64_t csr_num) {
  bool valid_csr = false;
  if (csr_num == 0xf11 || csr_num == 0xf12 || csr_num == 0xf13 ||
      csr_num == 0xf14 || csr_num == 0x300 || csr_num == 0x301 ||
      csr_num == 0x304 || csr_num == 0x305 || csr_num == 0x340 ||
      csr_num == 0x341 || csr_num == 0x342 || csr_num == 0x343 ||
      csr_num == 0x344) {
    valid_csr = true;
  }
  if (priv == 0 || valid_csr == false) {
    return false;
  }
  return true;
}

void processor::cause_interrupt(int cause) {
  set_csr(0x341, pc);                               // set mepc to pc
  set_csr(0x300, csr[0x300] | 0x0000000000000080);  // set mpie

  set_csr(0x342, 0x8000000000000000 + cause);  // set interrupt + cause

  if (csr[0x305] & 0x1) {  // if vectored mode
    pc = (csr[0x305] & 0xfffffffffffffffc) + (4 * cause);
  } else {  // not vectored
    pc = (csr[0x305] & 0xfffffffffffffffc);
  }
  if (priv == 0) {                                    // set mpp to priv (0)
    set_csr(0x300, csr[0x300] & 0xffffffffffffe7ff);  // e7 == 00 at mpp

    if ((csr[0x300] & 0x8) == false) {  // if mie = false, mpie = false
      set_csr(0x300, csr[0x300] & 0xffffffffffffff7f);
    }

    priv = 3;              // set priv to machine mode
  } else if (priv == 3) {  // if priv ==3, set mpp =3
    set_csr(0x300, csr[0x300] | 0x0000000000001800);
  }

  set_csr(0x300, csr[0x300] & 0xfffffffffffffff7);  // set mie to 0
}

uint64_t processor::get_instruction_count() { return instruction_count; }

// Used for Postgraduate assignment. Undergraduate assignment can return 0.
uint64_t processor::get_cycle_count() { return 0; }
