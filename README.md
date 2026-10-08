# MiniOS - Operating System Simulation Project

MiniOS is an educational operating system project built using C on Ubuntu Linux. It simulates basic operating system functions including a user shell, `/proc` system information parsing, process creation, and inter-process communication.

## Week 1-3 Features Progress
- **MiniOS Shell Interface:** Interactive CLI supporting `help`, `system`, `processes`, and `quit`.
- **System Information Module:** Reads `/proc/uptime` and `/proc/meminfo` for live system metrics.
- **Process Management Demos:** Demonstrates `fork()`, `wait()`, separate memory spaces, zombie/orphan processes, `execvp()` program replacement, and `pipe()` IPC.

## How to Build and Run
1. **Compile the Main Shell:**
   ```bash
   make
