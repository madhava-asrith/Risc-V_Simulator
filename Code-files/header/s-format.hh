#ifndef S_FORMAT_HH
#define S_FORMAT_HH
#include<unordered_map>
#include "b-format.hh"
#include <vector>
  
extern std::unordered_map<std::string, uint8_t> s_type;      // map to store S-format instructions

void S_type_init_map();

// Function to convert to binary
std::string convertSTypeToBinary(uint8_t opcode, uint8_t func3, uint8_t rs1, uint8_t rs2, uint16_t imm);

// Function to convert instruction line to hex 
std::string S_type_inst(std::vector<std::string>* vect);

void s_type_decoder(std::string bin_inst, uint32_t instruction);

#endif