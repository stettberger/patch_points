#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>
#include <stdbool.h>

#include "patch_point.h"

void
__patch_point_writable(char *asm_ptr, bool writeable) {
    // FIXME
    if (!writeable) return;

    static int pagesize = 0;
    if (pagesize == 0)
        pagesize = getpagesize();

    asm_ptr = asm_ptr - ((int)asm_ptr & (pagesize - 1));
    if (mprotect(asm_ptr, pagesize * 2, PROT_READ |
                 (writeable ? PROT_WRITE : 0)
                 | PROT_EXEC) != 0) {
        perror("mprotect");
        exit(-1);
    }
}

void
__patch_point_set_jump(struct patch_point *pp, bool on) {
    assert(pp);
    assert(pp->jump_offset != 0);

    __patch_point_writable(pp->jump_ptr, true);

    int i;
    if ((on && pp->jump_to_block) || (!on && !pp->jump_to_block)) {
        // Add two jumps to the block
        pp->jump_ptr[0] = 0xe9; // JMP (4 Bytes)
        int * addr = (int *)(pp->jump_ptr + 1);
        *addr = pp->jump_offset - 5;

        pp->jump_ptr[5] = 0xe9; // JMP (4 Bytes)
        addr = (int *)(pp->jump_ptr + 6);
        *addr = pp->jump_offset - 10;
    } else {
        // Remove the Jump
        for (i = 0; i < 10; i++) {
            pp->jump_ptr[i] = 0x90; // NOP == 0x90
        }
    }
    __patch_point_writable(pp->jump_ptr, false);
}

void
patch_point_set(patch_point_list *ppl, const char *name, bool on) {
    struct patch_point *pp = ppl->points;
    while (pp != NULL) {
        if (strcmp(pp->name, name) == 0)
            __patch_point_set_jump(pp, on);
        pp = pp->next;
    }

    // Set value in patch point values
    struct patch_point_value *pv = ppl->values;
    while(pv != NULL) {
        if (strcmp(pv->name, name) == 0) {
            pv->value = on;
            return;
        }
        pv = pv->next;
    }
    pv = malloc(sizeof(struct patch_point_value));
    if (!pv) { perror("malloc"); exit(-1);}
    pv->name = name;
    pv->value = on;
    pv->next = ppl->values;
    ppl->values = pv;
}

bool
patch_point_get(patch_point_list *ppl, const char *name) {
    struct patch_point_value *pv = ppl->values;
    while(pv != NULL) {
        if (strcmp(pv->name, name) == 0) {
            return pv->value;
        }
        pv = pv->next;
    }
    return true; // Per Default all patch_points are enabled
}



 __attribute__((noinline)) int
 __patch_point(patch_point_list *ppl, const char *name)
{
    /* Must be volatile to force movl %esp, %ebp */
    volatile int return_addr;
    (void) return_addr;

    asm ("movl 4(%%ebp), %0;" : "=r"(return_addr));
    /* A call is 5 bytes here, and the return address points after
       the call to this function */
    int call_address = return_addr - 5;

    unsigned char *asm_code = (char *) call_address;

    // call __patch_point
    // cmp %eax, 23
    // je $end_of_block

    /* Is a call instruction ? */
    assert(asm_code[0] == 0xe8);

    /* Is our function called ? */
    assert(*((int *)&asm_code[1]) + call_address + 5
           == (int)&__patch_point);

    /* cmp 23 %eax */
    assert(asm_code[6] == 23 || asm_code[7] == 23);
    assert(asm_code[5] == 0x83);  // cmp

    struct patch_point * pp = malloc(sizeof(struct patch_point));
    if (!pp) { perror("malloc"); exit(-1); }

    pp->jump_ptr = (char *) call_address;

    int consume = 8;
    if (asm_code[8] == 0x74) { // je 2 byte
        pp->jump_to_block = 1;

        // Jump offset from first byte of call
        // je {block_address}
        // {else_block}
        consume += 2;
        pp->jump_offset = consume + asm_code[9];
    } else if (asm_code[8] == 0x75) { // jne 2 byte
        pp->jump_to_block = 0;

        // jn {else_block}
        // {block}
        consume += 2;
        pp->jump_offset = consume + asm_code[9];
    } else if (asm_code[8] == 0x0f && asm_code[9] == 0x84) {
        // jee 4 byte
        pp->jump_to_block = 1;
        // 2 byte opcode + 4 byte address
        consume += 6;
        pp->jump_offset = consume + *((int *)&asm_code[10]);
    } else if (asm_code[8] == 0x0f && asm_code[9] == 0x85) {
        // jne 4 byte
        pp->jump_to_block = 0;
        // 2 byte opcode + 4 byte address
        consume += 6;
        pp->jump_offset = consume + *((int *)&asm_code[10]);
    } else {
        assert(false);
    }

    // Make code writable
    __patch_point_writable(asm_code, true);
    int i;
    for (i = 0; i < consume; i++)
        asm_code[i] = 0x90;

    // make code not writable again
    __patch_point_writable(asm_code, false);

    __patch_point_set_jump(pp, patch_point_get(ppl, name));

    // Save name of patch_point
    pp->name = name;

    // Insert into list of patch points
    // FIXME CAS
    pp->next = ppl->points;
    ppl->points = pp;

    return 0;
}
