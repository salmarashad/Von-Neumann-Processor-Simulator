#ifndef INSTDATA_H
#define INSTDATA_H

typedef struct InstData
{
    int currentInstruction;
    int opcode;
    int src1;
    int src2;
    int dest;
    int imm;
    int address;
    int shammt;
    int valuesrc1;
    int valuesrc2;
    int ALUoutput;
    int pc;
    int destValue;
} InstData;
#endif