#ifndef U_FORMAT_HH
#define U_FORMAT_HH
#include "cache.hh"

extern std::unordered_map<std::string, uint8_t> u_type;         // map to store U-format instructions

void U_type_init_map();
  
// Function to convert to binary
std::string convertUTypeToBinary(uint8_t opcode, uint8_t rd, uint32_t imm);

// Function to convert instruction line to hex
std::string U_type_inst(std::vector<std::string>* vect);

void u_type_decoder(std::string bin_inst, uint32_t instruction); 

#endif