
////////////////////
// neo6502_glue.c //
////////////////////

#include <unistd.h>
#include <stdint.h>
#include <peekpoke.h>
#include <assert.h>
#include <string.h>

#define ApiGroup        0xff00
#define ApiFunction     0xff01
#define ApiError        0xff02
#define ApiStatus       0xff03
#define ApiParam0       0xff04
#define ApiParam1       0xff05
#define ApiParam2       0xff06
#define ApiParam3       0xff07
#define ApiParam4       0xff08
#define ApiParam5       0xff09
#define ApiParam6       0xff0a
#define ApiParam7       0xff0b

#define ApiGroup_System 1
#define ApiGroup_System_Function_SystemReset 7
#define ApiGroup_Console 2
#define ApiGroup_Console_Function_ReadCharacter 1
#define ApiGroup_Console_Function_WriteCharacter 6
#define ApiGroup_Sound 8
#define ApiGroup_Sound_Function_PlaySound 5

///////

static void neo6502_api_request(uint8_t group, uint8_t function)
{
    do {} while (PEEK(ApiGroup) != 0);
    POKE(ApiFunction, function);
    POKE(ApiGroup, group);
    do {} while (PEEK(ApiGroup) != 0);
}

///////

void neo6502_system_reset(void)
{
     neo6502_api_request(ApiGroup_System, ApiGroup_System_Function_SystemReset);
}

void neo6502_sound_play_sound(uint8_t channel, uint8_t sound)
{
    POKE(ApiParam0, channel);
    POKE(ApiParam1, sound);
    neo6502_api_request(ApiGroup_Sound, ApiGroup_Sound_Function_PlaySound);
}

uint8_t neo6502_console_read_character(void)
{
    neo6502_api_request(ApiGroup_Console, ApiGroup_Console_Function_ReadCharacter);
    return PEEK(ApiParam0);
}

void neo6502_console_write_character(uint8_t c)
{
    POKE(ApiParam0, c);
    neo6502_api_request(ApiGroup_Console, ApiGroup_Console_Function_WriteCharacter);
}

///////

static char get_console_char(void)
{
    char c;
    do {
        c = neo6502_console_read_character();
    } while (c == 0);
    if (c == '\r')
    {
         c= '\n';
    }
    return c;
}

static void put_console_char(char c)
{
    if (c == '\n')
    {
         c = '\r';
    }

    if (c == '\a')
    {
        neo6502_sound_play_sound(0, 4);
    }
    else
    {
        neo6502_console_write_character(c);
    }
}


#define MAX_LBUF_SIZE 54
char LBUF[MAX_LBUF_SIZE];
uint8_t LBUF_SIZE = 0;

void linebuffer_interact(void)
{
    char c;
    for (;;)
    {
        c = get_console_char();
        if (c == '\n')
        {
            // Accept and return.
            LBUF[LBUF_SIZE++] = c;
            put_console_char(c);
            break;
        }
        else if (c == '\b')
        {
            if (LBUF_SIZE != 0)
            {
                --LBUF_SIZE;
                put_console_char(c);
            }
        }
        else
        {
            if (LBUF_SIZE + 1 < MAX_LBUF_SIZE)
            {
                // We're only willing to accept if there's still room for a newline after it.
                LBUF[LBUF_SIZE++] = c;
                put_console_char(c);
            }
            else
            {
                put_console_char('\a');
            }
        }
    }
}

void linebuffer_drop(uint8_t count)
{
    assert(LBUF_SIZE >= count);
    LBUF_SIZE -= count;
    memmove(LBUF, LBUF + count, LBUF_SIZE);
}

int read(int fd, void* buf, unsigned count)
{
    if (!(fd == STDIN_FILENO))
    {
        return -1;
    }

    if (count != 0)
    {
        while (LBUF_SIZE == 0)
        {
            linebuffer_interact();
        }

        if (LBUF_SIZE < count)
        {
            count = LBUF_SIZE;
        }

        memcpy(buf, LBUF, count);
        linebuffer_drop(count);
    }

    return count;
}

int write(int fd, const void *buf, unsigned count)
{
    unsigned k;
    char *cbuf = buf;

    if (!(fd == STDOUT_FILENO || fd == STDERR_FILENO))
    {
        return -1;
    }

    // Write characters to keyboard.

    for (k = 0; k < count; ++k)
    {
        put_console_char(cbuf[k]);
    }

    return count;
}
