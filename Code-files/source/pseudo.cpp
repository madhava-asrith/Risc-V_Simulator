#include "pseudo.hh"

std::unordered_map<std::string, int> pseudo;
bool if_two_inst = false;

void init_pseudo_inst()
{
    pseudo["mv"] = 1;
    pseudo["neg"] = 2;
    pseudo["not"] = 3;
    pseudo["jr"] = 4;
    pseudo["beqz"] = 5;
    pseudo["bnez"] = 6;
    pseudo["blez"] = 7;
    pseudo["bgez"] = 8;
    pseudo["bltz"] = 9;
    pseudo["bgtz"] = 10;
    pseudo["j"] = 11;
    pseudo["nop"] = 12;
}

void pseudo_inst(std::vector<std::string> *vect)
{
    std::vector<std::string> &v = *vect;   

    switch(pseudo.find(v[0]) -> second)
    {
        case 1:
            v[1].pop_back();

            change_alias(&(v[1]));              // changes the alias names to register names
            change_alias(&v[2]);

            v[1] = v[1].substr(1, v[1].length());          // removes the x in front of register 
            v[2] = v[2].substr(1, v[2].length());

            v[0] = "addi";
            v[1] = "x" + v[1] + ",";
            v[2] = "x" + v[2] + ",";
            v.push_back("0");
            break;

        case 2:
            v[1].pop_back();

            change_alias(&(v[1]));              // changes the alias names to register names
            change_alias(&v[2]);

            v[1] = v[1].substr(1, v[1].length());          // removes the x in front of register 
            v[2] = v[2].substr(1, v[2].length());

            v[0] = "sub";
            v[1] = "x" + v[1] + ",";
            v[2] = "x" + v[2];
            
            v.push_back(v[2]);
            v[2] = "x0,";
            break;

        case 3:
            v[0] = "xori";
            v[2] += ",";
            v.push_back("-1");
            break;

        case 4:
            v[0] = "jalr";
            v.push_back("0(" + v[1] + ")");
            v[1] = "x0,";
            break;
        
        case 5:
            v[0] = "beq";
            v.push_back(v[2]);
            v[2] = "x0,";
            break;

        case 6:
            v[0] = "bne";
            v.push_back(v[2]);
            v[2] = "x0,";
            break;
            
        case 7:
            v[0] = "bge";
            v.push_back(v[2]);
            v[2] = v[1];
            v[1] = "x0,";
            break;

        case 8:
            v[0] = "bge";
            v.push_back(v[2]);
            v[2] = "x0,";
            break;

        case 9:
            v[0] = "blt";
            v.push_back(v[2]);
            v[2] = "x0,";
            break;

        case 10:
            v[0] = "blt";
            v.push_back(v[2]);
            v[2] = v[1];
            v[1] = "x0,";
            break;
        
        case 11:
            v[0] = "jal";
            v.push_back(v[1]);
            v[1] = "x0,";
            break;
        
        case 12:
            v[0] = "add";
            v.push_back("x0,");
            v.push_back("x0,");
            v.push_back("x0");
            break;

    }

}