#ifndef B_FORMAT_HH
#define B_FORMAT_HH
#include <unordered_map>
#include "j-format.hh"
#include <vector>

extern std::unordered_map<std::string, uint8_t> b_type;         // map to store B-format instructions

void B_type_init_map();
std::string convertBTypeToBinary(uint8_t opcode, uint8_t func3, uint8_t rs1, uint8_t rs2, uint16_t imm);
std::string B_type_inst(std::vector<std::string>* vect, int cur_line);
void b_type_decoder(std::string bin_inst, uint32_t instruction);

#endif