// lab4.cpp : This file contains the 'main' function. Program execution begins and ends there.
// 

#include "../header/pseudo.hh"
#include <vector>
#include <sstream>
#include <fstream> 

void find_label(std::vector<std::string>* vect, int* cur_line);             // changed
int mem_load_hex(std::string file_name);
void decoder(std::string bin_inst, uint32_t instruction);
void load();

bool data_sec_exists = false;
bool exec_or_ld = false;

int main()
{	
	std::vector<std::string> inst;		// stores parsed line from input file

	while (1)
	{
		std::string command;				// to take line from user
        // std::cout << "\033[35m" << "simulator> " << "\033[37m";         // for extra color coding
        std::getline(std::cin,command);

        if(!is_valid_command(command))
        {
            std::cerr << "\033[31m" << "Invalid command." << "\033[37m" << std::endl << std::endl; 
            continue;
        }

        std::istringstream iss(command);
        std::vector<std::string> comm;
        std::string token;

        while (iss >> token) 
        {
            comm.push_back(token);
        }

		if (comm[0] == "load")
		{
			std::string file_name;				// stores file name for 'run' command
            file_name = comm[1];
            cache.file_name = file_name;

            load();
            
		}
		else if (comm[0] == "run")              
		{
			while (true)
			{
                if (is_breakpoint(regFile.pc))
                {
                    std::cout << "Execution stopped at breakpoint" << std::endl;
                    break;
                }

                if(call_stack.size() > 20000)
                {
                    std::cout << "Infinite loop encountered" << std::endl;
                    break;
                }

				uint32_t instruction = memory.load_instruction(regFile.pc);      
				if (instruction == 0)
                {
                    if(cache.enabled)
                    {
                        cache.stats();
                        cache.dump(cache.file_name.substr(0, cache.file_name.find(".")) + ".out");
                    }
                    exec_or_ld = false;

                    if(!call_stack.empty())
                    {
                        call_stack.pop();
                    }
                    break;
                }

                std::bitset<32> bin_inst(instruction);
                decoder(bin_inst.to_string(),instruction);

				if(!updated_pc)
                {
                    regFile.pc += 4; // Move to next instruction
                }
                else{
                    updated_pc = false;     // if pc already updated
                }

                if(inf_loop)            // if infinite loop encountered
                {
                    std::cout << "Infinite loop encountered at line " << regFile.pc/4 + 1 << ". Stopping the execution of code." << std::endl;
                    inf_loop = false;   
                    break;
                }
			}

            std::cout << std::endl;
		}
		else if (comm[0] == "step") 
		{
			uint32_t instruction = memory.load_instruction(regFile.pc);      
            if (instruction == 0)
            {
                if(!call_stack.empty())
                {
                    call_stack.pop();
                }
                if(cache.enabled && cache_status)
                {
                    cache.stats();
                    cache.dump(cache.file_name.substr(0, cache.file_name.find(".")) + ".out");
                    exec_or_ld = false;
                    cache_status = false;
                }

                std::cout << "Nothing to step" << std::endl;
            }

            std::bitset<32> bin_inst(instruction);
            //std::cout << std::hex << instruction << std::endl;
            decoder(bin_inst.to_string(),instruction);

            if(!updated_pc)
            {
                regFile.pc += 4; // Move to next instruction
            }
            else{
                updated_pc = false;
            }

            if(inf_loop)            // if infinite loop encountered
            {
                std::cout << "Infinite loop encountered at line " << regFile.pc/4 + 1 << ". Stopping the execution of code." << std::endl;
                inf_loop = false;   
            }

            std::cout << std::endl; 
		}
		else if (comm[0] == "regs")
		{
			regFile.print();
			std::cout << std::endl; 
		}
		else if (comm[0] == "mem")
		{
			uint64_t addr, count; 
            // std::cin >> std::hex >> addr;   // Input address in hex 
            // std::cin >> std::dec >> count;  // count in decimal

            std::istringstream(comm[1]) >> std::hex >> addr;
            std::istringstream(comm[2]) >> std::dec >> count;

			memory.print(addr, count); // Print memory values 
			std::cout << std::endl; 
		}
        else if (comm[0] == "break")
        {
            uint64_t addr;
            std::istringstream(comm[1]) >> std::dec >> addr;

            if(memory.load_instruction((addr - length_of_data - 1)*4))
            {
                breakpoints.insert((addr - length_of_data - 1) * 4);
                std::cout << "breakpoint set at line " << std::dec << addr << std::endl << std::endl;
            }
            else 
            {
                std::cout << "Line number " << addr << " out of bounds for the program" << std::endl << std::endl;
            }
        }
        else if (comm[0] == "del")
        {
            if (comm[1] == "break")
            {
                uint64_t line;
                std::istringstream(comm[2]) >> std::dec >> line;  // Read the line number
                breakpoints.erase((line - length_of_data - 1) * 4);

                std::cout << std::endl;
            }
        }
        else if (comm[0] == "show-stack")
        {
            show_stack();
        }
        else if(comm[0] == "cache_sim")
        {
            if(comm[1] == "enable")
            {
                if(exec_or_ld)
                {
                    std::cerr << "Program being loaded or executed. Cannot change cache config" << std::endl;
                    continue;
                }

                cache_status = true;
                cache.enable(comm[2]);
            }
            else if (comm[1] == "disable")
            {
                if(exec_or_ld)
                {
                    std::cerr << "Program being loaded or executed. Cannot change cache config" << std::endl;
                    continue;
                }
                cache_status = false;
                cache.disable();
            }
            else if(comm[1] == "status")
            {
                if(!cache.enabled)
                {
                    std::cerr << "Cache not enabled" << std::endl;
                }
                cache.status();
            }
            else if(comm[1] == "invalidate")
            {
                if(!cache.enabled)
                {
                    std::cerr << "Cache not enabled" << std::endl;
                }
                cache.invalidate();
            }
            else if(comm[1] == "dump")
            {
                if(!cache.enabled)
                {
                    std::cerr << "Cache not enabled" << std::endl;
                }
                cache.dump(comm[2]);
            }
            else if(comm[1] == "stats")
            {
                if(!cache.enabled)
                {
                    std::cerr << "Cache not enabled" << std::endl;
                }
                cache.stats();
            }
            else
            {
                std::cerr << "\033[31m" << "Invalid command." << "\033[37m" << std::endl << std::endl; 
            }

            std::cout << std::endl;
        }
		else if (comm[0] == "exit")
		{
			std::cout << "Exited the simulator" << std::endl;
			break;
		}
		else
		{
			std::cerr << "\033[31m" << "Invalid command." << "\033[37m" << std::endl << std::endl; 
		}
	}
	return 0;
}

void load()
{
    cache.invalidate(); // Invalidate the cache            
    regFile = RegisterFile(); // Reset registers 
    memory = Memory();        // Reset memory 
    label.clear();            // clear labels
    breakpoints.clear();      // clear breakpoints
    inst_pc_list.clear();       // clear instructions stored            

    call_stack = std::stack<std::pair<std::string, int>>(); // Reset call stack
    call_stack.push({"main", 0}); // Push main back onto the stack
    length_of_data = 0;

    // clears the cache log file name every time the program is loaded again
    cache.clear_output(cache.file_name);
    exec_or_ld = true;

    mem_load_hex(cache.file_name);	// load the .data and .text sections into memory
    std::cout << std::endl;
}

void find_label(std::vector<std::string>* vect, int* cur_line)       // function to find label and store it in a map
{             
	std::vector<std::string>& v = *vect;

	if (v[0].back() == ':') {
        v[0].pop_back();

		if (r_type.find(v[0]) == r_type.end() && s_type.find(v[0]) == s_type.end() && i_type.find(v[0]) == i_type.end() &&
			b_type.find(v[0]) == b_type.end() && j_type.find(v[0]) == j_type.end() && u_type.find(v[0]) == u_type.end() &&
			pseudo.find(v[0]) == pseudo.end())
		{
            if(v.size() > 1)
			{
                if (r_type.find(v[1]) != r_type.end() || i_type.find(v[1]) != i_type.end() || s_type.find(v[1]) != s_type.end() ||
				b_type.find(v[1]) != b_type.end() || j_type.find(v[1]) != j_type.end() || u_type.find(v[1]) != u_type.end() || 
				pseudo.find(v[1]) != pseudo.end())
                {
                    if (label.find(v[0]) != label.end()) {
                        std::cerr << "\033[31m" << "Label " << "\033[37m" << v[0] << "\033[31m" << " already exists at line: " << "\033[37m" << label.find(v[0])->second << std::endl;
                    }

                    label[v[0]] = *cur_line;

                    // std::cout << v[0] << " " << *cur_line << std::endl;
                }
                else
                {
                    std::cerr << "Label \"" << v[0] <<  "\" not inserted: invalid instruction after label" << std::endl;
                }
                
                if(j_type.find(v[1]) != j_type.end() && v[0] == v[3])
                {
                    std::cerr << "\033[33m" << "Warning: Infinite loop detected at line " << "\033[37m" << *cur_line << std::endl;
                }
            }
            else if(v.size() == 1)
            {
                label[v[0]] = *cur_line;                
                // std::cout << v[0] << " " << *cur_line << std::endl;
            }
		}
	}
}

int mem_load_hex(std::string file_name)
{
    int line_i = 1;             // variable to store the total number of instruction lines
    bool in_text_section = true; 

    R_type_init_map();          //initilizing the maps to contain the instructions for respective formats
    I_type_init_map();
    S_type_init_map();
    B_type_init_map();
    J_type_init_map();
    U_type_init_map();
    init_pseudo_inst();

    std::ifstream inputFile_1(file_name);           // loop 1 of file to map the labels and line numbers

    if (!inputFile_1.is_open()) {
        std::cerr << "\033[31m" << "File not open" << "\033[37m" << std::endl;
        return 1;
    }

    std::string lab;                       // a string for taking in a line from file        

    while (getline(inputFile_1, lab)) 
    {
        ltrim(lab);
        rtrim(lab);

        if(lab[0] == ';')
        {
            continue;
        }

        std::vector<std::string> v;        // for storing the seperated instruction and registers
        std::stringstream ss(lab);
        while (ss.good()) 
        {
            std::string substr;
            getline(ss, substr, ' ');
            v.push_back(substr);
        }

        if (v[0] == ".text") 
        {
            in_text_section = true;  // We are now in the `.text` section
            continue;
        }

        if (v[0] == ".data") 
        {
            in_text_section = false;  // We are now in the `.data` section
            continue;
        }

        if (in_text_section) 
        {
            if (!(v[0] == "\n")) 
            {
                if (!v.empty()) 
                {
                    if (v[0].back() == ':')  
                    {
                        find_label(&v, &line_i);  

                        if (v.size() > 1)  
                        {
                            line_i++;               // increment the line only if an instruction encountered
                        }
                    }
                    else  
                    {
                        line_i++; 
                    }
                }
            }
        }
        
    }
    inputFile_1.close();

    int curr_line = 1;                       // variable to indicate the current line
    bool mem_write = false;
    uint64_t cur_data = DATA_SECTION_START;
    std::string data_type_store;

    try {

        std::ifstream inputFile(file_name);         // loop 2 of file to go through all lines and convert them to hex

        if (!inputFile.is_open()) {
            throw std::runtime_error("File not open");
            return 1;
        }

        std::ofstream outputFile("output.hex");         // opening a file to write to 

        if (!outputFile.is_open()) {
            throw std::runtime_error("File not open");
            return 1;
        }

        std::string inst;                       // a string for taking in a line from file

        while (getline(inputFile, inst)) 
        {
            inst = ltrim(inst);             // trim leading whitespaces if any
            inst = rtrim(inst);             // trim trailing whitespace, if any

            if(inst[0] == ';')
            {
                continue;
            }

            size_t comment_pos = inst.find(';');  // Assuming ';' starts a comment
            if (comment_pos != std::string::npos) {
                if(comment_pos == 0)
                {
                    inst.clear();
                }
                else
                {
                    inst = inst.substr(0, comment_pos);  // Remove comment portion
                }
            }

            inst = ltrim(inst);             // trim leading whitespaces if any
            inst = rtrim(inst);             // trim trailing whitespace, if any

            std::vector<std::string> v;        
            std::stringstream ss(inst);
            while (ss.good()) {
                std::string substr;
                getline(ss, substr, ' ');
                v.push_back(substr);
            }

            if (v.empty()) continue;

            if (v[0] == ".data") {
                mem_write = true;
                data_sec_exists = true;
                
                // if(data_sec_exists)
                // {
                //     length_of_data++;
                // }
                // std::cout << "did the .data" << std::endl;

                length_of_data++;
                continue;
            }
            if (v[0] == ".text") {
                mem_write = false;
                length_of_data++;

                // std::cout << "did the .text" << std::endl;

                continue;
            }

            if (mem_write) {
                if(v[0] == ".dword" || v[0] == ".word" || v[0] == ".half" || v[0] == ".byte")
                {
                    data_type_store = v[0];
                    v.erase(v.begin());
                }
                if(v.size() == 0)
                {
                    length_of_data++;
                    continue;
                }

                DataType data_type = get_data_type(data_type_store);
                if (data_type != DataType::UNKNOWN) 
                {
                    process_data(v, data_type, cur_data);
                    length_of_data++;
                    // std::cout << "did the data section" << std::endl;
                } 
                else 
                {
                    std::cerr << "Unsupported data type: " << v[0] << std::endl;
                }
                continue;
            }
       
            if (!mem_write)
            {
                if(v[0] == "main:")
                {
                    length_of_data++;
                }

                std::string hexInst = "0";

                if (v[0].back() == ':') {            // removing the first substring in case it is a label
                    v.erase(v.begin());

                    size_t comment_pos = inst.find(':');        // labels end with ':'
                    if (comment_pos != std::string::npos) {
                        if(comment_pos == 0)
                        {
                            inst.clear();
                        }
                        else
                        {
                            inst = inst.erase(0, comment_pos + 2);  // Remove label portion
                            ltrim(inst);
                        }
                    }
                }

                if(v.empty())
                {
                    continue;               // if line only contains label, skip the conversion to hex part
                }

                bool in_pseudo = false;
                if (pseudo.find(v[0]) != pseudo.end())
                {
                    pseudo_inst(&v);
                    in_pseudo = true;
                    inst_pc_list.push_back(inst);
                }

                // tring to find the instruction in the different instruction maps            
                if (r_type.find(v[0]) != r_type.end()) {
                    hexInst = R_type_inst(&v);
                    if(!in_pseudo)
                    {
                        inst_pc_list.push_back(inst);
                    }
                }
                else if (i_type.find(v[0]) != i_type.end()) {
                    hexInst = I_type_inst(&v);
                    if(!in_pseudo)
                    {
                        inst_pc_list.push_back(inst);
                    }
                }
                else if (s_type.find(v[0]) != s_type.end()) {
                    hexInst = S_type_inst(&v);
                    if(!in_pseudo)
                    {
                        inst_pc_list.push_back(inst);
                    }
                }
                else if (b_type.find(v[0]) != b_type.end()) {
                    hexInst = B_type_inst(&v, curr_line);
                    if(!in_pseudo)
                    {
                        inst_pc_list.push_back(inst);
                    }
                }
                else if (j_type.find(v[0]) != j_type.end()) {
                    hexInst = J_type_inst(&v, curr_line);
                    if(!in_pseudo)
                    {
                        inst_pc_list.push_back(inst);
                    }
                }
                else if (u_type.find(v[0]) != u_type.end()) {
                    hexInst = U_type_inst(&v);
                    if(!in_pseudo)
                    {
                        inst_pc_list.push_back(inst);
                    }
                }
                else {
                    throw std::runtime_error("Invalid/Unsupported instruction: " + v[0]);
                }

                curr_line++;
                outputFile << std::setw(8) << std::hex << std::setfill('0') << hexInst << std::endl;                 // writing to the output file
            }
        }

        outputFile.close();
        inputFile.close();
        memory.load_program("output.hex");
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "\033[31m" << "Input Error: " << e.what() << " at line: " << "\033[37m" << curr_line << std::endl;
        return 1;
    }
    catch (const std::runtime_error& e) {
        std::cerr << "\033[31m" << "Runtime Error: " << e.what() << " at line: " << "\033[37m" << curr_line << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "\033[31m" << "Unknown Error: " << e.what() << " at line: " << "\033[37m" << curr_line << std::endl;
        return 1;
    }

    return 0;
}

void decoder(std::string bin_inst, uint32_t instruction)
{
    uint8_t opcode = instruction & 0x7F;
    switch (opcode)
    {
    case 0x33: 
        r_type_decoder(bin_inst, instruction);
        break;

    case 0x03:
        load_decoder(bin_inst,instruction);
        break;

    case 0x67:
        jalr_decoder(bin_inst, instruction);
        break;

    case 0x13:
        i_type_decoder(bin_inst, instruction);
        break;

    case 0x37:
    case 0x17:
        u_type_decoder(bin_inst, instruction);
        break;

    case 0x23: 
        s_type_decoder(bin_inst, instruction);
        break;
    
    case 0x63:
        b_type_decoder(bin_inst, instruction);
        break;
    
    case 0x6f:
        j_type_decoder(bin_inst, instruction);
        break;

    }
    
}