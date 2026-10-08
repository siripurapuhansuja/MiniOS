#ifndef MINIOS_H
#define MINIOS_H

void shell_loop(void);
void show_system_info(void);
int  run_program(char **args);   /* M2 writes this in process.c */

#endif

