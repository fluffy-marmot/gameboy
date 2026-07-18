#include "bus.h"
#include "cpu.h"

#define MAX_MCYCLE_INSTRUCTION 5

#define MASK_BYTE       0b11111111
#define MASK_ZERO       0b00000000
#define MASK_BIT        0b00000001
#define MASK_NIBBLE_L   0b00001111
#define MASK_NIBBLE_H   0b11110000
#define MASK_FLAGS      0b11110000
#define MASK_FLAG_Z     0b10000000
#define MASK_FLAG_N     0b01000000
#define MASK_FLAG_H     0b00100000
#define MASK_FLAG_C     0b00010000
#define MASK_REG_8b     0b00000111
#define MASK_REG_16b    0b00110000
#define MASK_REG_SPorAF 0b00000011

#define FLAG_Z ((cpu.F & MASK_FLAG_Z) >> 7)
#define FLAG_N ((cpu.F & MASK_FLAG_N) >> 6)
#define FLAG_H ((cpu.F & MASK_FLAG_H) >> 5)
#define FLAG_C ((cpu.F & MASK_FLAG_C) >> 4)

#define SET_Z (cpu.F |=  MASK_FLAG_Z              )
#define SET_N (cpu.F |=  MASK_FLAG_N              )
#define SET_H (cpu.F |=  MASK_FLAG_H              )
#define SET_C (cpu.F |=  MASK_FLAG_C              )
#define CLR_Z (cpu.F &= (MASK_FLAG_Z ^ MASK_FLAGS))
#define CLR_N (cpu.F &= (MASK_FLAG_N ^ MASK_FLAGS))
#define CLR_H (cpu.F &= (MASK_FLAG_H ^ MASK_FLAGS))
#define CLR_C (cpu.F &= (MASK_FLAG_C ^ MASK_FLAGS))

#define IR_REG_L  ((cpu.IR >> 3) & MASK_REG_8b       )
#define IR_REG_R   (cpu.IR       & MASK_REG_8b       )
#define IR_REG_16 ((cpu.IR       & MASK_REG_16b) >> 4)

// TODO probably define others here too instead of relying on endianness
#define BC ((uint16_t) ((cpu.B << 8) | cpu.C))
#define DE ((uint16_t) ((cpu.D << 8) | cpu.E))
#define HL ((uint16_t) ((cpu.H << 8) | cpu.L))

typedef struct {
    void (*cycles[MAX_MCYCLE_INSTRUCTION])(void);
    uint8_t cycle_count;
} instruction_t;

// Needed declarations - function is defined below one that uses it
static uint8_t add_and_set_carry_flags(uint8_t, uint8_t);

gb_cpu_t cpu;

// TODO: can I inline other stuff here? I'm unclear
// TODO: get rid of nop, leave blank?
static void nop(void) {};

static inline void SP_dec()         { cpu.SP--;                                          }
static inline void HL_dec()         { uint16_t hl = HL - 1; cpu.H = hl >> 8; cpu.L = hl; }
static inline void HL_inc()         { uint16_t hl = HL + 1; cpu.H = hl >> 8; cpu.L = hl; }

/* ############################################################################
###############################################################################

        Memory reads and writes            

###############################################################################
############################################################################ */

/*
Helper functions to load to temporary 8-bit latch Z or W from various 16-bit memory locations
*/
static void memory_read_PC_Z   () { cpu.Z = cpu.bus->read(cpu.PC++);            }
static void memory_read_PC_W   () { cpu.W = cpu.bus->read(cpu.PC++);            }
static void memory_read_SP_Z   () { cpu.Z = cpu.bus->read(cpu.SP++);            }
static void memory_read_SP_W   () { cpu.W = cpu.bus->read(cpu.SP++);            }
static void memory_read_BC_Z   () { cpu.Z = cpu.bus->read(BC);                  }
static void memory_read_DE_Z   () { cpu.Z = cpu.bus->read(DE);                  }
static void memory_read_HL_Z   () { cpu.Z = cpu.bus->read(HL);                  }
static void memory_read_HLdZ   () { cpu.Z = cpu.bus->read(HL); HL_dec();        }
static void memory_read_HLiZ   () { cpu.Z = cpu.bus->read(HL); HL_inc();        }
static void memory_read_WZ_Z   () { cpu.Z = cpu.bus->read(cpu.WZ);              }
static void memory_read__C_Z   () { cpu.Z = cpu.bus->read(0xFF00 + cpu.C);      }
static void memory_read__Z_Z   () { cpu.Z = cpu.bus->read(0xFF00 + cpu.Z);      }

/*
Helper functions to write to various 16-bit memory locations, various 8-bit values
*/
static void memory_write_HL_R  () { cpu.bus->write(HL, cpu.reg[IR_REG_R]);      }
static void memory_write_HL_Z  () { cpu.bus->write(HL, cpu.Z);                  }
static void memory_write_BC_A  () { cpu.bus->write(BC, cpu.A);                  }
static void memory_write_DE_A  () { cpu.bus->write(DE, cpu.A);                  }
static void memory_write_HLdA  () { cpu.bus->write(HL, cpu.A); HL_dec();        }
static void memory_write_HLiA  () { cpu.bus->write(HL, cpu.A); HL_inc();        }
static void memory_write_WZ_A  () { cpu.bus->write(cpu.WZ, cpu.A);              }
static void memory_write_WZ_SPl() { cpu.bus->write(cpu.WZ, cpu.SP); cpu.WZ++;   }
static void memory_write_WZ_SPh() { cpu.bus->write(cpu.WZ, cpu.SP >> 8);        }
static void memory_write__C_A  () { cpu.bus->write(0xFF00 + cpu.C, cpu.A);      }
static void memory_write__Z_A  () { cpu.bus->write(0xFF00 + cpu.Z, cpu.A);      }


/*
Helper functions that need to decide between the different 16-bit registers
*/
static void
memory_write_SP_rrh(void)
{
    if (IR_REG_16 == MASK_REG_SPorAF)
        cpu.bus->write(cpu.SP--, cpu.reg[IR_REG_16 * 2]);
    else
        cpu.bus->write(cpu.SP--, cpu.A);
}

static void
memory_write_SP_rrl(void)
{
    if (IR_REG_16 == MASK_REG_SPorAF)
        cpu.bus->write(cpu.SP, cpu.reg[IR_REG_16 * 2 + 1]);
    else
        cpu.bus->write(cpu.SP, cpu.F & MASK_NIBBLE_H);
}

static void
memory_write_rr_nn (void)
{

    if (IR_REG_16 == MASK_REG_SPorAF)
        cpu.SP = (cpu.W << 8) |  cpu.Z;
    else {
        cpu.reg[IR_REG_16 * 2]     = cpu.W;
        cpu.reg[IR_REG_16 * 2 + 1] = cpu.Z;
    }
}

/* ############################################################################
###############################################################################

        Load helpers and instructions                  

###############################################################################
############################################################################ */

static void
load_HL_SPel(void)
{
    CLR_N; CLR_Z;
    add_and_set_carry_flags((uint8_t) cpu.SP, cpu.Z);
    cpu.L = cpu.SP + (int8_t) cpu.Z;
}

static void
load_HL_SPeh(void)
{
    cpu.H = (cpu.SP + *(int8_t *) &cpu.Z) >> 8; 
}

// Load 8-bit value of Z to register A
static void
load_A_Z(void)
{
    cpu.A = cpu.Z;
}

// Load 16-bit value of HL to register SP
static void
load_SP_HL(void)
{
    cpu.SP = HL;
}

// Load 16-bit value of WZ latches to register SP
static void
load_SP_WZ(void)
{
    cpu.SP = cpu.WZ;
}

// Load 16-bit value of WZ latches to register rr
static void
load_rr_WZ(void)
{
    uint8_t reg16 = IR_REG_16;
    if (reg16 == MASK_REG_SPorAF) {
        cpu.A = cpu.W;
        cpu.F = cpu.Z & MASK_NIBBLE_H;
    } else {
        cpu.reg[reg16 * 2] = cpu.W;
        cpu.reg[reg16 * 2 + 1] = cpu.Z;
    }
}

/*
Helper to load data to the 8-bit register r from latch Z
Used by LD r n, LD r (HL)
*/
static void
load_register_Z(void)
{
    cpu.reg[IR_REG_R] = cpu.Z;
}

/*
Load register (register)
LD r r' : load to the 8-bit register r, data from 8-bit register r'
*/
static void
load_register_register(void)
{
    cpu.reg[IR_REG_L] = cpu.reg[IR_REG_R];
}

// INSTRUCTIONS ###############################################################

/*
Load register (register)
LD r r': Load to the 8-bit register r, data from the 8-bit register r'
*/
static const instruction_t LD_r_r = {
    .cycles = { load_register_register },
    .cycle_count = 1
};

/*
Load register (immediate)
LD r n : load to the 8-bit register r, the immediate data n
*/
static const instruction_t LD_r_n = {
    .cycles = { memory_read_PC_Z, load_register_Z },
    .cycle_count = 2
};

/*
Load register (indirect HL)
LD r (HL): load to the 8-bit register r, data from address specified by HL
*/
static const instruction_t LD_r_HL = {
    .cycles = { memory_read_HL_Z, load_register_Z },
    .cycle_count = 2
};

/* 
Load from register (indirect HL)
LD (HL) r: load to the address specified by HL, data from the 8-bit register r
*/
static const instruction_t LD_HL_r = {
    .cycles = { memory_write_HL_R, nop },
    .cycle_count = 2
};

/*
Load from immediate data (indirect HL)
LD (HL) n: load to the address specified by HL, the immediate data n
*/
static const instruction_t LD_HL_n = {
    .cycles = { memory_read_PC_Z, memory_write_HL_Z, nop },
    .cycle_count = 3
};

/*
Load accumulator (indirect BC)
LD A (BC): load to the 8-bit register A, data from address specified by BC
*/
static const instruction_t LD_A_BC = {
    .cycles = { memory_read_BC_Z, load_A_Z },
    .cycle_count = 2
};

/*
Load accumulator (indirect DE)
LD A (DE): load to the 8-bit register A, data from address specified by DE
*/
static const instruction_t LD_A_DE = {
    .cycles = { memory_read_DE_Z, load_A_Z },
    .cycle_count = 2
};

/*
Load from accumulator (indirect BC)
LD (BC) A: load to the address specified by BC, data from 8-bit register A
*/
static const instruction_t LD_BC_A = {
    .cycles = { memory_write_BC_A, nop },
    .cycle_count = 2
};

/*
Load from accumulator (indirect DE)
LD (DE) A: load to the address specified by DE, data from the 8-bit register A
*/
static const instruction_t LD_DE_A = {
    .cycles = { memory_write_DE_A, nop },
    .cycle_count = 2
};

/*
Load accumulator (direct)
LD A (nn): load to the 8-bit register A, data from the address specified by nn
*/
static const instruction_t LD_A_nn = {
    .cycles = { memory_read_PC_Z, memory_read_PC_W, memory_read_WZ_Z, load_A_Z },
    .cycle_count = 4
};

/*
Load from accumulator (direct)
LD (nn) A: load to the address specified by nn, data from the 8-bit register A
*/
static const instruction_t LD_nn_A = {
    .cycles = { memory_read_PC_Z, memory_read_PC_W, memory_write_WZ_A, nop },
    .cycle_count = 4
};

/*
Load accumulator (indirect 0xFF00 + C)
LDH A (C): load to the 8-bit register A, data from the address 0xFF00 + C
*/
static const instruction_t LDH_A__C = {
    .cycles = { memory_read__C_Z, load_A_Z },
    .cycle_count = 2
};

/*
Load from accumulator (indirect 0xFF00 + C)
LDH (C) A: load to the address 0xFF00 + C, data from the 8-bit register A
*/
static const instruction_t LDH__C_A = {
    .cycles = { memory_write__C_A, nop },
    .cycle_count = 2
};

/*
Load accumulator (direct 0xFF00 + n)
LDH A (n): load to the 8-bit register A, data from the address 0xFF00 + n
*/
static const instruction_t LDH_A__n = {
    .cycles = { memory_read_PC_Z, memory_read__Z_Z, load_A_Z },
    .cycle_count = 3
};

/*
Load from accumulator (direct 0xFF00 + n)
LDH (n) A: load to the address 0xFF00 + n, data from the 8-bit register A
*/
static const instruction_t LDH__n_A = {
    .cycles = { memory_read_PC_Z, memory_write__Z_A, nop },
    .cycle_count = 3
};

/*
Load accumulator (indirect HL, decrement)
LD A (HL-): load to the 8-bit register A, data from address specified by HL, decrement HL after
*/
static const instruction_t LD_A_HLd = {
    .cycles = { memory_read_HLdZ, load_A_Z },
    .cycle_count = 2
};

/*
Load from accumulator (indirect HL, decrement)
LD (HL-) A: load to the address HL, data from the 8-bit register A, decrement HL after
*/
static const instruction_t LD_HLd_A = {
    .cycles = { memory_write_HLdA, nop },
    .cycle_count = 2
};

/*
Load accumulator (indirect HL, increment)
LD A (HL+): load to the 8-bit register A, data from address specified by HL, increment HL after
*/
static const instruction_t LD_A_HLi = {
    .cycles = { memory_read_HLiZ, load_A_Z },
    .cycle_count = 2
};

/*
Load from accumulator (indirect HL, increment)
LD (HL+) A: load to the address HL, data from the 8-bit register A, increment HL after
*/
static const instruction_t LD_HLi_A = {
    .cycles = { memory_write_HLiA, nop },
    .cycle_count = 2
};

/*
Load 16-bit register / register pair
LD rr nn: Load to the 16-bit register rr, the immediate 16-bit data nn
*/
static const instruction_t LD_rr_nn = {
    .cycles = { memory_read_PC_Z, memory_read_PC_W, memory_write_rr_nn },
    .cycle_count = 3
};

/*
Load from stack pointer, direct
LD (nn) SP: Load to the 16-bit address specified by the 16-bit value nn, data from the 16-bit register SP
*/
static const instruction_t LD_nn_SP = {
    .cycles = { memory_read_PC_Z, memory_read_PC_W, memory_write_WZ_SPl, memory_write_WZ_SPh, nop },
    .cycle_count = 5
};

/*
Load stack pointer from HL
LD SP HL: Load to the 16-bit register SP, data from the 16-bit register HL
*/
static const instruction_t LD_SP_HL = {
    .cycles = { load_SP_HL, nop },
    .cycle_count = 2
};

/*
Push to stack
PUSH rr: Push to the stack memory, data from the 16-bit register rr
*/
static const instruction_t PUSH_rr = {
    .cycles = { SP_dec, memory_write_SP_rrh, memory_write_SP_rrl, nop},
    .cycle_count = 4
};

/*
Pop from stack
POP rr: Pops to the 16-bit register rr, data from the stack memory
*/
static const instruction_t POP_rr = {
    .cycles = { memory_read_SP_Z, memory_read_SP_W, load_rr_WZ },
    .cycle_count = 3
};

/*
Load HL from adjusted stack pointer
LD HL SP+e: Load to the HL register, 16 bit data calculated by adding the signed 8-bit operand e to the 16-bit
value of the SP register
*/
static const instruction_t LD_HL_SPe = {
    .cycles = { memory_read_PC_Z, load_HL_SPel, load_HL_SPel },
    .cycle_count = 3
};

/* ############################################################################
###############################################################################

        Add and add with carry helpers and instructions                  

###############################################################################
############################################################################ */

/*
Helper for adding two 8-bit values and setting both carry registers
*/
static uint8_t 
add_and_set_carry_flags(uint8_t val1, uint8_t val2)
{
    uint8_t  nibble = (val1 & MASK_NIBBLE_L) + (val2 & MASK_NIBBLE_L);
    if (nibble > MASK_NIBBLE_L) SET_H; else CLR_H;

    uint16_t result = val1 + val2;
    if (result > MASK_BYTE) SET_C; else CLR_C;

    return (uint8_t) result;
}

static void
add_to_A(uint8_t val)
{
    CLR_N;
    cpu.A = add_and_set_carry_flags(cpu.A, val);
    if (cpu.A) CLR_Z; else SET_Z;
}

static void add_r_to_A  () { add_to_A(cpu.reg[IR_REG_R]);                    }
static void adc_r_to_A  () { add_to_A(cpu.reg[IR_REG_R] + FLAG_C);           }
static void add_Z_to_A  () { add_to_A(cpu.Z);                                }
static void adc_Z_to_A  () { add_to_A(cpu.Z + FLAG_C);                       }
static void add_e_to_SPl() { add_and_set_carry_flags(cpu.SP, cpu.Z);         }  // Cheating a 'lil with Z latch
static void add_e_to_SPh() { cpu.WZ = cpu.SP + (int8_t) cpu.Z; CLR_N; CLR_Z; }

static void
add_to_reg(uint8_t *reg_addr, uint8_t val)
{
    CLR_N;
    *reg_addr = add_and_set_carry_flags(*reg_addr, val);
}

static void
add_rr_to_HLl()
{ 
    if (IR_REG_16 == MASK_REG_SPorAF)
        add_to_reg(&cpu.L, cpu.SP);
    else
        add_to_reg(&cpu.L, cpu.reg[IR_REG_16 * 2 + 1]);
}

static void
add_rr_to_HLh()
{ 
    if (IR_REG_16 == MASK_REG_SPorAF)
        add_to_reg(&cpu.H, cpu.SP >> 8);
    else
        add_to_reg(&cpu.H, cpu.reg[IR_REG_16 * 2]);
}

// INSTRUCTIONS ###############################################################

/*
Add (register)
ADD r: Adds to the 8-bit A register, the 8-bit r register, and stores the result back into the A register
*/
static const instruction_t ADD_r = {
    .cycles = { add_r_to_A },
    .cycle_count = 1
};

/*
Add with carry (register) 
ADC r: Adds to the 8-bit A register, the carry flag and the 8-bit r register, and
stores the result back into the A register
*/
static const instruction_t ADC_r = {
    .cycles = { adc_r_to_A },
    .cycle_count = 1
};

/*
Add (indirect HL)
ADD (HL): Adds to the 8-bit A register, data from the address specified by the 16-bit register HL, and stores
the result back into the A register
*/
static const instruction_t ADD_HL = {
    .cycles = { memory_read_HL_Z, add_Z_to_A },
    .cycle_count = 2
};

/*
Add with carry (indirect HL)
ADC (HL): Adds to the 8-bit A register,the carry flag and data from the address specified by the 16-bit
register HL, and stores the result back into the A register
*/
static const instruction_t ADC_HL = {
    .cycles = { memory_read_HL_Z, adc_Z_to_A },
    .cycle_count = 2
};

/*
Add (immediate)
ADD n: Adds to the 8-bit register A, the immediate data n, and stores the result back into the A register
*/
static const instruction_t ADD_n = {
    .cycles = { memory_read_PC_Z, add_Z_to_A },
    .cycle_count = 2
};

/*
Add with carry (immediate)
ADC n: Adds to the 8-bit register A, the carry flag and the immediate data n, and stores the result back into
the A register
*/
static const instruction_t ADC_n = {
    .cycles = { memory_read_PC_Z, adc_Z_to_A },
    .cycle_count = 2
};

/*
Add (16-bit register)
ADD HL rr: Adds to the 16-bit HL register, the 16-bit rr register, and stores the result back into HL
*/
static const instruction_t ADD_HL_rr = {
    .cycles = { add_rr_to_HLl, add_rr_to_HLh },
    .cycle_count = 2
};

/*
Add to stack pointer (relative)
ADD SP e: Loads to the 16-bit SP register, data calculated by adding the signed 8-bit operand e to the 16-bit
value of the SP register
*/
static const instruction_t ADD_SP_e = {
    .cycles = { memory_read_PC_Z, add_e_to_SPl, add_e_to_SPh, load_SP_WZ },
    .cycle_count = 4
};

/* ############################################################################
###############################################################################

        Subtract, subtract with carry, and compare helpers and instructions                  

###############################################################################
############################################################################ */

/*
Helper for subtracting two 8-bit values and setting both carry registers
*/
static uint8_t 
sub_and_set_carry_flags(uint8_t val1, uint16_t val2)
{
    if ((val1 & MASK_NIBBLE_L) < (val2 & MASK_NIBBLE_L)) SET_H; else CLR_H;
    if (val1 < val2) SET_C; else CLR_C;

    return (uint8_t) (val1 - val2);
}

static void
sub_from_A(uint16_t val)
{
    SET_N;
    cpu.A = sub_and_set_carry_flags(cpu.A, val);
    if (cpu.A) CLR_Z; else SET_Z;
}
static void sub_r_from_A() { sub_from_A(cpu.reg[IR_REG_R]);          }
static void sbc_r_from_A() { sub_from_A(cpu.reg[IR_REG_R] + FLAG_C); }
static void sub_Z_from_A() { sub_from_A(cpu.Z);                      }
static void sbc_Z_from_A() { sub_from_A(cpu.Z + FLAG_C);             }

/*
Helper for compare instructions, similar to subtraction but without updating the A register
*/
static void
cp_with_A(uint16_t val)
{
    SET_N;
    uint8_t result = sub_and_set_carry_flags(cpu.A, val);
    if (result) CLR_Z; else SET_Z;
}

static void cp_r_with_A()  { cp_with_A(cpu.reg[IR_REG_R]);           }
static void cp_Z_with_A()  { cp_with_A(cpu.Z);                       }

// INSTRUCTIONS ###############################################################

/*
Subtract (register)
SUB r: Subtracts from the 8-bit A register, the 8-bit r register, and stores the result back into the A register
*/
static const instruction_t SUB_r = {
    .cycles = { sub_r_from_A },
    .cycle_count = 1
};

/*
Subtract with carry (register) 
SBC r: Subtracts from the 8-bit A register, the carry flag and the 8-bit r register, and stores the result back
into the A register
*/
static const instruction_t SBC_r = {
    .cycles = { sbc_r_from_A },
    .cycle_count = 1
};

/*
Subtract (indirect HL)
SUB (HL): Subtracts from the 8-bit A register, data from the address specified by the 16-bit register HL, and
stores the result back into the A register
*/
static const instruction_t SUB_HL = {
    .cycles = { memory_read_HL_Z, sub_Z_from_A },
    .cycle_count = 2
};

/*
Subtract with carry (indirect HL)
SBC (HL): Subtracts from the 8-bit A register,the carry flag and data from the address specified by the 16-bit
register HL, and stores the result back into the A register
*/
static const instruction_t SBC_HL = {
    .cycles = { memory_read_HL_Z, sbc_Z_from_A },
    .cycle_count = 2
};

/*
Subtract (immediate)
SUB n: Subtracts from the 8-bit register A, the immediate data n, and stores the result back into the A register
*/
static const instruction_t SUB_n = {
    .cycles = { memory_read_PC_Z, sub_Z_from_A },
    .cycle_count = 2
};

/*
Subtract with carry (immediate)
SBC n: Subtracts from the 8-bit register A, the carry flag and the immediate data n, and stores the result back
into the A register
*/
static const instruction_t SBC_n = {
    .cycles = { memory_read_PC_Z, adc_Z_to_A },
    .cycle_count = 2
};

/*
Compare (register)
CP r: Subtracts from the 8-bit A register, the 8-bit register r, and updates flags based on the result; This
instruction is basically identical to SUB r, but doesn't update the A register
*/
static const instruction_t CP_r = {
    .cycles = { cp_r_with_A },
    .cycle_count = 1
};

/*
Compare (indirect HL)
CP (HL): Subtracts from the 8-bit A register, data from the address specified by the 16-bit register HL, and
updates flags based on the result; This instruction is basically identical to SUB (HL) , but doesn't update the
A register
*/
static const instruction_t CP_HL = {
    .cycles = { memory_read_HL_Z, cp_Z_with_A },
    .cycle_count = 2
};

/* 
Compare (immediate)
CP n: Subtracts from the 8-bit register A, the immediate data n, and updates flags based on the result; This
instruction is basically identical to SUB n, but doesn't update the A register
*/
static const instruction_t CP_n = {
    .cycles = { memory_read_PC_Z, cp_Z_with_A },
    .cycle_count = 2
};

/* ############################################################################
###############################################################################

        Increment and decrement helpers and instructions                  

###############################################################################
############################################################################ */

static void
inc_8bit(uint8_t *data)
{
    if ((*data & MASK_NIBBLE_L) == MASK_NIBBLE_L) SET_H; else CLR_H;
    if (++(*data)) CLR_Z; else SET_Z;
    CLR_N;
}

static void
dec_8bit(uint8_t *data)
{
    if ((*data & MASK_NIBBLE_L) == MASK_ZERO) SET_H; else CLR_H;
    if (--(*data)) CLR_Z; else SET_Z;
    SET_N;
}

static void
inc_or_dec_16bit(int8_t change)
{
    uint8_t reg16 = IR_REG_16;
    if (reg16 == MASK_REG_SPorAF) {
        cpu.SP += change;
    } else {
        uint16_t result = (cpu.reg[reg16 * 2] << 8) + cpu.reg[reg16 * 2 + 1] + change;
        cpu.reg[reg16 * 2    ] = (uint8_t) (result >> 8);
        cpu.reg[reg16 * 2 + 1] = (uint8_t)  result;
    }
}

static void inc_r()              { inc_8bit(&cpu.reg[IR_REG_L]);          }
static void dec_r()              { dec_8bit(&cpu.reg[IR_REG_L]);          }
static void inc_Z_mem_write_HL() { inc_8bit(&cpu.Z); memory_write_HL_Z(); }
static void dec_Z_mem_write_HL() { dec_8bit(&cpu.Z); memory_write_HL_Z(); }
static void inc_rr()             { inc_or_dec_16bit(+1);                  }
static void dec_rr()             { inc_or_dec_16bit(-1);                  }

// INSTRUCTIONS ###############################################################

/*
Increment (register)
INC r: increments data in the 8-bit register r
*/
static const instruction_t INC_r = {
    .cycles = { inc_r },
    .cycle_count = 1
};

/*
Increment (indirect HL)
INC (HL) increments data at the address specified by the 16-bit register HL
*/
static const instruction_t INC_HL = {
    .cycles = { memory_read_HL_Z, inc_Z_mem_write_HL, nop },
    .cycle_count = 3
};

/*
Decrement (register)
DEC r: decrements data in the 8-bit register r
*/
static const instruction_t DEC_r = {
    .cycles = { dec_r },
    .cycle_count = 1
};

/*
Decrement (indirect HL)
DEC (HL) decrements data at the address specified by the 16-bit register HL
*/
static const instruction_t DEC_HL = {
    .cycles = { memory_read_HL_Z, dec_Z_mem_write_HL, nop },
    .cycle_count = 3
};

/*
Increment 16-bit register
INC rr: increments data in the 16-bit register rr
*/
static const instruction_t INC_rr = {
    .cycles = { inc_rr, nop },
    .cycle_count = 2
};

/*
Decrement 16-bit register
DEC rr: decrements data in the 16-bit register rr
*/
static const instruction_t DEC_rr = {
    .cycles = { dec_rr, nop },
    .cycle_count = 2
};

/* ############################################################################
###############################################################################

        logical AND, OR, XOR helpers and instructions

###############################################################################
############################################################################ */

static void
and_val_with_A(uint8_t val)
{
    CLR_N; SET_H; CLR_C;
    cpu.A = cpu.A & val;
    if (cpu.A) CLR_Z; else SET_Z;
}

static void
or_val_with_A(uint8_t val)
{
    CLR_N; CLR_H; CLR_C;
    cpu.A = cpu.A | val;
    if (cpu.A) CLR_Z; else SET_Z;
}

static void
xor_val_with_A(uint8_t val)
{
    CLR_N; CLR_H; CLR_C;
    cpu.A = cpu.A ^ val;
    if (cpu.A) CLR_Z; else SET_Z;
}

static void and_r_with_A() { and_val_with_A(cpu.reg[IR_REG_R]); }
static void and_Z_with_A() { and_val_with_A(cpu.Z);             }
static void  or_r_with_A() {  or_val_with_A(cpu.reg[IR_REG_R]); }
static void  or_Z_with_A() {  or_val_with_A(cpu.Z);             }
static void xor_r_with_A() { xor_val_with_A(cpu.reg[IR_REG_R]); }
static void xor_Z_with_A() { xor_val_with_A(cpu.Z);             }


// INSTRUCTIONS ###############################################################

/*
Bitwise AND (register)
AND r: Performs a bitwise AND operation between the 8-bit A register and the 8-bit r register, and stores the
result back into the A register
*/
static const instruction_t AND_r = {
    .cycles = { and_r_with_A },
    .cycle_count = 1
};

/*
Bitwise AND (indirect HL)
AND (HL): Performs a bitwise AND operation between the 8-bit A register and data at the address specified by the
16-bit register HL, and stores the result back into the A register
*/
static const instruction_t AND_HL = {
    .cycles = { memory_read_HL_Z, and_Z_with_A },
    .cycle_count = 2
};

/*
Bitwise AND (immediate)
AND n: Performs a bitwise AND operation between the 8-bit A register and immediate data n, and stores the result
back into the A register
*/
static const instruction_t AND_n = {
    .cycles = { memory_read_PC_Z, and_Z_with_A },
    .cycle_count = 2
};

/*
Bitwise OR (register)
OR r: Performs a bitwise OR operation between the 8-bit A register and the 8-bit r register, and stores the
result back into the A register
*/
static const instruction_t OR_r = {
    .cycles = { or_r_with_A },
    .cycle_count = 1
};

/*
Bitwise OR (indirect HL)
OR (HL): Performs a bitwise OR operation between the 8-bit A register and data at the address specified by the
16-bit register HL, and stores the result back into the A register
*/
static const instruction_t OR_HL = {
    .cycles = { memory_read_HL_Z, or_Z_with_A },
    .cycle_count = 2
};

/*
Bitwise OR (immediate)
OR n: Performs a bitwise OR operation between the 8-bit A register and immediate data n, and stores the result
back into the A register
*/
static const instruction_t OR_n = {
    .cycles = { memory_read_PC_Z, or_Z_with_A },
    .cycle_count = 2
};

/*
Bitwise XOR (register)
XOR r: Performs a bitwise XOR operation between the 8-bit A register and the 8-bit r register, and stores the
result back into the A register
*/
static const instruction_t XOR_r = {
    .cycles = { xor_r_with_A },
    .cycle_count = 1
};

/*
Bitwise XOR (indirect HL)
XOR (HL): Performs a bitwise XOR operation between the 8-bit A register and data at the address specified by the
16-bit register HL, and stores the result back into the A register
*/
static const instruction_t XOR_HL = {
    .cycles = { memory_read_HL_Z, xor_Z_with_A },
    .cycle_count = 2
};

/*
Bitwise XOR (immediate)
XOR n: Performs a bitwise XOR operation between the 8-bit A register and immediate data n, and stores the result
back into the A register
*/
static const instruction_t XOR_n = {
    .cycles = { memory_read_PC_Z, xor_Z_with_A },
    .cycle_count = 2
};

/* ############################################################################
###############################################################################

        Misc. Flag and complement instructions

###############################################################################
############################################################################ */

static void        set_carry_flag () {CLR_N; CLR_H; SET_C;                }
static void complement_carry_flag () {CLR_N; CLR_H; cpu.F ^= MASK_FLAG_C; }
static void complement_accumulator() {SET_N; SET_H; cpu.A ^= MASK_BYTE;   }

// INSTRUCTIONS ###############################################################

/*
Complement carry flag
CCF: Flips the carry flag, and clears and N and H flags
*/
static const instruction_t CCF = {
    .cycles = { complement_carry_flag },
    .cycle_count = 1
};

/*
Set carry flag
SCF: Sets the carry flag, and clears and N and H flags
*/
static const instruction_t SCF = {
    .cycles = { set_carry_flag },
    .cycle_count = 1
};

/*
Complement accumulator
CPL: Flips all the bits in the 8-bit A register, and sets the N and H flags
*/
static const instruction_t CPL = {
    .cycles = { complement_accumulator },
    .cycle_count = 1
};

/* ############################################################################
###############################################################################

        Rotate, shift, and bit operation instructions

###############################################################################
############################################################################ */

static void
rlca()
{ 
    CLR_Z; CLR_N; CLR_H;
    if (cpu.A >> 7) SET_C; else CLR_C;
    cpu.A = (cpu.A << 1) | FLAG_C;
}

static void
rrca()
{
    CLR_Z; CLR_N; CLR_H;
    if (cpu.A & MASK_BIT) SET_C; else CLR_C;
    cpu.A = (cpu.A >> 1) | (FLAG_C << 7);
}

static void
rla()
{
    CLR_Z; CLR_N; CLR_H;
    uint8_t temp = (cpu.A >> 7);
    cpu.A = (cpu.A << 1) | FLAG_C;
    if (temp) SET_C; else CLR_C;
}

static void
rra()
{
    CLR_Z; CLR_N; CLR_H;
    uint8_t temp = (cpu.A & MASK_BIT);
    cpu.A = (cpu.A >> 1) | (FLAG_C << 7);
    if (temp) SET_C; else CLR_C;
}

// INSTRUCTIONS ###############################################################

/*
Rotate left circular (accumulator)
RLCA: Rotates the 8-bit A register value left in a circular manner
*/
static const instruction_t RLCA = {
    .cycles = { rlca },
    .cycle_count = 1
};

/*
Rotate right circular (accumulator)
RRCA: Rotates the 8-bit A register value right in a circular manner
*/
static const instruction_t RRCA = {
    .cycles = { rrca },
    .cycle_count = 1
};

/*
Rotate left (accumulator)
RLA: Rotates the 8-bit A register value left through the carry flag
*/
static const instruction_t RLA = {
    .cycles = { rla },
    .cycle_count = 1
};

/*
Rotate right (accumulator)
RRA: Rotates the 8-bit A register value right through the carry flag
*/
static const instruction_t RRA = {
    .cycles = { rra },
    .cycle_count = 1
};

/* ############################################################################
###############################################################################

        STUFF

###############################################################################
############################################################################ */

void
do_machine_cycle(void)
{

}

int
main(void)
{

}

static const instruction_t OPCODE_TABLE[256] = {
    [0x00] = ,
    [0x01] = LD_rr_nn,
    [0x02] = LD_BC_A,
    [0x03] = INC_rr,
    [0x04] = INC_r,
    [0x05] = DEC_r,
    [0x06] = LD_r_n,
    [0x07] = RLCA,
    [0x08] = LD_nn_SP,
    [0x09] = ADD_HL_rr,
    [0x0A] = LD_A_BC,
    [0x0B] = DEC_rr,
    [0x0C] = INC_r,
    [0x0D] = DEC_r,
    [0x0E] = LD_r_n,
    [0x0F] = RRCA,

    [0x10] = ,
    [0x11] = LD_rr_nn,
    [0x12] = LD_DE_A,
    [0x13] = INC_rr,
    [0x14] = INC_r,
    [0x15] = DEC_r,
    [0x16] = LD_r_n,
    [0x17] = RLA,
    [0x18] = ,
    [0x19] = ADD_HL_rr,
    [0x1A] = LD_A_DE,
    [0x1B] = DEC_rr,
    [0x1C] = INC_r,
    [0x1D] = DEC_r,
    [0x1E] = LD_r_n,
    [0x1F] = RRA,

    [0x20] = ,
    [0x21] = LD_rr_nn,
    [0x22] = LD_HLi_A,
    [0x23] = INC_rr,
    [0x24] = INC_r,
    [0x25] = DEC_r,
    [0x26] = LD_r_n,
    [0x27] = ,
    [0x28] = ,
    [0x29] = ADD_HL_rr,
    [0x2A] = LD_A_HLi,
    [0x2B] = DEC_rr,
    [0x2C] = INC_r,
    [0x2D] = DEC_r,
    [0x2E] = LD_r_n,
    [0x2F] = CPL,

    [0x30] = ,
    [0x31] = LD_rr_nn,
    [0x32] = LD_HLd_A,
    [0x33] = INC_rr,
    [0x34] = INC_HL,
    [0x35] = DEC_HL,
    [0x36] = LD_HL_n,
    [0x37] = SCF,
    [0x38] = ,
    [0x39] = ADD_HL_rr,
    [0x3A] = LD_A_HLd,
    [0x3B] = DEC_rr,
    [0x3C] = INC_r,
    [0x3D] = DEC_r,
    [0x3E] = LD_r_n,
    [0x3F] = CCF,

    [0x40] = LD_r_r,
    [0x41] = LD_r_r,
    [0x42] = LD_r_r,
    [0x43] = LD_r_r,
    [0x44] = LD_r_r,
    [0x45] = LD_r_r,
    [0x46] = LD_r_HL,
    [0x47] = LD_r_r,
    [0x48] = LD_r_r,
    [0x49] = LD_r_r,
    [0x4A] = LD_r_r,
    [0x4B] = LD_r_r,
    [0x4C] = LD_r_r,
    [0x4D] = LD_r_r,
    [0x4E] = LD_r_HL,
    [0x4F] = LD_r_r,

    [0x50] = LD_r_r,
    [0x51] = LD_r_r,
    [0x52] = LD_r_r,
    [0x53] = LD_r_r,
    [0x54] = LD_r_r,
    [0x55] = LD_r_r,
    [0x56] = LD_r_HL,
    [0x57] = LD_r_r,
    [0x58] = LD_r_r,
    [0x59] = LD_r_r,
    [0x5A] = LD_r_r,
    [0x5B] = LD_r_r,
    [0x5C] = LD_r_r,
    [0x5D] = LD_r_r,
    [0x5E] = LD_r_HL,
    [0x5F] = LD_r_r,

    [0x60] = LD_r_r,
    [0x61] = LD_r_r,
    [0x62] = LD_r_r,
    [0x63] = LD_r_r,
    [0x64] = LD_r_r,
    [0x65] = LD_r_r,
    [0x66] = LD_r_HL,
    [0x67] = LD_r_r,
    [0x68] = LD_r_r,
    [0x69] = LD_r_r,
    [0x6A] = LD_r_r,
    [0x6B] = LD_r_r,
    [0x6C] = LD_r_r,
    [0x6D] = LD_r_r,
    [0x6E] = LD_r_HL,
    [0x6F] = LD_r_r,

    [0x70] = LD_HL_r,
    [0x71] = LD_HL_r,
    [0x72] = LD_HL_r,
    [0x73] = LD_HL_r,
    [0x74] = LD_HL_r,
    [0x75] = LD_HL_r,
    [0x76] = ,
    [0x77] = LD_HL_r,
    [0x78] = LD_r_r,
    [0x79] = LD_r_r,
    [0x7A] = LD_r_r,
    [0x7B] = LD_r_r,
    [0x7C] = LD_r_r,
    [0x7D] = LD_r_r,
    [0x7E] = LD_r_HL,
    [0x7F] = LD_r_r,

    [0x80] = ADD_r,
    [0x81] = ADD_r,
    [0x82] = ADD_r,
    [0x83] = ADD_r,
    [0x84] = ADD_r,
    [0x85] = ADD_r,
    [0x86] = ADD_HL,
    [0x87] = ADD_r,
    [0x88] = ADC_r,
    [0x89] = ADC_r,
    [0x8A] = ADC_r,
    [0x8B] = ADC_r,
    [0x8C] = ADC_r,
    [0x8D] = ADC_r,
    [0x8E] = ADC_HL,
    [0x8F] = ADC_r,

    [0x90] = SUB_r,
    [0x91] = SUB_r,
    [0x92] = SUB_r,
    [0x93] = SUB_r,
    [0x94] = SUB_r,
    [0x95] = SUB_r,
    [0x96] = SUB_HL,
    [0x97] = SUB_r,
    [0x98] = SBC_r,
    [0x99] = SBC_r,
    [0x9A] = SBC_r,
    [0x9B] = SBC_r,
    [0x9C] = SBC_r,
    [0x9D] = SBC_r,
    [0x9E] = SBC_HL,
    [0x9F] = SBC_r,

    [0xA0] = AND_r,
    [0xA1] = AND_r,
    [0xA2] = AND_r,
    [0xA3] = AND_r,
    [0xA4] = AND_r,
    [0xA5] = AND_r,
    [0xA6] = AND_HL,
    [0xA7] = AND_r,
    [0xA8] = XOR_r,
    [0xA9] = XOR_r,
    [0xAA] = XOR_r,
    [0xAB] = XOR_r,
    [0xAC] = XOR_r,
    [0xAD] = XOR_r,
    [0xAE] = XOR_HL,
    [0xAF] = XOR_r,

    [0xB0] = OR_r,
    [0xB1] = OR_r,
    [0xB2] = OR_r,
    [0xB3] = OR_r,
    [0xB4] = OR_r,
    [0xB5] = OR_r,
    [0xB6] = OR_HL,
    [0xB7] = OR_r,
    [0xB8] = CP_r,
    [0xB9] = CP_r,
    [0xBA] = CP_r,
    [0xBB] = CP_r,
    [0xBC] = CP_r,
    [0xBD] = CP_r,
    [0xBE] = CP_HL,
    [0xBF] = CP_r,

    [0xC0] = ,
    [0xC1] = POP_rr,
    [0xC2] = ,
    [0xC3] = ,
    [0xC4] = ,
    [0xC5] = PUSH_rr,
    [0xC6] = ADD_n,
    [0xC7] = ,
    [0xC8] = ,
    [0xC9] = ,
    [0xCA] = ,
    [0xCB] = ,
    [0xCC] = ,
    [0xCD] = ,
    [0xCE] = ADC_n,
    [0xCF] = ,

    [0xD0] = ,
    [0xD1] = POP_rr,
    [0xD2] = ,
    [0xD3] = ,
    [0xD4] = ,
    [0xD5] = PUSH_rr,
    [0xD6] = SUB_n,
    [0xD7] = ,
    [0xD8] = ,
    [0xD9] = ,
    [0xDA] = ,
    [0xDB] = ,
    [0xDC] = ,
    [0xDD] = ,
    [0xDE] = SBC_n,
    [0xDF] = ,

    [0xE0] = LDH__n_A,
    [0xE1] = POP_rr,
    [0xE2] = LDH__C_A,
    [0xE3] = ,
    [0xE4] = ,
    [0xE5] = PUSH_rr,
    [0xE6] = AND_n,
    [0xE7] = ,
    [0xE8] = ADD_SP_e,
    [0xE9] = ,
    [0xEA] = LD_nn_A,
    [0xEB] = ,
    [0xEC] = ,
    [0xED] = ,
    [0xEE] = XOR_n,
    [0xEF] = ,

    [0xF0] = LDH_A__n,
    [0xF1] = POP_rr,
    [0xF2] = LDH_A__C,
    [0xF3] = ,
    [0xF4] = ,
    [0xF5] = PUSH_rr,
    [0xF6] = OR_n,
    [0xF7] = ,
    [0xF8] = LD_HL_SPe,
    [0xF9] = LD_SP_HL,
    [0xFA] = LD_A_nn,
    [0xFB] = ,
    [0xFC] = ,
    [0xFD] = ,
    [0xFE] = CP_n,
    [0xFF] = ,
};