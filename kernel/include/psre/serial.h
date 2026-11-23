#ifndef PSRE_SERIAL_H
#define PSRE_SERIAL_H
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void serial_initialize(void);
void serial_write_char(char c);
void serial_write(const char *text);
/* Optional tap: every character sent to the UART is also passed to the sink (NULL = off).
   Used by the GUI terminal to capture command output. */
typedef void (*serial_sink_t)(char c);
void serial_set_sink(serial_sink_t sink);
char serial_read_char(void);
bool serial_try_read_char(char *out);
void serial_read_line(char *buffer, unsigned long capacity);

#ifdef __cplusplus
}
#endif

#endif
