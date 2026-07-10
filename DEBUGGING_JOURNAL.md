# Debugging journal — embedded-forge

Five bugs cost the most time and taught the most across this project. Each
is written the way I'd walk through it in an interview: what broke, why,
how I found it, and the fix. Concept notes I took while implementing each
part are folded in above the relevant section, since the bug only makes
sense once the mechanism is clear.

---

## Part 1 — memory allocators

### Concept: bump vs. free-list

A **bump allocator** keeps one pointer to the next free byte in a
pre-reserved arena. Allocation is "return the pointer, then advance it by
the request size" — a few cycles, zero fragmentation, but no individual
`free()`; the entire arena is reclaimed at once by resetting the pointer.
Real-world use: phase-oriented workloads (per-request or per-frame
allocation, discarded as a batch) and Rust's `bumpalo` crate.

A **free-list allocator** is the general-purpose version behind `malloc`.
It carves the arena into variable-sized blocks, each with a small header
(`size`, `free` flag, `next` pointer) stored *inline*, immediately before
its own payload. "Placing a block" means pointer-casting a raw address to
`struct header*` — no allocation happens, you're just telling the compiler
to interpret bytes you already own as that struct's layout. Search
strategy (first-fit here) decides which free block satisfies a request;
if the block is much bigger than needed, it gets **split**, creating a new
free header in the leftover space. Freeing a block and **coalescing** it
with an adjacent free neighbor is the split operation run in reverse —
undoing fragmentation as it's created.

### Bug 1 — `void*` used to hold a byte count

```c
void* new_sizing = (size + 7) & ~(size_t)7;
```

The right-hand side is a `size_t` arithmetic expression; assigning it to a
`void*` with no cast is a type-category error, not a style nit. `void*`
means "address of unknown type" — it has no business holding a count.
Traced the same error twice more in the same file (`curr_offset` declared
`void*`, used as an integer everywhere). Once both were `size_t`, every
downstream error — `base_ptr + curr_offset` failing to compile, "must have
integral type" — disappeared, because pointer arithmetic is only defined
as *pointer + integer*, never *pointer + pointer*, and C won't do
arithmetic on `void*` at all without a GCC extension quietly papering
over it.

**Lesson:** a `void*` holding a count is the same mistake as `int i`
compared against `size_t n` — the type system is telling you something
real, not being pedantic.

### Bug 2 — free-list split silently truncates the list

```c
new_head->next = NULL;   // should have been the OLD ptr->next
ptr->next = new_head;
```

When splitting a block, the leftover sliver (`new_head`) was hardcoded to
`NULL` instead of inheriting whatever `ptr->next` pointed to *before* the
split. Every block physically after the split point became permanently
unreachable.

**Why the test suite didn't catch it:** every allocation in the test
happened to consume the newest, always-at-the-tail free block, where
`next` was already `NULL` — so the bug was invisible until a split
happened on a *non-tail* block. Passing tests only prove what they
actually exercise; this one exposed a real gap in test coverage, not just
a code bug.

**Fix:** capture `old_next = ptr->next` before overwriting anything, then
`new_head->next = old_next`.

### Bug 3 — `memmove`, unsigned loop counter underflow → segfault

```c
for (size_t i = n - 1; i >= 0; i--)
```

`size_t` is unsigned; `i >= 0` is true for every possible value, forever.
After the `i == 0` iteration, `i--` doesn't go negative — it wraps to the
largest representable `size_t` (~1.8×10¹⁹ on 64-bit), and the loop walks
off the buffer into unmapped memory.

**Found via:** the compiler's own warning (`comparison of unsigned
expression in '>= 0' is always true`) plus `make asan` (AddressSanitizer),
which pinpointed a `stack-buffer-overflow` one byte before the buffer —
diagnostic enough to fix on the first try once read carefully.

**Fix:** switch the loop counter to a signed type, or use the
`for (size_t i = n; i-- > 0; )` idiom, which checks-then-decrements so it
never needs to represent "negative."

**The conceptual bug underneath it:** earlier reasoning had been "the
pointer always moves forward, so overlap doesn't matter." Traced by hand
with `memmove(buf+1, buf, 3)` on `"ABCDE"` — forward-copying with
`dst > src` overwrites `src[i+1]` before it's read, corrupting the copy
(`"AAAAE"` instead of `"AABCE"`). `memmove` has to copy *backward* when
`dst > src`, forward otherwise — the two-direction branch is the entire
reason the function exists.

### Bug 4 — `memcmp`/`strcmp`, checking the wrong condition

First `memcmp` attempt checked `if (a[i] > b[i])` and did nothing else —
never advanced, never stopped, never returned anything but `0`. The actual
algorithm: walk until the **first** differing byte, then return based on
that byte alone — every byte before it must have matched, everything after
is irrelevant. Same root mistake repeated in `strcmp`'s first draft: it
checked only byte 0, so `"abc"` vs `"abd"` incorrectly returned equal.

**Fix, both cases:** advance while equal; the moment a mismatch (or a
string's terminating NUL) appears, return `a[i] - b[i]` and stop.

---

## Part 3 — interrupt-driven UART / ring buffer

### Concept: why `head`/`tail` alone can detect full and empty

A naive lock-free ring buffer with only `head`/`tail` has one ambiguity:
`head == tail` looks identical whether the buffer is completely empty or
completely full. Adding a third `size` counter seems like the fix — but
`size++` (in the ISR) and `size--` (in `main`) are each a
read-modify-write, not one atomic step. If the ISR fires between `main`'s
read and write of `size`, one of the two updates gets silently lost, and
`size` drifts wrong forever.

The actual fix: sacrifice one storage slot. Full is redefined as "pushing
would make `tail` wrap around to equal `head`" — checked *before* it
happens, refusing that last push rather than letting the wrap occur. This
makes `head == tail` mean *only* empty, at the cost of one unusable byte
of capacity. Derived from tracing `RB_CAP=4` by hand: after 3 pushes,
`(tail+1) & (RB_CAP-1) == head` correctly refuses a 4th, even though one
physical slot is technically free.

**Why no lock is needed at all:** `rb_push` (ISR) only ever *writes*
`tail` and *reads* `head`; `rb_pop` (main) only ever writes `head` and
reads `tail`. Neither side performs a compound read-modify-write on a
variable the other side also writes — the single-writer property is what
makes plain, `volatile`-qualified stores safe without a mutex.

### Bug — `rb_init` reassigning a pointer parameter did nothing

```c
void rb_init(ringbuf *rb) {
    rb = malloc(sizeof(ringbuf));   // rewrites the LOCAL COPY, not the caller's
}
```

Same root mistake as the free-list's `bump_init`/`fl_init` work earlier:
`rb` is a parameter — a local copy of whatever pointer was passed in.
Reassigning it only changes that local copy; the caller's actual variable
(`static ringbuf rx`) is untouched, and the freshly `malloc`'d memory is
immediately leaked with nothing pointing at it. Also unnecessary — `rx`
is already real, permanent storage in `.bss`; the fix was to delete the
`malloc` entirely and just zero the fields the caller already owns.

### Bug — NVIC enable using the IRQ number instead of a bit position

```c
NVIC_ISER0 |= UART0_IRQ;        // UART0_IRQ is 5 -- sets bits 0 and 2
NVIC_ISER0 |= (1u << UART0_IRQ); // correct: sets bit 5
```

"Enable IRQ *n*" on the NVIC means *set bit n*, not *OR in the value n*.
`5` in binary is `101` — the buggy version silently enabled IRQs 0 and 2
instead of the intended UART0 interrupt.

---

## Part 4 — bootloader / VTOR relocation

### Concept: a bootloader fakes a reset

Vector table slot 0 = initial stack pointer, slot 1 = entry point — the
same two slots hardware reads at a real cold boot. A bootloader reads
those same two slots out of a *second* table (the application's, at a
known flash offset), sanity-checks the stack pointer, sets `SCB->VTOR` to
point interrupt dispatch at that second table, then manually loads SP and
jumps. The application wakes up in a state indistinguishable from a real
hardware reset — it has no way to tell it was launched by software rather
than silicon.

Two separate handoffs have to happen, easy to do only one of: **SP/PC**
decide where execution goes *right now*; **VTOR** decides where every
*future* interrupt gets dispatched. Set only the first and the app runs
until the first interrupt, then silently dispatches into the bootloader's
dead handlers.

The jump itself can't be a normal C function call — changing SP mid-call
saws through the branch the function is standing on, and there's no C
syntax for "leave and never return." Hence a two-instruction `naked`
assembly function (`msr msp, r0` / `bx r1`), with the calling convention
(AAPCS: first two args in r0/r1) as the entire, unwritten contract between
the C call site and the hand-written asm.

### Bug — assigning a pointer to a dereferenced register with no cast

```c
SCB_VTOR = app_vec;   // app_vec is uint32_t*, SCB_VTOR is a uint32_t lvalue
```

Same category as Part 1's `void*`-holding-a-count bug: `SCB_VTOR` expands
to `*(volatile uint32_t*)0xE000ED08u` — a plain integer once dereferenced
— while `app_vec` is a pointer. Fixed with an explicit
`SCB_VTOR = (uint32_t)app_vec;`. Caught this one before running it, by
noticing the type mismatch while re-reading the line.

---

## Part 5 — cooperative context switch

### Concept: `tasks[].sp` is a mailbox, not a base pointer

Each task's 1KB stack has three distinct addresses that are easy to
conflate: the array's fixed low address (never referenced again after
setup), its fixed high address ("ceiling," used once to compute the
initial fake frame), and `tasks[id].sp` — the *live, constantly-moving*
value recording where that task's stack currently stands whenever it
isn't running. Only the third one matters to `yield()`; it's the direct
analogue of the bump allocator's `curr_offset` versus its fixed
`base_ptr` — the fixed addresses are dead information the moment the
array exists.

Switching tasks: save the outgoing task's live registers onto its own
stack (`push {r4-r11, lr}`), record its current SP into the mailbox array,
ask the scheduler who's next, load the incoming task's SP out of the
mailbox, and only *then* restore registers and return — everything before
the stack-pointer swap touches the old task's memory, everything after
touches the new task's. A never-yet-run task's initial "saved frame" is
hand-forged by `task_init` to look exactly like what a real `yield()` call
would have produced, with the "lr slot" holding the task's entry point
instead of a real return address — `yield()`'s restore code can't tell
the difference, so no special-casing is needed for first run.

`pick_next_task` — the actual scheduling policy — turned out to be one
line, reusing the same power-of-two mask trick as the ring buffer's
wraparound:

```c
current_task = (current_task + 1) & (NUM_TASKS - 1);
```

No bug here; worth recording because it's the same derivation, twice,
recognized independently the second time.

---

## What I didn't finish

A preemptive extension (SysTick forcing a switch via PendSV, without an
explicit `yield()`) was attempted and deliberately shelved rather than
shipped half-working. It surfaced two genuinely subtle hardware races —
the `svc` instruction's own automatic frame-push clobbering a
pre-positioned stack pointer, and a timer interrupt firing in the gap
between switching stacks and triggering the bootstrap trap — the same
class of bug real RTOS ports are documented to spend real care avoiding.
Recognizing *that* a fix works versus verifying it end-to-end in QEMU is a
meaningful gap on genuinely hard bootstrap code, and this is honestly
where that gap sat — logged here rather than papered over.
