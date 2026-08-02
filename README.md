# Lab 3: Custom CPU Register State Dumper

## Technical Objective
Uses x86-64 inline assembly to capture and print the live values of RAX, RBX, RSP, and RIP in a GDB-style register dump.

**Business Impact Summary:** Security tooling — debuggers, EDR agents, crash handlers — all depend on accurately reading raw CPU register state at a point in time. This lab proves the ability to build that primitive from scratch rather than treating it as a black box supplied by an existing debugger or library.

## The "Why": Engineering Value & Threat Impact
*   **Operational Risk / Threat Model:** Register-level state is what attackers manipulate during buffer overflows, ROP chains, and shellcode injection, and what defenders inspect during incident response and malware triage — reading it directly is a prerequisite for both.
*   **Engineering Mastery:** Demonstrates working knowledge of GCC extended inline assembly, AT&T syntax, register clobber semantics, and RIP-relative addressing without relying on a debugger API to do the work.
*   **Defensive Utility:** Establishes the same low-level register-capture technique that underlies custom instrumentation, crash-dump tooling, and behavioral EDR hooks used to baseline "normal" execution state.

## Architecture & System Boundary
*   **Language & Toolchain:** C / GCC, x86-64 inline assembly (AT&T syntax)
*   **Operating System Focus:** Linux x86-64
*   **Core APIs/Primitives Used:** GCC extended `asm volatile`, `movq` register transfers, RIP-relative `lea`

## Technical Execution (What & How)
*   **Sentinel loading:** RAX and RBX are loaded with known constants (`0xDEADBEEF`, `0xCAFEBABE`) immediately before capture, so the dump's correctness can be verified by inspection rather than trusting incidental register contents.
*   **RIP capture:** x86-64 has no direct `mov rip, reg`; the program counter is instead read via a RIP-relative `lea 0(%rip), %rax` — the same technique debuggers rely on internally.
*   **Safe transfer to memory:** All four registers are written to a `reg_state_t` struct through `"=m"` constraints with an explicit clobber list, so the compiler can't reorder or optimize away the capture.

## How to Build & Run Locally
```bash
gcc -Wall -Wextra -O0 -o regdump src/regdump.c
./regdump
```
