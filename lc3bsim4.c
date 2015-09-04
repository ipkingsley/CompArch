/*
	Name: Anurag Banerjee
	UTEID: ab39826

*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

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
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX,
    SR1MUX,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
/* MODIFY: you have to add all your new control signals */
	GATE_PSR,
	GATE_SP,
	GATE_INTEXC,
	LD_PSR,
	LD_SSP,
	LD_USP,
	SEL_R6,
	SPMUX1,SPMUX0,
	COND2,
	GATE_USP,
	GATE_PTBR,
	GATE_VA,
	LD_VA,
	LD_MARSTATE,
	VAB,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND2] <<2)+(x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]);}
int GetR_W(int *x)           { return(x[R_W]);}
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]);} 
int GetLSHF1(int *x)         { return(x[LSHF1]);}
/* MODIFY: you can add more Get functions for your new control signals */
int GetGATE_PSR(int *x) { return(x[GATE_PSR]);}
int	GetGATE_SP(int *x) { return (x[GATE_SP]);}
int	GetGATE_INTEXC(int *x) { return(x[GATE_INTEXC]);}
int	GetLD_PSR(int *x) { return (x[LD_PSR]);}
int	GetLD_SSP(int *x) { return (x[LD_SSP]);}
int	GetLD_USP(int *x) { return (x[LD_USP]);}
int	GetSEL_R6(int *x) { return (x[SEL_R6]);}
int	GetSPMUX(int *x) { return ((x[SPMUX1] << 1) + x[SPMUX0]); }
int	GetGATE_USP(int *x) { return(x[GATE_USP]);}



/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 

/* For lab 4 */
int INTV; /* Interrupt vector register */
int EXCV; /* Exception vector register */
int SSP; /* Initial value of system stack pointer */
/* MODIFY: You may add system latches that are required by your implementation */
int USP;
int INT;
int E;
int PSR;
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
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
/* Purpose   : Simulate the LC-3b until HALTed.                 */
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
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
	/*Changed*/
    printf("STATE_NUMBER integer : %u\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
	/*Debug code yo for psr*/
	printf("PSR			 : 0x%0.4x\n",CURRENT_LATCHES.PSR);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
	fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
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
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
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

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) { 
    int i;
    init_control_store(ucode_filename);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(program_filename);
	while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */

    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
	NEXT_LATCHES.PSR = 0x8000; /*Initial Value of the PSR*/
	NEXT_LATCHES.SSP = 0x3000; /*Initial value of system stack pointer*/
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 3) {
	printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argv[2], argc - 2);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated 
   with a "MODIFY:" comment.

   Do not modify the rdump and mdump functions.

   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/
#define G_MARMUX_YES 0
#define G_PC_YES 1
#define G_ALU_YES 2
#define G_SHF_YES 3
#define G_MDR_YES 4
#define G_PSR_YES 5
#define G_SP_YES 6
#define G_INTEXC_YES 7
#define G_USP_YES 8
#define EXCEPTION_STATE 41
#define EXC_ILLOP 0x04
#define INT_TIMER 0x01
#define EXC_PROT 0x02
#define EXC_UNALIGN 0x03
#define shouldInterrupt 300
int G_YES;
int ADDR2Output;
int MARMUXOutput;
int ADDR1Output;


int PCOutput;

int SR2MUXOutput;
int ALUOutput;

int SHFOutput;
int MDROutput;

int own_cycle_count;
int memContent;

int ILLOP;
int PROT;
int UNALIGN;
int interruptSignal;

int SPOutput;
int PSROutput;
int USPOutput;
int INTEXCOutput;
int SR1Output;
int testint = 0;
int debugState = 1;
void eval_micro_sequencer() {

  /* 
   * Evaluate the address of the next state according to the 
   * micro sequencer logic. Latch the next microinstruction.
  use getJ to get the next state. pass it microinstruction from current_latches
	 */

	int * instruc = CURRENT_LATCHES.MICROINSTRUCTION;
	int cond = GetCOND(instruc);	
	int jState = GetJ(instruc);
	int IRDbit = GetIRD(instruc);
	int IRreg = Low16bits(CURRENT_LATCHES.IR);
	int opCode = IRreg>>12;
		
	int BENcurrent = CURRENT_LATCHES.BEN;
	int R = CURRENT_LATCHES.READY;
	int IR11 = ((IRreg)&(0x0800))>>11;
	int currentState = CURRENT_LATCHES.STATE_NUMBER;
	int i;
	

	
	/*now latch next microinstruction based on cond bits and individual control signals like ready bit*/
	/*Do nothng if neither cond bit set*/
	/*Have to watch out for ird bit being set*/
	if(CYCLE_COUNT == shouldInterrupt){
	NEXT_LATCHES.INT = 1;
	printf("Interrupt should be initiated\n");
	testint = 1;
	}
	
	if(currentState == 8){
	printf("Entering RTI instruction debug");
	}
    

	if(!(IRDbit)){	
	if((cond == 1) &&(R)){
	jState = jState +2;
		
	}
	
	if((cond == 2) && (BENcurrent)){
	jState = jState +4;
	}

	if((cond == 3) && (IR11)){
		jState = jState +1;
	}
		
}
	/*Know IRD bit is set so we have to evaluate next state based on opcode*/
	/*CHECK THERE MIGHT BE AN ERROR IN LOGIC HERE!!! NOT SURE IF I NEED TO MAKE SURE E IS NOT SET?*/
	else if(((IRDbit == 1))){
		jState = opCode;
	
		/*know if we got inside here that we are in state 32. so now need to calculate ILLEGAL OPCODE EXCEPTION IF its going to be the case for the next state*/
		if(opCode == 10 || opCode == 11){
		ILLOP = 1;
		NEXT_LATCHES.EXCV = EXC_ILLOP;
		NEXT_LATCHES.E = 1;
		printf("Illegal OpCode exception detected. ");
		}
	}
	
	
	/*This basically is important when you're in an [E] bit state. If no exception has been detected, then you carry on. else automatically go to state 41. 
	At a hardware level, this is implemented through a 3 to 1 mux instead of a 2-1 mux for the microsequencer where the 3rd input is state 41*/
	if(CURRENT_LATCHES.E == 1){
	jState = EXCEPTION_STATE;
	NEXT_LATCHES.E = 0;
	printf("Exception State to be entered next cycle\n");
	}
	
	/*Check to see whether interrupt latch has been set, and whether currentState is either 18 or 19. (lsb is irrelevant so you can right shift)*/
	if((cond == 4) &&(CURRENT_LATCHES.INT == 1) && ((currentState>>1) == 9)){
	jState = EXCEPTION_STATE;
	NEXT_LATCHES.INTV = INT_TIMER;
	printf("Interrupt detected and in state 18/19 debug jState = %u \n",jState);
	}
	
	for( i = 0; i< CONTROL_STORE_BITS; i++){
	NEXT_LATCHES.MICROINSTRUCTION[i] = CONTROL_STORE[jState][i];
	
	}	
	NEXT_LATCHES.STATE_NUMBER = jState;	
	
	
	

		/*Still need to handle interrupts*/
	/*Notes: For state 41, need to hardcode PSR[15] goes to 0. you should do this in the gate psr stage maybe?
	For selR6 signal, it should override DRMux and SR1Mux
	PCMUX gets another input for PCMUX = 3 that -2's the bus output*/
	
		

}


void cycle_memory() {
 
  /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   

*/

int * instruc = CURRENT_LATCHES.MICROINSTRUCTION;
int MEMEN = GetMIO_EN(instruc);
int rw = GetR_W(instruc);	
int dataSize = GetDATA_SIZE(instruc);
int MAR = CURRENT_LATCHES.MAR;
int MDR = CURRENT_LATCHES.MDR;	

	if(MEMEN){
	own_cycle_count++;
	own_cycle_count%=6;/*Keep own cycle count from 0 to 5 inclusive*/
	
	if(own_cycle_count == 4){
		NEXT_LATCHES.READY = 1;/*Make sure to get ready ready for next cycle*/
	}

	if(own_cycle_count == 5){
	
	/*First check whether read or write operation then check for individual byte or 2 byte operation*/

	if(!rw){
	/*Know its a read*/
	
	if(dataSize == 0){
	int lowBit = MAR&0x01;
	int byte = Low16bits(MEMORY[MAR>>1][lowBit]);
	memContent = byte;	
	
	}


	if(dataSize ==1){
	int highByte = Low16bits(MEMORY[MAR>>1][1]);
	int lowByte = Low16bits(MEMORY[MAR>>1][0]);
	
	memContent = (highByte<<8) + (lowByte);	
	
	}
	}
	if(rw){
	/*Know its a rite so write contents from MDR*/
	int lowBit = MAR&0x01;
	int lowByte = Low16bits(MDR&0x00FF);
	int highByte = Low16bits((MDR&0xFF00)>>8);

	/*For least significant byte, just do it whenever the address is even*/
	if(lowBit == 0){
	/*Get address*/
	MEMORY[MAR>>1][0] = Low16bits(lowByte);
		
	}

	if((dataSize&&(!lowBit))||((!dataSize)&&lowBit)){
		MEMORY[MAR>>1][1] = Low16bits(highByte);
	}
	}
	

	NEXT_LATCHES.READY = 0;
	}
	
}

}



void eval_bus_drivers() {

  /* 
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers 
   *             Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR.
	basically method should be able to use the different mux signals from the control store to properly source the bus tristate drivers.



   */    

	int * instruc = CURRENT_LATCHES.MICROINSTRUCTION;	
		

	/*tristate gate signals*/	
	int G_MARMUX = GetGATE_MARMUX(instruc);
	int G_PC = GetGATE_PC(instruc);
	int G_ALU = GetGATE_ALU(instruc);
	int G_SHF = GetGATE_SHF(instruc);
	int G_MDR = GetGATE_MDR(instruc);
	
	/*Exception Signals*/
	int G_SP = GetGATE_SP(instruc);
	int G_USP = GetGATE_USP(instruc);
	int G_PSR = GetGATE_PSR(instruc);
	int G_INTEXC = GetGATE_INTEXC(instruc);
	int selR6 = GetSEL_R6(instruc);
	
	
	/*Control signals for GateMarmux*/
	int MARMUX = GetMARMUX(instruc);
	int ADDR1MUX = GetADDR1MUX(instruc);
	int ADDR2MUX = GetADDR2MUX(instruc);
	int LSHFSignal = GetLSHF1(instruc);
	int SPMUX = GetSPMUX(instruc);

	if(G_MARMUX){
	/*Need to figure out the source that gets fed into marmux.*/
	
	G_YES = G_MARMUX_YES;

	if(MARMUX == 0){
		/*Put in IR 7:0 Zext and LSHF1*/
	MARMUXOutput = extractIRbits(7,0);		
	}
	

	else{
	/*Put address adder sum into it*/
	switch(ADDR2MUX){

	case 0:
	ADDR2Output = 0;
	break;

	case 1:

	ADDR2Output = sEXT(extractIRbits(5, 0), (5-0+1));
	break;

	case 2:
	ADDR2Output = sEXT(extractIRbits(8,0),(8-0+1));	
	break;

	case 3:
		
	ADDR2Output = sEXT(extractIRbits(10,0),(10-0+1));	
	break;

}
	/*Remembert to lshs1*/
	if(LSHFSignal){
	ADDR2Output = Low16bits(ADDR2Output<<1);
	}
	/*Now calculate ADDR1Output*/	
	if(ADDR1MUX == 0){
		ADDR1Output = Low16bits(CURRENT_LATCHES.PC);
	}


	if (ADDR1MUX == 1){
		/*Extract base register*/
		int baseIndex = Low16bits(extractIRbits(8,6));
		ADDR1Output = CURRENT_LATCHES.REGS[baseIndex];
	}
				
	MARMUXOutput = Low16bits(ADDR1Output+ADDR2Output);
	}
	
	}

	if(G_PC){
	G_YES = G_PC_YES;
	PCOutput = Low16bits(CURRENT_LATCHES.PC);
	}
/*Program roughly verified up until here. Also have already checked sEXT and extractIR need to worry about SR1 mUX*/

	if(G_ALU){	
	int SR2MUXbit;
	int SR1MUX = GetSR1MUX(instruc);
	int imm5;
	int SR1;
	int SR2;
	int srIndex;
	int ALUK = GetALUK(instruc);	
	G_YES = G_ALU_YES;
	
	/*Know SR1 will be one arg into ALU now need to get output of SR2MUX*/

	SR2MUXbit = extractIRbits(5,5);
/*This logica determines SR2 based on immediate bit and SR1 based on SR1MUX output*/	
	if(SR2MUXbit){
		SR2 = sEXT(extractIRbits(4,0),(4-0+1));
		
		
	}
	if(!(SR2MUXbit)){
		srIndex = Low16bits(extractIRbits(2,0));
		SR2 = CURRENT_LATCHES.REGS[srIndex];
}

	if(SR1MUX){
	srIndex = Low16bits(extractIRbits(8,6));
	SR1 = CURRENT_LATCHES.REGS[srIndex];

	
}

	if(!(SR1MUX)){
	srIndex = Low16bits(extractIRbits(11,9));
	SR1 = CURRENT_LATCHES.REGS[srIndex];
}

	if(selR6){
	SR1 = CURRENT_LATCHES.REGS[6];
	SR1Output = SR1;
	}

	/*Now just do the appropriate operation on SR1 and SR2 according to ALUK*/
	SR1 = Low16bits(SR1);
	SR2 = Low16bits(SR2);

	switch(ALUK){
	case 0:
	ALUOutput = Low16bits(SR1+SR2);
	break;

	case 1:
	ALUOutput = Low16bits(SR1&SR2);
	
	break;

	case 2:
	ALUOutput = Low16bits(SR1^SR2);

	break;

	case 3:
	ALUOutput = Low16bits(SR1);
	break;

	}
		
	
	}	

	if(G_SHF){
	int shiftCond = extractIRbits(5, 4);
	int amount4 = extractIRbits(3,0);
	int SR1;
	int srIndex; 
	
	G_YES = G_SHF_YES;
	/*Can either be lshf, rshfl or rshfa operation*/
	if(shiftCond == 0){
	
		srIndex = Low16bits(extractIRbits(8,6));
		SR1 = CURRENT_LATCHES.REGS[srIndex];
		SHFOutput = Low16bits(SR1<<amount4);
		
	}

	if(shiftCond == 1){
		
		srIndex = Low16bits(extractIRbits(8,6));
		SR1 = Low16bits(CURRENT_LATCHES.REGS[srIndex]);
		SHFOutput = Low16bits(SR1>>amount4);
		
	}

	if(shiftCond == 3){
		
		/*Keeps track of signed bit for arithmentic right shifts*/
		int signMS = 0x8000;
		int signBit;
		int i;
		srIndex = Low16bits(extractIRbits(8,6));
		SR1 = Low16bits(CURRENT_LATCHES.REGS[srIndex]);
		signBit = ((SR1&signMS)>>15);
		for(i = 0; i<amount4; i++){
			if(signBit){
			 SR1 = SR1>>1;
			SR1 += signMS;
			}
			else{
			SR1>>1;
			}	
		}
		
		SHFOutput = Low16bits(SR1);	
	}	
	
	}

	if(G_MDR){
	int dataSize = GetDATA_SIZE(instruc);	
	G_YES = G_MDR_YES;

	if(dataSize == 0){
	MDROutput = Low16bits(memContent&0x00FF);
	}
		
	if(dataSize == 1){
	MDROutput = Low16bits(memContent);
	}	
		
	}

	/*The following calculates SPOutput, it just might not be always used*/
	if(SPMUX == 0){
	SPOutput = Low16bits(CURRENT_LATCHES.SSP);
	}
	
	if(SPMUX == 1){
		SPOutput = Low16bits(CURRENT_LATCHES.SSP +2);
	}
	
	if(SPMUX == 2){
		SPOutput = Low16bits(CURRENT_LATCHES.SSP -2);
	}
	if(G_SP){
	G_YES = G_SP_YES;
	/*NEEd to set up SSP for BUS travel if gate is on*/
	
	}
	
	if(G_USP){
	/*Starndard there's just a latch and a gate basically with a load signal*/
	G_YES = G_USP_YES;
	USPOutput = Low16bits(CURRENT_LATCHES.USP);
	}
	
	if(G_PSR){
	/*Starndard there's just a latch and a gate basically with a load signal*/
	G_YES = G_PSR_YES;
	PSROutput = Low16bits(CURRENT_LATCHES.PSR);
	}
	
	if(G_INTEXC){
	G_YES = G_INTEXC_YES;
	
	/*Here we can do handler logic, basically know that since both int and exc will not be set simultaneously can just and latches with what their output would be. logical predication*/
/**/
		
	if(CURRENT_LATCHES.INT){
		INTEXCOutput = ((CURRENT_LATCHES.INTV<<1) + 0x0200);
		NEXT_LATCHES.INT = 0;
	}
	
	else{
	INTEXCOutput = ((CURRENT_LATCHES.EXCV<<1) + 0x0200);
	}
	}
	
		if(selR6){
	SR1Output =  CURRENT_LATCHES.REGS[6];
	}

		
}


void drive_bus() {

  /* 
   * Datapath routine for driving the bus from one of the 5 possible 
   * tristate drivers. 
	make sure to put only    

*/

if(G_YES == G_MARMUX_YES){
	BUS = MARMUXOutput;
}

if(G_YES == G_PC_YES){
	BUS = PCOutput;

}       

if(G_YES == G_ALU_YES){
	BUS = ALUOutput;

}

if(G_YES == G_SHF_YES){
	BUS = SHFOutput;
}

if(G_YES == G_MDR_YES){
	BUS = MDROutput;

}

if(G_YES == G_PSR_YES){
	BUS = PSROutput;
}

if(G_YES == G_SP_YES){
	BUS = SPOutput;
}


if(G_YES == G_INTEXC_YES){
	BUS = INTEXCOutput;
}


if(G_YES == G_USP_YES){
	BUS = USPOutput;
}

BUS = Low16bits(BUS);

}


void latch_datapath_values() {

  /* 
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */       

	int * instruc = CURRENT_LATCHES.MICROINSTRUCTION;
	int MEMEN = GetMIO_EN(instruc);
	int N = CURRENT_LATCHES.N;
	int Z = CURRENT_LATCHES.Z;
	int P = CURRENT_LATCHES.P;
	
	int MAR = CURRENT_LATCHES.MAR;
	/*Get all load signals from datapath*/

	int ldMAR = GetLD_MAR(instruc);
	int ldMDR = GetLD_MDR(instruc);
	int ldIR = GetLD_IR(instruc);
	int ldBEN = GetLD_BEN(instruc);
	int ldREG = GetLD_REG(instruc);
	int ldCC = GetLD_CC(instruc);
	int ldPC = GetLD_PC(instruc);
	int dataSize = GetDATA_SIZE(instruc);
	
	
	/*Exception related stuff*/
	int ldPSR = GetLD_PSR(instruc);
	int ldSSP = GetLD_SSP(instruc);
	int selR6 = GetSEL_R6(instruc);	
	int ldUSP = GetLD_USP(instruc);	
	if(ldMAR){
	NEXT_LATCHES.MAR = Low16bits(BUS);
	/*This is where we should check for unaligned and protection exceptions*/
	
	/*Can use this for both finding unaligned and protection exceptions*/
	int priviledge = Low16bits((CURRENT_LATCHES.PSR&0x8000)>>15);
	int marbit0 = NEXT_LATCHES.MAR &0x0001;
	
	/*First see whether there is an unaligned exception*/
	if((marbit0 == 1) && (dataSize == 1)){
		UNALIGN = 1;
		NEXT_LATCHES.EXCV = EXC_UNALIGN;
		NEXT_LATCHES.E = 1;
		printf("Unaligned exception detected\n Currently in state %u \n MAR is 0x%0.4x \n  ",CURRENT_LATCHES.STATE_NUMBER,NEXT_LATCHES.MAR);
	}
	
	/*Now see whether theres a protection exception
	basically need to see whether the mar is between x0000 and x2FFF and its not in supervisor mode and its not a trap instrtuction*/
	
	if(priviledge && (CYCLE_COUNT>0)){
		if((NEXT_LATCHES.MAR >= 0x0000) && (NEXT_LATCHES.MAR <= 0x2FFF)){
			if(CURRENT_LATCHES.STATE_NUMBER != 15){
				PROT = 1;
				NEXT_LATCHES.EXCV = EXC_PROT;
				NEXT_LATCHES.E = 1;
				printf("Protection Exception Detected\n");
			}
		
		
		}
	
	
	
	}
	

}

	if(ldMDR){
	if(MEMEN){
	/*Not sure if I need to bit shift 8 bit values. Actually, you don't check other places later maybe?*/
	NEXT_LATCHES.MDR = Low16bits(memContent);
}	

	if(!MEMEN){
	if(dataSize == 1){
	NEXT_LATCHES.MDR = Low16bits(BUS);
	}
	
	if(dataSize ==0){
	/*Need to find out whether we are pushing high or low byte onto MDR*/
	
	if((MAR&0x01)){
	NEXT_LATCHES.MDR = Low16bits(BUS &0xFF00);
	}
	else{
	NEXT_LATCHES.MDR = Low16bits(BUS &0x00FF);	
	}
	}	
	}


	}

	if(ldIR){
	NEXT_LATCHES.IR = Low16bits(BUS);
	}

	if(ldBEN){
	/*Have to personally calculate*/

	NEXT_LATCHES.BEN = (extractIRbits(11,11))&&(N) || (extractIRbits(10,10))&&(Z) || (extractIRbits(9,9))&&(P);
	}
	

	if(ldREG){
	int DRMUX = GetDRMUX(instruc);		
		
		if(DRMUX == 0 && (selR6 != 1)){
		int drIndex = Low16bits(extractIRbits(11,9));
		NEXT_LATCHES.REGS[drIndex] = Low16bits(BUS); 
	}

		if(DRMUX == 1 && (selR6 != 1)){
		/*Know this needs to load register 7*/

		NEXT_LATCHES.REGS[7] = Low16bits(BUS);
		}
		
		if(selR6 == 1){
		NEXT_LATCHES.REGS[6] = Low16bits(BUS);
		}
}

	if(ldCC){
	/*evaluate the sign of the bus signal*/
	if(BUS == 0){

	NEXT_LATCHES.Z = 1;
	NEXT_LATCHES.N = 0;
	NEXT_LATCHES.P = 0;
	}	
	
	else if((BUS!=0)&&((BUS&0x8000)>>15)!=1){
	NEXT_LATCHES.P = 1;
	NEXT_LATCHES.Z = 0;
	NEXT_LATCHES.N = 0;
	
	}
	else{
	NEXT_LATCHES.N = 1;
	NEXT_LATCHES.P = 0;
	NEXT_LATCHES.Z = 0;
	}

/*Update PSR as well. Note that PSR contains condition codes in register for both user and supervisor mode*/
	
	CURRENT_LATCHES.PSR = (CURRENT_LATCHES.PSR&(0xFFF8));
	CURRENT_LATCHES.PSR = CURRENT_LATCHES.PSR + (NEXT_LATCHES.N<<2) + (NEXT_LATCHES.Z<<1) + (NEXT_LATCHES.P);
	NEXT_LATCHES.PSR = CURRENT_LATCHES.PSR;	
	}
	
	if(ldPC){
	

		/*First find output of PCMUX*/

	int PCMUX = GetPCMUX(instruc);
	int ADDR1MUX = GetADDR1MUX(instruc);
	int ADDR2MUX = GetADDR2MUX(instruc);
	int LSHFSignal = GetLSHF1(instruc);
	
	if(PCMUX == 0){
	NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC+2);
	}


	if(PCMUX == 1){
	NEXT_LATCHES.PC = Low16bits(BUS);
	
	/*DEbug stuff*/
	if(NEXT_LATCHES.PC == 0x1200){
	printf("Exception routine to be entered debug");
	}
	}
	
	
	/*EXCEPTION STUFF*/
	if(PCMUX == 3){
	NEXT_LATCHES.PC = Low16bits((Low16bits(BUS) - 2));
	}
	

	if(PCMUX == 2){
		switch(ADDR2MUX){

	case 0:
	ADDR2Output = 0;
	break;

	case 1:

	ADDR2Output = sEXT(extractIRbits(5, 0), (5-0+1));
	break;

	case 2:
	ADDR2Output = sEXT(extractIRbits(8,0),(8-0+1));	
	break;

	case 3:
		
	ADDR2Output = sEXT(extractIRbits(10,0),(10-0+1));	
	break;
}
	


	/*Remembert to lshs1*/
	if(LSHFSignal){	
	ADDR2Output = Low16bits(ADDR2Output<<1);
	}
	/*Now calculate ADDR1Output*/	
	if(ADDR1MUX == 0){
		ADDR1Output = Low16bits(CURRENT_LATCHES.PC);
	}


	if (ADDR1MUX == 1){
		/*Extract base register*/
		int baseIndex = Low16bits(extractIRbits(8,6));

		ADDR1Output = CURRENT_LATCHES.REGS[baseIndex];
		
		
	}
	

	NEXT_LATCHES.PC = Low16bits(ADDR1Output+ADDR2Output);
		
	}

}

	if(ldPSR){
	NEXT_LATCHES.PSR = Low16bits(BUS);
	}
	
	if(ldSSP){
	/*Need to always calculate SPOutput from MUX SPOutput should either be SSP, SSP+2,SSP-2, or BUS. If gateSP is 1, then BUS = SPOutput, */
		int SPMUX = GetSPMUX(instruc);	
	if(SPMUX ==3){
		SPOutput = Low16bits(BUS);
	}
		
		NEXT_LATCHES.SSP = Low16bits(SPOutput);
		
		
	
	}
	
	if(ldUSP){
	NEXT_LATCHES.USP = Low16bits(SR1Output);
	}

		/*Sort of janky but after doing so need to make sure that psr15 is set to 0 for supervisor mode.*/
		if(CURRENT_LATCHES.STATE_NUMBER == 41){
		NEXT_LATCHES.PSR &= 0x7FFF;
		printf("curren state is 41 clear psr");
		}
		
}


/*takes two indexes within the IR register and isolates it and returns it.*/
int extractIRbits(int high, int low){
int i;
int bitMask = 0;
int extracted;

/*Form a mask that is as large as high-low and perform that many add 1's and shifts lefts to the initial mask*/

int IRReg = CURRENT_LATCHES.IR;

for(i = 0; i<(high-low); i++){
	bitMask+=1;
	bitMask= bitMask<<1;

}

bitMask+=1;


bitMask = bitMask<<low;

IRReg = Low16bits(CURRENT_LATCHES.IR);
extracted = IRReg&bitMask;
extracted = extracted>>low;

return extracted;
}

/*Function written in lab 2, helps to sign extend a thing given its size*/
int sEXT(int num, int size) {
    /*First find the mask*/
    int mask = 1;
    int i;
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
    
    num = num | ~mask;
    
    return num;
}

