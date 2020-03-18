#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <fstream>
#include <map>

#include "gtc/matrix_transform.hpp"

#include "vanilla.h"

typedef std::map<std::string,int> Env; //environment
typedef std::vector<std::string> DNA;



struct Machine
{
    enum operators //current operand
    {
        NONE, //if we haven't found an operator yet
        IGNORE, //skip the next operand
        SET, //set variable to value
        ADD,
        SUB,
        MUL,
        DIV,
        MOD,
        EQU, //==
        GRT, //greather than
        LRT, //less than
        GRE, // >=
        LRE, // <=
        NEQ, // !=
        IFS, //if statements
        JMP  //Jump
    };
    operators op = NONE;
    int retVal = 0; //return value
    int operand = 0;
    int branch = 1;//where to branch to
    bool crashed = false; //if an error was encountered
};

class Program
{
    Machine mach;
    Env env;
    DNA* code = nullptr;
    int index = 0;
    void resetMach(); //resets mach
public:
    Program();
    void setDNA(DNA& dna);
    void step();
    bool isFinished();
    glm::vec2 getResult();
    void reset(); //resets entire program. Doesn't change DNA
};

bool isInt(std::string line);
int getVal(std::string operand, Env& env);
int getVal(std::string operand, Env& env);
void modify (std::string line,Machine& p, Env& env);
DNA parse(std::string filePath);
glm::vec2 interpret(const DNA& dna);
glm::vec2 interpret(std::string filePath);

#endif // PARSER_H_INCLUDED
