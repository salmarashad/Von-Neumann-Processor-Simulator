#define MAX_TOKENS 4
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "InstData.h"
void fetchInstruction(int clockCycle);
void decode(InstData *instruction);
void execute(InstData *instruction);
void memAccess(InstData *instruction);
void writeBack(InstData *instruction);
int Registers[32] = {0}; // including register 0
int memoryCounter = 0;   // how many instructions in memory so far
int memory[2048] = {0};
int currentInstruction; // will hold the instruction we fetch as if its CIR register
int clockCycle = 1;     // keeps track of which clock cycle we're currently in
int pc = 0;
InstData *Stages[5] = {NULL, NULL, NULL, NULL, NULL};
int executionCount = 0;
int decodeCount = 0;
bool flushInstFirst = 0;
bool flushInstNext = 0;

int getRegisterNumber(const char *str)
{
    int num;
    sscanf(str, "R%d", &num);
    return num;
}
// converts a number from decimal to binary in the required number of bits
int binaryWithLeadingZeros(signed int num, int numBits)
{
    int result = 0;

    for (int i = numBits - 1; i >= 0; i--)
    {
        result |= ((num >> i) & 1) << i;
    }

    return result;
}

// same as the one above but is used for printing (displaying it)
void printBinaryWithLeadingZeros(unsigned int num, int numBits)
{
    for (int i = numBits - 1; i >= 0; i--)
    {
        putchar((num >> i) & 1 ? '1' : '0');
    }
}

// takes an R type instruction and returns its equivalent binary representation
int translateRinstruction(int opcode, char **tokens, bool shift)

{
    unsigned int instruction = 0b00000000000000000000000000000000;
    instruction |= opcode << 28;
    instruction |= getRegisterNumber(tokens[1]) << 23;
    instruction |= getRegisterNumber(tokens[2]) << 18;
    if (shift == 0)
    {
        instruction |= getRegisterNumber(tokens[3]) << 13;
    }
    else
    {
        instruction |= 0b00000 << 13;
        instruction |= atoi(tokens[3]);
    }
    return instruction;
}

// takes an I type instruction and returns its equivalent binary representation
int translateIinstruction(int opcode, char **tokens)

{
    unsigned int instruction = 0b00000000000000000000000000000000;
    instruction |= opcode << 28;
    instruction |= getRegisterNumber(tokens[1]) << 23; // R1
    instruction |= getRegisterNumber(tokens[2]) << 18; // R2
    if (atoi(tokens[3]) > 0)
    {
        instruction |= atoi(tokens[3]);
    }
    else
    {
        int immediate = binaryWithLeadingZeros(atoi(tokens[3]), 18);
        instruction |= immediate;
    }

    return instruction;
}

void displayMemory()
{
    printf("Memory Contents:\n\n");
    printf("----------------------------------------------------------\n");
    for (int i = 0; i < 2048; i++)
    {
        if (i >= 0 && i <= 1023)
            printf("Instruction at address %d: ", i);
        if (i >= 1024 && i <= 2047)
            printf("data at address %d: ", i);
        printBinaryWithLeadingZeros(memory[i], 32);
        printf("\n");
    }
    printf("----------------------------------------------------------\n");
    printf("End of Memory Display\n \n");
}

void LoadIntoMemory()
{
    FILE *fptr;
    fptr = fopen("Assembly2.txt", "r");
    char myString[100];
    while (fgets(myString, 100, fptr))
    {
        unsigned int instruction = 0b00000000000000000000000000000000;

        int numTokens;
        char *token = strtok(myString, " "); // Split line into tokens separated by space
        numTokens = 0;
        char *tokens[MAX_TOKENS]; // Array to store tokens
        while (token != NULL && numTokens < MAX_TOKENS)
        {
            tokens[numTokens++] = token;
            token = strtok(NULL, " ");
        }
        char *operationName = tokens[0];
        if ((strcmp(operationName, "ADD") == 0))
        {
            instruction = translateRinstruction(0, tokens, false);
        }
        else if ((strcmp(operationName, "SUB") == 0))
        {
            instruction = translateRinstruction(1, tokens, false);
        }
        else if ((strcmp(operationName, "SLL") == 0))
        {
            instruction = translateRinstruction(8, tokens, true);
        }
        else if ((strcmp(operationName, "SRL") == 0))
        {
            instruction = translateRinstruction(9, tokens, true);
        }
        else if (((strcmp(operationName, "MULI") == 0)))
        {
            instruction = translateIinstruction(2, tokens);
        }

        else if (((strcmp(operationName, "ADDI") == 0)))
            instruction = translateIinstruction(3, tokens);
        else if (((strcmp(operationName, "BNE") == 0)))
            instruction = translateIinstruction(4, tokens);
        else if (((strcmp(operationName, "ANDI") == 0)))
        {
            instruction = translateIinstruction(5, tokens);
        }
        else if (((strcmp(operationName, "ORI") == 0)))
            instruction = translateIinstruction(6, tokens);
        else if (((strcmp(operationName, "LW") == 0)))
            instruction = translateIinstruction(10, tokens);
        else if (((strcmp(operationName, "SW") == 0)))
        {
            instruction = translateIinstruction(11, tokens);
        }
        else if (((strcmp(operationName, "J") == 0))) // tried
        {
            instruction |= 7 << 28;
            // printf("my jump address %d \n", atoi(tokens[1]));
            instruction |= atoi(tokens[1]);
        }

        memory[memoryCounter] = instruction;
        memoryCounter++;
        // printf("hi");
    }
    // displayMemory();
    fclose(fptr);
}
void printRegs()
{
    printf("----------------------------------------------------------\n");
    printf("Register File :\n");
    for (int i = 0; i < 32; i++)
    {
        printf("R%i = %i\n", i, Registers[i]);
    }
    printf("----------------------------------------------------------\n");
}

// Fetch instruction
void fetchInstruction(int clockCycle)
{
    if (pc < memoryCounter && clockCycle % 2 == 1)
    {
        //  printf("fetching instruction at pc= %d \n", pc);
        currentInstruction = memory[pc];

        InstData *inst = (InstData *)malloc(sizeof(InstData));

        pc++;
        (*inst).pc = pc;
        Stages[0] = inst;
        // enqueue(pipeline->ifStage, inst);
        // printf("\033[0;31m"); // Set text color to red
        printf("FETCHED %d, instruction: ", (*inst).pc);
        printBinaryWithLeadingZeros(memory[pc - 1], 32);

        printf("\n ");
    }
}
void decode(InstData *instruction)
{
    (*instruction).opcode = (0b11110000000000000000000000000000 & currentInstruction) >> 28; // 0xF0000000
    (*instruction).dest = (0b00001111100000000000000000000000 & currentInstruction) >> 23;   // 0x0F800000
    (*instruction).src1 = (0b00000000011111000000000000000000 & currentInstruction) >> 18;   // 0x007C0000
    (*instruction).src2 = (0b00000000000000111110000000000000 & currentInstruction) >> 13;   // 0x0003E000
    (*instruction).imm = (0b00000000000000111111111111111111 & currentInstruction) >> 0;     // 0X0003FFFF
    (*instruction).imm = (0x00020000 & currentInstruction) == 0 ? (*instruction).imm : (0xFFFC0000 | (*instruction).imm);
    (*instruction).shammt = (0b00000000000000000001111111111111 & currentInstruction) >> 0;  // 0x00001FFF
    (*instruction).address = (0b00001111111111111111111111111111 & currentInstruction) >> 0; // 0X0FFFFFFF

    (*instruction).valuesrc1 = Registers[(*instruction).src1];
    (*instruction).valuesrc2 = Registers[(*instruction).src2];
    (*instruction).destValue = Registers[(*instruction).dest];
    // printf("\033[0;32m"); // Set text color to green
    // printf("Instruction %i\n", pc);
    // printf("inst now \n");
    // printBinaryWithLeadingZeros(currentInstruction, 32);
    // printf("opcode = %i\n", (*instruction).opcode);
    // printf("destination = %i\n", (*instruction).dest);
    // printf("source 1 = %i\n", (*instruction).src1);
    // printf("source 2 = %i\n", (*instruction).src2);
    // printf("shift amount = %i\n", (*instruction).shammt);
    // printf("immediate = %i\n", (*instruction).imm);
    // printf("address = %i\n", (*instruction).address);
    // printf("value in source 1 = %i\n", (*instruction).valuesrc1);
    // printf("value in source 2 = %i\n", (*instruction).valuesrc2);
}
void execute(InstData *instruction)
{
    (*instruction).ALUoutput = 0;

    switch ((*instruction).opcode)
    {
    case 0:

        (*instruction).ALUoutput = (*instruction).valuesrc1 + (*instruction).valuesrc2;
        // printf("ADD %d , %d to give RESULT %d", (*instruction).valuesrc1, (*instruction).valuesrc2, (*instruction).ALUoutput);

        break;
    case 1:
        (*instruction).ALUoutput = (*instruction).valuesrc1 - (*instruction).valuesrc2;
        // printf("SUBTRACTED %d from %d to give RESULT %d", (*instruction).valuesrc2, (*instruction).valuesrc1, (*instruction).ALUoutput);
        break;
    case 2:
        (*instruction).ALUoutput = (*instruction).valuesrc1 * (*instruction).imm;
        // printf("MULTIPLIED %d with immediate value %d to give RESULT %d", (*instruction).valuesrc1, (*instruction).imm, (*instruction).ALUoutput);
        break;
    case 3:
        (*instruction).ALUoutput = (*instruction).valuesrc1 + (*instruction).imm;
        // printf("ADDED %d to immediate value %d to give RESULT %d", (*instruction).valuesrc1, (*instruction).imm, (*instruction).ALUoutput);
        break;
    case 4: // branch
        (*instruction).ALUoutput = (*instruction).valuesrc1 - (*instruction).destValue;
        // printf("the pc before checking condition of branch is %d \n", ((*instruction).pc));
        // printf("the pc after checking condition of branch is %d \n", pc);
        // printf("the two values to compare are : %d , %d", (*instruction).valuesrc1, (*instruction).destValue);
        // printf("the result of subtraction is %d \n", (*instruction).ALUoutput);

        if ((*instruction).ALUoutput != 0)
        {
            flushInstFirst = 1; // flush first either ways assuming immediate is never zero

            if ((*instruction).imm == 1)
            {
                printf(" Im branching and skipping one instruction only \n");
            }
            else if ((*instruction).imm > 1)
            {
                pc = (*instruction).ALUoutput == 0 ? pc : ((*instruction).pc + (*instruction).imm);
                flushInstNext = 1; // if we're skipping 2 or more flush the second to avoid flushing it then refetching it in case of skipping 1
            }
        }
        break;
    case 5:
        (*instruction).ALUoutput = (*instruction).valuesrc1 & (*instruction).imm;
        // printf("ANDED %d with immediate value %d to give RESULT %d", (*instruction).valuesrc1, (*instruction).imm, (*instruction).ALUoutput);
        break;
    case 6:
        (*instruction).ALUoutput = (*instruction).valuesrc1 | (*instruction).imm;
        // printf("OR %d with immediate value %d to give RESULT %d", (*instruction).valuesrc1, (*instruction).imm, (*instruction).ALUoutput);
        break;
    case 7: // jump
        (*instruction).ALUoutput = (pc & 0b11110000000000000000000000000000) | (*instruction).address;
        if ((*instruction).ALUoutput - (*instruction).pc == 1)
        {
            flushInstFirst = 1;
            // printf(" im jumping one only \n");
        }
        else if ((*instruction).ALUoutput - (*instruction).pc == 2)
        {
            flushInstFirst = 1;
            // printf(" im jumping two only \n");
        }
        else if ((*instruction).ALUoutput - (*instruction).pc > 2)
        {
            flushInstFirst = 1;
            pc = (*instruction).ALUoutput;
            // printf("the new pc is %d \n", pc);
        }

        if ((*instruction).ALUoutput - (*instruction).pc > 1)
        {
            flushInstNext = 1;
        }
        break;
    case 8:
        (*instruction).ALUoutput = (*instruction).valuesrc1 << (*instruction).shammt;
        // printf("SHIFT LEFT %d with shift value %d to give RESULT %d", (*instruction).valuesrc1, (*instruction).shammt, (*instruction).ALUoutput);
        break;
    case 9:
        (*instruction).ALUoutput = (*instruction).valuesrc1 >> (*instruction).shammt;
        // printf("SHIFT RIGHT %d with shift value %d to give RESULT %d", (*instruction).valuesrc1, (*instruction).shammt, (*instruction).ALUoutput);
        break;
    case 10:
        (*instruction).ALUoutput = (*instruction).valuesrc1 + (*instruction).imm;
        // printf("ADD %d with shift value %d to give EFFECTIVE ADDRESS for load %d", (*instruction).valuesrc1, (*instruction).imm, (*instruction).ALUoutput);
        break;
    case 11:
        (*instruction).ALUoutput = (*instruction).valuesrc1 + (*instruction).imm;
        // printf("ADD %d with shift value %d to give EFFECTIVE ADDRESS for store %d", (*instruction).valuesrc1, (*instruction).imm, (*instruction).ALUoutput);
        break;
    }
    // printf("\033[0;34m"); // Set text color to blue
    // printf("Execute stage Output %d \n", (*instruction).ALUoutput);
}
void memAccess(InstData *instruction)
{
    int readAddress;
    switch ((*instruction).opcode)
    {
    case 11:
        memory[(*instruction).ALUoutput] = Registers[(*instruction).dest];
        printf("Memory access :Store %d in address %d\n", Registers[(*instruction).dest], (*instruction).ALUoutput);

        break;
    case 10:
        readAddress = (*instruction).ALUoutput; // we get the data from the memory to load to the destRegister in write back
        (*instruction).ALUoutput = memory[readAddress];
        printf("Memory access :Read %d from address %d \n", memory[readAddress], readAddress);
        break;
    default:
        printf("Memory access:No Read or Store\n");
        break;
    }
}
void writeBack(InstData *instruction)
{

    switch ((*instruction).opcode)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 5:
    case 6:
    case 8:
    case 9:
    case 10:
        if ((*instruction).dest != 0)
        {
            Registers[(*instruction).dest] = (*instruction).ALUoutput;
            printf("Write-back: Wrote result %d to register R%d.\n", (*instruction).ALUoutput, (*instruction).dest);
        }
        else
        {
            printf("Write-back: You're trying to write result %d to destination register R%d.\n", (*instruction).ALUoutput, (*instruction).dest);
        }
        break;
    case 4:
    case 7:
    case 11:
        printf("No Write-back\n");
        break;
    }
}
// Execute pipeline stages
void pipelinefunc()
{
    // Write Back
    if (!(Stages[4] == NULL))
    {
        InstData *completedInstruction = Stages[4];
        Stages[4] = NULL;
        printf("Write back %d\n", (*completedInstruction).pc);
        writeBack(completedInstruction);
        printf("Completed instruction: %d\n", (*completedInstruction).pc);
    }
    // Memory
    if (!(Stages[3] == NULL))
    {

        InstData *instruction = Stages[3];
        Stages[3] = NULL;
        Stages[4] = instruction;
        printf("MEMORY ACCESS %d\n", (*instruction).pc);
        memAccess(instruction);
    }

    // Execute
    if (!(Stages[2] == NULL))
    {
        if (executionCount < 1)
        {
            InstData *instruction = Stages[2];
            printf("EXECUTED %d\n", (*instruction).pc);
            executionCount++;
        }
        else
        {
            InstData *instruction = Stages[2];
            Stages[3] = Stages[2];
            Stages[2] = NULL;
            execute(instruction);
            printf("EXECUTED pt2 %d\n", (*(Stages[3])).pc);

            executionCount = 0;
        }
    }
    // Decode
    if (!(Stages[1] == NULL))
    {
        if (decodeCount < 1)
        {
            InstData *instruction = Stages[1];
            decode(instruction);
            printf("DECODED %d\n", (*instruction).pc);
            decodeCount++;
        }
        else
        {

            Stages[2] = Stages[1];
            Stages[1] = NULL;
            printf("DECODED pt2 %d\n", (*(Stages[2])).pc);
            decodeCount = 0;
        }
    }
    // Fetch

    if (!(Stages[0] == NULL))
    {
        InstData *instruction = Stages[0];
        Stages[1] = Stages[0];
        Stages[0] = NULL;
    }

    if (flushInstFirst)
    {

        if (!(Stages[2] == NULL))
        {
            printf("Im flushing first instruction below me %d \n", (*(Stages[2])).pc);
            printf("\033[0m");
            Stages[2] = NULL;
            flushInstFirst = 0;
        }
        // decodeCount = 0;
    }
    if (flushInstNext)
    {

        if (!(Stages[1] == NULL))
        {
            printf("Im flushing second instruction below me %d \n", (*(Stages[1])).pc);
            printf("\033[0m");
            Stages[1] = NULL;
            flushInstNext = 0;
        }
        // decodeCount = 0;
    }
}

void StartComputer()
{

    LoadIntoMemory(); // get all the instructions into memory
    printf(" I HAVE IN THE MEMORY A TOTAL OF %d ELEMENTS \n", memoryCounter);

    while (1)
    {
        printf("\033[1;36m");
        printf("Clock Cycle %d:\n", clockCycle);
        printf("\033[0m");

        // we got the instuction into variable instruction

        fetchInstruction(clockCycle);
        pipelinefunc();

        if (Stages[0] == NULL && Stages[1] == NULL && Stages[2] == NULL && Stages[3] == NULL && Stages[4] == NULL)
        {
            break;
        }
        // printRegs();
        clockCycle++;
        printf("----------------------------------------------------------\n");
    }
}

int main()
{
    StartComputer();
    displayMemory();
    printRegs();
    printf("The value of pc at the end of the program is %d\n", pc);
}