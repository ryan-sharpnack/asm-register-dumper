/*
 * regdump.c — Custom CPU Register State Dumper
 *
 * Captures live RAX, RBX, RSP, and RIP values via x86-64 inline
 * assembly and prints them in a GDB "info registers" style layout.
 *
 * Build:  gcc -O0 -o regdump regdump.c
 * Run:    ./regdump
 *
 * Notes:
 *   - Built with -O0 so the compiler can't reorder or optimize
 *     away the sentinel writes before the snapshot is taken.
 *   - x86-64 / GCC extended inline asm (AT&T syntax) only.
 */

#include <stdio.h>
#include <stdint.h>

/* One snapshot of register state at the moment it's taken. */
typedef struct {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rsp;
    uint64_t rip;
} reg_state_t;

/*
 * Loads known sentinel values into RAX/RBX so the dump has something
 * deterministic to show, then reads RAX, RBX, RSP, and RIP straight
 * off the CPU into *out. RIP has no direct "mov" target on x86-64,
 * so it's captured with a RIP-relative LEA — the same trick debuggers
 * rely on to report the program counter.
 */
static void dump_registers(reg_state_t *out) {
    asm volatile (
        "movq $0xDEADBEEF, %%rax\n\t"   /* sentinel into RAX          */
        "movq $0xCAFEBABE, %%rbx\n\t"   /* sentinel into RBX          */
        "movq %%rax, %0\n\t"            /* save RAX before it's reused */
        "movq %%rbx, %1\n\t"            /* save RBX                   */
        "movq %%rsp, %2\n\t"            /* save live stack pointer    */
        "lea 0(%%rip), %%rax\n\t"       /* RAX = address of next insn */
        "movq %%rax, %3\n\t"            /* save it as our RIP sample  */
        : "=m" (out->rax), "=m" (out->rbx), "=m" (out->rsp), "=m" (out->rip)
        :
        : "rax", "rbx", "memory"
    );
}

/* Renders a snapshot the way a debugger's register view would. */
static void print_registers(const reg_state_t *r) {
    printf("Register dump (GDB-style):\n");
    printf("  rax   0x%016lx   %lu\n", r->rax, r->rax);
    printf("  rbx   0x%016lx   %lu\n", r->rbx, r->rbx);
    printf("  rsp   0x%016lx   (stack pointer)\n", r->rsp);
    printf("  rip   0x%016lx   (program counter)\n", r->rip);
}

int main(void) {
    reg_state_t regs;
    dump_registers(&regs);
    print_registers(&regs);
    return 0;
}
