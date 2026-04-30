#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include "Instructions.h"

using namespace std;

bool loadProgram(const string &filename, vector<Instructions> &outProgram);

#endif