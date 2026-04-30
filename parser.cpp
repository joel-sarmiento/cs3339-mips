#include "parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <cctype>

/* helper functions */

// removes any leading/trailing whitespaces from string
static string trim(const string &s) {
    size_t start = 0;

    // move start index forward past whitespace
    while (start < s.size() && isspace((unsigned char)s[start])) {
        start++;
    }

    size_t end = s.size();
    // move end index backward past whitespace
    while (end > start && isspace((unsigned char)s[end - 1])) {
        end--;
    }

    // substring from first non-space to last non space
    return s.substr(start, end - start);
}

// convert entire string to uppercase
static string toUpper(const string &s) {
    string res = s;
    transform(res.begin(), res.end(), res.begin(), [](unsigned char c) {
        return toupper(c);
    });
    return res;
}

// convert entire string to uppercase
static string toLower(const string &s) {
    string res = s;
    transform(res.begin(), res.end(), res.begin(), [](unsigned char c) {
        return tolower(c);
    });
    return res;
}

// convert a register string into a register number 0-31
static int parseRegister(const string &tok) {
    if (tok.empty()) {
        return -1;
    }

    string t = trim(tok);

    if (t[0] != '$') {
        return -1;
    }

    // removes leading $
    t = t.substr(1);
    t = toLower(t);

    // numeric registers
    if (!t.empty() && all_of(t.begin(), t.end(), ::isdigit)) {
        int n = stoi(t);
        if (n >= 0 && n < 32) {
            return n;
        }
        return -1;
    }

    // named registers
    static const unordered_map<string, int> regMap = {
        {"zero", 0}, {"at", 1},
        {"v0", 2}, {"v1", 3},
        {"a0", 4}, {"a1", 5}, {"a2", 6}, {"a3", 7},
        {"t0", 8}, {"t1", 9}, {"t2", 10}, {"t3", 11},
        {"t4", 12}, {"t5", 13}, {"t6", 14}, {"t7", 15},
        {"s0", 16}, {"s1", 17}, {"s2", 18}, {"s3", 19},
        {"s4", 20}, {"s5", 21}, {"s6", 22}, {"s7", 23},
        {"t8", 24}, {"t9", 25},
        {"k0", 26}, {"k1", 27},
        {"gp", 28},
        {"sp", 29},
        {"fp", 30}, {"s8", 30},
        {"ra", 31}
    };

    // known named register
    auto it = regMap.find(t);
    if (it != regMap.end()) {
        return it->second;
    }
    // invalid if neither numeric or named
    return -1;
}

bool loadProgram(const string &filename, vector<Instructions> &outProgram) {
    ifstream in(filename);
    if (!in) {
        cerr << "error: cannot open input file '" << filename << "'" << endl;
        return false;
    }

    outProgram.clear();

    // holds instruction text after labels removed
    vector<string> instructionLines;

    // maps label name to instruction index (PC)
    unordered_map<string, int> labels;

    string line;
    // counts how many instructions we've seen
    int instructionIndex = 0;

    // original file line number
    int lineNumber = 0;

    // pass 1 creates a map from labels to PC index
    while (getline(in, line)) {
        lineNumber++;

        // remove comments (anything after # or ;)
        size_t pos = line.find('#');
        if (pos != string::npos) {
            line = line.substr(0, pos);
        }

        pos = line.find(';');
        if (pos != string::npos) {
            line = line.substr(0, pos);
        }

        // trim whitespace and skip empty lines
        line = trim(line);
        if (line.empty()) {
            continue;
        }

        // find labels
        pos = line.find(':');
        if (pos != string::npos) {
            string label = trim(line.substr(0, pos));
            if (!label.empty()) {
                // check for duplicate labels
                if (labels.count(label)) {
                    cerr << "Error: duplicate label '" << label << "' on line " << lineNumber << endl;
                    return false;
                }

                // map label to the current instruction index
                labels[label] = instructionIndex;
            }
            
            // remove label and look for instruction
            line = trim(line.substr(pos + 1));
        }

        // if there's still text left then it's an instruction line
        if (!line.empty()) {
            instructionLines.push_back(line);
            instructionIndex++;
        }
    }

    // pass 2 parses opcodes and uses the map
    for (int i = 0; i < (int)instructionLines.size(); i++) {
        string s = instructionLines[i];

        // replace commas with spaces to make splitting easier
        replace(s.begin(), s.end(), ',', ' ');

        stringstream ss(s);

        // read opcode
        string opStr;
        ss >> opStr;  

        if (opStr.empty()) {
            cerr << "error: missing opcode at instruction " << i << endl;
            return false;
        }

        string opUpper = toUpper(opStr);

        // initialize new instruction with default values
        Instructions ins{};
        ins.op = NOP_OP;
        ins.rs = 0;
        ins.rt = 0;
        ins.rd = 0;
        ins.imm = 0;
        ins.address = 0;

        // r type
        if (opUpper == "ADD" || opUpper == "SUB" || opUpper == "MUL" || opUpper == "AND" || opUpper == "OR") {
            string rdTok, rsTok, rtTok;
            ss >> rdTok >> rsTok >> rtTok;

            // convert register tokens to register numbers
            int rd = parseRegister(rdTok);
            int rs = parseRegister(rsTok);
            int rt = parseRegister(rtTok);


            if (rd < 0 || rs < 0 || rt < 0) {
                cerr << "error: bad register in " << opUpper << " at instruction " << i << endl;
                return false;
            }

            // map opcode string to enum Opcode
            if (opUpper == "ADD") {
                ins.op = ADD;
            }
            else if (opUpper == "SUB") {
                ins.op = SUB;
            }
            else if (opUpper == "MUL") { 
                ins.op = MUL;
            }
            else if (opUpper == "AND") {
                ins.op = AND_OP;
            }
            else if (opUpper == "OR") { 
                ins.op = OR_OP;
            }

            // save r type register fields
            ins.rd = rd;
            ins.rs = rs;
            ins.rt = rt;
        }

        // i type instruction
        else if (opUpper == "ADDI") {
            string rtTok, rsTok, immTok;
            ss >> rtTok >> rsTok >> immTok;

            // convert register tokens to register numbers
            int rt = parseRegister(rtTok);
            int rs = parseRegister(rsTok);

            if (rt < 0 || rs < 0) {
                cerr << "error: bad register in ADDI at instruction " << i << endl;
                return false;
            }
            
            // assign immediate with token
            int imm = 0;
            try {
                imm = stoi(immTok);
            } catch (...) {
                cerr << "error: bad immediate in ADDI at instruction " << i << endl;
                return false;
            }

            // save i type register fields
            ins.op = ADDI;
            ins.rt = rt;
            ins.rs = rs;
            ins.imm = imm;
        }

        // shifts
        else if (opUpper == "SLL" || opUpper == "SRL") {
            string rdTok, rtTok, shTok;
            ss >> rdTok >> rtTok >> shTok;

            // convert register tokens to register numbers
            int rd = parseRegister(rdTok);
            int rt = parseRegister(rtTok);

            if (rd < 0 || rt < 0) {
                cerr << "error: bad register in " << opUpper << " at instruction " << i << endl;
                return false;
            }

            // assign shift with token
            int sh = 0;
            try {
                sh = stoi(shTok);
            } catch (...) {
                cerr << "error: bad shift amount in " << opUpper << " at instruction " << i << endl;
                return false;
            }

            if (opUpper == "SLL") {
                ins.op = SLL;
            } else {
                ins.op = SRL;
            }

            // store shift amount in imm
            ins.rd = rd;
            ins.rt = rt;
            ins.imm = sh;  
        }

        // memory
        else if (opUpper == "LW" || opUpper == "SW") {
            string rtTok, addrTok;
            ss >> rtTok >> addrTok;

            int rt = parseRegister(rtTok);
            if (rt < 0) {
                cerr << "error: bad rt in " << opUpper << " at instruction " << i << endl;
                return false;
            }

            // addrTok should look something line "offset(rs)" or "4($t1)"
            size_t lp = addrTok.find('(');
            size_t rp = addrTok.find(')');
            if (lp == string::npos || rp == string::npos || lp >= rp) {
                cerr << "error: bad address in " << opUpper << " at instruction " << i << endl;
                return false;
            }

            string offStr = addrTok.substr(0, lp); 
            string base = addrTok.substr(lp + 1, rp - lp - 1);

            int rs = parseRegister(base);
            if (rs < 0) {
                cerr << "error: bad base register in " << opUpper << " at instruction " << i << endl;
                return false;
            }

            int offset = 0;
            try {
                offset = stoi(offStr);
            } catch (...) {
                cerr << "error: bad offset in " << opUpper << " at instruction " << i << "\n";
                return false;
            }

            if (opUpper == "LW") {
                ins.op = LW;
            } else {
                ins.op = SW;
            }
            ins.rs = rs;
            ins.rt = rt;
            ins.imm = offset;
        }

        // beq
        else if (opUpper == "BEQ") {
            string rsTok, rtTok, labelTok;
            ss >> rsTok >> rtTok >> labelTok;

            int rs = parseRegister(rsTok);
            int rt = parseRegister(rtTok);

            if (rs < 0 || rt < 0) {
                cerr << "error: bad register in BEQ at instruction " << i << endl;
                return false;
            }
            // resolve label to absolute instruction index
            if (!labels.count(labelTok)) {
                cerr << "error: unknown label '" << labelTok << "' in BEQ at instruction " << i << endl;
                return false;
            }

            ins.op = BEQ;
            ins.rs = rs;
            ins.rt = rt;
            // store target PC index
            ins.address = labels[labelTok];  
        }

        // j type
        else if (opUpper == "J") {
            string labelTok;
            ss >> labelTok;

            if (!labels.count(labelTok)) {
                cerr << "error: unknown label '" << labelTok << "' in J at instruction " << i << endl;
                return false;
            }

            ins.op = J;
            ins.address = labels[labelTok];
        }

        // NOP instruction
        else if (opUpper == "NOP") {
            ins.op = NOP_OP;
        }
        // unknown opcode
        else {
            cerr << "error: unknown opcode '" << opStr << "' at instruction " << i << endl;
            return false;
        }

        // successfully parsed one instruction, add to program
        outProgram.push_back(ins);
    }

    // all instructions parsed successfully
    return true;
}