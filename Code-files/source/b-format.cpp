#include <unordered_map>
#include "b-format.hh"
#include <vector>

std::unordered_map<std::string, uint8_t> b_type;         // map to store B-format instructions

void B_type_init_map() {                     // initialize the map
    b_type["beq"] = 0x0;
    b_type["bne"] = 0x1;
    b_type["blt"] = 0x4;
    b_type["bge"] = 0x5;
    b_type["bltu"] = 0x6;
    b_type["bgeu"] = 0x7;
} 

// Function to convert to binary
std::string convertBTypeToBinary(uint8_t opcode, uint8_t func3, uint8_t rs1, uint8_t rs2, uint16_t imm) {
    validateFieldSize(opcode, 7, "Opcode");
    validateFieldSize(func3, 3, "Funct3");
    validateFieldSize(rs1, 5, "Source Register 1 (rs1)");
    validateFieldSize(rs2, 5, "Source Register 2 (rs2)");
    validateFieldSize_s(imm, 13, "Immediate value");


    std::bitset<7> opcode_bits(opcode);
    std::bitset<3> funct3_bits(func3);
    std::bitset<5> rs2_bits(rs2);
    std::bitset<5> rs1_bits(rs1);
    std::bitset<13> immediate(imm);

    std::string imm_1 = immediate.to_string().substr(0, 1);
    std::string imm_4 = immediate.to_string().substr(1, 1);
    std::string imm_2 = immediate.to_string().substr(2, 6);
    std::string imm_3 = immediate.to_string().substr(8, 4);

    std::string binaryInstruction = imm_1 + imm_2 + rs2_bits.to_string() + rs1_bits.to_string() +
        funct3_bits.to_string() + imm_3 + imm_4 + opcode_bits.to_string();

    return binaryInstruction;
}

// Function to convert instruction line to hex
std::string B_type_inst(std::vector<std::string>* vect, int cur_line) 
{
    std::vector<std::string>& v = *vect;
    v[1].pop_back();
    change_alias(&(v[1]));
    v[1] = v[1].substr(1, v[1].length());

    v[2].pop_back();
    change_alias(&v[2]);
    v[2] = v[2].substr(1, v[2].length());


    uint8_t opcode, rs1, rs2, func3;
    uint16_t imm;

    auto iter = label.find(v[3]);
    if (iter == label.end()) {
        throw std::invalid_argument("Label:" + v[3] + " missing.");
    }

    imm = (uint16_t)(4 * (iter->second - cur_line));

    opcode = 0x63;
    auto it_1 = b_type.find(v[0]);
    func3 = it_1->second;
    rs1 = (uint8_t)std::stoul(v[1]);
    rs2 = (uint8_t)std::stoul(v[2]);

    std::string binInst = convertBTypeToBinary(opcode, func3, rs1, rs2, imm);
    std::string hexInst = binaryToHex_signed(binInst);

    return hexInst;
}

void b_type_decoder(std::string bin_inst, uint32_t instruction)
{
    uint16_t rs1, func3, rs2;
    rs1 = std::stoi(bin_inst.substr(12, 5), nullptr, 2); 
    func3 = std::stoi(bin_inst.substr(17, 3), nullptr, 2);
    rs2 = std::stoi(bin_inst.substr(7,5), nullptr, 2);

    int64_t imm1, imm2, imm3, imm4;
    imm1 = (instruction >> 31) & 0x1;
    imm2 = (instruction >> 7) & 0x1;
    imm3 = (instruction >> 25) & 0x3f;
    imm4 = (instruction >> 8) & 0xf;

    int64_t imm = (imm1 << 12) | (imm2 << 11) | (imm3 << 5) | (imm4 << 1);
    imm = (imm & 0x1000) ? (imm | 0xFFFFFFFFFFFFE000) : imm;  // Sign-extend if imm[12] is 1

    update_last_executed_line();
    
    switch(func3)
    {
        case 0x0:   // beq

            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;

            if(static_cast<int64_t>(regFile.read(rs1)) == static_cast<int64_t>(regFile.read(rs2)))
            {
                regFile.pc += imm;                
                updated_pc = true;
            }
            break;

        case 0x1:   // bne  
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
            
            if(static_cast<int64_t>(regFile.read(rs1)) != static_cast<int64_t>(regFile.read(rs2)))
            {
                regFile.pc += imm;                
                updated_pc = true;
            }
            break;

        case 0x4:   // blt
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
            
            if(static_cast<int64_t>(regFile.read(rs1)) < static_cast<int64_t>(regFile.read(rs2)))
            {
                regFile.pc += imm;                
                updated_pc = true;
            }
            break;

        case 0x5:   // bge
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
            
            if(static_cast<int64_t>(regFile.read(rs1)) >= static_cast<int64_t>(regFile.read(rs2)))
            {
                regFile.pc += imm;                
                updated_pc = true;
            }
            break;

        case 0x6:   // bltu
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
            
            if(static_cast<uint64_t>(regFile.read(rs1)) < static_cast<uint64_t>(regFile.read(rs2)))
            {
                regFile.pc += imm;                
                updated_pc = true;
            }
            break;

        case 0x7:   // bgeu
            std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
            
            if(static_cast<uint64_t>(regFile.read(rs1)) >= static_cast<uint64_t>(regFile.read(rs2)))
            {
                regFile.pc += imm;                
                updated_pc = true;
            }
            break;
    }
}