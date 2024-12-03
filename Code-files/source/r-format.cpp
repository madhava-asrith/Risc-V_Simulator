#include "r-format.hh"

std::unordered_map<std::string, int> r_type;            // map to store R-format instructions
 
void R_type_init_map() {                     // initialize the map
    r_type["add"] = 0;
    r_type["sub"] = 1;
    r_type["and"] = 2;
    r_type["or"] = 3;
    r_type["xor"] = 4;
    r_type["sll"] = 5;
    r_type["srl"] = 6;
    r_type["sra"] = 7;
    r_type["slt"] = 8;
    r_type["sltu"] = 9;
}

// Function to convert RISC-V R-type instruction to binary
std::string convertRTypeToBinary(uint8_t opcode, uint8_t rd, uint8_t funct3, uint8_t rs1, uint8_t rs2, uint8_t funct7) {
    validateFieldSize(opcode, 7, "Opcode");
    validateFieldSize(rd, 5, "Destination Register (rd)");
    validateFieldSize(funct3, 3, "Funct3");
    validateFieldSize(rs1, 5, "Source Register 1 (rs1)");
    validateFieldSize(rs2, 5, "Source Register 2 (rs2)");
    validateFieldSize(funct7, 7, "Funct7");

    std::bitset<7> opcode_bits(opcode);
    std::bitset<5> rd_bits(rd);
    std::bitset<3> funct3_bits(funct3);
    std::bitset<5> rs1_bits(rs1);
    std::bitset<5> rs2_bits(rs2);
    std::bitset<7> funct7_bits(funct7);

    // Construct the R-type binary instruction
    std::string binaryInstruction = funct7_bits.to_string() + rs2_bits.to_string() + rs1_bits.to_string() +
        funct3_bits.to_string() + rd_bits.to_string() + opcode_bits.to_string();

    return binaryInstruction;
}

// Function to check the instruction and initialize func3 and func7 values
void check_inst(std::string str, uint8_t* f3, uint8_t* f7) {
    if (str == "add") {
        *f3 = 0x0;
        *f7 = 0x0;
    }
    else if (str == "sub") {
        *f3 = 0x0;
        *f7 = 0x20;
    }
    else if (str == "and") {
        *f3 = 0x7;
        *f7 = 0x0;
    }
    else if (str == "or") {
        *f3 = 0x6;
        *f7 = 0x0;
    }
    else if (str == "xor") {
        *f3 = 0x4;
        *f7 = 0x0;
    }
    else if (str == "sll") {
        *f3 = 0x1;
        *f7 = 0x0;
    }
    else if (str == "srl") {
        *f3 = 0x5;
        *f7 = 0x0;
    }
    else if (str == "sra") {
        *f3 = 0x5;
        *f7 = 0x20;
    }
    else if (str == "slt")
    {
        *f3 = 0x2;
        *f7 = 0x00;
    }
    else if (str == "sltu")
    {
        *f3 = 0x3;
        *f7 = 0x00;
    }
}

// Function to convert the instruction to binary then to hex
std::string R_type_inst(std::vector<std::string>* vect) {
    std::vector<std::string>& v = *vect;
    v[2].pop_back();                    // removes the ,
    v[1].pop_back();

    if (v.size() > 4)
    {
        throw std::invalid_argument("More than 3 registers for R-type instruction");
    }

    change_alias(&v[1]);              // changes the alias names to register names
    change_alias(&v[2]);
    change_alias(&v[3]);

    v[3] = v[3].substr(1, v[3].length());          // removes the x in front of register 
    v[2] = v[2].substr(1, v[2].length());
    v[1] = v[1].substr(1, v[1].length());


    // intialize the values
    uint8_t opcode, rd, rs1, rs2, func3, func7;
    opcode = 0x33;                      // updating opcode
    rd = (uint8_t)std::stoul(v[1], nullptr);     // updating destination reg
    rs1 = (uint8_t)std::stoul(v[2], nullptr);   // updating source reg1
    rs2 = (uint8_t)std::stoul(v[3], nullptr);    // updating source reg2

    check_inst(v[0], &func3, &func7);     // updating func3, func7


    // // send the values to convertRTypetoBinary === it returns a binary
    std::string binInst = convertRTypeToBinary(opcode, rd, func3, rs1, rs2, func7);
    std::string hexInst = binaryToHex(binInst);

    return hexInst;
}

// Function to convert the instruction to binary then to hex
int r_type_decoder(std::string bin_inst, uint32_t instruction) 
{
    uint16_t rd, rs1, rs2, func3, func7;
    func7 = std::stoi(bin_inst.substr(0, 7), nullptr, 2);
    rs2 = std::stoi(bin_inst.substr(7, 5), nullptr, 2);
    rs1 = std::stoi(bin_inst.substr(12, 5), nullptr, 2);
    func3 = std::stoi(bin_inst.substr(17, 3), nullptr, 2);
    rd = std::stoi(bin_inst.substr(20, 5), nullptr, 2);
    
    uint64_t result;

    if(rd == 0)
    {
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        return 0;
    }
    
    switch (func3)
    {
    case 0:
        if (func7 == 0)
        {
            result = regFile.read(rs1) + regFile.read(rs2);
            regFile.write(rd, result);

            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;

            break;
        }
        if (func7 == 0x20)
        {
            result = regFile.read(rs1) - regFile.read(rs2);
            regFile.write(rd, result);
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
            break;
        }
        break;

    case 1:
        result = regFile.read(rs1) << regFile.read(rs2);
        regFile.write(rd, result);
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        break;

    case 2:
        if ((int64_t)regFile.read(rs1) < (int64_t)regFile.read(rs2))
        {
            regFile.write(rd, 1);
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        }
        else
        {
            regFile.write(rd,0);
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        }
        break;

    case 3:
        if (regFile.read(rs1) < regFile.read(rs2))
        {
            regFile.write(rd, regFile.read(rs1));
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        }
        else
        {
            regFile.write(rd,0);
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        }
        break;

    case 4:
        result = regFile.read(rs1) ^ regFile.read(rs2);
        regFile.write(rd, result);
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        break;

    case 5:
        if (func7 == 0)
        {
            result = regFile.read(rs1) >> regFile.read(rs2);
            regFile.write(rd, result);
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        }
        if (func7 == 32)
        {
            result = (int64_t)regFile.read(rs1) >> regFile.read(rs2);
            regFile.write(rd, result);
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        }
        break;

    case 6:
        result = regFile.read(rs1) | regFile.read(rs2);
        regFile.write(rd, result);
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        break;

    case 7:
        result = regFile.read(rs1) & regFile.read(rs2);
        regFile.write(rd, result);
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        break;

    }

    update_last_executed_line();

    return 0;
}
