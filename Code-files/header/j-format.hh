#include "u-format.hh"

extern std::unordered_map<std::string, int> j_type;             // map to store J-format instructions
extern bool inf_loop;
 
void J_type_init_map();
 
// Function to convert to binary
std::string convertJTypeToBinary(uint8_t opcode, uint8_t rd, uint32_t imm);

// Function to convert instruction line to hex
std::string J_type_inst(std::vector<std::string>* vect, int cur_line);

void j_type_decoder(std::string bin_inst, uint32_t instruction);