#ifndef instructions_h
#define instructions_h

#include "../common.hpp"

#define R8_MOV      0xB0 // 8-bit `mov` instructions are 0xB0-0xB8
#define R16_MOV     0xB8 // 16-bit `mov` instructions are 0xB8-0xBB

typedef enum _RegisterTypes
{
    AX=0x1, AH=0x5, AL=0x9,
    BX=0x2, BH=0x6, BL=0xA,
    CX=0x3, CH=0x7, CL=0xB,
    DX=0x4, DH=0x8, DL=0xC
} RegisterTypes;

template<typename T>
    requires (std::same_as<T, uint8_t> || std::same_as<T, uint16_t>)
struct Registers
{
    RegisterTypes parent_reg; // if it is `ax`, `bx`, `cx`, or `dx` both `parent_reg` and `reg` will be the same thing
    RegisterTypes reg;
    T data;
    bool is_lower_half; // `al`, `bl`, `cl`, and `dl`

    /*uint16_t ax; uint8_t ah; uint8_t al;
    uint16_t bx; uint8_t bh; uint8_t bl;
    uint16_t cx; uint8_t ch; uint8_t cl;
    uint16_t dx; uint8_t dh; uint8_t dl;*/
};

struct Registers<uint8_t> child_regs[8] = {
    {.parent_reg=AX, .reg=AH, .data=0, .is_lower_half=false}, {.parent_reg=AX, .reg=AL, .data=0, .is_lower_half=true},
    {.parent_reg=BX, .reg=BH, .data=0, .is_lower_half=false}, {.parent_reg=BX, .reg=BL, .data=0, .is_lower_half=true},
    {.parent_reg=CX, .reg=CH, .data=0, .is_lower_half=false}, {.parent_reg=CX, .reg=CL, .data=0, .is_lower_half=true},
    {.parent_reg=DX, .reg=DH, .data=0, .is_lower_half=false}, {.parent_reg=DX, .reg=DL, .data=0, .is_lower_half=true},
};

struct Registers<uint16_t> parent_regs[4] = {
    {.parent_reg=AX, .reg=AX, .data=0},
    {.parent_reg=BX, .reg=BX, .data=0},
    {.parent_reg=CX, .reg=CX, .data=0},
    {.parent_reg=DX, .reg=DX, .data=0},
};

template<typename T=uint8_t>
    requires (std::same_as<T, uint8_t> || std::same_as<T, uint16_t>)
void update_register_value(RegisterTypes for_register, T data)
{
    if(for_register >= AX && for_register <= DX)
    {
        // Parent reg
        uint8_t high_bits = (data >> 8) & 0xFF;
        uint8_t low_bits = data & 0xFF;

        for(int i = 0; i < 4; i++)
        {
            if(parent_regs[i].reg == for_register)
            {
                parent_regs[i].data = data;

                printf("Parent: %X, ", data);

                for(int x = 0; x < 8; x++)
                {
                    if(child_regs[x].parent_reg == for_register)
                    {
                        if(child_regs[x].is_lower_half)
                        {
                            child_regs[x].data = low_bits;
                            printf("Low-Child: %X\n", low_bits);
                        }
                        else
                        {
                            child_regs[x].data = high_bits;
                            printf("High-Child: %X, ", high_bits);
                        }
                    }
                }

                break;
            }
        }
    } else {
        // Child reg
        for(int i = 0; i < 8; i++)
        {
            if(child_regs[i].reg == for_register)
            {
                int x = 0;
                for(x = 0; x < 4; x++)
                    if(parent_regs[x].reg == child_regs[i].parent_reg)
                        break;

                child_regs[i].data = data;
                if(child_regs[i].is_lower_half)
                {
                    uint8_t high_bits = (parent_regs[x].data >> 8) & 0xFF;
                    parent_regs[x].data = (high_bits << 8) | data;
                } else {
                    uint8_t low_bits = parent_regs[x].data & 0xFF;
                    parent_regs[x].data = (data << 8) | low_bits;
                }

                printf("Child: %X, Parent: %X\n", child_regs[i].data, parent_regs[x].data);

                break;
            }
        }
    }
}



#endif