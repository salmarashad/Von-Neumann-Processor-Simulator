# Von-Neumann-Processor-Simulator

## Introduction
This project simulates a fictional computer processor named "Spicy Von Neumann Fillet with Extra Shifts". The processor integrates program instructions and data within a unified memory space. It supports three instruction formats: R-Format (arithmetic and logical operations), I-Format (operations with immediate values), and J-Format (branch and jump instructions). Instructions go through five stages: Instruction Fetch (IF), Instruction Decode (ID), Execute (EX), Memory Access (MEM), and Write Back (WB). Pipelining is implemented to maximize utilization and address data and control hazards.

## Methodology
### 1. Loading into Memory
Instructions are parsed from a text file and converted to binary using helper functions `translateRInstruction` and `translateIInstruction`.

### 2. Instruction Stages
- **Fetch (IF)**: Retrieves instructions from memory.
- **Decode (ID)**: Extracts opcode, registers, and other fields using bit masking.
- **Execute (EX)**: Performs operations based on the opcode.
- **Memory Access (MEM)**: Handles load and store operations.
- **Write Back (WB)**: Writes results back to the appropriate registers.

### 3. Pipeline Implementation
Pipelining allows instruction stages to run in parallel, improving processor utilization. The pipeline sequence is managed using an array `Stages[5]` to hold instruction stages. Structural hazards are mitigated by alternating active stages, while data hazards are addressed using stalling and forwarding mechanisms.

### 4. Data Hazards Handling
Data hazards are managed by detecting dependencies during the decode stage and implementing a stalling mechanism followed by forwarding in the execute stage. This ensures correct data usage during instruction execution.

## Results
### Discussion
Implementing pipelining reduced the number of clock cycles needed for execution significantly. For example, five instructions without pipelining required 35 clock cycles, whereas with pipelining, only 15 cycles were needed.

### Sample Program Tests
The pipeline efficiently handles various instructions, including branches and jumps, ensuring accurate results despite potential hazards. For instance, the ADD instruction waits for preceding ADDI instructions to complete, utilizing stalling and forwarding to ensure accurate execution.

```plaintext
Example Instructions:
ADDI R1 R1 75
ADDI R2 R2 63
ADD R5 R1 R2  // R5 = R1 + R2
```
## Team Members
- [Hana Mohammed Seif](https://github.com/hanaseif19)
- [Yara Ahmed](https://github.com/YaraElkousy)
- [Salma Rashad](https://github.com/salmarashad)
- [Mennah Mohamed Shaker](https://github.com/mennahmohamed)
- [Nada Ibrahim Shetewi](https://github.com/NadaShetewi)
