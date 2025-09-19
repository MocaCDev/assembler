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
#include <vector>

static int windowHeight = 800, windowWidth = 600;

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
        windowHeight, windowWidth,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
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
    TTF_Font* font = TTF_OpenFont("Inter_18pt-Regular.ttf", 12);
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

    std::string testString("Moonlit hills bloomed with vivid wildflowers, swaying gently under a starlit velvet sky, enchanting wanderers who roamed through misty valleys seeking ancient secrets hidden in shadows.\n"
                         "Restless rivers carved winding paths through rugged stones, their ripples echoing forgotten tales of heroic knights, mystical creatures, and lost kingdoms whispered by the evening breeze.\n"
                         "By sunrise, bustling markets teemed with merchants offering exotic spices, shimmering silks, handcrafted treasures, and rare artifacts, drawing crowds eager for adventure and tales of distant lands.\n"
                         "Children raced across vibrant meadows, their colorful kites soaring high against golden clouds, sparking dreams of epic quests.");
    std::vector<std::string> lines;
    std::string word;
    std::string currentLine;
    int maxW = 600; // 6px of padding on left/right side of screen 
    size_t i = 0;

    /*while(i < testString.size() + 1)
    {
        char c = (i < testString.size()) ? testString[i] : ' ';

        if(i == ' ' || i == '\n' || i == testString.size())
        {
            if (!word.empty()) {
                std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
                int w, h;
                TTF_SizeText(font, testLine.c_str(), &w, &h);

                if (w > maxW) {
                    // line too long → push currentLine, start new line with word
                    if (!currentLine.empty()) lines.push_back(currentLine);
                    currentLine = word;
                } else {
                    currentLine = testLine;
                }

                word.clear();
            }

            if (c == '\n') {
                lines.push_back(currentLine);
                currentLine.clear();
            }
        }  else {
            word += c;
        }

        i++;
    }*/

    uint32_t bp = 6;
    SDL_Texture **textures = NULL;
    SDL_Rect *rects = NULL;
    size_t total_textures = 0;

    const auto createTextures = [&](int ww)
    {
        i = 0;
        bp = 6;
        lines.clear();
        
        if(textures != NULL) {
            for(size_t i = 0; i < total_textures - 1; i++)
                SDL_DestroyTexture(textures[i]);

            free(rects);
            free(textures);

            textures = nullptr;
            rects = nullptr;
        }

        total_textures = 0;

        while(i < testString.size())
    {
        currentLine += testString[i] != '\n' ? testString[i] : '\0';
        int w, h;
        TTF_SizeText(font, currentLine.c_str(), &w, &h);

        if(testString[i] == '\n' || i == testString.size() - 1 || w >= ww)
        {
            lines.push_back(currentLine);
            currentLine.clear();
        }

        i++;
    }

        for(const std::string& line: lines)
    {
        if(textures == nullptr) {
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

        printf("Line: %s\n", line.c_str());

        SDL_Surface *surface = TTF_RenderText_Solid(
            font,
            line.c_str(),
            color
        );

        textures[total_textures - 1] = SDL_CreateTextureFromSurface(renderer, surface);

        int textW, textH;
        SDL_QueryTexture(textures[total_textures - 1], nullptr, nullptr, &textW, &textH);
        rects[total_textures - 1] = {6, static_cast<int>(bp), textW, textH};

        SDL_FreeSurface(surface);

        bp += 20;
    }
    };

    createTextures(windowWidth);

    bool running = true;
    bool resized = false;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            switch(event.type)
            {
                case SDL_QUIT: running=false;break;
                case SDL_WINDOWEVENT:
                {
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        windowHeight = event.window.data2;
                        windowWidth = event.window.data1;

                        if(windowWidth < 400 || windowHeight < 400)
                        {
                            windowWidth = std::max(windowWidth, 400);
                            windowHeight = std::max(windowHeight, 400);
                            SDL_SetWindowSize(window, windowWidth, windowHeight);
                        }

                        createTextures(windowWidth - 15);
                    }

                    break;
                }
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

    free(rects);

    // Cleanup
    for(int i = 0; i < total_textures; i++)
        SDL_DestroyTexture(textures[i]);

    free(textures);

    //free_line_data();
    r.free_ram();
    
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
    //RAM ram = RAM(max_ram_size); // 4gb
    /*RAMAllocation *a1 = ram.init_RAM_section(4);
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

    display_RAM_information(ram);*/
    Emulator emulator("test.bin");

    return 0;
}
