#ifndef GB_BUS
#define GB_BUS

#include <stdint.h>

typedef uint16_t memaddr;

typedef struct {
    uint8_t (*read)(memaddr);
    void (*write)(memaddr, uint8_t);
    void (*read_to)(memaddr, uint8_t *);
} gb_bus_t;

void init_test_bus(gb_bus_t *bus);

#endif