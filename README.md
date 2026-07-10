# embedded-forge

A build-to-learn track for the hardware/software boundary. You write the
code; the infra (build system, linker script, QEMU targets, test harness,
vector tables) is already done so you spend zero time on setup.

The rule: **do NOT paste in a full solution.** Every file is stubbed with
the contract, the edge cases, and a spot to write your approach *before*
you code. The tests define correctness — make them pass.

## Quickstart

```bash
./setup.sh                 # installs gcc, arm-none-eabi-gcc, qemu (once)
cd part1_libc && make test # start here — instant pass/fail loop
```

## The five practice disciplines (do these every single time)

These target the specific things that cost you in live coding. Treat them
as non-negotiable reps, not suggestions:

1. **Write the approach + complexity in the comment block BEFORE coding.**
   One line. This is the "state your approach first" interview habit.
2. **Trust the simple version.** Write the byte-by-byte / obvious solution
   first. Only optimize after it passes. Do not reach for word-copies,
   fancy structures, or clever tricks up front.
3. **Test unprompted, on multiple cases.** After it compiles, run it. The
   harness already gives you edge cases — add your own when you spot a gap.
4. **When stuck, narrate — don't go silent.** Write your confusion as a
   comment: "not sure if dst>src needs reverse copy — testing that." Keep
   moving on paper.
5. **Verify after coding.** Before declaring done, re-read against the
   contract and predict which test would fail. Then run it.

## Parts

- **part1_libc/** — reimplement `memset/memcpy/memmove/memcmp`,
  `strlen/strcpy/strncpy/strcmp`, then a **bump allocator** and a
  **free-list allocator** (split + coalesce). Host build, instant tests.
  `make test`, and `make asan` to catch pointer bugs in the allocator.
- **part2_startup/** — write the **reset handler**: copy `.data` from
  flash to RAM, zero `.bss`, call `main`. Runs on QEMU's Cortex-M3.
  `make qemu` — it prints whether each step worked. (Ctrl-A then x quits.)
- **part3_drivers/** — write an **interrupt-driven UART RX driver** with a
  lock-free ring buffer. This is where `volatile` / atomicity / memory
  ordering stop being theory. `make qemu`, then type — it echoes.
- **part4_bootloader/** — write the **bootloader handoff**: validate an
  application image's vector table, relocate `SCB->VTOR` to it, and jump
  into it. This is the real mechanism behind firmware updates and
  multi-stage boot. `make qemu` — prints the app's SP/PC, relocates,
  jumps, and the app confirms VTOR landed correctly. (Note: to run in
  QEMU with a single `-kernel` load, both "images" are linked into one
  ELF at two different flash addresses — a real system would flash them
  separately. Said plainly in the linker script.)
- **part5_concurrency/** — build a **cooperative round-robin scheduler**:
  two tasks, each with its own stack, switched via an explicit `yield()`
  that saves/restores registers and swaps the stack pointer by hand. This
  is a real context switch — same idea as a coroutine/fiber library.
  `make qemu` — correct output is `ABABABAB...` forever, proving both
  tasks' state survives being paused and resumed. The scheduling *policy*
  (round-robin) is your exercise; the register save/restore mechanics are
  provided, since getting that asm subtly wrong fails silently rather
  than with a clear error — not a good place to learn by trial and error.

## Beyond this track

Preemptive scheduling (a timer interrupt forces a switch even if a task
never calls `yield()`) is the natural next step after Part 5, using
SysTick + PendSV + a PSP/MSP stack split. It's deliberately not scaffolded
here — bootstrapping that first switch correctly is genuinely fiddly even
for experienced embedded engineers (real RTOS ports carry a lot of care
right at that handoff). Worth reading FreeRTOS's Cortex-M `port.c` once
Part 5 feels easy, as a next real target rather than a scaffolded exercise.


## Why QEMU, not your STM32

Fastest possible edit→run loop, no flashing, no ST-Link, fully
reproducible. Everything here maps directly onto real Cortex-M silicon —
when you want hardware realism (real clock/pin init, a logic analyzer on
the wire), the STM32 track is a one-file swap of the toolchain and the
register base addresses.
