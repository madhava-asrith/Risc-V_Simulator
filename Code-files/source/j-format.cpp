#include "j-format.hh"

std::unordered_map<std::string, int> j_type;             // map to store J-format instructions
bool inf_loop = false;

void J_type_init_map() {
    j_type["jal"] = 0;
}
 
// Function to convert to binary
std::string convertJTypeToBinary(uint8_t opcode, uint8_t rd, uint32_t imm) {
    validateFieldSize(opcode, 7, "Opcode");
    validateFieldSize(rd, 5, "Destination Register (rd)");
    validateFieldSize_s(imm, 21, "Immediate value");

    std::bitset<7> opcode_bits(opcode);
    std::bitset<5> rd_bits(rd);
    std::bitset<21> immediate(imm);
 
    std::string imm_1 = immediate.to_string().substr(0, 1);
    std::string imm_4 = immediate.to_string().substr(1, 8);
    std::string imm_3 = immediate.to_string().substr(9, 1);
    std::string imm_2 = immediate.to_string().substr(10, 10);

    std::string binaryInstruction = imm_1 + imm_2 + imm_3 + imm_4 + rd_bits.to_string() + opcode_bits.to_string();

    return binaryInstruction;
}

// Function to convert instruction line to hex
std::string J_type_inst(std::vector<std::string>* vect, int cur_line) {
    std::vector<std::string>& v = *vect;
    v[1].pop_back();
    change_alias(&(v[1]));
    v[1] = v[1].substr(1, v[1].length());


    uint8_t opcode, rd;
    uint32_t imm;

    auto iter = label.find(v[2]);
    if (iter == label.end()) {
        throw std::invalid_argument("Label:" + v[2] + " missing");
    }

    imm = (uint32_t)(4 * (iter->second - cur_line));
    // std::cout << std::dec << cur_line << " " << std::dec << iter->second << " " << std::dec << imm << std::endl;

    opcode = 0x6f;
    rd = (uint8_t)std::stoul(v[1]);

    std::string binInst = convertJTypeToBinary(opcode, rd, imm);
    std::string hexInst = binaryToHex_signed(binInst);

    return hexInst;
}

void j_type_decoder(std::string bin_inst, uint32_t instruction)
{
    uint16_t rd = std::stoi(bin_inst.substr(20,5), nullptr, 2);
    int64_t imm_20, imm_10, imm_11, imm_19;

    imm_20 = (instruction >> 31) & 0x1;
    imm_10 = (instruction >> 21) & 0x3ff;
    imm_11 = (instruction >> 20) & 0x1;
    imm_19 = (instruction >> 12) & 0xff; 

    int64_t imm = (imm_20 << 20) | (imm_19 << 12) | (imm_11 << 11) | (imm_10 << 1);
    imm = (imm & 0x100000) ? (imm | 0xFFFFFFFFFFE00000) : imm;

    regFile.write(rd, regFile.pc + 4);

    uint32_t jump_target = regFile.pc + imm;
    // std::cout << jump_target << std::endl;

    if(jump_target == regFile.pc)
    {
        inf_loop = true;
    }

    // Look up the label corresponding to the target PC
    std::string jump_label = "";
    for (const auto& entry : label) 
    {
        if (entry.second == jump_target/4 + 1) 
        {
            jump_label = entry.first;
            break;
        }
    }

    // Output the decoded instruction with label or immediate
    if (!jump_label.empty()) 
    {
        // If a label is found, display it
        std::cout << "Executed: " << inst_pc_list[regFile.pc / 4]  << "; PC=0x" << std::setw(8) << std::setfill('0') << std::hex << regFile.pc << std::dec << std::endl;
    } 
    else 
    {
        // If no label is found, display the immediate value
        std::cerr << "\033[31m" << "Jump Label not found" << "\033[37m" << std::endl;
    }

    update_last_executed_line();
    call_stack.push({jump_label, regFile.pc / 4 + length_of_data + 1});

    regFile.pc += imm;
    updated_pc = true;
}