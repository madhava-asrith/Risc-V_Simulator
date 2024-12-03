#include "i-format.hh"

std::unordered_map<std::string, uint8_t> i_type;              // map to store I-format instructions 

void I_type_init_map() {                        // initialize the map
    i_type["addi"] = 0x0;
    i_type["andi"] = 0x7;
    i_type["ori"] = 0x6;
    i_type["xori"] = 0x4;
    i_type["slli"] = 0x1;
    i_type["srli"] = 0x5;
    i_type["srai"] = 0x5;
    i_type["ld"] = 0x3;
    i_type["lw"] = 0x2;
    i_type["lh"] = 0x1;
    i_type["lb"] = 0x0;
    i_type["lwu"] = 0x6;
    i_type["lhu"] = 0x5;
    i_type["lbu"] = 0x4;
    i_type["jalr"] = 0x0;
}

// Function to convert instruction line into hex
std::string I_type_inst(std::vector<std::string>* vect) {
    std::vector<std::string>& v = *vect;
    if (v.size() == 3) {
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

        v[2] = v[2].substr(temp.length() + 1, v[2].length() - temp.length() - 2);
        change_alias(&(v[2]));
        v[2] = v[2].substr(1, v[2].length());             // removes the x in front of register 

        v.push_back(temp);
    }
    else {
        v[2].pop_back();                    // removes the ,
        v[1].pop_back();

        change_alias(&(v[1]));              // changes the alias names to register names
        change_alias(&v[2]);

        v[2] = v[2].substr(1, v[2].length());             // removes the x in front of register 
        v[1] = v[1].substr(1, v[1].length());
    }


    // intialize the values
    uint8_t opcode, rd, rs1, func3;
    int16_t imm;
    uint8_t func6 = 0x00;
    int flag = 0;                                   // shows when to initialize func6 

    if (v[0] == "ld" || v[0] == "lb" || v[0] == "lw" || v[0] == "lh" || v[0] == "lbu" || v[0] == "lwu" || v[0] == "lhu") {
        opcode = 0x03;
    }
    else if (v[0] == "jalr") {
        opcode = 0x67;
    }
    else {
        opcode = 0x13;
    }

    rd = (uint8_t)std::stoul(v[1], nullptr);     // updating destination reg
    rs1 = (uint8_t)std::stoul(v[2], nullptr);   // updating source reg1
    auto iter = i_type.find(v[0]);
    func3 = iter->second;
    // imm = (int16_t)std::stoi(v[3]);

    if(is_valid_hex(v[3]))
    {
        std::istringstream(v[3]) >> std::hex >> imm;        // changed
    }
    else if(is_valid_dec(v[3]))
    {
        std::istringstream(v[3]) >> std::dec >> imm;
    }
    else
    {
        std::cerr << "\033[31m" << "Invalid immediate value for I-type instruction" << "\033[37m" << std::endl;
    }

    init_func6(v[0], &func6, &flag);

    std::string binInst = convertITypeToBinary(opcode, rd, func3, rs1, imm, func6, flag);
    std::string hexInst = binaryToHex_signed(binInst);

    return hexInst;
}

// Function to convert RISC-V I-type instruction to binary
std::string convertITypeToBinary(uint8_t opcode, uint8_t rd, uint8_t func3, uint8_t rs1, int16_t imm, uint8_t func6, int flag) {
    validateFieldSize(opcode, 7, "Opcode");
    validateFieldSize(rd, 5, "Destination Register (rd)");
    validateFieldSize(func3, 3, "Funct3");
    validateFieldSize(rs1, 5, "Source Register 1 (rs1)");

    std::bitset<7> opcode_bits(opcode);
    std::bitset<5> rd_bits(rd);
    std::bitset<3> funct3_bits(func3);
    std::bitset<5> rs1_bits(rs1);


    if (flag == 0) {
        validateFieldSize_s(imm, 12, "Immediate");
        std::bitset<12> immediate(imm);

        std::string binaryInstruction = immediate.to_string() + rs1_bits.to_string() +
            funct3_bits.to_string() + rd_bits.to_string() + opcode_bits.to_string();

        return binaryInstruction;
    }
    else {
        validateFieldSize(imm, 6, "Immediate");
        validateFieldSize(func6, 6, "Funct6");

        std::bitset<6> immediate(imm);
        std::bitset<6> func6_bits(func6);

        std::string binaryInstruction = func6_bits.to_string() + immediate.to_string() + rs1_bits.to_string() +
            funct3_bits.to_string() + rd_bits.to_string() + opcode_bits.to_string();

        return binaryInstruction;
    }
}

// Function to initialize func6 value for shift instructions
void init_func6(std::string str, uint8_t* f6, int* flag) {
    if (str == "slli") {
        *f6 = 0x00;
        *flag = 1;
    }
    else if (str == "srli") {
        *f6 = 0x00;
        *flag = 1;
    }
    else if (str == "srai") {
        *f6 = 0x10;
        *flag = 1;
    }
}

void i_type_decoder(std::string bin_inst, uint32_t instruction)
{    
    uint16_t rd, rs1, func3, func6;
    rs1 = std::stoi(bin_inst.substr(12, 5), nullptr, 2); 
    func3 = std::stoi(bin_inst.substr(17, 3), nullptr, 2); 
    rd = std::stoi(bin_inst.substr(20, 5), nullptr, 2);
    func6 = std::stoi(bin_inst.substr(0, 6), nullptr, 2);

    int32_t imm = (instruction >> 20) & 0xFFF;                  // immediate for i-type
    uint32_t s_imm = (imm & 0x3f);       // immediate for shift instructions 
    imm = (imm & 0x800) ? (imm | 0xFFFFFFFFFFFFF000) : imm;

    int64_t result;
    uint64_t res;

    switch (func3)
    {
    case 0x0:
        result = regFile.read(rs1) + imm;
        regFile.write(rd, result);
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        break;

    case 0x1:
        result = regFile.read(rs1) << imm;
        regFile.write(rd, result);
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        break;

    case 0x2:
        if ((int64_t)regFile.read(rs1) < imm)
        {
            regFile.write(rd,0x01);
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        }
        else
        {
            regFile.write(rd, 0x0);
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        }
        break;

    case 0x3:
        if (regFile.read(rs1) < imm)
        {
            regFile.write(rd, 0x1);
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        }
        else
        {
            regFile.write(rd, 0x0);
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        }
        break;

    case 0x4:
        result = regFile.read(rs1) ^ imm;
        regFile.write(rd, result);
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        break;

    case 0x5:
        switch (func6)
        {
        case 0x0: 
            res = regFile.read(rs1) >> s_imm;
            regFile.write(rd, res);
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
            break;

        case 0x10:
            result = static_cast<int64_t>(regFile.read(rs1)) >> s_imm;
            regFile.write(rd, result);
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
            break;
        }
        break;

    case 0x6:
        result = regFile.read(rs1) | imm;
        regFile.write(rd, result);
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        break;

    case 0x7:
        result = regFile.read(rs1) & imm;
        regFile.write(rd, result);
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
        break;
    }

    update_last_executed_line();
}

void load_decoder(std::string bin_inst, uint32_t instruction)
{
    uint64_t rd, rs1, func3, func6;
    rs1 = std::stoi(bin_inst.substr(12, 5), nullptr, 2);
    func3 = std::stoi(bin_inst.substr(17, 3), nullptr, 2);
    rd = std::stoi(bin_inst.substr(20, 5), nullptr, 2);
    func6 = std::stoi(bin_inst.substr(0, 6), nullptr, 2);

    int32_t imm = (instruction >> 20) & 0xFFF;                  // immediate for i-type
    imm = (imm & 0x800) ? (imm | 0xFFFFFFFFFFFFF000) : imm;

    int64_t result;
    int64_t res = regFile.read(rs1) + imm;

    switch (func3)
    {
    case 0x0:   // lb

        if(!cache.enabled)
        {
            result = static_cast<int64_t>(memory.read_byte_signed(res)); 
        }
        else 
        {
            result = std::get<1>(cache.access_cache(res, false, 1, 0, cache.file_name));
        }

        regFile.write(rd, result);
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl; 
        break;

    case 0x1:   // lh

        if(!cache.enabled)
        {
            result = memory.read_hw_signed(res); 
        }
        else 
        {
            result = std::get<1>(cache.access_cache(res, false, 2, 0, cache.file_name));
        }

        regFile.write(rd, result); 
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl; 
        break;

    case 0x2:   // lw

        if(!cache.enabled)
        {
            result = memory.read_w_signed(res); 
        }
        else 
        {
            result = std::get<1>(cache.access_cache(res, false, 4, 0, cache.file_name));
        }

        regFile.write(rd, result); 
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl; 
        break;

    case 0x3:   // ld

        if(!cache.enabled)
        {
            result = memory.read_dw(res); 
        }
        else 
        {
            result = std::get<1>(cache.access_cache(res, false, 8, 0, cache.file_name));
        }

        regFile.write(rd, result); 
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl; 
        break;

    case 0x4:   // lbu

        if(!cache.enabled)
        {
            result = memory.read_byte_unsigned(res); 
        }
        else 
        {
            result = std::get<0>(cache.access_cache(res, false, 1, 0, cache.file_name));
        }

        regFile.write(rd, result);
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl; 
        break;

    case 0x5:   // lhu

        if(!cache.enabled)
        {
            result = memory.read_hw_unsigned(res); 
        }
        else 
        {
            result = std::get<0>(cache.access_cache(res, false, 2, 0, cache.file_name));
        }

        regFile.write(rd, result); 
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl; 
        break;

    case 0x6:   // lwu

        if(!cache.enabled)
        {
            result = memory.read_w_unsigned(res); 
        }
        else 
        {
            result = std::get<0>(cache.access_cache(res, false, 4, 0, cache.file_name));
        }

        regFile.write(rd, result); 
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl; 
        break;
    }

    update_last_executed_line();
}

void jalr_decoder(std::string bin_inst, uint32_t instruction)
{
    uint64_t rd, rs1, func3, func6;
    rs1 = std::stoi(bin_inst.substr(12, 5), nullptr, 2);
    func3 = std::stoi(bin_inst.substr(17, 3), nullptr, 2);
    rd = std::stoi(bin_inst.substr(20, 5), nullptr, 2);

    int32_t imm = (instruction >> 20) & 0xFFF;                  // immediate for i-type
    imm = (imm & 0x800) ? (imm | 0xFFFFFFFFFFFFF000) : imm;

    regFile.write(rd, regFile.pc + 4);
    int64_t pc_imm = regFile.read(rs1) + imm;
    
    std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl; 

    regFile.pc = pc_imm;
    updated_pc = true;

    if (!call_stack.empty()) {
        call_stack.pop(); // Pop the current function
    }
}