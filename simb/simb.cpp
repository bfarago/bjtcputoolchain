#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <conio.h>
#define FOPEN(f, n, m) fopen_s(&(fi), (n), (m))
#else /////////////////////////////////////////////////
#include <termios.h>
#define fread_s(b,s,a,n,f) fread(b,a,n,f)
#define _getch getch
static struct termios told, tnew;
/* Initialize new terminal i/o settings */
void initTermios(int echo)
{
	tcgetattr(0, &told); /* grab old terminal i/o settings */
	tnew = told; /* make new settings same as old settings */
	tnew.c_lflag &= ~ICANON; /* disable buffered i/o */
	tnew.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
	tcsetattr(0, TCSANOW, &tnew); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void){ tcsetattr(0, TCSANOW, &told);}

/* Read 1 character - echo defines echo mode */
char getch_(int echo) { 
	char ch;
	initTermios(echo);
	ch = getchar();
	resetTermios();
	return ch;
}

/* Read 1 character without echo */
char getch(void){ return getch_(0);}

/* Read 1 character with echo */
char getche(void){ return getch_(1);}

#define FOPEN(f, n, m) f=fopen((n),(m))
#endif /////////////////////////////////////////////////

#include <ctype.h>  
#define MAXMEMORY (1<<12)

typedef enum {
	bd_Tristate,
	bd_Read,
	bd_Write,
	bd_max
} busDirection_t;

typedef struct {
	int address;
	char data;
	busDirection_t dir;
} bus_t;

typedef struct {
	int pc;
	int pcnext;
	int addr;
	char alu;
	char carry;
	char op;
	char state;
	int data;
	char ilen;
} cpu_t;

bus_t bus;
cpu_t cpu;
char memory[MAXMEMORY];

void dumpheader() {
	printf("Addr  D S | S PC    I Wrk | Alu Flag | Perif");
	printf("\n");
}
void dumpmem(int addr) {
	printf("%x", memory[addr] & 0xf);
}

#undef TOK
#define TOK(x, opc) #x

static const char *gMnemonics[16] = 
{ "mvi a,","sta","lda","ad0","ad1","adc","nand","nor","rrm","jmp","jc","jnc","jz","jnz","jm","jp"};

void dump() {
	printf("0x%03x %x %c | ", bus.address, bus.data, "XRW"[bus.dir]);
	printf("%c 0x%03x ", cpu.state, cpu.pc);
	if (cpu.op>=0) printf("%x", cpu.op);
	else printf("x");
	if (cpu.data >= 0) {
		printf(" %03x", cpu.data);
	} else {
		printf(" xxx");
	}
	printf(" |  %x  %c%c%c", cpu.alu &0x1f,
		cpu.carry ? 'C' : ' ', cpu.alu & 8 ? 'M' : 'P', !cpu.alu?'Z':' ');
	printf("  |");
	
	dumpmem(0xc00);
	dumpmem(0xc01);
	dumpmem(0xc02);
	dumpmem(0xc03);
	dumpmem(4093);
	dumpmem(4094);
	dumpmem(4095);

	printf(" | ");
	if ('D'==cpu.state)
	if (cpu.op>=0) printf("%s %03x", gMnemonics[cpu.op], cpu.data);

	printf("\n");
}

char perif_Read(int addr) {
	char r = 0xf;
	switch (addr) {
	case 0xc00: break;
	case 0xc01: break;
	case 0xc02: break;
	case 0xc03: break;
	case 0xd00: break;
	case 0xd01: break;
	case 0xd02: break;
	case 0xd03: break;
	}
	return r;
}

void perif_Write(int addr, char data) {
	switch (addr) {
	case 0xc00: break;
	case 0xc01: break;
	case 0xc02: break;
	case 0xc03: break;
	case 0xd00: break;
	case 0xd01: break;
	case 0xd02: break;
	case 0xd03: break;
	}
}

char cpu_busRead(int addr) {
	cpu.addr = addr;
	bus.address = addr;
	bus.dir = bd_Read;
	dump();
	if (addr >= 0xc00) {
		bus.data=perif_Read(addr);
	}
	else {
		bus.data = memory[addr] & 0x0f;
	}
	return bus.data;
}

void cpu_busWrite(int addr, char data) {
	cpu.addr = addr;
	bus.address = addr;
	bus.dir = bd_Write;
	bus.data = data & 0x0f;
	cpu.state = 'S';
	dump();
	if (addr >= 0xc00) {
		perif_Write(addr, bus.data);
	}
	else {
		memory[addr] = bus.data;
	}
}

void fetch() {
	cpu.ilen = 2;
	cpu.state = 'F';
	//cpu.op = -1;
	cpu.data = -1;
	cpu.op = cpu_busRead(cpu.pc);
	cpu.data = cpu_busRead(cpu.pc + 1);
	if (cpu.op > 0) {
		// cpu.data <<= 8;
		cpu.data |= cpu_busRead(cpu.pc + 2) << 4;
		cpu.data |= cpu_busRead(cpu.pc + 3) << 8;
		cpu.ilen = 4;
	}
	cpu.state = 'D';
	cpu.pcnext = cpu.pc + cpu.ilen;
	dump();
	if ((cpu.op > 1) && (cpu.op < 9)) cpu.state = 'L';
	switch (cpu.op) {
		#undef TOK
		#define TOK(x, xop) case xop:
		TOK(T_mvi, 0) cpu.alu = cpu.data; break; //no effect on carry
		TOK(T_sta, 1) cpu_busWrite(cpu.data, cpu.alu); break;
		TOK(T_lda, 2) cpu.alu = cpu_busRead(cpu.data); cpu.carry = 0; break;
		TOK(T_ad0, 3) cpu.alu += cpu_busRead(cpu.data); cpu.carry = 0; break;
		TOK(T_ad1, 4) cpu.alu += cpu_busRead(cpu.data)+1; cpu.carry = 0; break;
		TOK(T_adc, 5) cpu.alu += cpu_busRead(cpu.data)+cpu.carry; cpu.carry = 0; break;
		TOK(T_nand, 6) cpu.alu = ~ (cpu.alu & cpu_busRead(cpu.data)); break;
		TOK(T_nor, 7) cpu.alu = ~(cpu.alu | cpu_busRead(cpu.data));	break;
		TOK(T_rrm, 8)cpu.alu = cpu_busRead(cpu.data)>>1;break;

		TOK(T_jmp, 9)cpu.pcnext = cpu.data;	break;
		TOK(T_jc, 10) if (cpu.carry) cpu.pcnext = cpu.data;	break;
		TOK(T_jnc, 11) if (!cpu.carry) cpu.pcnext = cpu.data; break;
		TOK(T_jz, 12)  if (!cpu.alu) cpu.pcnext = cpu.data; break;
		TOK(T_jnz, 13) if (cpu.alu) cpu.pcnext = cpu.data; break;
		TOK(T_jm, 14) if (cpu.alu & 0x8) cpu.pcnext = cpu.data; break;
		TOK(T_jp, 15) if (!(cpu.alu & 0x8)) cpu.pcnext = cpu.data; break;
	}
	if (cpu.op) //mvi no effects to carry
	if (cpu.alu > 0xf) {
		cpu.carry = 1; cpu.alu &= 0xf;
	}
	cpu.pc = cpu.pcnext;
}

int main(int ac, char** av) {
	int r = 0;
	FILE* fi = NULL;
	int fi_size = 0;
	int run = 1;
	int maxsteplen = -1;
	if (ac > 1) {
		FOPEN(fi, av[1], "rb");
		if (!fi) {
			printf("Unable to open file?\n");
			return -3;
		}
		fseek(fi, 0, SEEK_END);
		fi_size=ftell(fi);
		fseek(fi, 0, SEEK_SET);
		size_t rs=fread_s(memory, MAXMEMORY, 1, fi_size, fi);
		fclose(fi);
		if (rs != fi_size) {
			printf("Unable to read file?\n");
			return -2;
		}
	} else {
		printf("usage: %s a.out [maxstep]\n press Q to exit, R to run, or any key to step.\n",av[0]);
		return -1;
	}
	if (ac > 2) {
		maxsteplen = atol(av[2]);
		run = 0;
	}
	int i = 0;
	int ch ='R';
	//dumpheader();
	do{
		if (!(i++%5)) dumpheader();
		fetch();
		if (run) {
			run--;
		}
		else {
			if (maxsteplen >= 0) {
				if (!maxsteplen--) {
					ch = 'Q';
				}
			} else {
				ch = _getch();
				ch = toupper(ch);
				if ('R' == ch) { run = 500; }
			}
		}
	} while (ch != 'Q');
	return r;
}
