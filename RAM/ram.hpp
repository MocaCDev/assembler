#ifndef ram_h
#define ram_h

#include "../common.hpp"

#define ram_page_size 4096
#define max_ram_size (1024LL * 1024 * 1024) * 4 // 4GB, 1,048,576 pages

typedef struct _RAMAllocation
{
    // Address in RAM of the given allocation
    uint64_t address;

    // The length of given allocation
    size_t length;

    // Some RAM allocations can take up multiple pages
    uint32_t start_page;
    uint32_t end_page;

    // `start` will be utilized to assign data to the allocation
    uint8_t *start;

    // `end` will be utilized to assign `0` at the end of the allocation
    uint8_t *end;
} RAMAllocation;

#define determine_RAM_address(p) p * ram_page_size

class RAM
{
private:
    size_t RAM_size = 0;
    uint8_t *RAM_bin = NULL;
    size_t total_allocated_pages = 0;
    RAMAllocation **allocations = NULL;
    size_t total_allocations = 0; // due to the fact multiple allocations can happen on one page, we need to keep track of how many there are in total

public:
    RAM(size_t RAM_size): RAM_size(RAM_size)
    {
        // We will allocation `RAM_size`, but not all of it will be utilized if not need be.
        RAM_bin = (uint8_t *)malloc(RAM_size);
    }

    size_t pages_used() { return this->total_allocated_pages; }
    uint8_t *get_RAM() { return this->RAM_bin; }

    // Allocate a part of RAM of `length` size
    RAMAllocation *init_RAM_section(size_t length)
    {
        // We need at least 1 page minimum
        uint32_t pages_needed = length / page_size == 0 ? 1 : length / page_size;

        if(this->total_allocations == 0)
        {
            this->allocations = (RAMAllocation **)calloc(1, sizeof(*this->allocations));
            this->allocations[this->total_allocations] = new RAMAllocation{
            .address=determine_RAM_address(this->total_allocated_pages),
            .length=length,
            .start_page=static_cast<uint32_t>(this->total_allocated_pages),
            .end_page=static_cast<uint32_t>(this->total_allocated_pages + pages_needed),
            .start=RAM_bin+(determine_RAM_address(this->total_allocated_pages)),
            .end=RAM_bin+(determine_RAM_address(this->total_allocated_pages) + length)
        };
            this->total_allocations++;
        } else {
            this->allocations = (RAMAllocation **)realloc(
                this->allocations,
                (this->total_allocations + 1) * sizeof(*this->allocations)
            );

            this->allocations[this->total_allocations] = new RAMAllocation{
            .address=determine_RAM_address(this->total_allocated_pages),
            .length=length,
            .start_page=static_cast<uint32_t>(this->total_allocated_pages),
            .end_page=static_cast<uint32_t>(this->total_allocated_pages + pages_needed),
            .start=RAM_bin+(determine_RAM_address(this->total_allocated_pages)),
            .end=RAM_bin+(determine_RAM_address(this->total_allocated_pages) + length)
        };
            this->total_allocations++;
        }

        this->total_allocated_pages += pages_needed;

        return this->allocations[this->total_allocations - 1];
    }

    template<typename T>
        requires (
            std::same_as<T, uint8_t> || std::same_as<T, uint8_t *> || std::same_as<T, uint32_t>
        )
    void assign_RAM_data(RAMAllocation *allocation, T data) {
        size_t index = 0;
        uint8_t *raw_data;

        if constexpr (std::is_same_v<T, uint32_t>)
        {
            uint8_t buffer[20] = { 0 };
            snprintf(reinterpret_cast<char *>(buffer), sizeof(buffer), "%d", data);
            raw_data = (uint8_t *)calloc(strlen(reinterpret_cast<const char *>(buffer)), sizeof(*raw_data));

            for(int i = 0; i < strlen(reinterpret_cast<const char *>(buffer)); i++)
                raw_data[i] = buffer[i];

            // Ensure we adjust the end pointer accordingly, as the raw value of the integer
            // could be more than the allocated size.
            allocation->length = strlen(reinterpret_cast<const char *>(buffer));
            allocation->end = RAM_bin+(allocation->address + allocation->length);
        } else {
            raw_data = data;
        }

        // Continue assigning so long as `index` is < allocation length, and the current index
        // of `data` is not NULL
        while(index < allocation->length && *(raw_data + index) != 0)
        {
            *(allocation->start + index) = *(raw_data + index);
            index++;
        }

        if constexpr (std::is_same_v<T, uint32_t>)
        {
            free(raw_data);
        }

        // Force terminating character at end of data if it falls short of the allocated chunks length.
        if(index < allocation->length)
        {
            #ifdef __DEBUG__
            printf("NOTE: Allocated %ld bytes, utilizing only %ld of it. Resizing to be %ld bytes.\n",
                allocation->length,
                index,
                index
            );
            #endif

            allocation->length = index;
            allocation->end = RAM_bin+(allocation->address + allocation->length);

            *allocation->end = 0;
            return;
        }

        *allocation->end = 0;
    }

    // Allocate more memory from a current allocated page (`allocation`)
    RAMAllocation *allocate_from_current_page(RAMAllocation *allocation, size_t length)
    {
        if(this->total_allocations == 0)
        {
            this->allocations = (RAMAllocation **)calloc(1, sizeof(*this->allocations));
            this->allocations[this->total_allocations] = new  RAMAllocation{
            .address=allocation->address + allocation->length + 1,
            .length=length,
            .start_page=allocation->start_page,
            .end_page=allocation->end_page,
            .start=RAM_bin+(allocation->address + allocation->length + 1),
            .end=RAM_bin+(allocation->address + allocation->length + 1 + length)
        };
            this->total_allocations++;
        } else {
            this->allocations = (RAMAllocation **)realloc(
                this->allocations,
                (this->total_allocations + 1) * sizeof(*this->allocations)
            );

            this->allocations[this->total_allocations] = new  RAMAllocation{
            .address=allocation->address + allocation->length + 1,
            .length=length,
            .start_page=allocation->start_page,
            .end_page=allocation->end_page,
            .start=RAM_bin+(allocation->address + allocation->length + 1),
            .end=RAM_bin+(allocation->address + allocation->length + 1 + length)
        };
            this->total_allocations++;
        }
        // `allocation->length + 1` to account for the null terminator in prior allocations
        return this->allocations[this->total_allocations - 1];
    }

    void free_ram()
    {
        if(RAM_bin != NULL) {
            printf("\nFreeing up %lu pages.\n\n", this->total_allocated_pages);

            for(uint32_t i = 0; i < this->total_allocated_pages; i++)
            {
                printf("Page: %d\n", i + 1);
                for(uint16_t x = 0; x < ram_page_size; x++)
                    printf("%c", RAM_bin[i * ram_page_size + x] == 0 ? '?' : RAM_bin[i * ram_page_size + x]);

                printf("\n\n");
            }

            printf("\n");

            // Free up each individual allocation first.
            for(size_t i = 0; i < this->total_allocations; i++)
                delete this->allocations[i];

            free(this->allocations);

            // Free up the actual RAM. 
            printf("Freeing up 4gb of RAM.\n");
            free(RAM_bin);
            RAM_bin = NULL;
        }
    }

    ~RAM()
    {}
};

#endif
