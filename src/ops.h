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
} mnemonic_t;

/**
 * @brief Operation group types.
 *
 */
typedef enum {
    OPGROUP_NONE = 0b00,
    OPGROUP_R = 0b01,
    OPGROUP_RW = 0b11,
    OPGROUP_W = 0b10,
} opgroup_t;

/**
 * @brief Operation code structure.
 *
 */
typedef struct {
    /**
     * @brief Operation.
     *
     */
    mnemonic_t mnemonic;

    /**
     * @brief Addressing mode.
     *
     */
    address_mode_t address_mode;

    /**
     * @brief Operation group.
     *
     */
    opgroup_t group;
} operation_t;

/**
 * @brief Table of operation codes.
 *
 */
static const operation_t OP_TABLE[0x100] = {
    {OP_BRK, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_ORA, ADDR_INDIRECT_X, OPGROUP_R},
    {OP_JAM, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_SLO, ADDR_INDIRECT_X, OPGROUP_RW},
    {OP_NOP, ADDR_ZERO_PAGE, OPGROUP_R},
    {OP_ORA, ADDR_ZERO_PAGE, OPGROUP_R},
    {OP_ASL, ADDR_ZERO_PAGE, OPGROUP_RW},
    {OP_SLO, ADDR_ZERO_PAGE, OPGROUP_RW},
    {OP_PHP, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_ORA, ADDR_IMMEDIATE, OPGROUP_R},
    {OP_ASL, ADDR_ACCUMULATOR, OPGROUP_RW},
    {OP_ANC, ADDR_IMMEDIATE, OPGROUP_NONE},
    {OP_NOP, ADDR_ABSOLUTE, OPGROUP_R},
    {OP_ORA, ADDR_ABSOLUTE, OPGROUP_R},
    {OP_ASL, ADDR_ABSOLUTE, OPGROUP_RW},
    {OP_SLO, ADDR_ABSOLUTE, OPGROUP_RW},
    {OP_BPL, ADDR_RELATIVE, OPGROUP_NONE},
    {OP_ORA, ADDR_INDIRECT_Y, OPGROUP_R},
    {OP_JAM, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_SLO, ADDR_INDIRECT_Y, OPGROUP_RW},
    {OP_NOP, ADDR_ZERO_PAGE_X, OPGROUP_R},
    {OP_ORA, ADDR_ZERO_PAGE_X, OPGROUP_R},
    {OP_ASL, ADDR_ZERO_PAGE_X, OPGROUP_RW},
    {OP_SLO, ADDR_ZERO_PAGE_X, OPGROUP_RW},
    {OP_CLC, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_ORA, ADDR_ABSOLUTE_Y, OPGROUP_R},
    {OP_NOP, ADDR_IMPLIED, OPGROUP_R},
    {OP_SLO, ADDR_ABSOLUTE_Y, OPGROUP_RW},
    {OP_NOP, ADDR_ABSOLUTE_X, OPGROUP_R},
    {OP_ORA, ADDR_ABSOLUTE_X, OPGROUP_R},
    {OP_ASL, ADDR_ABSOLUTE_X, OPGROUP_RW},
    {OP_SLO, ADDR_ABSOLUTE_X, OPGROUP_RW},
    {OP_JSR, ADDR_ABSOLUTE, OPGROUP_NONE},
    {OP_AND, ADDR_INDIRECT_X, OPGROUP_R},
    {OP_JAM, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_RLA, ADDR_INDIRECT_X, OPGROUP_RW},
    {OP_BIT, ADDR_ZERO_PAGE, OPGROUP_R},
    {OP_AND, ADDR_ZERO_PAGE, OPGROUP_R},
    {OP_ROL, ADDR_ZERO_PAGE, OPGROUP_RW},
    {OP_RLA, ADDR_ZERO_PAGE, OPGROUP_RW},
    {OP_PLP, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_AND, ADDR_IMMEDIATE, OPGROUP_R},
    {OP_ROL, ADDR_ACCUMULATOR, OPGROUP_RW},
    {OP_ANC, ADDR_IMMEDIATE, OPGROUP_NONE},
    {OP_BIT, ADDR_ABSOLUTE, OPGROUP_R},
    {OP_AND, ADDR_ABSOLUTE, OPGROUP_R},
    {OP_ROL, ADDR_ABSOLUTE, OPGROUP_RW},
    {OP_RLA, ADDR_ABSOLUTE, OPGROUP_RW},
    {OP_BMI, ADDR_RELATIVE, OPGROUP_NONE},
    {OP_AND, ADDR_INDIRECT_Y, OPGROUP_R},
    {OP_JAM, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_RLA, ADDR_INDIRECT_Y, OPGROUP_RW},
    {OP_NOP, ADDR_ZERO_PAGE_X, OPGROUP_R},
    {OP_AND, ADDR_ZERO_PAGE_X, OPGROUP_R},
    {OP_ROL, ADDR_ZERO_PAGE_X, OPGROUP_RW},
    {OP_RLA, ADDR_ZERO_PAGE_X, OPGROUP_RW},
    {OP_SEC, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_AND, ADDR_ABSOLUTE_Y, OPGROUP_R},
    {OP_NOP, ADDR_IMPLIED, OPGROUP_R},
    {OP_RLA, ADDR_ABSOLUTE_Y, OPGROUP_RW},
    {OP_NOP, ADDR_ABSOLUTE_X, OPGROUP_R},
    {OP_AND, ADDR_ABSOLUTE_X, OPGROUP_R},
    {OP_ROL, ADDR_ABSOLUTE_X, OPGROUP_RW},
    {OP_RLA, ADDR_ABSOLUTE_X, OPGROUP_RW},
    {OP_RTI, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_EOR, ADDR_INDIRECT_X, OPGROUP_R},
    {OP_JAM, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_SRE, ADDR_INDIRECT_X, OPGROUP_RW},
    {OP_NOP, ADDR_ZERO_PAGE, OPGROUP_R},
    {OP_EOR, ADDR_ZERO_PAGE, OPGROUP_R},
    {OP_LSR, ADDR_ZERO_PAGE, OPGROUP_RW},
    {OP_SRE, ADDR_ZERO_PAGE, OPGROUP_RW},
    {OP_PHA, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_EOR, ADDR_IMMEDIATE, OPGROUP_R},
    {OP_LSR, ADDR_ACCUMULATOR, OPGROUP_RW},
    {OP_ALR, ADDR_IMMEDIATE, OPGROUP_NONE},
    {OP_JMP, ADDR_ABSOLUTE, OPGROUP_NONE},
    {OP_EOR, ADDR_ABSOLUTE, OPGROUP_R},
    {OP_LSR, ADDR_ABSOLUTE, OPGROUP_RW},
    {OP_SRE, ADDR_ABSOLUTE, OPGROUP_RW},
    {OP_BVC, ADDR_RELATIVE, OPGROUP_NONE},
    {OP_EOR, ADDR_INDIRECT_Y, OPGROUP_R},
    {OP_JAM, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_SRE, ADDR_INDIRECT_Y, OPGROUP_RW},
    {OP_NOP, ADDR_ZERO_PAGE_X, OPGROUP_R},
    {OP_EOR, ADDR_ZERO_PAGE_X, OPGROUP_R},
    {OP_LSR, ADDR_ZERO_PAGE_X, OPGROUP_RW},
    {OP_SRE, ADDR_ZERO_PAGE_X, OPGROUP_RW},
    {OP_CLI, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_EOR, ADDR_ABSOLUTE_Y, OPGROUP_R},
    {OP_NOP, ADDR_IMPLIED, OPGROUP_R},
    {OP_SRE, ADDR_ABSOLUTE_Y, OPGROUP_RW},
    {OP_NOP, ADDR_ABSOLUTE_X, OPGROUP_R},
    {OP_EOR, ADDR_ABSOLUTE_X, OPGROUP_R},
    {OP_LSR, ADDR_ABSOLUTE_X, OPGROUP_RW},
    {OP_SRE, ADDR_ABSOLUTE_X, OPGROUP_RW},
    {OP_RTS, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_ADC, ADDR_INDIRECT_X, OPGROUP_R},
    {OP_JAM, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_RRA, ADDR_INDIRECT_X, OPGROUP_RW},
    {OP_NOP, ADDR_ZERO_PAGE, OPGROUP_R},
    {OP_ADC, ADDR_ZERO_PAGE, OPGROUP_R},
    {OP_ROR, ADDR_ZERO_PAGE, OPGROUP_RW},
    {OP_RRA, ADDR_ZERO_PAGE, OPGROUP_RW},
    {OP_PLA, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_ADC, ADDR_IMMEDIATE, OPGROUP_R},
    {OP_ROR, ADDR_ACCUMULATOR, OPGROUP_RW},
    {OP_ARR, ADDR_IMMEDIATE, OPGROUP_NONE},
    {OP_JMP, ADDR_INDIRECT, OPGROUP_NONE},
    {OP_ADC, ADDR_ABSOLUTE, OPGROUP_R},
    {OP_ROR, ADDR_ABSOLUTE, OPGROUP_RW},
    {OP_RRA, ADDR_ABSOLUTE, OPGROUP_RW},
    {OP_BVS, ADDR_RELATIVE, OPGROUP_NONE},
    {OP_ADC, ADDR_INDIRECT_Y, OPGROUP_R},
    {OP_JAM, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_RRA, ADDR_INDIRECT_Y, OPGROUP_RW},
    {OP_NOP, ADDR_ZERO_PAGE_X, OPGROUP_R},
    {OP_ADC, ADDR_ZERO_PAGE_X, OPGROUP_R},
    {OP_ROR, ADDR_ZERO_PAGE_X, OPGROUP_RW},
    {OP_RRA, ADDR_ZERO_PAGE_X, OPGROUP_RW},
    {OP_SEI, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_ADC, ADDR_ABSOLUTE_Y, OPGROUP_R},
    {OP_NOP, ADDR_IMPLIED, OPGROUP_R},
    {OP_RRA, ADDR_ABSOLUTE_Y, OPGROUP_RW},
    {OP_NOP, ADDR_ABSOLUTE_X, OPGROUP_R},
    {OP_ADC, ADDR_ABSOLUTE_X, OPGROUP_R},
    {OP_ROR, ADDR_ABSOLUTE_X, OPGROUP_RW},
    {OP_RRA, ADDR_ABSOLUTE_X, OPGROUP_RW},
    {OP_NOP, ADDR_IMMEDIATE, OPGROUP_R},
    {OP_STA, ADDR_INDIRECT_X, OPGROUP_W},
    {OP_NOP, ADDR_IMMEDIATE, OPGROUP_R},
    {OP_SAX, ADDR_INDIRECT_X, OPGROUP_W},
    {OP_STY, ADDR_ZERO_PAGE, OPGROUP_W},
    {OP_STA, ADDR_ZERO_PAGE, OPGROUP_W},
    {OP_STX, ADDR_ZERO_PAGE, OPGROUP_W},
    {OP_SAX, ADDR_ZERO_PAGE, OPGROUP_W},
    {OP_DEY, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_NOP, ADDR_IMMEDIATE, OPGROUP_R},
    {OP_TXA, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_ANE, ADDR_IMMEDIATE, OPGROUP_NONE},
    {OP_STY, ADDR_ABSOLUTE, OPGROUP_W},
    {OP_STA, ADDR_ABSOLUTE, OPGROUP_W},
    {OP_STX, ADDR_ABSOLUTE, OPGROUP_W},
    {OP_SAX, ADDR_ABSOLUTE, OPGROUP_W},
    {OP_BCC, ADDR_RELATIVE, OPGROUP_NONE},
    {OP_STA, ADDR_INDIRECT_Y, OPGROUP_W},
    {OP_JAM, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_SHA, ADDR_INDIRECT_Y, OPGROUP_NONE},
    {OP_STY, ADDR_ZERO_PAGE_X, OPGROUP_W},
    {OP_STA, ADDR_ZERO_PAGE_X, OPGROUP_W},
    {OP_STX, ADDR_ZERO_PAGE_Y, OPGROUP_W},
    {OP_SAX, ADDR_ZERO_PAGE_Y, OPGROUP_W},
    {OP_TYA, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_STA, ADDR_ABSOLUTE_Y, OPGROUP_W},
    {OP_TXS, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_TAS, ADDR_ABSOLUTE_Y, OPGROUP_NONE},
    {OP_SHY, ADDR_ABSOLUTE_X, OPGROUP_NONE},
    {OP_STA, ADDR_ABSOLUTE_X, OPGROUP_W},
    {OP_SHX, ADDR_ABSOLUTE_Y, OPGROUP_NONE},
    {OP_SHA, ADDR_ABSOLUTE_Y, OPGROUP_NONE},
    {OP_LDY, ADDR_IMMEDIATE, OPGROUP_R},
    {OP_LDA, ADDR_INDIRECT_X, OPGROUP_R},
    {OP_LDX, ADDR_IMMEDIATE, OPGROUP_R},
    {OP_LAX, ADDR_INDIRECT_X, OPGROUP_R},
    {OP_LDY, ADDR_ZERO_PAGE, OPGROUP_R},
    {OP_LDA, ADDR_ZERO_PAGE, OPGROUP_R},
    {OP_LDX, ADDR_ZERO_PAGE, OPGROUP_R},
    {OP_LAX, ADDR_ZERO_PAGE, OPGROUP_R},
    {OP_TAY, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_LDA, ADDR_IMMEDIATE, OPGROUP_R},
    {OP_TAX, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_LXA, ADDR_IMMEDIATE, OPGROUP_NONE},
    {OP_LDY, ADDR_ABSOLUTE, OPGROUP_R},
    {OP_LDA, ADDR_ABSOLUTE, OPGROUP_R},
    {OP_LDX, ADDR_ABSOLUTE, OPGROUP_R},
    {OP_LAX, ADDR_ABSOLUTE, OPGROUP_R},
    {OP_BCS, ADDR_RELATIVE, OPGROUP_NONE},
    {OP_LDA, ADDR_INDIRECT_Y, OPGROUP_R},
    {OP_JAM, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_LAX, ADDR_INDIRECT_Y, OPGROUP_R},
    {OP_LDY, ADDR_ZERO_PAGE_X, OPGROUP_R},
    {OP_LDA, ADDR_ZERO_PAGE_X, OPGROUP_R},
    {OP_LDX, ADDR_ZERO_PAGE_Y, OPGROUP_R},
    {OP_LAX, ADDR_ZERO_PAGE_Y, OPGROUP_R},
    {OP_CLV, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_LDA, ADDR_ABSOLUTE_Y, OPGROUP_R},
    {OP_TSX, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_LAS, ADDR_ABSOLUTE_Y, OPGROUP_NONE},
    {OP_LDY, ADDR_ABSOLUTE_X, OPGROUP_R},
    {OP_LDA, ADDR_ABSOLUTE_X, OPGROUP_R},
    {OP_LDX, ADDR_ABSOLUTE_Y, OPGROUP_R},
    {OP_LAX, ADDR_ABSOLUTE_Y, OPGROUP_R},
    {OP_CPY, ADDR_IMMEDIATE, OPGROUP_NONE},
    {OP_CMP, ADDR_INDIRECT_X, OPGROUP_R},
    {OP_NOP, ADDR_IMMEDIATE, OPGROUP_R},
    {OP_DCP, ADDR_INDIRECT_X, OPGROUP_RW},
    {OP_CPY, ADDR_ZERO_PAGE, OPGROUP_NONE},
    {OP_CMP, ADDR_ZERO_PAGE, OPGROUP_R},
    {OP_DEC, ADDR_ZERO_PAGE, OPGROUP_RW},
    {OP_DCP, ADDR_ZERO_PAGE, OPGROUP_RW},
    {OP_INY, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_CMP, ADDR_IMMEDIATE, OPGROUP_R},
    {OP_DEX, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_SBX, ADDR_IMMEDIATE, OPGROUP_NONE},
    {OP_CPY, ADDR_ABSOLUTE, OPGROUP_NONE},
    {OP_CMP, ADDR_ABSOLUTE, OPGROUP_R},
    {OP_DEC, ADDR_ABSOLUTE, OPGROUP_RW},
    {OP_DCP, ADDR_ABSOLUTE, OPGROUP_RW},
    {OP_BNE, ADDR_RELATIVE, OPGROUP_NONE},
    {OP_CMP, ADDR_INDIRECT_Y, OPGROUP_R},
    {OP_JAM, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_DCP, ADDR_INDIRECT_Y, OPGROUP_RW},
    {OP_NOP, ADDR_ZERO_PAGE_X, OPGROUP_R},
    {OP_CMP, ADDR_ZERO_PAGE_X, OPGROUP_R},
    {OP_DEC, ADDR_ZERO_PAGE_X, OPGROUP_RW},
    {OP_DCP, ADDR_ZERO_PAGE_X, OPGROUP_RW},
    {OP_CLD, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_CMP, ADDR_ABSOLUTE_Y, OPGROUP_R},
    {OP_NOP, ADDR_IMPLIED, OPGROUP_R},
    {OP_DCP, ADDR_ABSOLUTE_Y, OPGROUP_RW},
    {OP_NOP, ADDR_ABSOLUTE_X, OPGROUP_R},
    {OP_CMP, ADDR_ABSOLUTE_X, OPGROUP_R},
    {OP_DEC, ADDR_ABSOLUTE_X, OPGROUP_RW},
    {OP_DCP, ADDR_ABSOLUTE_X, OPGROUP_RW},
    {OP_CPX, ADDR_IMMEDIATE, OPGROUP_NONE},
    {OP_SBC, ADDR_INDIRECT_X, OPGROUP_R},
    {OP_NOP, ADDR_IMMEDIATE, OPGROUP_R},
    {OP_ISC, ADDR_INDIRECT_X, OPGROUP_RW},
    {OP_CPX, ADDR_ZERO_PAGE, OPGROUP_NONE},
    {OP_SBC, ADDR_ZERO_PAGE, OPGROUP_R},
    {OP_INC, ADDR_ZERO_PAGE, OPGROUP_RW},
    {OP_ISC, ADDR_ZERO_PAGE, OPGROUP_RW},
    {OP_INX, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_SBC, ADDR_IMMEDIATE, OPGROUP_R},
    {OP_NOP, ADDR_IMPLIED, OPGROUP_R},
    {OP_SBC, ADDR_IMMEDIATE, OPGROUP_R},
    {OP_CPX, ADDR_ABSOLUTE, OPGROUP_NONE},
    {OP_SBC, ADDR_ABSOLUTE, OPGROUP_R},
    {OP_INC, ADDR_ABSOLUTE, OPGROUP_RW},
    {OP_ISC, ADDR_ABSOLUTE, OPGROUP_RW},
    {OP_BEQ, ADDR_RELATIVE, OPGROUP_NONE},
    {OP_SBC, ADDR_INDIRECT_Y, OPGROUP_R},
    {OP_JAM, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_ISC, ADDR_INDIRECT_Y, OPGROUP_RW},
    {OP_NOP, ADDR_ZERO_PAGE_X, OPGROUP_R},
    {OP_SBC, ADDR_ZERO_PAGE_X, OPGROUP_R},
    {OP_INC, ADDR_ZERO_PAGE_X, OPGROUP_RW},
    {OP_ISC, ADDR_ZERO_PAGE_X, OPGROUP_RW},
    {OP_SED, ADDR_IMPLIED, OPGROUP_NONE},
    {OP_SBC, ADDR_ABSOLUTE_Y, OPGROUP_R},
    {OP_NOP, ADDR_IMPLIED, OPGROUP_R},
    {OP_ISC, ADDR_ABSOLUTE_Y, OPGROUP_RW},
    {OP_NOP, ADDR_ABSOLUTE_X, OPGROUP_R},
    {OP_SBC, ADDR_ABSOLUTE_X, OPGROUP_R},
    {OP_INC, ADDR_ABSOLUTE_X, OPGROUP_RW},
    {OP_ISC, ADDR_ABSOLUTE_X, OPGROUP_RW},
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

#endif