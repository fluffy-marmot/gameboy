#ifndef GB_CPU
#define GB_CPU

#include "_specification.h"
#include "bus.h"
#include "irq.h"

#define MAX_MCYCLE_INSTRUCTION 6

/*
An instruction is an array of callables (each of which is a machine cycle's microinstruction)
The struct also has the number of machine cycles the instruction takes (excluding overlapping fetch)
*/
typedef struct {
    void    (*cycles[MAX_MCYCLE_INSTRUCTION])(void);
    uint8_t   cycle_count;
} instruction_t;

typedef struct {
    union {
        uint8_t reg[8];                         // access to registers via parameter
        struct {
            uint8_t B;                          // 0x00 0b000
            uint8_t C;                          // 0x01 0b001
            uint8_t D;                          // 0x02 0b010
            uint8_t E;                          // 0x03 0b011
            uint8_t H;                          // 0x04 0b100
            uint8_t L;                          // 0x05 0b101
            uint8_t F;                          // z, n, h, c flags (zero, subtraction, half carry, carry)        
            uint8_t A;                          // 0x06 0b111
        };
    };
    uint16_t PC;                                // program counter
    uint16_t SP;                                // stack pointer
    uint8_t IR;                                 // instruction register

    uint8_t Z;                                  // latch used to store data between M-cycles
    uint8_t W;                                  // latch used to store data between M-cycles

    uint8_t IME;                                // interrupt master enable flag
    uint8_t IME_latch;                          // used to delay enabling the IME until next machine cycle

    bus_interface_t *bus;                       // access to memory bus read/write interface
    bus_interface_t *bus_oam_corruption;        // a separate interface for handling OAM corruption bug
    gb_irq_handler_t *irq;                      // access to interrupt interface

    uint8_t cycle_num;                          // the machine cycle within current instruction
    uint8_t cb_instruction;                     // 1 if next fetch should use CB table
    uint8_t halt_bug_flag;                      // track whether to trigger halt bug behavior
    const instruction_t *instruction;           // current instruction
} sm83_cpu_t;

typedef sm83_cpu_t gb_cpu_t;

typedef enum {
    CPU_FETCH_OVERLAPPING,
    CPU_FETCH_TESTMODE_SINGLE_INSTRUCTION,
    CPU_FETCH_MANUAL_CONTROL
} cpu_fetch_t;

gb_cpu_t *init_gameboy_cpu(gb_bus_t *, gb_irq_handler_t *);
void cycle_mcycle_cpu(cpu_fetch_t);
void fetch_instruction(void);

#endif