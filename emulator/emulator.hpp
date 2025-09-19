#ifndef emulator_h
#define emulator_h

#include "../common.hpp"
#include "instructions.hpp"
#include "../RAM/ram.hpp"

class Emulator : RAM
{
private:
    uint8_t *binary = NULL;

public:
    Emulator(const char *filename): RAM(max_ram_size)
    {
        FILE *image = fopen(filename, "rb");

        ASSERT(
            image,
            {
                free_ram();
            },
            "Unable to open the file %s\n",
            filename
        )

        fseek(image, 0, SEEK_END);
        size_t file_size = ftell(image);
        fseek(image, 0, SEEK_SET);

        this->binary = (uint8_t *)calloc(file_size, sizeof(*this->binary));

        ASSERT(
            this->binary,
            {
                fclose(image);
                free_ram();
            },
            "Unable to allocate %ld bytes for the image.\n",
            file_size
        )

        fread(this->binary, sizeof(*this->binary), file_size, image);

        fclose(image);

        /* Check to make sure `55AA` is at the 511th and 512th byte. */
        ASSERT(
            this->binary[510] == 0x55 && this->binary[511] == 0xAA,
            {
                free(this->binary);
                free_ram();
            },
            "Missing MBR signature.\n"
        )

        uint8_t mbr[512] = { 0 };
        memcpy((void *)mbr, this->binary, 512);

        RAMAllocation *MBR = allocate_at_address<uint16_t>(0x7C00, 512);
        assign_RAM_data<uint8_t *>(MBR, mbr, true);

        size_t i = 0;
        uint8_t current_char = this->binary[i];

        const auto next_char = [this, &i, &current_char]()
        {
            i++;
            current_char = this->binary[i];
        };

        while(i < file_size)
        {
            if(current_char >= 0xB0 && current_char <= 0xBB)
            {
                if(current_char >= 0xB0 && current_char < 0xB8)
                {
                    switch(current_char & 0x0F)
                    {
                        case 0x4: // `ah`
                        {
                            next_char();
                            update_register_value<uint8_t>(AH, current_char);
                            //child_regs.ah = current_char;

                            break;
                        }
                        case 0x0: // `al`
                        {
                            next_char();
                            update_register_value<uint8_t>(AL, current_char);
                            //child_regs.al = current_char;
                            
                            break;
                        }
                        case 0x7: // `bh`
                        {
                            next_char();
                            update_register_value<uint8_t>(BH, current_char);
                            //child_regs.bh = current_char;
                            
                            break;
                        }
                        case 0x3: // `bl`
                        {
                            next_char();
                            update_register_value<uint8_t>(BL, current_char);
                            //child_regs.bl = current_char;
                            
                            break;
                        }
                        case 0x5: // `ch`
                        {
                            next_char();
                            update_register_value<uint8_t>(CH, current_char);
                            //child_regs.ch = current_char;
                            
                            break;
                        }
                        case 0x1: // `cl`
                        {
                            next_char();
                            update_register_value<uint8_t>(CL, current_char);
                            //child_regs.cl = current_char;
                            
                            break;
                        }
                        case 0x6: // `dh`
                        {
                            next_char();
                            update_register_value<uint8_t>(DH, current_char);
                            //child_regs.dh = current_char;
                            
                            break;
                        }
                        case 0x2: // `dl`
                        {
                            next_char();
                            update_register_value<uint8_t>(DL, current_char);
                            //child_regs.dl = current_char;
                            
                            break;
                        }
                    }

                    next_char();
                    continue;
                }

                uint8_t reg = current_char;
                
                next_char();
                uint8_t low_bits = current_char;
                
                next_char();
                uint8_t high_bits = current_char;

                uint16_t value = (high_bits << 8) | low_bits;

                switch(reg & 0x0F)
                {
                    case 0x8: // `ax`
                    {
                        update_register_value<uint16_t>(AX, value);
                        break;
                    }
                    case 0xB: // `bx`
                    {
                        update_register_value<uint16_t>(BX, value);
                        break;
                    }
                    case 0x9: // `cx`
                    {
                        update_register_value<uint16_t>(CX, value);
                        break;
                    }
                    case 0xA: // `dx`
                    {
                        update_register_value<uint16_t>(DX, value);
                        break;
                    }
                    default: {
                        fprintf(stderr, "Something went wrong.\n");
                        free(this->binary);
                        free_ram();
                        exit(EXIT_FAILURE);
                    }
                }

                next_char();
                continue;
            }

            next_char();
        }

        /*uint8_t v1 = this->binary[1];
        uint8_t v2 = this->binary[2];
        uint16_t v3 = (this->binary[2] << 8) | this->binary[1];
        printf("%X\n", 0xB8 & 0xB0);*/

        free(this->binary);
        free_ram();
    }
};

#endif