#ifndef GB_CPU
#define GB_CPU

#include "bus.h"

#include <stdint.h>

// TODO probably get rid of the endianness-dependant unions
typedef struct {
    union {
        uint8_t reg[8];
        struct {
            uint8_t B;              // 0x00 0b000
            uint8_t C;              // 0x01 0b001
            uint8_t D;              // 0x02 0b010
            uint8_t E;              // 0x03 0b011
            uint8_t H;              // 0x04 0b100
            uint8_t L;              // 0x05 0b101
            union {
                uint16_t AF;        // this pair has little endianness in natural order, unlike others
                struct {
                    uint8_t F;      // z, n, h, c flags (zero, subtraction, half carry, carry)        
                    uint8_t A;      // 0x06 0b111
                };
            };
        };
    };
    uint16_t PC;                    // program counter
    uint16_t SP;
    uint8_t IR;                     // instruction register
    uint8_t IE;                     // interrupt enable
    union {
        uint16_t WZ;
        struct {
            uint8_t Z;              // latch used to store data between M-cycles
            uint8_t W;              // latch used to store data between M-cycles
        };
    };

    uint8_t cycle_num;
    gb_bus_t *bus;                  // access to memory read/write
} sm83_cpu_t;

typedef sm83_cpu_t gb_cpu_t;

extern gb_cpu_t cpu;

#endif