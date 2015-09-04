/*
    Name 1: Anurag Banerjee
    UTEID 1: ab39826
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator - Lab 6                                   */
/*                                                             */
/*   EE 460N -- Spring 2013                                    */
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
void FETCH_stage();
void DE_stage();
void AGEX_stage();
void MEM_stage();
void SR_stage();
/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define TRUE  1
#define FALSE 0

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
/* control signals from the control store */
enum CS_BITS {
  SR1_NEEDED,
  SR2_NEEDED,
  DRMUX,
  
  ADDR1MUX,
  ADDR2MUX1, ADDR2MUX0, 
  LSHF1,
  ADDRESSMUX,
  SR2MUX, 
  ALUK1, ALUK0,
  ALU_RESULTMUX,

  BR_OP,
  UNCOND_OP,
  TRAP_OP,
  BR_STALL,
  
  DCACHE_EN,
  DCACHE_RW,
  DATA_SIZE,

  DR_VALUEMUX1, DR_VALUEMUX0,
  LD_REG,
  LD_CC,
  NUM_CONTROL_STORE_BITS

} CS_BITS;


enum AGEX_CS_BITS {

  AGEX_ADDR1MUX,
  AGEX_ADDR2MUX1, AGEX_ADDR2MUX0, 
  AGEX_LSHF1,
  AGEX_ADDRESSMUX,
  AGEX_SR2MUX,
  AGEX_ALUK1, AGEX_ALUK0, 
  AGEX_ALU_RESULTMUX,

  AGEX_BR_OP,
  AGEX_UNCOND_OP,
  AGEX_TRAP_OP,
  AGEX_BR_STALL,
  AGEX_DCACHE_EN,
  AGEX_DCACHE_RW,
  AGEX_DATA_SIZE,
  
  AGEX_DR_VALUEMUX1, AGEX_DR_VALUEMUX0,
  AGEX_LD_REG,
  AGEX_LD_CC,
  NUM_AGEX_CS_BITS
} AGEX_CS_BITS;

enum MEM_CS_BITS {
  MEM_BR_OP,
  MEM_UNCOND_OP,
  MEM_TRAP_OP,
  MEM_BR_STALL,
  MEM_DCACHE_EN,
  MEM_DCACHE_RW,
  MEM_DATA_SIZE,

  MEM_DR_VALUEMUX1, MEM_DR_VALUEMUX0,
  MEM_LD_REG,
  MEM_LD_CC,
  NUM_MEM_CS_BITS
} MEM_CS_BITS;

enum SR_CS_BITS {
  SR_DR_VALUEMUX1, SR_DR_VALUEMUX0,
  SR_LD_REG,
  SR_LD_CC,
  NUM_SR_CS_BITS
} SR_CS_BITS;


/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int Get_SR1_NEEDED(int *x)     { return (x[SR1_NEEDED]); }
int Get_SR2_NEEDED(int *x)     { return (x[SR2_NEEDED]); }
int Get_DRMUX(int *x)          { return (x[DRMUX]);}
int Get_DE_BR_OP(int *x)       { return (x[BR_OP]); } 
int Get_ADDR1MUX(int *x)       { return (x[AGEX_ADDR1MUX]); }
int Get_ADDR2MUX(int *x)       { return ((x[AGEX_ADDR2MUX1] << 1) + x[AGEX_ADDR2MUX0]); }
int Get_LSHF1(int *x)          { return (x[AGEX_LSHF1]); }
int Get_ADDRESSMUX(int *x)     { return (x[AGEX_ADDRESSMUX]); }
int Get_SR2MUX(int *x)          { return (x[AGEX_SR2MUX]); }
int Get_ALUK(int *x)           { return ((x[AGEX_ALUK1] << 1) + x[AGEX_ALUK0]); }
int Get_ALU_RESULTMUX(int *x)  { return (x[AGEX_ALU_RESULTMUX]); }
int Get_BR_OP(int *x)          { return (x[MEM_BR_OP]); }
int Get_UNCOND_OP(int *x)      { return (x[MEM_UNCOND_OP]); }
int Get_TRAP_OP(int *x)        { return (x[MEM_TRAP_OP]); }
int Get_DCACHE_EN(int *x)      { return (x[MEM_DCACHE_EN]); }
int Get_DCACHE_RW(int *x)      { return (x[MEM_DCACHE_RW]); }
int Get_DATA_SIZE(int *x)      { return (x[MEM_DATA_SIZE]); } 
int Get_DR_VALUEMUX1(int *x)   { return ((x[SR_DR_VALUEMUX1] << 1 ) + x[SR_DR_VALUEMUX0]); }
int Get_AGEX_LD_REG(int *x)    { return (x[AGEX_LD_REG]); }
int Get_AGEX_LD_CC(int *x)     { return (x[AGEX_LD_CC]); }
int Get_MEM_LD_REG(int *x)     { return (x[MEM_LD_REG]); }
int Get_MEM_LD_CC(int *x)      { return (x[MEM_LD_CC]); }
int Get_SR_LD_REG(int *x)      { return (x[SR_LD_REG]); }
int Get_SR_LD_CC(int *x)       { return (x[SR_LD_CC]); }
int Get_DE_BR_STALL(int *x)    { return (x[BR_STALL]); }
int Get_AGEX_BR_STALL(int *x)  { return (x[AGEX_BR_STALL]); }
int Get_MEM_BR_STALL(int *x)   { return (x[MEM_BR_STALL]); }



/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][NUM_CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/
/* The LC-3b register file.                                      */
/***************************************************************/
#define LC3b_REGS 8
int REGS[LC3b_REGS];
/***************************************************************/
/* architectural state */
/***************************************************************/
int  PC,  	/* program counter */
     N,		/* n condition bit */
     Z = 1,	/* z condition bit */
     P;		/* p condition bit */ 
/***************************************************************/
/* LC-3b State info.                                             */
/***************************************************************/

typedef struct PipeState_Entry_Struct{
  
  /* DE latches */
int DE_NPC,
    DE_IR,
    DE_V,
    /* AGEX lateches */
    AGEX_NPC,
    AGEX_SR1, 
    AGEX_SR2,
    AGEX_CC,
    AGEX_IR,
    AGEX_DRID,
    AGEX_V,
    AGEX_CS[NUM_AGEX_CS_BITS],
    /* MEM latches */
    MEM_NPC,
    MEM_ALU_RESULT,
    MEM_ADDRESS,
    MEM_CC,
    MEM_IR,
    MEM_DRID,
    MEM_V,
    MEM_CS[NUM_MEM_CS_BITS],
    /* SR latches */
    SR_NPC, 
    SR_DATA,
    SR_ALU_RESULT, 
    SR_ADDRESS,
    SR_IR,
    SR_DRID,
    SR_V,
    SR_CS[NUM_SR_CS_BITS];
    
} PipeState_Entry;

/* data structure for latch */
PipeState_Entry PS, NEW_PS;

/* simulator signal */
int RUN_BIT;

/* Internal stall signals */ 
int   dep_stall,
      v_de_br_stall,
      v_agex_br_stall,
      v_mem_br_stall,
      mem_stall,
      icache_r;

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
    printf("rdump            -  dump the architectural state    \n");
    printf("idump            -  dump the internal state         \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

void print_CS(int *CS, int num)
{
  int ii ;
  for ( ii = 0 ; ii < num; ii++) {
    printf("%d",CS[ii]);
  }
  printf("\n");
}
/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {
  NEW_PS = PS; 
  SR_stage();
  MEM_stage(); 
  AGEX_stage();
  DE_stage();
  FETCH_stage();
  PS = NEW_PS; 
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
	if (PC == 0x0000) {
	    cycle();
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
    if ((RUN_BIT == FALSE) || (PC == 0x0000)) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }
    printf("Simulating...\n\n");
    /* initialization */
    while (PC != 0x0000)
      cycle();
    cycle();
      RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a region of memory to the output file.     */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
    int address; /* this is a byte address */

    printf("\nMemory content [0x%04x..0x%04x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%04x (%d) : 0x%02x%02x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%04x..0x%04x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%04x (%d) : 0x%02x%02x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current architectural state  to the       */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
    int k; 

    printf("\nCurrent architectural state :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count : %d\n", CYCLE_COUNT);
    printf("PC          : 0x%04x\n", PC);
    printf("CCs: N = %d  Z = %d  P = %d\n", N, Z, P);
    printf("Registers:\n");
    for (k = 0; k < LC3b_REGS; k++)
	printf("%d: 0x%04x\n", k, (REGS[k] & 0xFFFF));
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent architectural state :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC          : 0x%04x\n", PC);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", N, Z, P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%04x\n", k, (REGS[k] & 0xFFFF));
    fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : idump                                           */
/*                                                             */
/* Purpose   : Dump current internal state to the              */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void idump(FILE * dumpsim_file) {
    int k; 

    printf("\nCurrent architectural state :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count     : %d\n", CYCLE_COUNT);
    printf("PC              : 0x%04x\n", PC);
    printf("CCs: N = %d  Z = %d  P = %d\n", N, Z, P);
    printf("Registers:\n");
    for (k = 0; k < LC3b_REGS; k++)
	printf("%d: 0x%04x\n", k, (REGS[k] & 0xFFFF));
    printf("\n");
    
    printf("------------- Stall Signals -------------\n");
    printf("ICACHE_R        :  %d\n", icache_r);
    printf("DEP_STALL       :  %d\n", dep_stall);
    printf("V_DE_BR_STALL   :  %d\n", v_de_br_stall);
    printf("V_AGEX_BR_STALL :  %d\n", v_agex_br_stall);
    printf("MEM_STALL       :  %d\n", mem_stall);
    printf("V_MEM_BR_STALL  :  %d\n", v_mem_br_stall);    
    printf("\n");

    printf("------------- DE   Latches --------------\n");
    printf("DE_NPC          :  0x%04x\n", PS.DE_NPC );
    printf("DE_IR           :  0x%04x\n", PS.DE_IR );
    printf("DE_V            :  %d\n", PS.DE_V);
    printf("\n");
    
    printf("------------- AGEX Latches --------------\n");
    printf("AGEX_NPC        :  0x%04x\n", PS.AGEX_NPC );
    printf("AGEX_SR1        :  0x%04x\n", PS.AGEX_SR1 );
    printf("AGEX_SR2        :  0x%04x\n", PS.AGEX_SR2 );
    printf("AGEX_CC         :  %d\n", PS.AGEX_CC );
    printf("AGEX_IR         :  0x%04x\n", PS.AGEX_IR );
    printf("AGEX_DRID       :  %d\n", PS.AGEX_DRID);
    printf("AGEX_CS         :  ");
    for ( k = 0 ; k < NUM_AGEX_CS_BITS; k++) {
      printf("%d",PS.AGEX_CS[k]);
    }
    printf("\n");
    printf("AGEX_V          :  %d\n", PS.AGEX_V);
    printf("\n");

    printf("------------- MEM  Latches --------------\n");
    printf("MEM_NPC         :  0x%04x\n", PS.MEM_NPC );
    printf("MEM_ALU_RESULT  :  0x%04x\n", PS.MEM_ALU_RESULT );
    printf("MEM_ADDRESS     :  0x%04x\n", PS.MEM_ADDRESS ); 
    printf("MEM_CC          :  %d\n", PS.MEM_CC );
    printf("MEM_IR          :  0x%04x\n", PS.MEM_IR );
    printf("MEM_DRID        :  %d\n", PS.MEM_DRID);
    printf("MEM_CS          :  ");
    for ( k = 0 ; k < NUM_MEM_CS_BITS; k++) {
      printf("%d",PS.MEM_CS[k]);
    }
    printf("\n");
    printf("MEM_V           :  %d\n", PS.MEM_V);
    printf("\n");

    printf("------------- SR   Latches --------------\n");
    printf("SR_NPC          :  0x%04x\n", PS.SR_NPC );
    printf("SR_DATA         :  0x%04x\n", PS.SR_DATA );
    printf("SR_ALU_RESULT   :  0x%04x\n", PS.SR_ALU_RESULT );
    printf("SR_ADDRESS      :  0x%04x\n", PS.SR_ADDRESS );
    printf("SR_IR           :  0x%04x\n", PS.SR_IR );
    printf("SR_DRID         :  %d\n", PS.SR_DRID);
    printf("SR_CS           :  ");
    for ( k = 0 ; k < NUM_SR_CS_BITS; k++) {
      printf("%d",PS.SR_CS[k]);
    }
    printf("\n");
    printf("SR_V            :  %d\n", PS.SR_V);
    
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file,"\nCurrent architectural state :\n");
    fprintf(dumpsim_file,"-------------------------------------\n");
    fprintf(dumpsim_file,"Cycle Count     : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file,"PC              : 0x%04x\n", PC);
    fprintf(dumpsim_file,"CCs: N = %d  Z = %d  P = %d\n", N, Z, P);
    fprintf(dumpsim_file,"Registers:\n");
    for (k = 0; k < LC3b_REGS; k++)
      fprintf(dumpsim_file,"%d: 0x%04x\n", k, (REGS[k] & 0xFFFF));
    fprintf(dumpsim_file,"\n");
    
    fprintf(dumpsim_file,"------------- Stall Signals -------------\n");
    fprintf(dumpsim_file,"ICACHE_R        :  %d\n", icache_r);
    fprintf(dumpsim_file,"DEP_STALL       :  %d\n", dep_stall);
    fprintf(dumpsim_file,"V_DE_BR_STALL   :  %d\n", v_de_br_stall);
    fprintf(dumpsim_file,"V_AGEX_BR_STALL :  %d\n", v_agex_br_stall);
    fprintf(dumpsim_file,"MEM_STALL       :  %d\n", mem_stall);
    fprintf(dumpsim_file,"V_MEM_BR_STALL  :  %d\n", v_mem_br_stall);
    fprintf(dumpsim_file,"\n");

    fprintf(dumpsim_file,"------------- DE   Latches --------------\n");
    fprintf(dumpsim_file,"DE_NPC          :  0x%04x\n", PS.DE_NPC );
    fprintf(dumpsim_file,"DE_IR           :  0x%04x\n", PS.DE_IR );
    fprintf(dumpsim_file,"DE_V            :  %d\n", PS.DE_V);
    fprintf(dumpsim_file,"\n");
    
    fprintf(dumpsim_file,"------------- AGEX Latches --------------\n");
    fprintf(dumpsim_file,"AGEX_NPC        :  0x%04x\n", PS.AGEX_NPC );
    fprintf(dumpsim_file,"AGEX_SR1        :  0x%04x\n", PS.AGEX_SR1 );
    fprintf(dumpsim_file,"AGEX_SR2        :  0x%04x\n", PS.AGEX_SR2 );
    fprintf(dumpsim_file,"AGEX_CC         :  %d\n", PS.AGEX_CC );
    fprintf(dumpsim_file,"AGEX_IR         :  0x%04x\n", PS.AGEX_IR );
    fprintf(dumpsim_file,"AGEX_DRID       :  %d\n", PS.AGEX_DRID);
    fprintf(dumpsim_file,"AGEX_CS         :  ");
    for ( k = 0 ; k < NUM_AGEX_CS_BITS; k++) {
      fprintf(dumpsim_file,"%d",PS.AGEX_CS[k]);
    }
    fprintf(dumpsim_file,"\n");
    fprintf(dumpsim_file,"AGEX_V          :  %d\n", PS.AGEX_V);
    fprintf(dumpsim_file,"\n");

    fprintf(dumpsim_file,"------------- MEM  Latches --------------\n");
    fprintf(dumpsim_file,"MEM_NPC         :  0x%04x\n", PS.MEM_NPC );
    fprintf(dumpsim_file,"MEM_ALU_RESULT  :  0x%04x\n", PS.MEM_ALU_RESULT );
    fprintf(dumpsim_file,"MEM_ADDRESS     :  0x%04x\n", PS.MEM_ADDRESS ); 
    fprintf(dumpsim_file,"MEM_CC          :  %d\n", PS.MEM_CC );
    fprintf(dumpsim_file,"MEM_IR          :  0x%04x\n", PS.MEM_IR );
    fprintf(dumpsim_file,"MEM_DRID        :  %d\n", PS.MEM_DRID);
    fprintf(dumpsim_file,"MEM_CS          :  ");
    for ( k = 0 ; k < NUM_MEM_CS_BITS; k++) {
      fprintf(dumpsim_file,"%d",PS.MEM_CS[k]);
    }
    fprintf(dumpsim_file,"\n");
    fprintf(dumpsim_file,"MEM_V           :  %d\n", PS.MEM_V);
    fprintf(dumpsim_file,"\n");

    fprintf(dumpsim_file,"------------- SR   Latches --------------\n");
    fprintf(dumpsim_file,"SR_NPC          :  0x%04x\n", PS.SR_NPC );
    fprintf(dumpsim_file,"SR_DATA         :  0x%04x\n",PS.SR_DATA );
    fprintf(dumpsim_file,"SR_ALU_RESULT   :  0x%04x\n", PS.SR_ALU_RESULT );
    fprintf(dumpsim_file,"SR_ADDRESS      :  0x%04x\n", PS.SR_ADDRESS );
    fprintf(dumpsim_file,"SR_IR           :  0x%04x\n", PS.SR_IR );
    fprintf(dumpsim_file,"SR_DRID         :  %d\n", PS.SR_DRID);
    fprintf(dumpsim_file,"SR_CS           :  ");
    for ( k = 0 ; k < NUM_SR_CS_BITS; k++) {
    fprintf(dumpsim_file, "%d",PS.SR_CS[k]);
    }
    fprintf(dumpsim_file,"\n");
    fprintf(dumpsim_file,"SR_V            :  %d\n", PS.SR_V);
    
    fprintf(dumpsim_file,"\n");
    
    
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

    case 'I':
    case 'i':
        idump(dumpsim_file);
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

	for (j = 0; j < NUM_CONTROL_STORE_BITS; j++) {
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


/***************************************************************/
/*                                                             */
/* Procedure : init_state                                      */
/*                                                             */
/* Purpose   : Zero out all latches and registers              */
/*                                                             */
/***************************************************************/
void init_state() {
  
  memset(&PS, 0 ,sizeof(PipeState_Entry)); 
  memset(&NEW_PS, 0 , sizeof(PipeState_Entry));
  
  dep_stall       = 0; 
  v_de_br_stall   = 0;
  v_agex_br_stall = 0;
  v_mem_br_stall  = 0;
  mem_stall       = 0;
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
	program_base = word >> 1 ;
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

    if (PC == 0) PC  = program_base << 1 ;
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
    init_state();
    
    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* dcache_access                                               */
/*                                                             */
/***************************************************************/
void dcache_access(int dcache_addr, int *read_word, int write_word, int *dcache_r, 
		    int mem_w0, int mem_w1) {
  
  int addr = dcache_addr >> 1 ; 
  int random = CYCLE_COUNT % 9;

  if (!random) {
    *dcache_r = 0;
    *read_word = 0xfeed ;
  }
  else {
    *dcache_r = 1;
    
    *read_word = (MEMORY[addr][1] << 8) | (MEMORY[addr][0] & 0x00FF);
    if(mem_w0) MEMORY[addr][0] = write_word & 0x00FF;
    if(mem_w1) MEMORY[addr][1] = (write_word & 0xFF00) >> 8;
  }
}
/***************************************************************/
/*                                                             */
/* icache_access                                               */
/*                                                             */
/***************************************************************/
void icache_access(int icache_addr, int *read_word, int *icache_r) {
	
  int addr = icache_addr >> 1 ; 
  int random = CYCLE_COUNT % 13;

  if (!random) {
    *icache_r = 0;
    *read_word = 0xfeed;
  }
  else {
    *icache_r = 1;
    *read_word = MEMORY[addr][1] << 8 | MEMORY[addr][0];
  }
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
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.
   
   RUN_BIT
   REGS
   MEMORY

   PC
   N
   Z
   P

   dep_stall
   v_de_br_stall
   v_agex_br_stall
   v_mem_br_stall
   mem_stall 
   icache_r
 
   PS
   NEW_PS


   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.


   Begin your code here 	  			       */
/***************************************************************/
#define COPY_AGEX_CS_START 3 
#define COPY_MEM_CS_START 9
#define COPY_SR_CS_START  7
#define bitSet 1
/* Signals generated by SR stage and needed by previous stages in the
   pipeline are declared below. */
int sr_reg_data, 
    sr_n, sr_z, sr_p,
    v_sr_ld_cc,
    v_sr_ld_reg,
    sr_drid;


/************************* SR_stage() *************************/
void SR_stage() {
  
  /* You are given the code for SR_stage to get you started. Look at
     the figure for SR stage to see how this code is implemented. */
  
  switch (Get_DR_VALUEMUX1(PS.SR_CS))
  {
  case 0: 
    sr_reg_data = PS.SR_ADDRESS ;
    break;
  case 1:
    sr_reg_data = PS.SR_DATA ;
    break;
  case 2:
    sr_reg_data = PS.SR_NPC ;
    break;
  case 3:
    sr_reg_data = PS.SR_ALU_RESULT ;
    break;
  }

  sr_drid = PS.SR_DRID; 
  v_sr_ld_reg = Get_SR_LD_REG(PS.SR_CS) & PS.SR_V;
  v_sr_ld_cc = Get_SR_LD_CC(PS.SR_CS) & PS.SR_V ;

  /* CC LOGIC  */
  sr_n = ((sr_reg_data & 0x8000) ? 1 : 0);
  sr_z = ((sr_reg_data & 0xFFFF) ? 0 : 1);
  sr_p = 0;
  if ((!sr_n) && (!sr_z))
    sr_p = 1;
  
}


/************************* MEM_stage() *************************/
/* Signals generated by MEM stage and needed by previous stages in the
   pipeline are declared below. */
   
   int target_pc; /*These three signals are needed by fetch stage, possibly other stages as well.*/
   int trap_pc;
   int mem_pcmux;
   
   int v_mem_ld_cc;
   int v_mem_ld_reg;
   /*v_mem_br_stall already defined above I think but organizationally it's kinda here.*/
   int mem_drid;
   
   
void MEM_stage() {

  int ii,jj = 0;
  
  /* your code for MEM stage goes here 
  Notes: target_pc taken directly from Mem_address latch. Calculated in agex stage.
  
  BASIC SIGNALS:
    MEM_DCACHE_EN,
  MEM_DCACHE_RW,
  MEM_DATA_SIZE
  */
  
  int we0, we1;
  
  int brnzpBits = extractIRbits(11, 9, PS.MEM_IR);
  int lowAddressBit = (PS.MEM_ADDRESS & 0x0001);
  int dataSize = PS.MEM_CS[MEM_DATA_SIZE];
	
  int memAddr = Low16bits(PS.MEM_ADDRESS);
  int memDatatoWrite = Low16bits(PS.MEM_ALU_RESULT);
  int memDatatoRead;
  int memReady;
  int DCACHE_EN = ((Get_DCACHE_EN(PS.MEM_CS)) & (PS.MEM_V));
  
   /*BRANCH LOGIC: calculates value of PCMUX for fetch stage. This is important for the fetch stage.*/
  if(PS.MEM_V){
if((PS.MEM_CS[BR_OP]&&(brnzpBits && PS.MEM_CC)) || PS.MEM_CS[UNCOND_OP]){
		mem_pcmux = 1;
		/*Basically checking whether its EITHER a branch operation and one of CC/brnzp bit pairs matches (should branch) OR its an unconditional instruction*/
}
else if(PS.MEM_CS[TRAP_OP]){
mem_pcmux = 2;
}
else{
mem_pcmux = 0;
}
}

  /*NOW MEMORY ACCESS STUFF*/
  
/*FIRST calculate WE signals*/
/*Standard byte logic for write enable: Make sure a write instruction, then just copy logic from memmapped io appendix. We don't have to take care
of unaligned access exception!*/
if(PS.MEM_CS[MEM_DCACHE_RW]){
if(!(dataSize)){
if(!lowAddressBit){ 
we0 = 1;
memDatatoWrite = Low16bits((memDatatoWrite)&(0x00FF));

}
else{ 
we1 = 1;
memDatatoWrite = Low16bits((memDatatoWrite)&(0x00FF));
memDatatoWrite = memDatatoWrite<<8;
}



}
/*Standard word logic for write enable*/
  if(dataSize && (!lowAddressBit)){
	we0 = 1;
	we1 = 1;
	memDatatoWrite = Low16bits(memDatatoWrite);
  }
}

/*NEXT: We have address and data from latches defined above. Now we need to just call dcacheyaccess*/


dcache_access(memAddr, &memDatatoRead, memDatatoWrite, &memReady, we0, we1);



/*Ok, so at this point, dcacheyaccess has either succeeded or failed. We can probably worry about that in a little bit, but we should
set up the read data appropriately right now.

afterwards calculate special signals, then latch values then check whether latches are valid or not.*/

if(!dataSize){
/*Not sure whether I should keep the byte associated with Mar low bit =1 in the high 8 bits or just shift it down. I'll shift down DEBUG LATER*/
if(!lowAddressBit){
memDatatoRead = Low16bits(memDatatoRead &0x00FF);
}

if(lowAddressBit){
memDatatoRead = Low16bits(memDatatoRead &0xFF00)>>8;
}
}

/*Now generate signals that get used for other stages (not latched though)*/
  target_pc = Low16bits(PS.MEM_ADDRESS);
  trap_pc = memDatatoRead;
 
  
  /*Debug: Figure out what this does. Don't really understand why the diagram says to do this. probably 
  has to do with inserting bubbles or stalling or something.
  
  ok I figured it out. for future reference. need it for the dependency logic for comparing old instructions that haven't written to the 
  reg file and comparing the sr1/2's of a newer instruction that woudl currently be in the decode stage.*/
  if(PS.MEM_V){
    v_mem_ld_cc = PS.MEM_CS[LD_CC];
    v_mem_ld_reg = PS.MEM_CS[LD_REG];
    v_mem_br_stall = PS.MEM_CS[BR_STALL];
  }
  else{
    v_mem_ld_cc = 0;
    v_mem_ld_reg = 0;
    v_mem_br_stall = 0;
  }

	/*Now calculate memory stall: As log as the dache has been enabled and we see that it's not ready. (thus a valid reading for memREady then we know to stall*/
	mem_stall = (DCACHE_EN & (!memReady));
	
  /*Latching things for sr stage. Mostly shwon in lab document. Just do what it says there.*/
	NEW_PS.SR_ADDRESS = PS.MEM_ADDRESS;
	
	if(DCACHE_EN){
	NEW_PS.SR_DATA = memDatatoRead;
	}
	else{
	NEW_PS.SR_DATA = 0;
	}
	/*Control store taken care of from shell code*/
	
	NEW_PS.SR_NPC = PS.MEM_NPC;
	NEW_PS.SR_ALU_RESULT = PS.MEM_ALU_RESULT;
	NEW_PS.SR_IR = PS.MEM_IR;
	NEW_PS.SR_DRID = PS.MEM_DRID;
	/*DEBUG: Figure out what to do for NEW_PS.SR_V valid bit.
	
	ok, for now, just make sure memory is valid and dcache is enabled and memory is enabled.
	
	try and ask what happens if dcache is not enabled???*/
	
	if(PS.MEM_V){
	NEW_PS.SR_V = ((DCACHE_EN & (memReady)) | (!DCACHE_EN));
	}
	else{
	NEW_PS.SR_V = 0;
	}
  
	mem_drid = PS.MEM_DRID;
  /* The code below propagates the control signals from MEM.CS latch
     to SR.CS latch. You still need to latch other values into the
     other SR latches. */
  for (ii=COPY_SR_CS_START; ii < NUM_MEM_CS_BITS; ii++) {
    NEW_PS.SR_CS [jj++] = PS.MEM_CS [ii];
  }

}
/*Some globals generated in agex*/
int v_agex_ld_cc;
int v_agex_ld_reg;
int agex_drid;
/************************* AGEX_stage() *************************/
void AGEX_stage() {

  int ii, jj = 0;
  int LD_MEM; /* You need to write code to compute the value of LD.MEM
		 signal */

  /* your code for AGEX stage goes here */
int addr1muxOutput;
int addr2muxOutput;
int addrTotesmuxOutput;
int addressMux = Get_ADDRESSMUX(PS.AGEX_CS);
int shfOutput;
int aluOutput;
int resultOutput;
int IR = PS.AGEX_IR;
int ADDR2MUX = Get_ADDR2MUX(PS.AGEX_CS);
int lshf1 = Get_LSHF1(PS.AGEX_CS);
int sr2Mux = Get_SR2MUX(PS.AGEX_CS);
int sr2muxOutput;
int sr1muxOutput = PS.AGEX_SR1; /*I know there's no mux here, but it's consistent name with sr2*/
int aluk = Get_ALUK(PS.AGEX_CS);
int shiftCode = extractIRbits(5,4,IR);
int shiftAmount = extractIRbits(3,0,IR);
int aluResultMux = Get_ALU_RESULTMUX(PS.AGEX_CS);

/*stuff for arith right shift*/
int signMS = 0x8000;
int signBit;
int i;

/*Calculate Addr1Muxoutput*/
 if(PS.AGEX_CS[AGEX_ADDR1MUX]){
 addr1muxOutput = PS.AGEX_SR1;
 }
 else{
 addr1muxOutput = PS.AGEX_NPC;
 }
  addr1muxOutput = Low16bits(addr1muxOutput);


/*Calculate Addr2Muxoutput*/  
  switch(ADDR2MUX){
  
  case 0:
  addr2muxOutput = 0;
  break;
  
  case 1:
  addr2muxOutput = extractIRbits(5,0,IR);
  addr2muxOutput = sEXT(addr2muxOutput, 16);
  addr2muxOutput = Low16bits(addr2muxOutput);
  break;
  
  case 2:
  addr2muxOutput = extractIRbits(8,0,IR);
  addr2muxOutput = sEXT(addr2muxOutput, 16);
  addr2muxOutput = Low16bits(addr2muxOutput);
  break;
 
  case 3:
  addr2muxOutput = extractIRbits(10,0,IR);
  addr2muxOutput = sEXT(addr2muxOutput, 16);
  addr2muxOutput = Low16bits(addr2muxOutput);
  break;
 
  default:
  printf("Something wrong AGEx");
  break;
  
  }
  
  /*Don't Forget to left shift!*/
  if(lshf1){
  addr2muxOutput = addr2muxOutput<<1;
  }

  
  /*Address MUX Logics*/
  if(addressMux){
	addrTotesmuxOutput = Low16bits(addr1muxOutput + addr2muxOutput);
  }
  else{
  addrTotesmuxOutput = extractIRbits(7,0,IR);
  addrTotesmuxOutput = sEXT(addrTotesmuxOutput, 16);
  addrTotesmuxOutput = Low16bits(addrTotesmuxOutput);
  }
  
  /*Now need to calculate ALU/Shifter stuff*/
  if(sr2Mux){
  sr2muxOutput = Low16bits(extractIRbits(4,0,IR));
  }
  else{
  sr2muxOutput = Low16bits(PS.AGEX_SR2);
  }
  
  /*sr1muxOutput is A. sr2muxOUtput is B.
  order is Add, and, xor, PassB
  */
  switch(aluk){
   case 0:
		aluOutput = Low16bits(sr1muxOutput + sr2muxOutput);
	break;
	
	case 1:
		aluOutput = Low16bits((sr1muxOutput & sr2muxOutput));
	break;
  
	case 2:
		aluOutput = Low16bits((sr1muxOutput ^ sr2muxOutput));
	break;
	
	case 3:
		aluOutput = Low16bits(sr2muxOutput); /*Just pass B*/
	break;
  
  }
  
  /*Shift output. extract bits 5:0. 3:0 specify amount, 5:4 are code for left/right arithmetic logicla shift*/
	switch(shiftCode){
	case 0:
	shfOutput = (sr1muxOutput<<shiftAmount);
	break;
	case 1:
	shfOutput = Low16bits((sr1muxOutput>>shiftAmount));
	break;
	case 3:
	/*Keeps track of signed bit for arithmentic right shifts*/
		
		shfOutput = sr1muxOutput;
		signBit = ((shfOutput&signMS)>>15);
		for(i = 0; i<shiftAmount; i++){
			if(signBit){
			 shfOutput = shfOutput>>1;
			shfOutput += signMS;
			}
			else{
			shfOutput>>1;
			}	
		}
		
		shfOutput = Low16bits(shfOutput);	
	break;	
	}

	if(aluResultMux){
	resultOutput = aluOutput;
	}
	else{
	resultOutput = shfOutput;
	}
  
  
    if(PS.AGEX_V){
    v_agex_ld_cc = PS.AGEX_CS[AGEX_LD_CC];
    v_agex_ld_reg = PS.AGEX_CS[AGEX_LD_REG];
    v_agex_br_stall = PS.AGEX_CS[AGEX_BR_STALL];
  }
  else{
    v_mem_ld_cc = 0;
    v_mem_ld_reg = 0;
    v_mem_br_stall = 0;
  }
  
  agex_drid = PS.AGEX_DRID;
  if (LD_MEM) {
    /* Your code for latching into MEM latches goes here */
    NEW_PS.MEM_ADDRESS = addrTotesmuxOutput;
	NEW_PS.MEM_NPC = PS.AGEX_NPC;
	NEW_PS.MEM_CC = PS.AGEX_CC;
	NEW_PS.MEM_ALU_RESULT = resultOutput;
	NEW_PS.MEM_IR = PS.AGEX_IR;
	NEW_PS.MEM_DRID = PS.AGEX_DRID;
	NEW_PS.MEM_V = PS.AGEX_V; /*DEBUG: NOT ENTIRELY SURE WHAT TO DO HERE. WORRY ABOUT LATER. for NEW_PS.MEM_V valid bit*/

    /* The code below propagates the control signals from AGEX.CS latch
       to MEM.CS latch. */
    for (ii = COPY_MEM_CS_START; ii < NUM_AGEX_CS_BITS; ii++) {
      NEW_PS.MEM_CS [jj++] = PS.AGEX_CS [ii]; 
    }
  }
}



/************************* DE_stage() *************************/
void DE_stage() {

  int CONTROL_STORE_ADDRESS;  /* You need to implement the logic to
			         set the value of this variable. Look
			         at the figure for DE stage */ 
  int ii, jj = 0;
  int LD_AGEX; /* You need to write code to compute the value of
		  LD.AGEX signal */
   int IR = PS.DE_IR;
   int sr2idmuxOutput;
   int sr1idmuxOutput = extractIRbits(8, 6, IR);
   int IR5bit = extractIRbits(5, 5, IR);
   int IR13bit = extractIRbits(5, 5, IR);
   int drmux;
   int drmuxOutput;
   int nzpVal;
   
   /*Dependency stuff*/
	int sr1needed;
	int sr2needed;
	int br_op;
	
   
  /* your code for DE stage goes here. Remember, there's a line going from SR back here to write to the register file.
	Dependency logic going to be most difficult part. do that last.
  */
  /*First calculate CS Addreess*/
  CONTROL_STORE_ADDRESS = extractIRbits(15, 11, IR);
  CONTROL_STORE_ADDRESS = ((CONTROL_STORE_ADDRESS<<1)|IR5bit);
  
  /*Getting a couple of control store signals here because we need them for decode/dependency logic*/
  drmux = Get_DRMUX(CONTROL_STORE[CONTROL_STORE_ADDRESS]);
  sr1needed = Get_SR1_NEEDED(CONTROL_STORE[CONTROL_STORE_ADDRESS]);
  sr2needed = Get_SR2_NEEDED(CONTROL_STORE[CONTROL_STORE_ADDRESS]);
  br_op = Get_BR_OP(CONTROL_STORE[CONTROL_STORE_ADDRESS]);
  
	if(IR13bit){
		sr2idmuxOutput = extractIRbits(11, 9, IR);
	}
	else{
		sr2idmuxOutput = extractIRbits(2, 0, IR);
	}

	
	/*Now have the ids for sr1 and sr2. Should calculate destinatino register id */
	if(!drmux){
	drmuxOutput = Low16bits(extractIRbits(11, 9, IR));
	}
	else{
	drmuxOutput = 7;
	}

	/*Ok, time for dependency logic. Just need to calculate dep_stall global variable
	basically find matches between sr1/2 register and any of the drid regs let's just do this dumb and simple.
	use sr1idmuxOutput*/
	dep_stall = 0;
	if(sr1needed){
	if(v_agex_ld_reg){
		if(sr1idmuxOutput == agex_drid){
		dep_stall |= bitSet;
		}
	}
		if(v_mem_ld_reg){
		if(sr1idmuxOutput == mem_drid){
		dep_stall |= bitSet;
		}
	}
		if(v_sr_ld_reg){
		if(sr1idmuxOutput == sr_drid){
		dep_stall |= bitSet;
		}
	}
	}
	
	if(sr2needed){
	if(v_agex_ld_reg){
		if(sr2idmuxOutput == agex_drid){
		dep_stall |= bitSet;
		}
	}
		if(v_mem_ld_reg){
	
		if(sr2idmuxOutput == mem_drid){
		dep_stall |= bitSet;
		}
	}
		if(v_sr_ld_reg){
	
		if(sr2idmuxOutput == sr_drid){
		dep_stall |= bitSet;
		}
	}
	}
	/*Now just need to check with condition codes*/
	if(br_op){
	if(v_agex_ld_cc||v_mem_ld_cc||v_sr_ld_cc){
			dep_stall|= bitSet;
		}
	}
	
	
	/* SR Data/Condition codes. Basically, we need to use anycc/sr stuff calculated in the sr stage if its valid and latch it into agex ps/ register file
	Luckily, we don't have to actually calculate any of the stuff because sr takes care of it for us.*/
	
	if(v_sr_ld_reg){
        REGS[sr_drid] = sr_reg_data;
        }
	
		if(v_sr_ld_cc){
		N = sr_n;
		Z = sr_z;
		P = sr_p;
		
		nzpVal = (N<<2)|(Z<<1)|(P);
		}
		
		/*DEBUG: Not sure whether memstall is the only thing preventing loading the agex stage.*/
	if(mem_stall){
	LD_AGEX = 0;
	}
	else{
	LD_AGEX = 1;
	}
	
	
  if (LD_AGEX) {
    /* Your code for latching into AGEX latches goes here */
    /*Just follow what chart says*/
	
	  NEW_PS.AGEX_NPC = PS.DE_NPC;
      NEW_PS.AGEX_IR = IR;
	  NEW_PS.AGEX_SR1 = REGS[sr1idmuxOutput];
      NEW_PS.AGEX_SR2 = REGS[sr2idmuxOutput];
	  NEW_PS.AGEX_CC = nzpVal;
	  NEW_PS.AGEX_DRID = drmuxOutput;          
	  
	  /*DEBUG: Not sure whether this is right or not. possibly needs */
      NEW_PS.AGEX_V = ((!dep_stall) & (!v_de_br_stall) & (PS.DE_V));

    /* The code below propagates the control signals from the CONTROL
       STORE to the AGEX.CS latch. */
    for (ii = COPY_AGEX_CS_START; ii< NUM_CONTROL_STORE_BITS; ii++) {
      NEW_PS.AGEX_CS[jj++] = CONTROL_STORE[CONTROL_STORE_ADDRESS][ii];
    }
  }

}



/************************* FETCH_stage() *************************/
void FETCH_stage() {

  /* your code for FETCH stage goes here */
  int PC_NEXTCYCLE; /*this is a temp for what we calculate PC to be for the next cycle*/
  int ld_pc;
  int instruc;
 
  /*PCMUX calculates potential next pc value.*/
  if(mem_pcmux == 0){
	PC_NEXTCYCLE = PC +2;
  }
  
   if(mem_pcmux == 1){
		PC_NEXTCYCLE = target_pc;
  }
  
   if(mem_pcmux == 2){
	PC_NEXTCYCLE = trap_pc;
  }
  
  
  /*Now we need to see whether PC should be loaded with NEXTCYCLE or not. If, there's a stall anywhere else down the pipeline
  then it doesn't make sense to update the pc, because you still have to retrieve the current pc value from icache. Also, if the 
  current pc value gets an icache miss, then you want to try again and not update pc.*/
  
  /*DEBUG: somethign seems to be going wrong with tthe fetch stage. My guess is right now that my assumption for loading pc might be wrong.
  Temporarily just changing it to only memory stage stall because that's where target and trap pc are calculated.*/
  
  icache_access(PC,&instruc, &icache_r);
  
	if(((icache_r))&&!(mem_stall)){
	/*Know we can update PC*/
		PC = PC_NEXTCYCLE;
	}
  
	/*Doc is implying that dependency stall signal and mem stall signal condition latching values. Just need to update instruction and pc and valid bits
	here.*/
	
	if((!(dep_stall))&&!(mem_stall)){
		NEW_PS.DE_NPC = PC_NEXTCYCLE;
		NEW_PS.DE_IR = instruc;
		NEW_PS.DE_V = (((icache_r))&&!((dep_stall)||(v_de_br_stall)||(v_agex_br_stall)||(v_mem_br_stall))); /*Need to make sure either i-cache hit or no dependency stalls 
		or any of the later stages are performing a valid control instruction which requires stalls in earlier parts of the pipeline.*/
		
	}
	
  
  
}  

/*takes two indexes within the IR register and isolates it and returns it.*/
int extractIRbits(int high, int low, int regis){
int i;
int bitMask = 0;
int extracted;

/*Form a mask that is as large as high-low and perform that many add 1's and shifts lefts to the initial mask*/

int IRReg = regis;

for(i = 0; i<(high-low); i++){
	bitMask+=1;
	bitMask= bitMask<<1;

}

bitMask+=1;


bitMask = bitMask<<low;

IRReg = Low16bits(regis);
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

