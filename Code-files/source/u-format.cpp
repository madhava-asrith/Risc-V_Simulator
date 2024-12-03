#include "u-format.hh"

std::unordered_map<std::string, uint8_t> u_type;         // map to store U-format instructions

void U_type_init_map() {                         // initialize the map
    u_type["lui"] = 0x37;
    u_type["auipc"] = 0x17;
}
  
// Function to convert to binary
std::string convertUTypeToBinary(uint8_t opcode, uint8_t rd, uint32_t imm) {
    validateFieldSize(opcode, 7, "Opcode");
    validateFieldSize(rd, 5, "Destination register (rd)");
    validateFieldSize(imm, 20, "Immediate value");

    std::bitset<7> opcode_bits(opcode);
    std::bitset<5> rd_bits(rd);
    std::bitset<20> immediate(imm);

    std::string binInst = immediate.to_string() + rd_bits.to_string() + opcode_bits.to_string();

    return binInst;
}
 
// Function to convert instruction line to hex
std::string U_type_inst(std::vector<std::string>* vect) {
    std::vector<std::string>& v = *vect;
    v[1].pop_back();
    change_alias(&(v[1]));
    v[1] = v[1].substr(1, v[1].length());

    uint8_t opcode, rd;
    uint32_t imm = 0;

    auto iter = u_type.find(v[0]);
    opcode = iter->second;
    try {
        rd = static_cast<uint8_t>(std::stoi(v[1]));
    } catch (const std::exception& e) {
        std::cout << "Conversion failed: " << e.what() << std::endl;
    }

    // imm = (uint32_t)std::stoul(v[2]);
    if(is_valid_hex(v[2]))
    {
        std::istringstream(v[2]) >> std::hex >> imm;        // changed
    }
    else if(is_valid_dec(v[2]))
    {
        std::istringstream(v[2]) >> std::dec >> imm;
    }
    else
    {
        std::cerr << "\033[31m" << "Invalid immediate value for U-type instruction" << "\033[37m" << std::endl;
    }

    if (imm < 0) {
        throw std::invalid_argument("Immediate value cannot be negative");
    }

    std::string binInst = convertUTypeToBinary(opcode, rd, imm);
    std::string hexInst = binaryToHex(binInst);

    return hexInst;
}

void u_type_decoder(std::string bin_inst, uint32_t instruction)
{
    uint8_t rd = static_cast<uint8_t>(std::stoi(bin_inst.substr(20, 5), nullptr, 2));
    uint8_t opcode = instruction & 0x7f;
    int32_t imm = instruction & 0xFFFFF000;     

    update_last_executed_line();

    switch (opcode)
    {
    case 0x37:
        regFile.write(rd, imm); 
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        break;

    case 0x17:
        // imm = imm >> 12;
        uint64_t result = regFile.pc + imm;
        regFile.write(rd, result);
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;

        break;
    }
}