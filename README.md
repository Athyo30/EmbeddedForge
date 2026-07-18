# embedded-forge

A hands-on bare-metal embedded systems study track in C and ARM assembly,
targeting Cortex-M3 on QEMU. Five parts, each building on the last: a libc
reimplementation, bare-metal startup, an interrupt-driven driver, a
bootloader, and a context-switching scheduler.



Update: A follow-up project focussed on custom RTOS has been built: https://github.com/Athyo30/Forge-RTOS/tree/main

## Structure

```
part1_libc/          libc reimplementation + a bump and free-list allocator
                      (host build, no hardware/emulator needed)
part2_startup/        bare-metal reset handler: .data copy, .bss zero, vector table
part3_drivers/        interrupt-driven UART RX with a lock-free ring buffer
part4_bootloader/     vector table relocation + jump to a second application image
part5_concurrency/     cooperative round-robin scheduler with a hand-rolled context switch
setup.sh              installs the toolchain
DEBUGGING_JOURNAL.md  full bug-by-bug write-up
```

## What I did vs. what was scaffolded

I implemented every function in every part — the allocators, the UART
driver, the bootloader's validation and VTOR relocation logic, the
scheduler's switching policy — and debugged the real bugs that came up
along the way (a segfault from unsigned integer underflow, a linked-list
corruption bug that passed its own test suite, a race condition between
an ISR and `main`, among others).

Build tooling was AI-scaffolded so study time went into the concepts
rather than environment setup: linker scripts, Makefiles, the test
harness, and a few pieces of ARM assembly boilerplate too fiddly to be a
good place to learn by trial and error (the naked jump-to-application
function in Part 4, the register save/restore skeleton in Part 5).
Full write-up of every bug, with root cause and fix, is in
[`DEBUGGING_JOURNAL.md`](./DEBUGGING_JOURNAL.md).

A preemptive scheduling extension (SysTick + PendSV, forcing a context
switch without an explicit `yield()`) was attempted and deliberately
shelved rather than shipped half-working — noted at the bottom of the
journal.

## Key learnings

- **`void*` used to hold a byte count or an integer** is a type-category
  error, not a style nit — it showed up three separate times (an
  allocator, a stack offset, a peripheral register write) before the
  pattern was obvious.
- **Unsigned loop counters can't go negative** — a backward loop bounded
  by `i >= 0` on a `size_t` never terminates; it underflows and wraps
  instead, which is a segfault waiting to happen, not a warning to
  ignore.
- **Passing tests only prove what they exercise.** A free-list split bug
  that truncated the block list was invisible for several test runs
  because every allocation happened to land on the list's tail.
- **Lock-free doesn't mean uncoordinated** — a single-producer/single-
  consumer ring buffer is safe without a mutex only because each shared
  field has exactly one writer; a shared counter updated from both an ISR
  and `main` breaks that invariant immediately.
- **A bootloader is a program that fakes a hardware reset** — jumping to
  an application means setting SP/PC *and* relocating `SCB->VTOR`, two
  independent mechanisms that are easy to only do one of.
- **AAPCS, not compiler magic, is what lets C and hand-written assembly
  cooperate** — arguments in r0/r1, callee-saved r4-r11 — the same
  convention underwrites the bootloader's asm jump and the scheduler's
  context switch.

## Toolchain

- `gcc` / `make` - host build for Part 1 (no target hardware involved)
- `arm-none-eabi-gcc` - cross-compiler for Cortex-M3, Parts 2-5
- `qemu-system-arm` - emulates the `lm3s6965evb` board (no physical
  hardware needed for any part)

Tested on WSL2 (Ubuntu) and native Linux. Run `./setup.sh` once, then
`cd partN_* && make test` (Part 1) or `make qemu` (Parts 2-5).
