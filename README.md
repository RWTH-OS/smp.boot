SMP Boot 2 Benchmark
====================

This is a small test kernel for 32- and 64-bit x86 systems. It started as an
experiment for Multiboot (see README.multiboot), but evolved around r555
(2011-09-01) into a vehicle for low level system benchmarks supporting multiple
CPUs (as of 2011-09-19, r589, see README.smp).

Contact: Georg Wassen <wassen@lfbs.rwth-aachen.de>

Architecture (as of r578, 2011-09-11)
-------------------------------------

    start32.asm        start64.asm      [32-bit early boot code]
               start.asm                [32-bit early boot code for both architectures]
               boot32.c                 [32-bit machine check in real-mode]
                       jump64.asm       [64 bit set-up]
               start_smp.inc            [16 bit bootstrap code into 32 bit protected mode]
    ...................................................................................
                 main.c                 [actual kernel]
                   

| File            | Content                                                                            |
|-----------------|------------------------------------------------------------------------------------|
| `start32.asm`   | Multiboot/a.out start code, set-up of GDT+Interrupts for 32-bit kernel             |
| `start64.asm`   | Multiboot/elf32 start code, continue in jump64.asm                                 |
| `start.asm`     | common initialization code for both                                                |
| `boot32.c`      | 32-bit real-mode C code for bootstrap (hardware analyzation)                       |
| `jump64.asm`    | set-up of GDT+Interrupts for 64-bit kernel                                         |
| `start_smp.inc` | bootstap code for Application processors (included in start32.asm and jump64.asm)  |
|                 |                                                                                    |
| `lib.c`         | service functions for 32-bit start code and both kernels                           |
| `screen.c`      | screen output for 32-bit start code and both kernels                               |
|                 |                                                                                    |
| `main.c`        | start routine of actual kernel in protected mode                                   |
| `*.c`           | other helper functions and subsystems                                              |


