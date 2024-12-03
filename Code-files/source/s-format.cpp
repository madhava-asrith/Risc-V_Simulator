#include "s-format.hh" 

std::unordered_map<std::string, uint8_t> s_type;      // map to store S-format instructions

void S_type_init_map() {             // initialize the map
    s_type["sd"] = 0x3;
    s_type["sw"] = 0x2;
    s_type["sh"] = 0x1;
    s_type["sb"] = 0x0;
} 

// Function to convert to binary
std::string convertSTypeToBinary(uint8_t opcode, uint8_t func3, uint8_t rs1, uint8_t rs2, uint16_t imm) {
    validateFieldSize(opcode, 7, "Opcode");
    validateFieldSize(func3, 3, "Funct3");
    validateFieldSize(rs1, 5, "Source Register 1 (rs1)");
    validateFieldSize(rs2, 5, "Source Register 2 (rs2)");
    validateFieldSize_s(imm, 12, "Immediate value");

    std::bitset<7> opcode_bits(opcode);
    std::bitset<3> funct3_bits(func3);
    std::bitset<5> rs2_bits(rs2);
    std::bitset<5> rs1_bits(rs1);
    std::bitset<12> immediate(imm);

    std::string imm_1 = immediate.to_string().substr(0, 7);
    std::string imm_2 = immediate.to_string().substr(7, 5);

    std::string binaryInstruction = imm_1 + rs2_bits.to_string() + rs1_bits.to_string() +
        funct3_bits.to_string() + imm_2 + opcode_bits.to_string();

    return binaryInstruction;
}

// Function to convert instruction line to hex 
std::string S_type_inst(std::vector<std::string>* vect) {
    std::vector<std::string>& v = *vect;
    v[1].pop_back();
    change_alias(&(v[1]));
    v[1] = v[1].substr(1, v[1].length());

    std::string temp;
    std::stringstream ss(v[2]);
    while (ss.good()) {
        std::string substr;
        getline(ss, substr, '(');
        temp = substr;
        ss.clear();
        break;
    }

    v[2] = v[2].substr(temp.length() + 1, v[2].length() - temp.length() - 2);        // gets only the register
    change_alias(&(v[2]));
    v[2] = v[2].substr(1, v[2].length());             // removes the x in front of register 

    v.push_back(temp);

    uint8_t opcode, func3, rs1, rs2;
    uint16_t imm;

    opcode = 0x23;
    auto iter = s_type.find(v[0]);
    func3 = iter->second;
    rs1 = (uint8_t)std::stoul(v[2]);
    rs2 = (uint8_t)std::stoul(v[1]);
    imm = (uint16_t)std::stoi(v[3]);

    std::string binInst = convertSTypeToBinary(opcode, func3, rs1, rs2, imm);
    std::string hexInst = binaryToHex_signed(binInst);

    return hexInst;
}

void s_type_decoder(std::string bin_inst, uint32_t instruction)
{
    uint16_t rs1, func3, rs2;
    rs1 = std::stoi(bin_inst.substr(12, 5), nullptr, 2); 
    func3 = std::stoi(bin_inst.substr(17, 3), nullptr, 2);
    rs2 = std::stoi(bin_inst.substr(7,5), nullptr, 2);

    int64_t imm_1 = (instruction >> 25) & 0x7f;          // 7-bits
    int64_t imm_2 = (instruction >> 7) & 0x1f;          // 5-bits
    int64_t imm = (imm_1 << 5) | imm_2;

    imm = (imm & 0x800) ? (imm | 0xFFFFFFFFFFFFF000) : imm;

    int64_t addr = regFile.read(rs1) + imm;
    uint64_t value = regFile.read(rs2);

    switch(func3)
    {
        case 0x0:
            if(cache.config.writeBackPolicy == "WT" || !cache.enabled)
            {
                memory.store_byte(addr,value);
            }
            else 
            {
                std::get<0>(cache.access_cache(addr, true, 1, value, cache.file_name));
            }

            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
            break;

        case 0x1:
            if(cache.config.writeBackPolicy == "WT" || !cache.enabled)
            {
                memory.store_hw(addr,value);
            }
            else
            {
                std::get<0>(cache.access_cache(addr, true, 2, value, cache.file_name));
            }

            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
            break;

        case 0x2:
            if(cache.config.writeBackPolicy == "WT" || !cache.enabled)
            {
                memory.store_w(addr,value);
            }
            else
            {
                std::get<0>(cache.access_cache(addr, true, 4, value, cache.file_name));
            }

            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
            break;
            
        case 0x3:
            if(cache.config.writeBackPolicy == "WT" || !cache.enabled)
            {
                memory.store_dw(addr,value);
            }
            else
            {
                std::get<0>(cache.access_cache(addr, true, 8, value, cache.file_name));
            }

            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
            break;
            
    }

    update_last_executed_line();
}