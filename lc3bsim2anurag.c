/*
    Remove all unnecessary lines (including this one) 
    in this comment.
    REFER TO THE SUBMISSION INSTRUCTION FOR DETAILS

    Name 1: Anurag Banerjee
    Name 2: Ben Seroussi
    UTEID 1: UT EID of the first partner
    UTEID 2: UT EID of the second partner
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
  int address; /* this is a byte address */

  printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
  int k; 

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%0.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%0.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3b-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch(buffer[0]) {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
    else {
	    scanf("%d", &cycles);
	    run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
  int i;

  for (i=0; i < WORDS_IN_MEM; i++) {
    MEMORY[i][0] = 0;
    MEMORY[i][1] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
  FILE * prog;
  int ii, word, program_base;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word >> 1;
  else {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
             program_filename, ii);
	    exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii][0] = word & 0x00FF;
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

  printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
  NEXT_LATCHES = CURRENT_LATCHES;
    
  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
  FILE * dumpsim_file;

  /* Error Checking */
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/
#define Low8bits(x) ((x) & 0xFF)
#define High8bits(x) (((x) & 0x0000FF00)>>8)
void processTrap(int instruction);
void processJMP(int instruction);
void processBranch(int instruction);
void processJMP(int instruction);
void processJSR(int instruction);
void processJSR(int instruction);
void processLoad(int instruction);
void processStore(int instruction);
void processBranch(int instruction);
void processLoad(int instruction);
void processShiftLea(int instruction);
void processTrap(int instruction);
void processAAX(int instruction);
int sEXT(int, int);
void setCC(int result);

void process_instruction(){
    /*  function: process_instruction
     *
     *    Process one instruction at a time
     *       -Fetch one instruction
     *       -Decode
     *       -Execute
     *       -Update NEXT_LATCHES
     */
    
    int ourPC = CURRENT_LATCHES.PC;
    /* Check to make sure [0] corresponds to high byte of instruction */
    int instruction = (MEMORY[ourPC>>1][1]<<8) + (MEMORY[ourPC>>1][0]);
   /*instruction code works for sure*/ 
   
    NEXT_LATCHES.PC = ourPC+2;
    
    /*Now we need to decode the instruction to get the op_code*/
    
    int opCode = instruction>>12;
    switch(opCode) {
            
        case 1:
        case 5:
        case 9:
            processAAX(instruction);
            break;
            
        case 15:
            processTrap(instruction);
            break;
            
        case 13:
        case 14:
            processShiftLea(instruction);
            break;
            
        case 2:
        case 6:
            processLoad(instruction);
            break;
            
            
        case 7:
        case 3:
            processStore(instruction);
            break;
            
        case 4:
            processJSR(instruction);
            break;
            
        case 12:
            processJMP(instruction);
            break;
            
        case 0:
            processBranch(instruction);
            break;
            
        default:
            printf("Unknown OPCode %x",opCode);
            exit(1);
    }
    
    /*this is where we would set next latches*/
}

int sEXT(int num, int size) {
    /*First find the mask*/
    int mask = 1;
	int i = 1;
    for(i = 1; i < size; i += 1) {
        mask = mask << 1;
    }
    
    /*Figure out if the number needs to be sign extended*/
    if(!(num & mask)) {/*No sign bit*/
        return num;
    }
    
    /*Now sign extend the number. First find the mask to use*/
    mask = 1;
    for(i = 1; i < size; i += 1) {
        mask <<= 1;
        mask += 1;
    }
    
    num = num ^ ~mask;
    
    return num;
}

/*Process Add, And and Xor instructions.*/
void processAAX(int instruction){
    /*Compute the destination register, the source register and whether there is an immmediate or a register*/
    int dr = (instruction & 0x00000e00) >> 9;
    int sr1 = (instruction &0x000001c0) >> 6;
    int flag = instruction & 0x0000020;
    int sr2 = instruction & 0x0000001F;/*Assumption that this is an immediate. Does not have to be*/
    int answer = 0;
    int sizeOfOp2 = 5; /*Again, make the assumption that this is an immediate, change it if incorrect*/
    
    /*Get the actual operands*/
    sr1 = CURRENT_LATCHES.REGS[sr1];
    if(!flag) {
        sr2 = CURRENT_LATCHES.REGS[sr2];
        sizeOfOp2 = 16;
    }
    
    /*Sign extend the numbers as needed for a 32 bit math*/
    sr1 = sEXT(sr1, 16);
    sr2 = sEXT(sr2, sizeOfOp2);
    
    /*Now take care of the instruction*/
    if(instruction & 0x00008000) {
        /*xor*/
        answer = sr1 ^ sr2;
    }
    else if(instruction & 0x00004000) {
        /*and*/
        answer = sr1 & sr2;
    }
    else {
        /*Add*/
        answer = sr1 + sr2;
    }
    
    /*Put into DR*/
    NEXT_LATCHES.REGS[dr] = Low16bits(answer);
    setCC(answer);
    
}



void processTrap(int instruction){
    /*Get the vector. Note we do not need to left shift as the addresses are computed by right shifting */
    int vector = instruction & 0x0FF;
    
    
    /*Read from memory and set r7 equal to the pc*/
    NEXT_LATCHES.REGS[7] = NEXT_LATCHES.PC;
    int MDR = MEMORY[vector][0];
    MDR = MDR + (MEMORY[vector][1] << 8);
    NEXT_LATCHES.PC = MDR;
    
}

void processShiftLea(int instruction){
    int dr = instruction & 0x00000E00;
    int answer = 0;
    
    if(!(instruction &0x00001000)) {/*Lea*/
        int offset = (instruction & 0x000001FF) << 1;
        answer = NEXT_LATCHES.PC + offset;
        NEXT_LATCHES.REGS[dr] = Low16bits(answer);
        return;
    }
    else {/*Shift*/
        int amount = instruction & 0x0F;
        int sr = instruction & 0x000001C0;
        sr >>= 6;
        sr = CURRENT_LATCHES.REGS[sr];
        
        /*Now determine the correct shift*/
        if(instruction & 0x00000010) {
            /*Left*/
            answer = sr << amount;
        }
        else if(instruction & 0x00000020) {
            /*Right Arithmetic*/
            answer = sEXT(sr, 16);
            answer = answer >> amount;
        }
        else {
            /*Right logical*/
            answer = sr >> amount;
        }
    }
    
    answer = Low16bits(answer);
    NEXT_LATCHES.REGS[dr] = answer;
    setCC(sEXT(answer, 16));
}

/*Process load will handle ldb and ldw instructions */
void processLoad(int instruction){
  /*should parse all components of instruction*/

int opCode = (Low16bits(instruction))>>12;
int dR = ((Low16bits(instruction))>>9)&(0x07);
int baseR = ((Low16bits(instruction))>>6)&(0x07);

/*this extracts low 16 bits, masks relevant low 6 bits from instruction, and then sign extends it using the sEXT function not entirely sure on order here*/
int bOffset = sEXT((instruction)&(0x3F),6);

printf("\n opCode is %d \n dR is %d \nbaseR is %d",opCode,dR,baseR);

if(opCode == 2){
/*know this is an ldb instruction
calculate memory offset from datasheet and set condition codes*/
int memOffset = CURRENT_LATCHES.REGS[baseR] + bOffset;
NEXT_LATCHES.REGS[dR] = sEXT(MEMORY[memOffset>>1][0],16); /*Need to check to make sure this is right */
printf("\nMemOff is %x",memOffset);

setCC(NEXT_LATCHES.REGS[dR]);
return;
}

else if(opCode == 6){
/*know this is an ldw instruction
calculate memory offset from datasheet and set condition codes*/

int memOffset = CURRENT_LATCHES.REGS[baseR] + (bOffset<<1);
CURRENT_LATCHES.REGS[dR] = (MEMORY[memOffset>>1][1]) + (MEMORY[memOffset>>1][0]<<8);
setCC(CURRENT_LATCHES.REGS[dR]);
return;
}

else{
exit(1); /* there was an error somewhere */
}

    
}

void processStore(int instruction){
    /*Get the offsets and registers*/
    int sr = instruction & 0x00000E00;
    sr >>= 9;
    int baser = instruction & 0x000001C0;
    int offset = instruction & 0x0000003F;
    
    baser = CURRENT_LATCHES.REGS[baser];/*Now holds the current address without offset*/
    sr = CURRENT_LATCHES.REGS[sr];
    if(instruction & 0x00004000) {
        /*STW*/
        baser += (offset << 1);
        baser >>= 1;
        MEMORY[baser][0] = Low8bits(sr);
        MEMORY[baser][1] = High8bits(sr);
    }
    else {
        /*STB*/
        baser += offset;
        int sideOfMem = baser & 0x01;
        baser >>= 1;
        MEMORY[baser][sideOfMem] = Low8bits(sr);
    }
    
}

void processJSR(int instruction){
    /*handles both jsr and jsrr
	 use next latches PC which is already incremented by 2.
	 
	 first need to check bit 11 of the instruction to determine if jsr or jsrr*/
	 int temp = NEXT_LATCHES.PC;
	 int steer = ((Low16bits(instruction))>>11)&(0x01);
	 
	 
	 if(!steer){
	 /*know this is jsrr*/
		int baseR = ((Low16bits(instruction))>>6)&(0x07);
		CURRENT_LATCHES.PC = CURRENT_LATCHES.REGS[baseR];
	 }
	 
	 else if(steer){
	 /*know this is jsr*/
	 int pcOffset11 = sEXT((instruction&(0x07FF)),11)<<1; /*not entirely sure on order whether sEXT or low16 comes last */
	 CURRENT_LATCHES.PC = CURRENT_LATCHES.PC+2+pcOffset11;
	 
	 
	 }
	 CURRENT_LATCHES.REGS[7] = temp;
	 return;
	 
}

void processJMP(int instruction){
    /*This handles both jump and return
     We should note that return works exactly like jmp does by just filling up PC with contents of the base register*/
    
    
    int baseR = ((Low16bits(instruction))>>6)&(0x07);
    CURRENT_LATCHES.PC = CURRENT_LATCHES.REGS[baseR];
    
}

int branchEnable(int instruction) {
    int n = instruction >> 11;
    int z = (instruction >> 10) & ~0x2;
    int p = (instruction >> 9) & ~0x6;
    
    return (CURRENT_LATCHES.N && n) || (CURRENT_LATCHES.Z && z) || (CURRENT_LATCHES.P && p);
    
}


void processBranch(int instruction){
/*First calculate BEN, then update PC accordingly. Condition codes not set for this instruction*/
int ben = branchEnable(instruction);
if(!ben){
return;
}
else if(ben){
	int pcOffset9 = sEXT((instruction&(0x01FF)),9)<<1; /*not entirely sure on order whether sEXT or low16 comes last */
	int updatedPC = CURRENT_LATCHES.PC+2+pcOffset9;
	NEXT_LATCHES.PC = updatedPC;
	return;

}
return;    
}

void setCC(int result){
	int n = 0;
    int z = 0;
    int p = 0;
    
    /*Figure out what to set*/
    if(result < 0) {
        n = 1;
    } else if( result == 0) {
        z = 1;
    } else {
        p = 1;
    }
    
    /*Set the condition codes*/
    NEXT_LATCHES.N = n;
    NEXT_LATCHES.Z = z;
    NEXT_LATCHES.P = p;
    
    
}
