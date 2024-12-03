#ifndef PSEUDO_HH
#define PSEUDO_HH
#include "r-format.hh"

extern std::unordered_map<std::string, int> pseudo;

void init_pseudo_inst();

extern bool if_two_inst;

void pseudo_inst(std::vector<std::string> *vect);

#endif