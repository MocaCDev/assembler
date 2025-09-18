#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>

/*#define page_size 1024

uint8_t *ROM = NULL;

void init_ROM(size_t ROM_size) {
    ROM = (uint8_t *)malloc(ROM_size);
}

void init_ROM_page(uint8_t page) {
    uint16_t offset = 0;

    while(offset < page_size) {
        ROM[(page * page_size) + offset] = 0;
        offset++;
    }

    if(page + 1 < 2)
        ROM[((page * page_size) + offset) + 1] = 0;

    printf("Initialized page %d (region %X-%X).\n",
        page + 1,
        page == 0 ? 0x0 : ((page - 1) << 10) | 0x3FF, (page << 10) | 0x3FF
    );
}*/

/*typedef struct _ROMAllocation {
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

ROMAllocation init_ROM_section(uint8_t page, uint16_t offset, size_t length) {
    size_t original_length = length;

    while(length > 0) {
        ROM[(page * page_size) + (offset + length)] = '%';
        length--;
    }

    return(ROMAllocation){
        .address=static_cast<uint16_t>((page << 10) | offset),
        .start=ROM+((page * page_size) + (offset + 0)),
        .end=ROM+((page * page_size) + (offset + original_length)),
        length=original_length,
        .assign=assign_ROM_data
    };
}*/

#define __DEBUG__

#include "ROM/rom.hpp"
#include "RAM/ram.hpp"
#include "emulator/emulator.hpp"
#include <string.h>

void display_RAM_information(RAM r)
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return;
    }

    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "SDL2 Text",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont("/Users/aidanwhite/Downloads/Inter/static/Inter_24pt-Black.ttf", 12);
    if (!font) {
        std::cerr << "TTF_OpenFont failed: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return;
    }

    // Render text
    SDL_Color color = {255, 255, 255, 255}; // White text
    char text[60] = { 0 };
    snprintf(text, sizeof(text), "%ld pages allocated out of 1,000,000.", r.pages_used());
    char other_text[135] = { 0 };// (char *)calloc(r.pages_used() * ram_page_size, sizeof(*other_text));
    for(int i = 0; i < 134; i++)
        other_text[i] = '1';
    other_text[133] = '2';
    
    add_to_line_data((uint8_t *)other_text);
    add_to_line_data((uint8_t *)"faggot1faggot1faggot1faggot1faggot1faggot1faggot1faggot1faggot1faggot1faggot1faggot1faggot1faggot1faggot1faggot1faggot1\0");
    add_to_line_data((uint8_t *)"faggot2");
    add_to_line_data((uint8_t *)"faggot3");
    add_to_line_data((uint8_t *)"faggot4");

    uint32_t bp = 6;
    SDL_Texture **textures = NULL;
    SDL_Rect *rects = NULL;
    size_t total_textures = 0;

    for(int i = 0; i < total_lines; i++)
    {
        if(textures == NULL) {
            textures = (SDL_Texture **)calloc(1, sizeof(*textures));
            rects = (SDL_Rect *)calloc(1, sizeof(*rects));
            total_textures++;
        } else {
            textures = (SDL_Texture **)realloc(
                textures,
                (total_textures + 1) * sizeof(*textures)
            );
            rects = (SDL_Rect *)realloc(
                rects,
                (total_textures + 1) * sizeof(*rects)
            );

            total_textures++;
        }

        SDL_Surface *surface = TTF_RenderText_Solid(
            font,
            reinterpret_cast<const char *>(line_data[i].data),
            color
        );

        textures[total_textures - 1] = SDL_CreateTextureFromSurface(renderer, surface);

        int textW, textH;
        SDL_QueryTexture(textures[total_textures - 1], nullptr, nullptr, &textW, &textH);
        rects[total_textures - 1] = {6, static_cast<int>(bp), textW, textH};

        SDL_FreeSurface(surface);
        bp += 20;
    }

    /*SDL_Surface* surface = TTF_RenderText_Solid(font, reinterpret_cast<const char *>(text), color);
    if (!surface) {
        std::cerr << "TTF_RenderText_Solid failed: " << TTF_GetError() << std::endl;
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return;
    }

    SDL_Surface* surface2 = TTF_RenderText_Solid(font, reinterpret_cast<const char *>(other_text), color);
    if (!surface) {
        std::cerr << "TTF_RenderText_Solid failed: " << TTF_GetError() << std::endl;
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return;
    }

    SDL_Texture* texture2 = SDL_CreateTextureFromSurface(renderer, surface2);
    if (!texture2) {
        std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface2);
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return;
    }

    SDL_FreeSurface(surface);

    // Get text dimensions
    int textW, textH;
    SDL_QueryTexture(texture, nullptr, nullptr, &textW, &textH);
    SDL_Rect dstRect = {6, 6, textW, textH}; // Center text

    int textW2, textH2;
    SDL_QueryTexture(texture2, nullptr, nullptr, &textW2, &textH2);
    SDL_Rect dstRect2 = {6, 26, textW2, textH2};*/

    // Main loop
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
        SDL_RenderClear(renderer);

        // Render text
        for(int i = 0; i < total_textures; i++)
        {
            SDL_RenderCopy(renderer, textures[i], nullptr, &rects[i]);
        }
        //SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
        //SDL_RenderCopy(renderer, texture2, nullptr, &dstRect2);

        // Update screen
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    for(int i = 0; i < total_textures; i++)
        SDL_DestroyTexture(textures[i]);

    free(rects);
    //SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

int main(int args, char *argv[]) {
    /*init_ROM(2048); // 2 pages

    uint8_t page = 0;
    while(page < 2) {
        init_ROM_page(page);
        page++;
    }

    ROMAllocation data = init_ROM_section(0, 10, 4);
    data.assign(data, (uint8_t *)"data");

    printf("%X\n", data.address);

    FILE *ROM_file = fopen("ROM.bin", "wb");
    fwrite((void *)ROM, sizeof(uint8_t), 2048, ROM_file);
    fclose(ROM_file);

    free(ROM);*/

    //ROM r = ROM((1024LL * 1024 * 1024) * 8);//(page_size * 2); // 2 pages (2,048 bytes)
    RAM ram = RAM(max_ram_size); // 4gb
    RAMAllocation *a1 = ram.init_RAM_section(4);
    ram.assign_RAM_data<uint8_t *>(a1, (uint8_t *)"data");

    RAMAllocation *test = ram.allocate_from_current_page(a1, 4);
    ram.assign_RAM_data<uint8_t *>(test, (uint8_t *)"test");

    RAMAllocation *a2 = ram.allocate_from_current_page(test, 5);
    ram.assign_RAM_data<uint8_t *>(a2, (uint8_t *)"data2");

    RAMAllocation *a3 = ram.allocate_from_current_page(a2, 5);
    ram.assign_RAM_data<uint8_t *>(a3, (uint8_t *)"data3");

    RAMAllocation *a4 = ram.allocate_from_current_page(a3, sizeof(uint32_t));
    ram.assign_RAM_data<uint32_t>(a4, (uint32_t)69);

    printf("%s\n%s\n%s\n%s\n%s\n", a1->start, test->start, a2->start, a3->start, a4->start);

    display_RAM_information(ram);

    return 0;
}
