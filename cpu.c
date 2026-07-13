#include "bus.h"
#include "cpu.h"

#define MAX_MCYCLE_INSTRUCTION 4

#define REG_MASK 0b00000111
#define IR_REG_L ((cpu.IR >> 3) & REG_MASK)
#define IR_REG_R  (cpu.IR       & REG_MASK)

#define BC ((uint16_t) ((cpu.B << 8) | cpu.C))
#define DE ((uint16_t) ((cpu.D << 8) | cpu.E))
#define HL ((uint16_t) ((cpu.H << 8) | cpu.L))
#define WZ ((uint16_t) ((cpu.W << 8) | cpu.Z))


typedef struct {
    void (*cycles[MAX_MCYCLE_INSTRUCTION])(void);
    uint8_t cycle_count;
} instruction_t;

gb_cpu_t cpu;

void nop(void) {};

/*
Helper functions to load to temporary 8-bit latch Z from various 16-bit memory locations
*/
cpu_read_memory_PC_W()  { cpu.W = cpu.bus->read(cpu.PC++);       }
cpu_read_memory_PC_Z()  { cpu.Z = cpu.bus->read(cpu.PC++);       }
cpu_read_memory_BC_Z()  { cpu.Z = cpu.bus->read(BC);             }
cpu_read_memory_DE_Z()  { cpu.Z = cpu.bus->read(DE);             }
cpu_read_memory_HL_Z()  { cpu.Z = cpu.bus->read(HL);             }
cpu_read_memory_WZ_Z()  { cpu.Z = cpu.bus->read(WZ);             }
cpu_read_memory__C_Z()  { cpu.Z = cpu.bus->read(0xFF00 + cpu.C); }


/*
Helper functions to write to various 16-bit memory locations, various 8-bit values
*/
cpu_write_memory_HL_R() { cpu.bus->write(HL, cpu.reg[IR_REG_R]); }
cpu_write_memory_HL_Z() { cpu.bus->write(HL, cpu.Z);             }
cpu_write_memory_BC_A() { cpu.bus->write(BC, cpu.A);             }
cpu_write_memory_DE_A() { cpu.bus->write(DE, cpu.A);             }
cpu_write_memory_WZ_A() { cpu.bus->write(WZ, cpu.A);             }
cpu_write_memory__C_A() { cpu.bus->write(0xFF00 + cpu.C, cpu.A); }


// Load 8-bit value of Z to register A
void
LD_load_A_Z(void)
{
    cpu.A = cpu.Z;
}

/*
Helper to load data to the 8-bit register r from latch Z
Used by LD r n, LD r (HL)
*/


void
LD_load_register_Z(void)
{
    cpu.reg[IR_REG_R] = cpu.Z;
}

/*
Load register (register)
LD r r' : load to the 8-bit register r, data from 8-bit register r'
*/
void
LD_load_register_register(void)
{
    cpu.reg[IR_REG_L] = cpu.reg[IR_REG_R];
}

const instruction_t LD_r_r = {
    .cycles = { LD_load_register_register },
    .cycle_count = 1
};

/*
Load register (immediate)
LD r n : load to the 8-bit register r, the immediate data n
*/
const instruction_t LD_r_n = {
    .cycles = { cpu_read_memory_PC_Z, LD_load_register_Z },
    .cycle_count = 2
};

/*
Load register (indirect HL)
LD r (HL): load to the 8-bit register r, data from address specified by HL
*/
const instruction_t LD_r_HL = {
    .cycles = { cpu_read_memory_HL_Z, LD_load_register_Z },
    .cycle_count = 2
};

/* 
Load from register (indirect HL)
LD (HL) r: load to the address specified by HL, data from the 8-bit register r
*/
const instruction_t LD_HL_r = {
    .cycles = { cpu_write_memory_HL_R, nop },
    .cycle_count = 2
};

/*
Load from immediate data (indirect HL)
LD (HL) n: load to the address specified by HL, the immediate data n
*/
const instruction_t LD_HL_n = {
    .cycles = { cpu_read_memory_PC_Z, cpu_write_memory_HL_Z, nop },
    .cycle_count = 3
};

/*
Load accumulator (indirect BC)
LD A (BC): load to the 8-bit register A, data from address specified by BC
*/
const instruction_t LD_A_BC = {
    .cycles = { cpu_read_memory_BC_Z, LD_load_A_Z },
    .cycle_count = 2
};

/*
Load accumulator (indirect DE)
LD A (DE): load to the 8-bit register A, data from address specified by DE
*/
const instruction_t LD_A_DE = {
    .cycles = { cpu_read_memory_DE_Z, LD_load_A_Z },
    .cycle_count = 2
};

/*
Load from accumulator (indirect BC)
LD (BC) A: load to the address specified by BC, data from 8-bit register A
*/
const instruction_t LD_BC_A = {
    .cycles = { cpu_write_memory_BC_A, nop },
    .cycle_count = 2
};

/*
Load from accumulator (indirect DE)
LD (DE) A: load to the address specified by DE, data from the 8-bit register A
*/
const instruction_t LD_DE_A = {
    .cycles = { cpu_write_memory_DE_A, nop },
    .cycle_count = 2
};

/*
Load accumulator (direct)
LD A (nn): load to the 8-bit register A, data from the address specified by nn
*/
const instruction_t LD_A_nn = {
    .cycles = { cpu_read_memory_PC_Z, cpu_read_memory_PC_W, cpu_read_memory_WZ_Z, LD_load_A_Z },
    .cycle_count = 4
};

/*
Load from accumulator (direct)
LD (nn) A: load to the address specified by nn, data from the 8-bit register A
*/
const instruction_t LD_nn_A = {
    .cycles = { cpu_read_memory_PC_Z, cpu_read_memory_PC_W, cpu_write_memory_WZ_A, nop },
    .cycle_count = 4
};

/*
Load accumulator (indirect 0xFF00 + C)
LDH A (C): load to the 8-bit register A, data from the address 0xFF00 + C
*/
const instruction_t LDH_A__C = {
    .cycles = { cpu_read_memory__C_Z, LD_load_A_Z },
    .cycle_count = 2
};

/*
Load from accumulator (indirect 0xFF00 + C)
LDH (C) A: load to the address 0xFF00 + C, data from the 8-bit register A
*/
const instruction_t LDH__C_A = {
    .cycles = { cpu_write_memory__C_A, nop },
    .cycle_count = 2
};

void
do_machine_cycle(void)
{

}

int
main(void)
{

}

instruction_t OPCODE_TABLE[256] = {
    [0x00] = ,
    [0x01] = ,
    [0x02] = LD_BC_A,
    [0x03] = ,
    [0x04] = ,
    [0x05] = ,
    [0x06] = LD_r_n,
    [0x07] = ,
    [0x08] = ,
    [0x09] = ,
    [0x0A] = LD_A_BC,
    [0x0B] = ,
    [0x0C] = ,
    [0x0D] = ,
    [0x0E] = LD_r_n,
    [0x0F] = ,

    [0x10] = ,
    [0x11] = ,
    [0x12] = LD_DE_A,
    [0x13] = ,
    [0x14] = ,
    [0x15] = ,
    [0x16] = LD_r_n,
    [0x17] = ,
    [0x18] = ,
    [0x19] = ,
    [0x1A] = LD_A_DE,
    [0x1B] = ,
    [0x1C] = ,
    [0x1D] = ,
    [0x1E] = LD_r_n,
    [0x1F] = ,

    [0x20] = ,
    [0x21] = ,
    [0x22] = ,
    [0x23] = ,
    [0x24] = ,
    [0x25] = ,
    [0x26] = LD_r_n,
    [0x27] = ,
    [0x28] = ,
    [0x29] = ,
    [0x2A] = ,
    [0x2B] = ,
    [0x2C] = ,
    [0x2D] = ,
    [0x2E] = LD_r_n,
    [0x2F] = ,

    [0x30] = ,
    [0x31] = ,
    [0x32] = ,
    [0x33] = ,
    [0x34] = ,
    [0x35] = ,
    [0x36] = LD_HL_n,
    [0x37] = ,
    [0x38] = ,
    [0x39] = ,
    [0x3A] = ,
    [0x3B] = ,
    [0x3C] = ,
    [0x3D] = ,
    [0x3E] = LD_r_n,
    [0x3F] = ,

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

    [0x80] = ,
    [0x81] = ,
    [0x82] = ,
    [0x83] = ,
    [0x84] = ,
    [0x85] = ,
    [0x86] = ,
    [0x87] = ,
    [0x88] = ,
    [0x89] = ,
    [0x8A] = ,
    [0x8B] = ,
    [0x8C] = ,
    [0x8D] = ,
    [0x8E] = ,
    [0x8F] = ,

    [0x90] = ,
    [0x91] = ,
    [0x92] = ,
    [0x93] = ,
    [0x94] = ,
    [0x95] = ,
    [0x96] = ,
    [0x97] = ,
    [0x98] = ,
    [0x99] = ,
    [0x9A] = ,
    [0x9B] = ,
    [0x9C] = ,
    [0x9D] = ,
    [0x9E] = ,
    [0x9F] = ,

    [0xA0] = ,
    [0xA1] = ,
    [0xA2] = ,
    [0xA3] = ,
    [0xA4] = ,
    [0xA5] = ,
    [0xA6] = ,
    [0xA7] = ,
    [0xA8] = ,
    [0xA9] = ,
    [0xAA] = ,
    [0xAB] = ,
    [0xAC] = ,
    [0xAD] = ,
    [0xAE] = ,
    [0xAF] = ,

    [0xB0] = ,
    [0xB1] = ,
    [0xB2] = ,
    [0xB3] = ,
    [0xB4] = ,
    [0xB5] = ,
    [0xB6] = ,
    [0xB7] = ,
    [0xB8] = ,
    [0xB9] = ,
    [0xBA] = ,
    [0xBB] = ,
    [0xBC] = ,
    [0xBD] = ,
    [0xBE] = ,
    [0xBF] = ,

    [0xC0] = ,
    [0xC1] = ,
    [0xC2] = ,
    [0xC3] = ,
    [0xC4] = ,
    [0xC5] = ,
    [0xC6] = ,
    [0xC7] = ,
    [0xC8] = ,
    [0xC9] = ,
    [0xCA] = ,
    [0xCB] = ,
    [0xCC] = ,
    [0xCD] = ,
    [0xCE] = ,
    [0xCF] = ,

    [0xD0] = ,
    [0xD1] = ,
    [0xD2] = ,
    [0xD3] = ,
    [0xD4] = ,
    [0xD5] = ,
    [0xD6] = ,
    [0xD7] = ,
    [0xD8] = ,
    [0xD9] = ,
    [0xDA] = ,
    [0xDB] = ,
    [0xDC] = ,
    [0xDD] = ,
    [0xDE] = ,
    [0xDF] = ,

    [0xE0] = ,
    [0xE1] = ,
    [0xE2] = LDH__C_A,
    [0xE3] = ,
    [0xE4] = ,
    [0xE5] = ,
    [0xE6] = ,
    [0xE7] = ,
    [0xE8] = ,
    [0xE9] = ,
    [0xEA] = LD_nn_A,
    [0xEB] = ,
    [0xEC] = ,
    [0xED] = ,
    [0xEE] = ,
    [0xEF] = ,

    [0xF0] = ,
    [0xF1] = ,
    [0xF2] = LDH_A__C,
    [0xF3] = ,
    [0xF4] = ,
    [0xF5] = ,
    [0xF6] = ,
    [0xF7] = ,
    [0xF8] = ,
    [0xF9] = ,
    [0xFA] = LD_A_nn,
    [0xFB] = ,
    [0xFC] = ,
    [0xFD] = ,
    [0xFE] = ,
    [0xFF] = ,
};