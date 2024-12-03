#ifndef I_FORMAT_HH
#define I_FORMAT_HH

#include<unordered_map>
#include "s-format.hh"
#include<vector> 

extern std::unordered_map<std::string, uint8_t> i_type;              // map to store I-format instructions 
 
void I_type_init_map();

std::string convertITypeToBinary(uint8_t opcode, uint8_t rd, uint8_t func3, uint8_t rs1, int16_t imm, uint8_t func6, int flag);
void init_func6(std::string str, uint8_t* f6, int* flag);

// Function to convert instruction line into hex
std::string I_type_inst(std::vector<std::string>* vect);

void i_type_decoder(std::string bin_inst, uint32_t instruction);

void load_decoder(std::string bin_inst, uint32_t instruction);

void jalr_decoder(std::string bin_inst, uint32_t instruction);

#endif