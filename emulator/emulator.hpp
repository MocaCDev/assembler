#ifndef emulator_h
#define emulator_h

#include "../common.hpp"

#define max_character_per_line 131

typedef struct _LineData
{
    size_t length;
    uint8_t *data;
} LineData;

static LineData *line_data = NULL;
static size_t total_lines = 0;

void add_to_line_data(uint8_t *data)
{
    if(line_data == NULL)
    {
        line_data = (LineData *)calloc(1, sizeof(*line_data));
        total_lines++;
    } else {
        line_data = (LineData *)realloc(
            line_data,
            (total_lines + 1) * sizeof(*line_data)
        );
        total_lines++;
    }

    printf("%s, %ld, %ld\n", data, strlen(reinterpret_cast<const char *>(data)), max_character_per_line);

    if(strlen(reinterpret_cast<const char *>(data)) >= max_character_per_line)
    {
        line_data[total_lines-1].data = (uint8_t *)calloc(max_character_per_line, sizeof(uint8_t));
        memcpy(line_data[total_lines-1].data, data, max_character_per_line - 1);
        line_data[total_lines-1].data[max_character_per_line-1] = 0;

        if(!(strlen(reinterpret_cast<const char *>(data)) == max_character_per_line))
        {
            line_data = (LineData *)realloc(
                line_data,
                (total_lines + 1) * sizeof(*line_data)
            );
            total_lines++;

            line_data[total_lines-1].data = (uint8_t *)calloc((strlen(reinterpret_cast<const char *>(data)) - max_character_per_line) + 1, sizeof(uint8_t));
            memcpy(line_data[total_lines-1].data, data+(max_character_per_line), (strlen(reinterpret_cast<const char *>(data)) - max_character_per_line));
            line_data[total_lines-1].data[strlen(reinterpret_cast<const char *>(data)) - max_character_per_line + 1] = 0;
        }
    } else {
        line_data[total_lines-1].data = (uint8_t *)calloc(strlen(reinterpret_cast<const char *>(data)), sizeof(uint8_t));
        memcpy(line_data[total_lines-1].data, data, strlen(reinterpret_cast<const char *>(data)));
    }
}

#endif