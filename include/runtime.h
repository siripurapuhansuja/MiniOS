#ifndef MINIOS_RUNTIME_H
#define MINIOS_RUNTIME_H

/* Initialize the queue, reader, and worker pool. */
int runtime_start(void);

/* Submit a job to the worker queue. */
void runtime_submit(const char *name);

/* Stop workers, drain results, and release resources. */
void runtime_stop(void);
void runtime_print_workers(void);

#endif
