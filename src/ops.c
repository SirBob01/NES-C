#include "./ops.h"

operand_t immediate_addr(cpu_t *cpu) {
    operand_t operand;
    operand.address = cpu->pc + 1;
    operand.page_crossed = 0;
    return operand;
}

operand_t zero_page_addr(cpu_t *cpu) {
    operand_t operand;
    operand.address = read_byte_cpu_bus(cpu->bus, cpu->pc + 1);
    operand.page_crossed = 0;
    return operand;
}

operand_t zero_page_x_addr(cpu_t *cpu) {
    unsigned char base = read_byte_cpu_bus(cpu->bus, cpu->pc + 1);
    unsigned char addr = base + cpu->x;

    operand_t operand;
    operand.address = addr;
    operand.page_crossed = 0;
    return operand;
}

operand_t zero_page_y_addr(cpu_t *cpu) {
    unsigned char base = read_byte_cpu_bus(cpu->bus, cpu->pc + 1);
    unsigned char addr = base + cpu->y;

    operand_t operand;
    operand.address = addr;
    operand.page_crossed = 0;
    return operand;
}

operand_t absolute_addr(cpu_t *cpu) {
    operand_t operand;
    operand.address = read_short_cpu_bus(cpu->bus, cpu->pc + 1);
    operand.page_crossed = 0;
    return operand;
}

operand_t absolute_x_addr(cpu_t *cpu) {
    address_t base = read_short_cpu_bus(cpu->bus, cpu->pc + 1);
    operand_t operand;
    operand.address = base + cpu->x;
    operand.page_crossed = (base & 0xff) + cpu->x > 0xff;
    return operand;
}

operand_t absolute_y_addr(cpu_t *cpu) {
    address_t base = read_short_cpu_bus(cpu->bus, cpu->pc + 1);
    operand_t operand;
    operand.address = base + cpu->y;
    operand.page_crossed = (base & 0xff) + cpu->y > 0xff;
    return operand;
}

operand_t indirect_addr(cpu_t *cpu) {
    // NOTE: The original 6502 had a bug in fetching the target address
    // of an indirect jump. If the indirect vector fell on a page boundary
    // (ie. address $xxFF where xx is any value from $00 to $FF) then the
    // low byte would be fetched from the correct page, but the high byte
    // would be fetched from the next page.
    address_t ptr_addr = read_short_cpu_bus(cpu->bus, cpu->pc + 1);
    address_t next_addr;
    if ((ptr_addr & 0xff) == 0xff) {
        next_addr = ptr_addr & 0xff00;
    } else {
        next_addr = ptr_addr + 1;
    }
    unsigned char a0 = read_byte_cpu_bus(cpu->bus, ptr_addr);
    unsigned char a1 = read_byte_cpu_bus(cpu->bus, next_addr);

    operand_t operand;
    operand.address = (a1 << 8) | a0;
    operand.page_crossed = 0;
    return operand;
}

operand_t indirect_x_addr(cpu_t *cpu) {
    unsigned char ptr_addr = read_byte_cpu_bus(cpu->bus, cpu->pc + 1);
    operand_t operand;
    operand.address = read_short_zp_cpu_bus(cpu->bus, ptr_addr + cpu->x);
    operand.page_crossed = 0;
    return operand;
}

operand_t indirect_y_addr(cpu_t *cpu) {
    unsigned char ptr_addr = read_byte_cpu_bus(cpu->bus, cpu->pc + 1);
    address_t base = read_short_zp_cpu_bus(cpu->bus, ptr_addr);
    operand_t operand;
    operand.address = base + cpu->y;
    operand.page_crossed = (base & 0xff) + cpu->y > 0xff;
    return operand;
}

operand_t relative_addr(cpu_t *cpu) {
    signed char offset = read_byte_cpu_bus(cpu->bus, cpu->pc + 1);
    address_t base = cpu->pc + 2;
    operand_t operand;
    operand.address = base + offset;
    operand.page_crossed = (base & 0xff) + offset > 0xff;
    return operand;
}