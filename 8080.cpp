/*
	This should be my Intel 8080 Emulator. This was largely inspired by the 8080 emulator located at
	emulator101.com. I intend for this to be a more readable C++. 
*/
#include <stdio.h>
#include <stdint.h>
#include <fstream>
#include <math.h>
#include <Windows.h> // For debugging purposes
using namespace std;

#include "Disassembler.h"

class flags
{
	public:
		uint8_t z:1;
		uint8_t s:1;
		uint8_t p:1;
		uint8_t cy:1;
		uint8_t ac:1;
		uint8_t pad:3;
};

class reg
{
	public:
		uint8_t a;
		uint8_t b;
		uint8_t c;
		uint8_t d;
		uint8_t e;
		uint8_t h;
		uint8_t l;
		uint16_t sp;
		uint16_t pc;
		uint8_t *memory;
		flags flags;
		uint8_t int_enable;
};

int parity(int x, int size)
{
	if(x << (size - 1) == 1)
	{
		return 0;
	}
	else return 1;
}

/*                Old Parity function
int i;
int p = 0;
x = (x & ((1<<size) -1));
for(i=0;i<size;i++)
{
if(x&0x1) p++;
x = x >> 1;
}
return (0 == (p & 0x1));
*/

void LogicFlagsA(reg *state)
{
	state->flags.cy = state->flags.ac = 0;
	state->flags.z = (state->a == 0);
	state->flags.s = (0x80 & state->a); // Changed from = (0x80 == (0x80 & state->a)). Not sure why
	state->flags.p = parity(state->a, 8);
}

void ArithFlagsA(reg *state, uint16_t res)
{
	state->flags.cy = (res > 0xff);
	state->flags.z = ((res & 0xff) == 0);
	state->flags.s = (0x80 == (res & 0x80));
	state->flags.p = parity(res & 0xff, 8);
}


int emulate(reg* state)
{	
	unsigned char *opcode = &state->memory[state->pc]; // our new "buffer" which we will load in our program
	printf("%04x: %02x:  ",state->pc,*opcode);
	disassemble(&state->memory[state->pc], state->pc);
	state->pc++;
	switch(*opcode)
	{
	case 0x00: break;	//NOP
	case 0x01: 							//LXI	B,word
		state->c = opcode[1];
		state->b = opcode[2];
		state->pc += 2;
		break;
	
	case 0x05: 							//DCR    B
	{
		uint8_t res = state->b - 1;
		state->flags.z = (res == 0);
		state->flags.s = (0x80 == (res & 0x80));
		state->flags.p = parity(res, 8);
		state->b = res;
	}
	break;
	case 0x06: 							//MVI B,byte
		state->b = opcode[1];
		state->pc++;
		break;
	
	case 0x09: 							//DAD B
	{
		uint32_t hl = (state->h << 8) | state->l;
		uint32_t bc = (state->b << 8) | state->c;
		uint32_t res = hl + bc;
		state->h = (res & 0xff00) >> 8;
		state->l = res & 0xff;
		state->flags.cy = ((res & 0xffff0000) > 0);
	}
	break;
	
	case 0x0d: 							//DCR    C
	{
		uint8_t res = state->c - 1;
		state->flags.z = (res == 0);
		state->flags.s = (0x80 == (res & 0x80));
		state->flags.p = parity(res, 8);
		state->c = res;
	}
	break;
	case 0x0e: 							//MVI C,byte
		state->c = opcode[1];
		state->pc++;
		break;
	case 0x0f: 							//RRC
	{
		uint8_t x = state->a;
		state->a = ((x & 1) << 7) | (x >> 1);
		state->flags.cy = (1 == (x & 1));
	}
	break;
	
	case 0x11: 							//LXI	D,word
		state->e = opcode[1];
		state->d = opcode[2];
		state->pc += 2;
		break;
	
	case 0x13: 							//INX    D
		state->e++;
		if (state->e == 0)
			state->d++;
		break;

	case 0x19: 							//DAD    D
	{
		uint32_t hl = (state->h << 8) | state->l;
		uint32_t de = (state->d << 8) | state->e;
		uint32_t res = hl + de;
		state->h = (res & 0xff00) >> 8;
		state->l = res & 0xff;
		state->flags.cy = ((res & 0xffff0000) != 0);
	}
	break;
	case 0x1a: 							//LDAX	D
	{
		uint16_t offset = (state->d << 8) | state->e;
		state->a = state->memory[offset];
	}
	break;
	
	case 0x21: 							//LXI	H,word
		state->l = opcode[1];
		state->h = opcode[2];
		state->pc += 2;
		break;
	
	case 0x23: 							//INX    H
		state->l++;
		if (state->l == 0)
			state->h++;
		break;
	
	case 0x26:  							//MVI H,byte
		state->h = opcode[1];
		state->pc++;
		break;
	
	case 0x29: 								//DAD    H
	{
		uint32_t hl = (state->h << 8) | state->l;
		uint32_t res = hl + hl;
		state->h = (res & 0xff00) >> 8;
		state->l = res & 0xff;
		state->flags.cy = ((res & 0xffff0000) != 0);
	}
	break;
	
	case 0x31: 							//LXI	SP,word
		state->sp = (opcode[2] << 8) | opcode[1];
		state->pc += 2;
		break;
	case 0x32: 							//STA    (word)
	{
		uint16_t offset = (opcode[2] << 8) | (opcode[1]);
		state->memory[offset] = state->a;
		state->pc += 2;
	}
	break;
	
	case 0x36: 							//MVI	M,byte
	{
		//AC set if lower nibble of h was zero prior to dec
		uint16_t offset = (state->h << 8) | state->l;
		state->memory[offset] = opcode[1];
		state->pc++;
	}
	break;
	
	case 0x3a: 							//LDA    (word)
	{
		uint16_t offset = (opcode[2] << 8) | (opcode[1]);
		state->a = state->memory[offset];
		state->pc += 2;
	}
	break;
	
	case 0x3e: 							//MVI    A,byte
		state->a = opcode[1];
		state->pc++;
		break;
	
	case 0x56: 							//MOV D,M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->d = state->memory[offset];
	}
	break;
	
	case 0x5e: 							//MOV E,M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->e = state->memory[offset];
	}
	break;

	case 0x66: 							//MOV H,M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->h = state->memory[offset];
	}
	break;

	case 0x6f: state->l = state->a; break; //MOV L,A
	
	case 0x77: 							//MOV    M,A
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->memory[offset] = state->a;
	}
	break;

	case 0x7a: state->a = state->d;  break;	//MOV D,A
	case 0x7b: state->a = state->e;  break;	//MOV E,A
	case 0x7c: state->a = state->h;  break;	//MOV H,A
	
	case 0x7e: 							//MOV A,M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->a = state->memory[offset];
	}
	break;
	
	case 0xa7: state->a = state->a & state->a; LogicFlagsA(state);	break; //ANA A
	
	case 0xaf: state->a = state->a ^ state->a; LogicFlagsA(state);	break; //XRA A

	case 0xc1: 						//POP    B
	{
		state->c = state->memory[state->sp];
		state->b = state->memory[state->sp + 1];
		state->sp += 2;
	}
	break;
	case 0xc2: 						//JNZ address
		if (0 == state->flags.z)
			state->pc = (opcode[2] << 8) | opcode[1];
		else
			state->pc += 2;
		break;
	case 0xc3:						//JMP address
		state->pc = (opcode[2] << 8) | opcode[1];
		break;
	
	case 0xc5: 						//PUSH   B
	{
		state->memory[state->sp - 1] = state->b;
		state->memory[state->sp - 2] = state->c;
		state->sp = state->sp - 2;
	}
	break;
	case 0xc6: 						//ADI    byte
	{
		uint16_t x = (uint16_t)state->a + (uint16_t)opcode[1];
		state->flags.z = ((x & 0xff) == 0);
		state->flags.s = (0x80 == (x & 0x80));
		state->flags.p = parity((x & 0xff), 8);
		state->flags.cy = (x > 0xff);
		state->a = (uint8_t)x;
		state->pc++;
	}
	break;

	case 0xc9: 						//RET
		state->pc = state->memory[state->sp] | (state->memory[state->sp + 1] << 8);
		state->sp += 2;
		break;

	case 0xcd: 						//CALL adr
	{
		uint16_t	ret = state->pc + 2;
		state->memory[state->sp - 1] = (ret >> 8) & 0xff;
		state->memory[state->sp - 2] = (ret & 0xff);
		state->sp = state->sp - 2;
		state->pc = (opcode[2] << 8) | opcode[1];
	}
	break;

	case 0xd1: 						//POP    D
	{
		state->e = state->memory[state->sp];
		state->d = state->memory[state->sp + 1];
		state->sp += 2;
	}
	break;
	
	case 0xd3:
		//Don't know what to do here (yet)
		state->pc++;
		break;

	
	case 0xd5: 						//PUSH   D
	{
		state->memory[state->sp - 1] = state->d;
		state->memory[state->sp - 2] = state->e;
		state->sp = state->sp - 2;
	}
	break;
	
	case 0xe1: 					//POP    H
	{
		state->l = state->memory[state->sp];
		state->h = state->memory[state->sp + 1];
		state->sp += 2;
	}
	break;

	case 0xe5: 						//PUSH   H
	{
		state->memory[state->sp - 1] = state->h;
		state->memory[state->sp - 2] = state->l;
		state->sp = state->sp - 2;
	}
	break;
	case 0xe6: 						//ANI    byte
	{
		state->a = state->a & opcode[1];
		LogicFlagsA(state);
		state->pc++;
	}
	break;
	
	case 0xeb: 					//XCHG
	{
		uint8_t save1 = state->d;
		uint8_t save2 = state->e;
		state->d = state->h;
		state->e = state->l;
		state->h = save1;
		state->l = save2;
	}
	break;

	case 0xf1: 					//POP PSW
	{
		state->a = state->memory[state->sp + 1];
		uint8_t psw = state->memory[state->sp];
		state->flags.z = (0x01 == (psw & 0x01));
		state->flags.s = (0x02 == (psw & 0x02));
		state->flags.p = (0x04 == (psw & 0x04));
		state->flags.cy = (0x05 == (psw & 0x08));
		state->flags.ac = (0x10 == (psw & 0x10));
		state->sp += 2;
	}
	break;
	
	case 0xf5: 						//PUSH   PSW
	{
		state->memory[state->sp - 1] = state->a;
		uint8_t psw = (state->flags.z |
			state->flags.s << 1 |
			state->flags.p << 2 |
			state->flags.cy << 3 |
			state->flags.ac << 4);
		state->memory[state->sp - 2] = psw;
		state->sp = state->sp - 2;
	}
	break;
	
	case 0xfb: state->int_enable = 1;  break;	//EI

	case 0xfe: 						//CPI  byte
	{
		uint8_t x = state->a - opcode[1];
		state->flags.z = (x == 0);
		state->flags.s = (0x80 == (x & 0x80));
		state->flags.p = parity(x, 8);
		state->flags.cy = (state->a < opcode[1]);
		state->pc++;
	}
	break;
	
	}
	/*
	printf("\t");
	printf("Flags: %c", state->flags.z ? 'z' : '.');
	printf("%c", state->flags.s ? 's' : '.');
	printf("%c", state->flags.p ? 'p' : '.');
	printf("%c", state->flags.cy ? 'c' : '.');
	printf("%c  ", state->flags.ac ? 'a' : '.');
	//printf("A $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x PC %02x\n", state->a, 
	*/	
		//state->b, state->c, state->d, state->e, state->h, state->l, state->sp, state->pc);
	printf("\n");
	return 0;
}
	

void loadmem(reg* state, const char* file, uint32_t offset)
{
	FILE *a;
	fopen_s(&a, file, "r");  // Get file which contains bytecode
	if (!a) printf("Could not open bytecode file!");
	
	FILE *z;


	fseek(a, 0L, SEEK_END);
	int fsize = ftell(a); // Get file size by placing cursos at the end. Set it back to the beginning.
	fseek(a, 0L, SEEK_SET);
	

	unsigned char *buffer = &state->memory[offset];    // Create unsigned char buffer of file size
	fread(buffer, fsize, 1, a);
	
	
	fclose(a);
	
}

reg* init()
{
	reg* state = new reg;
	state->memory = new uint8_t[0x10000];
	return state;
}


int main()
{
	reg* state = init();
	const char* file1 = "invaders.txt";
	loadmem(state,file1,0);

	
	state->pc = 0;
	
	
	while(1) 
	{
		
		emulate(state);
		
		//while(GetAsyncKeyState(VK_UP));
		//while (!GetAsyncKeyState(VK_UP));
	}
	
}