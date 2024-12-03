#ifndef R_FORMAT_HH
#define R_FORMAT_HH
#include<unordered_map>
#include<vector>
#include "i-format.hh"

extern std::unordered_map<std::string, int> r_type;            // map to store R-format instructions
 
void R_type_init_map();

// Function to convert RISC-V R-type instruction to binary
std::string convertRTypeToBinary(uint8_t opcode, uint8_t rd, uint8_t funct3, uint8_t rs1, uint8_t rs2, uint8_t funct7);

// Function to check the instruction and initialize func3 and func7 values
void check_inst(std::string str, uint8_t* f3, uint8_t* f7);

// Function to convert the instruction to binary then to hex
std::string R_type_inst(std::vector<std::string>* vect);

// Function to convert the instruction to binary then to hex
int r_type_decoder(std::string bin_inst, uint32_t instruction);

#endif