#include "methods.hh"

RegisterFile regFile;
Memory memory;
bool updated_pc = false;
std::unordered_map<std::string, int> label;
std::unordered_set<uint64_t> breakpoints;
std::stack<std::pair<std::string, int>> call_stack;
std::vector<std::string> inst_pc_list;

int length_of_data = 0;
bool cache_status = false;

void show_stack()
{
    std::stack<std::pair<std::string, int>> temp_stack = call_stack;
    std::stack<std::pair<std::string, int>> show_stack;

    if(temp_stack.empty())
    {
        std::cout << "Empty Call Stack: Execution complete" << std::endl;
    }
    else
    {
        std::cout << "Call Stack:" << std::endl;
    }

    while(!temp_stack.empty())                  // reversing the stack to print it downwards
    {
        auto [func,line] = temp_stack.top();
        show_stack.push({func,line});
        temp_stack.pop();
    }
 
    while(!show_stack.empty())
    {
        auto [func,line] = show_stack.top();
        std::cout << " " << func << ":" << line << std::endl;
        show_stack.pop();   
    }
    std::cout << std::endl;
}

void update_last_executed_line() 
{
    if (!call_stack.empty()) 
    {
        auto [function_name, _] = call_stack.top(); // Get the function name
        call_stack.pop(); // Remove current entry
        call_stack.push({function_name, regFile.pc / 4 + length_of_data + 1}); // Push updated entry with last executed line number
    }
}


bool is_valid_hex(const std::string& value) 
{
    // Check if the string starts with "0x" or "0X" and follows with valid hex digits
    if (value.size() > 2 && (value[0] == '0') && (value[1] == 'x' || value[1] == 'X')) {
        // Check if the rest of the string contains valid hex digits
        for (size_t i = 2; i < value.size(); ++i) 
        {
            if (!isxdigit(value[i])) 
            {
                return false;  // Non-hex digit found
            }
        }
        return true;  // All digits are valid hex digits
    }
    return false;  // Doesn't start with "0x" or invalid size
}

bool is_valid_dec(const std::string& value)
{
    if (value.empty()) return false; // Empty string is not valid

    size_t start = 0;

    // Check if the first character is a negative sign
    if (value[0] == '-') {
        if (value.size() == 1) // Edge case: "-" by itself is not valid
            return false;
        start = 1; // Skip the negative sign and start checking digits
    }

    // Iterate through the rest of the characters
    for (size_t i = start; i < value.size(); ++i)
    {
        if (!isdigit(value[i])) { // If a non-digit character is found, return false
            return false;
        }
    }

    return true; // If all characters are digits, return true
}

bool is_valid_command(const std::string& command) 
{
    std::istringstream iss(command);
    std::vector<std::string> tokens;
    std::string token;

    while (iss >> token) 
    {
        tokens.push_back(token);
    }

    // Check if there are any tokens
    if (tokens.empty()) 
    {
        return false; // No command entered
    }

    // Validate specific commands

    if (tokens[0] == "load") 
    {
        return tokens.size() == 2; // Valid if "load <filename>"
    }

    if (tokens[0] == "run") 
    {
        return tokens.size() == 1; // Valid if just "run"
    }

    if(tokens[0] == "step")
    {
        return tokens.size() == 1;
    }

    if(tokens[0] == "regs")
    {   
        return tokens.size() == 1;
    }

    if(tokens[0] == "mem")
    {
        if(tokens.size() == 3 && is_valid_hex(tokens[1]) && is_valid_dec(tokens[2]))
        {
            return true;
        }
        return false;
    }

    if(tokens[0] == "break")
    {
        if(tokens.size() == 2 && is_valid_dec(tokens[1]))
        {
            return true;
        }
        return false;
    }

    if (tokens[0] == "del") {
        // Check for the valid "del break <line>"
        if (tokens.size() == 3 && tokens[1] == "break") {
            // Check if the third token is a number
            if (!is_valid_dec(tokens[2])) {
                return false; // Invalid line number
            }
            return true; // Valid command "del break <line>"
        }
        return false; // Invalid command for "del"
    }

    if(tokens[0] == "show-stack")
    {
        if(tokens.size() == 1)
        {
            return true;
        }
        return false;
    }

    if(tokens[0] == "exit")
    {
        if(tokens.size() == 1)
        {
            return true;
        }
        return false;
    }

    if(tokens[0] == "cache_sim")
    {
        if(tokens.size() < 2 || tokens.size() > 3)
        {
            return false;
        }
        return true;
    }

    return false; // If no known command matched
}

// Function to convert binary to hexadecimal (signed)
std::string binaryToHex_signed(const std::string& binary) 
{
    try {
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::stoll(binary, nullptr, 2);
        return ss.str();
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error converting binary to hexadecimal: " + std::string(e.what()));
    }
}

// Function to convert binary to hexadecimal
std::string binaryToHex(const std::string& binary)
{
    try {
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::stoull(binary, nullptr, 2);
        return ss.str();
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error converting binary to hexadecimal: " + std::string(e.what()));
    }
}

// Function to validate whether the input value can be store in the slots
void validateFieldSize(uint64_t value, uint8_t maxBits, const std::string& fieldName)
{
    if (value >= (1ULL << maxBits)) {
        throw std::invalid_argument(fieldName + " value exceeds " + std::to_string(maxBits) + "-bit size.");
    }
}

// signed version of validate field size
void validateFieldSize_s(int16_t value, uint8_t maxBits, const std::string& fieldName)
{
    if (value >= 0 && value >= (1ULL << maxBits)) {
        throw std::invalid_argument(fieldName + " value exceeds " + std::to_string(maxBits) + "-bit size.");
    }
    else if (value < 0 && value <= -pow(2, maxBits)) {
        throw std::invalid_argument(fieldName + " value exceeds " + std::to_string(maxBits) + "-bit size.");
    }
}

// Function to change aliases back to register names
void change_alias(std::string* alias)
{
    if ((*alias == "zero") || (*alias == "x0")) {
        *alias = "x0";
    }
    else if (*alias == "ra" || *alias == "x1") {
        *alias = "x1";
    }
    else if (*alias == "sp" || *alias == "x2") {
        *alias = "x2";
    }
    else if (*alias == "gp" || *alias == "x3") {
        *alias = "x3";
    }
    else if (*alias == "tp" || *alias == "x4") {
        *alias = "x4";
    }
    else if (*alias == "t0" || *alias == "x5") {
        *alias = "x5";
    }
    else if (*alias == "t1" || *alias == "x6") {
        *alias = "x6";
    }
    else if (*alias == "t2" || *alias == "x7") {
        *alias = "x7";
    }
    else if (*alias == "s0" || *alias == "fp" || *alias == "x8") {
        *alias = "x8";
    }
    else if (*alias == "s1" || *alias == "x9") {
        *alias = "x9";
    }
    else if (*alias == "a0" || *alias == "x10") {
        *alias = "x10";
    }
    else if (*alias == "a1" || *alias == "x11") {
        *alias = "x11";
    }
    else if (*alias == "a2" || *alias == "x12") {
        *alias = "x12";
    }
    else if (*alias == "a3" || *alias == "x13") {
        *alias = "x13";
    }
    else if (*alias == "a4" || *alias == "x14") {
        *alias = "x14";
    }
    else if (*alias == "a5" || *alias == "x15") {
        *alias = "x15";
    }
    else if (*alias == "a6" || *alias == "x16") {
        *alias = "x16";
    }
    else if (*alias == "a7" || *alias == "x17") {
        *alias = "x17";
    }
    else if (*alias == "s2" || *alias == "x18") {
        *alias = "x18";
    }
    else if (*alias == "s3" || *alias == "x19") {
        *alias = "x19";
    }
    else if (*alias == "s4" || *alias == "x20") {
        *alias = "x20";
    }
    else if (*alias == "s5" || *alias == "x21") {
        *alias = "x21";
    }
    else if (*alias == "s6" || *alias == "x22") {
        *alias = "x22";
    }
    else if (*alias == "s7" || *alias == "x23") {
        *alias = "x23";
    }
    else if (*alias == "s8" || *alias == "x24") {
        *alias = "x24";
    }
    else if (*alias == "s9" || *alias == "x25") {
        *alias = "x25";
    }
    else if (*alias == "s10" || *alias == "x26") {
        *alias = "x26";
    }
    else if (*alias == "s11" || *alias == "x27") {
        *alias = "x27";
    }
    else if (*alias == "t3" || *alias == "x28") {
        *alias = "x28";
    }
    else if (*alias == "t4" || *alias == "x29") {
        *alias = "x29";
    }
    else if (*alias == "t5" || *alias == "x30") {
        *alias = "x30";
    }
    else if (*alias == "t6" || *alias == "x31") {
        *alias = "x31";
    }
    else {
        throw std::invalid_argument("Invalid register name: " + *alias);
    }
}

DataType get_data_type(const std::string& str) {
    if (str == ".dword") return DataType::DWORD;
    if (str == ".word") return DataType::WORD;
    if (str == ".half") return DataType::HALF;
    if (str == ".byte") return DataType::BYTE;
    return DataType::UNKNOWN;
}

void process_data(const std::vector<std::string>& v, DataType data_type, uint64_t& cur_data) 
{
    if (v.size() <= 0) 
    {
        return; // No valid data to process
    }

    for (int i = 0; i < v.size(); i++) 
    {
        std::string value = v[i];
        if (value.back() == ',') 
        {
            value.pop_back();
        }

        uint64_t temp;
        try {
            if (is_valid_dec(value)) 
            {
                temp = (uint64_t)std::stoull(value);
            } 
            else if (is_valid_hex(value)) 
            {
                temp = (uint64_t)std::stoull(value, nullptr, 16);
            } 
            else 
            {
                std::cerr << "Invalid input: " << value << std::endl;
                continue;
            }

            // Store the data using the appropriate size
            std::bitset<64> dword(static_cast<unsigned long long>(temp));
            memory.store_dw(cur_data, dword.to_ullong());
            cur_data += static_cast<int>(data_type); // Increment by the size of the data type
            
        } catch (const std::invalid_argument& e) {
            std::cerr << "Input Error: stoi failed for '" << value << "'" << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Out of range error for value: '" << value << "'" << std::endl;
        }
    }
}

