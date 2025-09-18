#ifndef rom_h
#define rom_h

#include "../common.hpp"

#define page_size 1024
#define max_rom_size page_size * 2
#define page_offset(p, o) (p * page_size) + offset

// Data over any given region in ROM that is being allocated
typedef struct _ROMAllocation {
    uint16_t address;
    uint8_t *start;
    uint8_t *end;
    size_t length;
    void (*assign) (struct _ROMAllocation, uint8_t*);
} ROMAllocation;

void assign_ROM_data(struct _ROMAllocation RA, uint8_t *data) {
    size_t index = 0;

    while(index < RA.length)
        *(RA.start++) = *(data + index++);

    *RA.end = 0;
}

class ROM
{
private:
    size_t ROM_size = 0;
    uint8_t *ROM_data = NULL;

public:
    ROM(size_t rom_size): ROM_size(rom_size)
    {
        ROM_data = (uint8_t *)malloc(ROM_size);
    } 

    ROMAllocation init_ROM_section(uint8_t page, uint16_t offset, size_t length) {
        size_t original_length = length;

        while(length > 0) {
            this->ROM_data[(page * page_size) + (offset + length)] = '%';
            length--;
        }

        return(ROMAllocation){
            .address=static_cast<uint16_t>((page << 10) | offset),
            .start=this->ROM_data+((page * page_size) + (offset + 0)),
            .end=this->ROM_data+((page * page_size) + (offset + original_length)),
            .length=original_length,
            .assign=assign_ROM_data
        };
    }

    ~ROM()
    {
        free(this->ROM_data);
    }
};

#endif