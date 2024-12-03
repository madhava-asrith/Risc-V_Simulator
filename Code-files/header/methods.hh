#ifndef METHODS_HH
#define METHODS_HH

#include <bitset>
#include <string>
#include <sstream>
#include <cstdint>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <vector>
#include <cctype>
#include <list>
#include <unordered_set>
#include <variant>

const int NUM_REGISTERS = 32;
const uint64_t MEMORY_SIZE = 0x50000; 
const uint64_t TEXT_SECTION_START = 0x0;
const uint64_t DATA_SECTION_START = 0x10000;
const uint64_t STACK_START = 0x50000;

enum class DataType {
    DWORD = 8,
    WORD = 4,
    HALF = 2,
    BYTE = 1,
    UNKNOWN = 0
};

class RegisterFile {
public:
    RegisterFile() 
    {
        registers.resize(NUM_REGISTERS, 0);
        pc = TEXT_SECTION_START; // Initialize PC to start of text section
    }

    // Write to a register
    void write(int reg, uint64_t value) 
    {
        if (reg != 0)  // x0 is always zero
        {
            registers[reg] = value;
        }
    }

    // Read from a register
    uint64_t read(int reg) const 
    {
        return registers[reg];
    }

    // Print all registers in hex
    void print() const 
    {
        std::cout << "Registers:" << std::endl;
        for (int i = 0; i < NUM_REGISTERS; ++i) 
        {
            std::cout << "x" << i << " = 0x" 
                // << std::setw(16) << std::setfill('0') 
                << std::uppercase << std::hex << registers[i] << std::dec << std::endl;
        }
    }

    uint64_t pc; // Program Counter (PC)

private:
    std::vector<uint64_t> registers; // 64-bit registers
};

class Memory {
public:
    Memory()
    {
        memory.resize(MEMORY_SIZE, 0);
    }

    // Load a 4-byte instruction from memory
    int32_t load_instruction(uint64_t address) const 
    {
        if (address < MEMORY_SIZE && address + 3 < MEMORY_SIZE) 
        {
            uint32_t instruction = 0;   

            // Manually combine 4 bytes into a 32-bit instruction
            instruction |= static_cast<uint32_t>(memory[address]) << 0; 
            instruction |= static_cast<uint32_t>(memory[address + 1]) << 8; 
            instruction |= static_cast<uint32_t>(memory[address + 2]) << 16; 
            instruction |= static_cast<uint32_t>(memory[address + 3]) << 24;    

            return instruction;
        }

        std::cerr << "Instruction load out of bounds for address = " << std::hex << address << std::endl;
        return 0;
    }

    // Store a 64-bit value (doubleword) into memory
    void store_dw(uint64_t address, uint64_t value) 
    {
        if (address < MEMORY_SIZE && (address + 8) <= MEMORY_SIZE)
        {
            for (int i = 0; i < 8; ++i) 
            {
                memory[address + i] = (value >> (i * 8)) & 0xFF;
            }
        } else {
            std::cerr << "Memory store out of bounds\n";
        }
    }

    // Store a 32-bit value (word) into memory
    void store_w(uint64_t address, uint64_t value) 
    {
        if (address < MEMORY_SIZE && (address + 4) <= MEMORY_SIZE)
         {
            for (int i = 0; i < 4; ++i) 
            {
                memory[address + i] = (value >> (i * 8)) & 0xFF;
            }
        } else {
            std::cerr << "Memory store out of bounds\n";
        }
    }

    // Store a 16-bit value (half-word) into memory
    void store_hw(uint64_t address, uint64_t value) 
    {
        if (address < MEMORY_SIZE && (address + 2) <= MEMORY_SIZE) 
        {
            for (int i = 0; i < 2; ++i) 
            {
                memory[address + i] = (value >> (i * 8)) & 0xFF;
            }
        } else {
            std::cerr << "Memory store out of bounds\n";
        }
    }

    // Store an 8-bit value (byte) into memory
    void store_byte(uint64_t address, uint64_t value) 
    {
        if (address <= MEMORY_SIZE) 
        {
            memory[address] = value & 0xFF;  // Only need the least significant byte
        } else {
            std::cerr << "Memory store out of bounds\n";
        }
    }


    // Print count memory locations starting from address
    void print(uint64_t addr, uint64_t count) const 
    {
        if(addr >= MEMORY_SIZE && addr + count > MEMORY_SIZE)
        {
            std::cerr << "\033[31m" << "Memory access out of bounds" << "\033[37m" << std::endl;
        }

        for (uint64_t i = 0; i < count; ++i)
        {
            if(addr + count <= MEMORY_SIZE)
            {
                std::cout << "Memory[0x" << std::hex << addr + i << "] = 0x";
                std::cout 
                        // << std::setw(2) << std::setfill('0')
                        << std::uppercase << std::hex << (int) memory[addr + i ];
                std::cout << std::dec << std::endl;
            }
        }
    }

    // Load instructions from a file into the text section of memory
    void load_program(const std::string& filename) 
    {
        std::ifstream file(filename);
        if (!file) 
        {
            std::cerr << "Unable to open file.\n";
            return;
        }

        std::string inst; 
        uint8_t i = 0x0;

        while (getline(file, inst))
        {
            for (int j = 7; j > -1; j = j-2)
            {
                char in1 = inst[j];
                char in2 = inst[j - 1];

                auto c3 = std::string(1, in2) + in1;

                std::stringstream ss;
                ss << std::hex << c3;

                int temp;
                ss >> temp;

                memory[i] = static_cast<uint8_t>(temp);      
                i++;
            }
        }

        file.close();
    }

    
    // Read byte signed
    int64_t read_byte_signed(uint64_t addr) const 
    {
        int8_t byte = memory[addr];  // Read signed 8-bit value
        return static_cast<int64_t>(byte);  // Sign extend to 64-bit
    }

    // Read byte unsigned
    uint64_t read_byte_unsigned(uint64_t addr) const 
    {
        uint8_t byte = memory[addr];  // Read unsigned 8-bit value
        return static_cast<uint64_t>(byte);  
    }

    // Read half-word signed
    int64_t read_hw_signed(uint64_t addr) const 
    {
        int16_t halfword = (memory[addr]) | (memory[addr + 1] << 8);  // Read signed 16-bit value
        return static_cast<int64_t>(halfword); 
    }

    // Read half-word unsigned
    uint64_t read_hw_unsigned(uint64_t addr) const 
    {
        uint16_t halfword = (memory[addr]) | (memory[addr + 1] << 8);  // Read unsigned 16-bit value
        return static_cast<uint64_t>(halfword);  
    }

    // Read word signed
    int64_t read_w_signed(uint64_t addr) const 
    {
        int32_t word = (memory[addr]) | (memory[addr + 1] << 8)
                     | (memory[addr + 2] << 16) | (memory[addr + 3] << 24);  // Reading signed 32-bit value
        return static_cast<int64_t>(word);  // Signed extension to 64-bit
    }

    // Read word unsigned
    uint64_t read_w_unsigned(uint64_t addr) const
    {
        uint32_t word = (memory[addr]) | (memory[addr + 1] << 8)
                      | (memory[addr + 2] << 16) | (memory[addr + 3] << 24);  // Reading unsigned 32-bit value
        return static_cast<uint64_t>(word);  // Unsigned extension to 64-bit
    }

    // Read doubleword
    uint64_t read_dw(uint64_t addr) const 
    {
        uint64_t dword = (static_cast<uint64_t>(memory[addr])) 
                        | (static_cast<uint64_t>(memory[addr + 1]) << 8)
                        | (static_cast<uint64_t>(memory[addr + 2]) << 16)
                        | (static_cast<uint64_t>(memory[addr + 3]) << 24)
                        | (static_cast<uint64_t>(memory[addr + 4]) << 32)
                        | (static_cast<uint64_t>(memory[addr + 5]) << 40)
                        | (static_cast<uint64_t>(memory[addr + 6]) << 48)
                        | (static_cast<uint64_t>(memory[addr + 7]) << 56);  // Reading 64-bit value
        return dword; 
    }

private:
    std::vector<uint8_t> memory; // Byte-addressable memory
};

extern std::unordered_map<std::string, int> label;        // map to hold the labels and their line numbers
extern std::unordered_set<uint64_t> breakpoints;           // holds the breakpoint line number
extern std::stack<std::pair<std::string, int>> call_stack;         // holds the call stack: function name and line number
extern std::vector<std::string> inst_pc_list;          // holds the instructions from the file to print as is

extern RegisterFile regFile;
extern Memory memory;  
extern bool updated_pc;                // flag to decide if pc is to be incremented by 4 or not
extern int length_of_data;
extern bool cache_status;

inline bool is_breakpoint(uint64_t addr)
{
    return breakpoints.find(addr) != breakpoints.end();
}

// trim all leading whitespaces
inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim trailing whitespaces
inline std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

void show_stack();
void update_last_executed_line() ;
bool is_valid_hex(const std::string& value);
bool is_valid_dec(const std::string& value);
bool is_valid_command(const std::string& command);

std::string binaryToHex_signed(const std::string& binary);
std::string binaryToHex(const std::string& binary);
void validateFieldSize(uint64_t value, uint8_t maxBits, const std::string& fieldName);
void validateFieldSize_s(int16_t value, uint8_t maxBits, const std::string& fieldName);
void change_alias(std::string* alias);

DataType get_data_type(const std::string& str);
void process_data(const std::vector<std::string>& v, DataType data_type, uint64_t& cur_data);

#endif
