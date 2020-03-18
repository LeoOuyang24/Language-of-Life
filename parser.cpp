#include <algorithm>

#include "parser.h"

Program::Program()
{

}

void Program::setDNA(DNA& dna)
{
    code = &dna;
}

void Program::step()
{
    if (code)
    {
        modify(code->at(index),mach,env);
        index += mach.branch;
        //std::cout << index << std::endl;
        resetMach();
    }
    else
    {
        std::cout << "No DNA!" << std::endl;
    }
}

bool Program::isFinished()
{
    if (code)
    {
       return index >= code->size();
    }
}

glm::vec2 Program::getResult()
{
    return {env["("], env[")"]};
}

void Program::resetMach()
{
    mach.op = Machine::NONE;
    mach.retVal = 0; //return value
    mach.operand = 0;
    mach.branch = 1;//where to branch to
    mach.crashed = false; //if an error was encountered
}

void Program::reset()
{
    resetMach();
    //env.clear();
    env["("] = 0;
    env[")"] = 0;
    index = 0;
}

bool isInt(std::string line)
{
    int size = line.size();
    for (int i = 0; i < size; ++i)
    {
        int diff = line[i] - '0';
        if (!(diff >= 0 && diff <= 9))
        {
            return false;
        }
    }
    return true;
}

int getVal(std::string operand, Env& env)
{
    if (isInt(operand))
    {
        return convert(operand);
    }
    else
    {
        if (env.count(operand) > 0)
        {
            return env[operand];
        }
        else
        {
            return 0;
        }
    }
}

void modify (std::string line,Machine& p, Env& env)
{
    std::string eat = "";
    for (int i = line.size() - 1; i >= 0; i--)
    {
        if (line[i] != ' ')
        {
            eat += line[i];
        }
        if (i == 0 || (line[i] == ' ' && eat != ""))
        {
            if (eat.size() > 1)
            {
                std::reverse(eat.begin(), eat.end());
            }
            if (eat == "=")
            {
                p.op = Machine::SET;
            }
            else if (eat == "+")
            {
                p.op = Machine::ADD;
            }
            else if (eat == "-")
            {
                p.op = Machine::SUB;
            }
            else if (eat == "*")
            {
                p.op = Machine::MUL;
            }
            else if (eat == "/")
            {
                p.op = Machine::DIV;
            }
            else if (eat == "%")
            {
                p.op = Machine::MOD;
            }
            else if (eat == "==")
            {
                p.op = Machine::EQU;
            }
            else if (eat == "<")
            {
                p.op = Machine::GRT;
            }
            else if (eat == ">")
            {
                p.op = Machine::LRT;
            }
            else if (eat == "<=")
            {
                p.op = Machine::GRE;
            }
            else if (eat == ">=")
            {
                p.op = Machine::LRE;
            }
            else if (eat == "!")
            {
                p.op = Machine::NEQ;
            }
            else if (eat == "@")
            {
                p.op = Machine::JMP;
            }
            else if (eat == "?")
            {
                if (p.retVal == 0 )
                {
                    p.op = Machine::JMP;
                }
                else
                {
                    p.op = Machine::IGNORE;
                }
            }
            else //we've found an operand
            {
                int val = getVal(eat,env); //operand value
                switch (p.op)
                {
                    case Machine::SET:
                        if (isInt(eat))
                        {
                            p.retVal = convert(eat); //if the operand is an int, just return it
                        }
                        else
                        {
                            env[eat] = p.retVal;
                        }
                        break;
                    case Machine::IGNORE:
                        break;
                    case Machine::NONE:
                        p.retVal = val;
                        break;
                    case Machine::ADD:
                        p.retVal = p.retVal + val;
                        break;
                    case Machine::SUB:
                       // std::cout << p.retVal << " " << val << std::endl;
                        p.retVal = p.retVal - val;
                        break;
                    case Machine::MUL:
                        p.retVal = p.retVal * val;
                        break;
                    case Machine::DIV:
                        if (val == 0)
                        {
                            p.crashed = true;
                        }
                        else
                        {
                            p.retVal = p.retVal / val;
                        }
                        break;
                    case Machine::MOD:
                        if (val == 0)
                        {
                            p.crashed = true;
                        }
                        else
                        {
                            p.retVal = p.retVal % val;
                        }
                        break;
                    case Machine::EQU:
                        p.retVal = p.retVal == val;
                        break;
                    case Machine::GRT:
                        p.retVal = p.retVal > val;
                        break;
                    case Machine::LRT:
                        p.retVal = p.retVal < val;
                        break;
                    case Machine::GRE:
                        p.retVal = p.retVal >= val;
                        break;
                    case Machine::LRE:
                        p.retVal = p.retVal <= val;
                        break;
                    case Machine::NEQ:
                        p.retVal = p.retVal != val;
                        break;
                    case Machine::IFS:
                        if (p.retVal == 0)
                        {
                            if (!isInt(eat) && env.count(eat) == 0)
                            {
                                p.branch = 1;
                            }
                            else
                            {
                                p.branch = val;
                            }
                        }
                        break;
                    case Machine::JMP:
                        if (!isInt(eat) && env.count(eat) == 0)
                        {
                            p.branch = 1;
                        }
                        else
                        {
                            p.branch = val;
                        }
                        break;
                    default:
                        p.op = Machine::NONE;
                        p.retVal = val;
                        break;
                }
                p.op = Machine::NONE;
            }
            eat = "";
        }
        if (p.crashed)
        {
            break;
        }
    }
}

DNA parse(std::string filepath)
{
    std::ifstream myFile (filepath);
    std::string read;
    DNA code;
    if (myFile.is_open())
    {
        while(getline(myFile,read))
        {
            code.push_back(read);
        }
        //std::cout << env["("] << " " << env[")"] << std::endl;
        myFile.close();
        return code;
     }
     else
     {
         throw std::logic_error("Couldn't open genetic code path!");
     }
}

glm::vec2 interpret(const DNA& dna)
{
    Env env;
    env["("] = 0;
    env[")"] = 0;
    int index = 0;
    while (index < dna.size())
    {
        Machine p;
        modify(dna[index],p, env);
        index += p.branch;
    }
    return {env["("], env[")"]};
}


glm::vec2 interpret(std::string filePath)
{
    try
    {
       return interpret(parse(filePath));
    }
    catch (const std::logic_error& e)
    {
        std::cerr << e.what();
        return {0,0};
    }
}
