#ifndef OPS_H
#define OPS_H

#include "./cpu.h"
#include "./rom.h"

/**
 * @brief Addressing mode types.
 *
 */
typedef enum {
    ADDR_IMPLIED,
    ADDR_IMMEDIATE,
    ADDR_ACCUMULATOR,
    ADDR_RELATIVE,
    ADDR_ABSOLUTE,
    ADDR_ABSOLUTE_X,
    ADDR_ABSOLUTE_Y,
    ADDR_ZERO_PAGE,
    ADDR_ZERO_PAGE_X,
    ADDR_ZERO_PAGE_Y,
    ADDR_INDIRECT, // Special case, only for JMP
    ADDR_INDIRECT_X,
    ADDR_INDIRECT_Y,
} address_mode_t;

/**
 * @brief Operation mode types.
 *
 */
typedef enum {
    OP_ADC,
    OP_AND,
    OP_ASL,
    OP_BCC,
    OP_BCS,
    OP_BEQ,
    OP_BIT,
    OP_BMI,
    OP_BNE,
    OP_BPL,
    OP_BRK,
    OP_BVC,
    OP_BVS,
    OP_CLC,
    OP_CLD,
    OP_CLI,
    OP_CLV,
    OP_CMP,
    OP_CPX,
    OP_CPY,
    OP_DEC,
    OP_DEX,
    OP_DEY,
    OP_EOR,
    OP_INC,
    OP_INX,
    OP_INY,
    OP_JMP,
    OP_JSR,
    OP_LDA,
    OP_LDX,
    OP_LDY,
    OP_LSR,
    OP_NOP,
    OP_ORA,
    OP_PHA,
    OP_PHP,
    OP_PLA,
    OP_PLP,
    OP_ROL,
    OP_ROR,
    OP_RTI,
    OP_RTS,
    OP_SBC,
    OP_SEC,
    OP_SED,
    OP_SEI,
    OP_STA,
    OP_STX,
    OP_STY,
    OP_TAX,
    OP_TAY,
    OP_TSX,
    OP_TXA,
    OP_TXS,
    OP_TYA,

    // Invalid opcodes
    OP_ALR,
    OP_ANC,
    OP_ANE,
    OP_ARR,
    OP_DCP,
    OP_ISC,
    OP_LAS,
    OP_LAX,
    OP_LXA,
    OP_RLA,
    OP_RRA,
    OP_SAX,
    OP_SBX,
    OP_SHA,
    OP_SHX,
    OP_SHY,
    OP_SLO,
    OP_SRE,
    OP_TAS,
    OP_JAM,
} op_mode_t;

/**
 * @brief Operation code structure.
 *
 */
typedef struct {
    /**
     * @brief Operation.
     *
     */
    op_mode_t op_mode;

    /**
     * @brief Addressing mode.
     *
     */
    address_mode_t address_mode;

    /**
     * @brief Base number of cycles.
     *
     * Does not include additional delays (e.g. page crossing).
     *
     */
    unsigned char cycles;
} opcode_t;

/**
 * @brief Table of operation codes.
 *
 */
static const opcode_t OP_TABLE[0x100] = {
    {OP_BRK, ADDR_IMPLIED, 7},     {OP_ORA, ADDR_INDIRECT_X, 6},
    {OP_JAM, ADDR_IMPLIED, 2},     {OP_SLO, ADDR_INDIRECT_X, 8},
    {OP_NOP, ADDR_ZERO_PAGE, 3},   {OP_ORA, ADDR_ZERO_PAGE, 3},
    {OP_ASL, ADDR_ZERO_PAGE, 5},   {OP_SLO, ADDR_ZERO_PAGE, 5},
    {OP_PHP, ADDR_IMPLIED, 3},     {OP_ORA, ADDR_IMMEDIATE, 2},
    {OP_ASL, ADDR_ACCUMULATOR, 2}, {OP_ANC, ADDR_IMMEDIATE, 2},
    {OP_NOP, ADDR_ABSOLUTE, 4},    {OP_ORA, ADDR_ABSOLUTE, 4},
    {OP_ASL, ADDR_ABSOLUTE, 6},    {OP_SLO, ADDR_ABSOLUTE, 6},
    {OP_BPL, ADDR_RELATIVE, 2},    {OP_ORA, ADDR_INDIRECT_Y, 5},
    {OP_JAM, ADDR_IMPLIED, 2},     {OP_SLO, ADDR_INDIRECT_Y, 8},
    {OP_NOP, ADDR_ZERO_PAGE_X, 4}, {OP_ORA, ADDR_ZERO_PAGE_X, 4},
    {OP_ASL, ADDR_ZERO_PAGE_X, 6}, {OP_SLO, ADDR_ZERO_PAGE_X, 6},
    {OP_CLC, ADDR_IMPLIED, 2},     {OP_ORA, ADDR_ABSOLUTE_Y, 4},
    {OP_NOP, ADDR_IMPLIED, 2},     {OP_SLO, ADDR_ABSOLUTE_Y, 7},
    {OP_NOP, ADDR_ABSOLUTE_X, 4},  {OP_ORA, ADDR_ABSOLUTE_X, 4},
    {OP_ASL, ADDR_ABSOLUTE_X, 7},  {OP_SLO, ADDR_ABSOLUTE_X, 7},
    {OP_JSR, ADDR_ABSOLUTE, 6},    {OP_AND, ADDR_INDIRECT_X, 6},
    {OP_JAM, ADDR_IMPLIED, 2},     {OP_RLA, ADDR_INDIRECT_X, 8},
    {OP_BIT, ADDR_ZERO_PAGE, 3},   {OP_AND, ADDR_ZERO_PAGE, 3},
    {OP_ROL, ADDR_ZERO_PAGE, 5},   {OP_RLA, ADDR_ZERO_PAGE, 5},
    {OP_PLP, ADDR_IMPLIED, 4},     {OP_AND, ADDR_IMMEDIATE, 2},
    {OP_ROL, ADDR_ACCUMULATOR, 2}, {OP_ANC, ADDR_IMMEDIATE, 2},
    {OP_BIT, ADDR_ABSOLUTE, 4},    {OP_AND, ADDR_ABSOLUTE, 4},
    {OP_ROL, ADDR_ABSOLUTE, 6},    {OP_RLA, ADDR_ABSOLUTE, 6},
    {OP_BMI, ADDR_RELATIVE, 2},    {OP_AND, ADDR_INDIRECT_Y, 5},
    {OP_JAM, ADDR_IMPLIED, 2},     {OP_RLA, ADDR_INDIRECT_Y, 8},
    {OP_NOP, ADDR_ZERO_PAGE_X, 4}, {OP_AND, ADDR_ZERO_PAGE_X, 4},
    {OP_ROL, ADDR_ZERO_PAGE_X, 6}, {OP_RLA, ADDR_ZERO_PAGE_X, 6},
    {OP_SEC, ADDR_IMPLIED, 2},     {OP_AND, ADDR_ABSOLUTE_Y, 4},
    {OP_NOP, ADDR_IMPLIED, 2},     {OP_RLA, ADDR_ABSOLUTE_Y, 7},
    {OP_NOP, ADDR_ABSOLUTE_X, 4},  {OP_AND, ADDR_ABSOLUTE_X, 4},
    {OP_ROL, ADDR_ABSOLUTE_X, 7},  {OP_RLA, ADDR_ABSOLUTE_X, 7},
    {OP_RTI, ADDR_IMPLIED, 6},     {OP_EOR, ADDR_INDIRECT_X, 6},
    {OP_JAM, ADDR_IMPLIED, 2},     {OP_SRE, ADDR_INDIRECT_X, 8},
    {OP_NOP, ADDR_ZERO_PAGE, 3},   {OP_EOR, ADDR_ZERO_PAGE, 3},
    {OP_LSR, ADDR_ZERO_PAGE, 5},   {OP_SRE, ADDR_ZERO_PAGE, 5},
    {OP_PHA, ADDR_IMPLIED, 3},     {OP_EOR, ADDR_IMMEDIATE, 2},
    {OP_LSR, ADDR_ACCUMULATOR, 2}, {OP_ALR, ADDR_IMMEDIATE, 2},
    {OP_JMP, ADDR_ABSOLUTE, 3},    {OP_EOR, ADDR_ABSOLUTE, 4},
    {OP_LSR, ADDR_ABSOLUTE, 6},    {OP_SRE, ADDR_ABSOLUTE, 6},
    {OP_BVC, ADDR_RELATIVE, 2},    {OP_EOR, ADDR_INDIRECT_Y, 5},
    {OP_JAM, ADDR_IMPLIED, 2},     {OP_SRE, ADDR_INDIRECT_Y, 8},
    {OP_NOP, ADDR_ZERO_PAGE_X, 4}, {OP_EOR, ADDR_ZERO_PAGE_X, 4},
    {OP_LSR, ADDR_ZERO_PAGE_X, 6}, {OP_SRE, ADDR_ZERO_PAGE_X, 6},
    {OP_CLI, ADDR_IMPLIED, 2},     {OP_EOR, ADDR_ABSOLUTE_Y, 4},
    {OP_NOP, ADDR_IMPLIED, 2},     {OP_SRE, ADDR_ABSOLUTE_Y, 7},
    {OP_NOP, ADDR_ABSOLUTE_X, 4},  {OP_EOR, ADDR_ABSOLUTE_X, 4},
    {OP_LSR, ADDR_ABSOLUTE_X, 7},  {OP_SRE, ADDR_ABSOLUTE_X, 7},
    {OP_RTS, ADDR_IMPLIED, 6},     {OP_ADC, ADDR_INDIRECT_X, 6},
    {OP_JAM, ADDR_IMPLIED, 2},     {OP_RRA, ADDR_INDIRECT_X, 8},
    {OP_NOP, ADDR_ZERO_PAGE, 3},   {OP_ADC, ADDR_ZERO_PAGE, 3},
    {OP_ROR, ADDR_ZERO_PAGE, 5},   {OP_RRA, ADDR_ZERO_PAGE, 5},
    {OP_PLA, ADDR_IMPLIED, 4},     {OP_ADC, ADDR_IMMEDIATE, 2},
    {OP_ROR, ADDR_ACCUMULATOR, 2}, {OP_ARR, ADDR_IMMEDIATE, 2},
    {OP_JMP, ADDR_INDIRECT, 5},    {OP_ADC, ADDR_ABSOLUTE, 4},
    {OP_ROR, ADDR_ABSOLUTE, 6},    {OP_RRA, ADDR_ABSOLUTE, 6},
    {OP_BVS, ADDR_RELATIVE, 2},    {OP_ADC, ADDR_INDIRECT_Y, 5},
    {OP_JAM, ADDR_IMPLIED, 2},     {OP_RRA, ADDR_INDIRECT_Y, 8},
    {OP_NOP, ADDR_ZERO_PAGE_X, 4}, {OP_ADC, ADDR_ZERO_PAGE_X, 4},
    {OP_ROR, ADDR_ZERO_PAGE_X, 6}, {OP_RRA, ADDR_ZERO_PAGE_X, 6},
    {OP_SEI, ADDR_IMPLIED, 2},     {OP_ADC, ADDR_ABSOLUTE_Y, 4},
    {OP_NOP, ADDR_IMPLIED, 2},     {OP_RRA, ADDR_ABSOLUTE_Y, 7},
    {OP_NOP, ADDR_ABSOLUTE_X, 4},  {OP_ADC, ADDR_ABSOLUTE_X, 4},
    {OP_ROR, ADDR_ABSOLUTE_X, 7},  {OP_RRA, ADDR_ABSOLUTE_X, 7},
    {OP_NOP, ADDR_IMMEDIATE, 2},   {OP_STA, ADDR_INDIRECT_X, 6},
    {OP_NOP, ADDR_IMMEDIATE, 2},   {OP_SAX, ADDR_INDIRECT_X, 6},
    {OP_STY, ADDR_ZERO_PAGE, 3},   {OP_STA, ADDR_ZERO_PAGE, 3},
    {OP_STX, ADDR_ZERO_PAGE, 3},   {OP_SAX, ADDR_ZERO_PAGE, 3},
    {OP_DEY, ADDR_IMPLIED, 2},     {OP_NOP, ADDR_IMMEDIATE, 2},
    {OP_TXA, ADDR_IMPLIED, 2},     {OP_ANE, ADDR_IMMEDIATE, 2},
    {OP_STY, ADDR_ABSOLUTE, 4},    {OP_STA, ADDR_ABSOLUTE, 4},
    {OP_STX, ADDR_ABSOLUTE, 4},    {OP_SAX, ADDR_ABSOLUTE, 4},
    {OP_BCC, ADDR_RELATIVE, 2},    {OP_STA, ADDR_INDIRECT_Y, 6},
    {OP_JAM, ADDR_IMPLIED, 2},     {OP_SHA, ADDR_INDIRECT_Y, 6},
    {OP_STY, ADDR_ZERO_PAGE_X, 4}, {OP_STA, ADDR_ZERO_PAGE_X, 4},
    {OP_STX, ADDR_ZERO_PAGE_Y, 4}, {OP_SAX, ADDR_ZERO_PAGE_Y, 4},
    {OP_TYA, ADDR_IMPLIED, 2},     {OP_STA, ADDR_ABSOLUTE_Y, 5},
    {OP_TXS, ADDR_IMPLIED, 2},     {OP_TAS, ADDR_ABSOLUTE_Y, 5},
    {OP_SHY, ADDR_ABSOLUTE_X, 5},  {OP_STA, ADDR_ABSOLUTE_X, 5},
    {OP_SHX, ADDR_ABSOLUTE_Y, 5},  {OP_SHA, ADDR_ABSOLUTE_Y, 5},
    {OP_LDY, ADDR_IMMEDIATE, 2},   {OP_LDA, ADDR_INDIRECT_X, 6},
    {OP_LDX, ADDR_IMMEDIATE, 2},   {OP_LAX, ADDR_INDIRECT_X, 6},
    {OP_LDY, ADDR_ZERO_PAGE, 3},   {OP_LDA, ADDR_ZERO_PAGE, 3},
    {OP_LDX, ADDR_ZERO_PAGE, 3},   {OP_LAX, ADDR_ZERO_PAGE, 3},
    {OP_TAY, ADDR_IMPLIED, 2},     {OP_LDA, ADDR_IMMEDIATE, 2},
    {OP_TAX, ADDR_IMPLIED, 2},     {OP_LXA, ADDR_IMMEDIATE, 2},
    {OP_LDY, ADDR_ABSOLUTE, 4},    {OP_LDA, ADDR_ABSOLUTE, 4},
    {OP_LDX, ADDR_ABSOLUTE, 4},    {OP_LAX, ADDR_ABSOLUTE, 4},
    {OP_BCS, ADDR_RELATIVE, 2},    {OP_LDA, ADDR_INDIRECT_Y, 5},
    {OP_JAM, ADDR_IMPLIED, 2},     {OP_LAX, ADDR_INDIRECT_Y, 5},
    {OP_LDY, ADDR_ZERO_PAGE_X, 4}, {OP_LDA, ADDR_ZERO_PAGE_X, 4},
    {OP_LDX, ADDR_ZERO_PAGE_Y, 4}, {OP_LAX, ADDR_ZERO_PAGE_Y, 4},
    {OP_CLV, ADDR_IMPLIED, 2},     {OP_LDA, ADDR_ABSOLUTE_Y, 4},
    {OP_TSX, ADDR_IMPLIED, 2},     {OP_LAS, ADDR_ABSOLUTE_Y, 4},
    {OP_LDY, ADDR_ABSOLUTE_X, 4},  {OP_LDA, ADDR_ABSOLUTE_X, 4},
    {OP_LDX, ADDR_ABSOLUTE_Y, 4},  {OP_LAX, ADDR_ABSOLUTE_Y, 4},
    {OP_CPY, ADDR_IMMEDIATE, 2},   {OP_CMP, ADDR_INDIRECT_X, 6},
    {OP_NOP, ADDR_IMMEDIATE, 2},   {OP_DCP, ADDR_INDIRECT_X, 8},
    {OP_CPY, ADDR_ZERO_PAGE, 3},   {OP_CMP, ADDR_ZERO_PAGE, 3},
    {OP_DEC, ADDR_ZERO_PAGE, 5},   {OP_DCP, ADDR_ZERO_PAGE, 5},
    {OP_INY, ADDR_IMPLIED, 2},     {OP_CMP, ADDR_IMMEDIATE, 2},
    {OP_DEX, ADDR_IMPLIED, 2},     {OP_SBX, ADDR_IMMEDIATE, 2},
    {OP_CPY, ADDR_ABSOLUTE, 4},    {OP_CMP, ADDR_ABSOLUTE, 4},
    {OP_DEC, ADDR_ABSOLUTE, 6},    {OP_DCP, ADDR_ABSOLUTE, 6},
    {OP_BNE, ADDR_RELATIVE, 2},    {OP_CMP, ADDR_INDIRECT_Y, 5},
    {OP_JAM, ADDR_IMPLIED, 2},     {OP_DCP, ADDR_INDIRECT_Y, 8},
    {OP_NOP, ADDR_ZERO_PAGE_X, 4}, {OP_CMP, ADDR_ZERO_PAGE_X, 4},
    {OP_DEC, ADDR_ZERO_PAGE_X, 6}, {OP_DCP, ADDR_ZERO_PAGE_X, 6},
    {OP_CLD, ADDR_IMPLIED, 2},     {OP_CMP, ADDR_ABSOLUTE_Y, 4},
    {OP_NOP, ADDR_IMPLIED, 2},     {OP_DCP, ADDR_ABSOLUTE_Y, 7},
    {OP_NOP, ADDR_ABSOLUTE_X, 4},  {OP_CMP, ADDR_ABSOLUTE_X, 4},
    {OP_DEC, ADDR_ABSOLUTE_X, 7},  {OP_DCP, ADDR_ABSOLUTE_X, 7},
    {OP_CPX, ADDR_IMMEDIATE, 2},   {OP_SBC, ADDR_INDIRECT_X, 6},
    {OP_NOP, ADDR_IMMEDIATE, 2},   {OP_ISC, ADDR_INDIRECT_X, 8},
    {OP_CPX, ADDR_ZERO_PAGE, 3},   {OP_SBC, ADDR_ZERO_PAGE, 3},
    {OP_INC, ADDR_ZERO_PAGE, 5},   {OP_ISC, ADDR_ZERO_PAGE, 5},
    {OP_INX, ADDR_IMPLIED, 2},     {OP_SBC, ADDR_IMMEDIATE, 2},
    {OP_NOP, ADDR_IMPLIED, 2},     {OP_SBC, ADDR_IMMEDIATE, 2},
    {OP_CPX, ADDR_ABSOLUTE, 4},    {OP_SBC, ADDR_ABSOLUTE, 4},
    {OP_INC, ADDR_ABSOLUTE, 6},    {OP_ISC, ADDR_ABSOLUTE, 6},
    {OP_BEQ, ADDR_RELATIVE, 2},    {OP_SBC, ADDR_INDIRECT_Y, 5},
    {OP_JAM, ADDR_IMPLIED, 2},     {OP_ISC, ADDR_INDIRECT_Y, 8},
    {OP_NOP, ADDR_ZERO_PAGE_X, 4}, {OP_SBC, ADDR_ZERO_PAGE_X, 4},
    {OP_INC, ADDR_ZERO_PAGE_X, 6}, {OP_ISC, ADDR_ZERO_PAGE_X, 6},
    {OP_SED, ADDR_IMPLIED, 2},     {OP_SBC, ADDR_ABSOLUTE_Y, 4},
    {OP_NOP, ADDR_IMPLIED, 2},     {OP_ISC, ADDR_ABSOLUTE_Y, 7},
    {OP_NOP, ADDR_ABSOLUTE_X, 4},  {OP_SBC, ADDR_ABSOLUTE_X, 4},
    {OP_INC, ADDR_ABSOLUTE_X, 7},  {OP_ISC, ADDR_ABSOLUTE_X, 7},
};

/**
 * @brief Size of an op + operand by address mode.
 *
 * This will determine the program counter increment.
 *
 */
static const unsigned char ADDRESS_MODE_SIZES[13] = {
    1, // ADDR_IMPLIED
    2, // ADDR_IMMEDIATE
    1, // ADDR_ACCUMULATOR
    2, // ADDR_RELATIVE
    3, // ADDR_ABSOLUTE
    3, // ADDR_ABSOLUTE_X
    3, // ADDR_ABSOLUTE_Y
    2, // ADDR_ZERO_PAGE
    2, // ADDR_ZERO_PAGE_X
    2, // ADDR_ZERO_PAGE_Y
    3, // ADDR_INDIRECT
    2, // ADDR_INDIRECT_X
    2, // ADDR_INDIRECT_Y
};

/**
 * @brief Operand.
 *
 */
typedef struct {
    /**
     * @brief Address of the operand.
     *
     */
    address_t address;

    /**
     * @brief Page crossed.
     *
     */
    unsigned char page_crossed;
} operand_t;

operand_t addr_immediate(cpu_t *cpu, rom_t *rom);
operand_t addr_zero_page(cpu_t *cpu, rom_t *rom);
operand_t addr_zero_page_x(cpu_t *cpu, rom_t *rom);
operand_t addr_zero_page_y(cpu_t *cpu, rom_t *rom);
operand_t addr_absolute(cpu_t *cpu, rom_t *rom);
operand_t addr_absolute_x(cpu_t *cpu, rom_t *rom);
operand_t addr_absolute_y(cpu_t *cpu, rom_t *rom);
operand_t addr_indirect(cpu_t *cpu, rom_t *rom);
operand_t addr_indirect_x(cpu_t *cpu, rom_t *rom);
operand_t addr_indirect_y(cpu_t *cpu, rom_t *rom);
operand_t addr_relative(cpu_t *cpu, rom_t *rom);

// No handler for implicit or accumulator addressing modes.

#endif