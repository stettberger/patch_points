#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>
#include <stdbool.h>

#include "patch_point.h"

extern char etext;

void
__patch_point_writable(char *asm_ptr, bool writeable) {
    static int pagesize = 0;
    if (pagesize == 0)
        pagesize = getpagesize();

    asm_ptr = asm_ptr - ((int)asm_ptr & (pagesize - 1));
    char *data_segment = &etext - ((int)&etext & (pagesize - 1));

    if (!writeable && ((asm_ptr + 1) >= data_segment))
        return;

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

    int jump_address;
    int i = 0;
    if ((on && pp->jump_to_block) || (!on && !pp->jump_to_block)) {
        jump_address = pp->jump_offset;
    } else {
        jump_address = pp->jump_space;
    }

    while (true) {
        // Add two jumps to the block
        pp->jump_ptr[i] = 0xe9; // JMP (4 Bytes)
        int * addr = (int *)(pp->jump_ptr + 1 + i);
        *addr = jump_address - 5 - i;
        i += 5;
        /* Space for a next jump? */
        if ((i + 5) > pp->jump_space) break;
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




static bool
__patch_point_decode_jump(unsigned char *jmp_ptr, struct patch_point *pp, int *consume) {
    if (jmp_ptr[0] == 0x74) { // je 2 byte
        pp->jump_to_block = 1;

        // Jump offset from first byte of call
        // je {block_address}
        // {else_block}
        *consume += 2;
        pp->jump_offset += (signed char)jmp_ptr[1] + 2;

    } else if (jmp_ptr[0] == 0x75) { // jne 2 byte
        pp->jump_to_block = 0;

        // jn {else_block}
        // {block}
        *consume += 2;
        pp->jump_offset += (signed char) jmp_ptr[1] + 2;
    } else if (jmp_ptr[0] == 0x0f && jmp_ptr[1] == 0x84) {
        // jee 4 byte
        pp->jump_to_block = 1;
        // 2 byte opcode + 4 byte address
        *consume += 6;
        pp->jump_offset += *((int *)&jmp_ptr[2]) + 6;

    } else if (jmp_ptr[0] == 0x0f && jmp_ptr[1] == 0x85) {
        // jne 4 byte
        pp->jump_to_block = 0;
        // 2 byte opcode + 4 byte address
        *consume += 6;
        pp->jump_offset += *((int *)&jmp_ptr[2]) + 6;
    } else {
        return false;
    }
    return true;
}

bool
__patch_point_find_cmp(unsigned char *call_ptr, int *consume) {
    // Jump over call
    call_ptr += 5;
    /* Determine where the compare starts, since there can be
       instructions between call and compare */
    int i;
    for (i = 0; i < 100; i++) {
        if (call_ptr[i] != 0x83)
            continue;
        if (call_ptr[i + 1] != 23
            && call_ptr[i + 2] != 23)
            continue;
        break;
    }
    if (i == 100) return false;
    call_ptr += i;

    /* cmp 23 %eax */
    if (!(call_ptr[1] == 23
          || call_ptr[2] == 23))
        return false;
    if (!(call_ptr[0] == 0x83)) // cmp
        return false;

    *consume = 5 + i + 3;
    return true;
}



__attribute__((noinline))
__attribute__((fastcall))
int
 __patch_point(patch_point_list *ppl, const char *name)
{
    /* Must be volatile to force movl %esp, %ebp */
    volatile int return_addr;
    (void) return_addr;

    asm ("movl 4(%%ebp), %0;" : "=r"(return_addr));
    /* A call is 5 bytes here, and the return address points after
       the call to this function */
    unsigned char *call_address = (unsigned char *)return_addr - 5;


    /* This macro is used for developing, there the assert has to
       fail, to improve development. In real production code we use
       patch_point_get as an fallback
    */

#ifdef WNAZI
#define __patch_point_assert(cond) assert(cond)
#else
#warning Production Code!! IDIOT! ;-)
#define __patch_point_assert(cond) if (!(cond)) {\
        return patch_point_get(ppl, name); \
    }
#endif

    // call __patch_point
    // cmp %eax, 23
    // je $end_of_block

    /* Is a call instruction ? */
    __patch_point_assert(call_address[0] == 0xe8);

    /* Is our function called ? */
    __patch_point_assert(*((int *)&call_address[1]) + call_address + 5
                         == (unsigned char *)&__patch_point);


    struct patch_point * pp = malloc(sizeof(struct patch_point));
    __patch_point_assert(pp);
    pp->jump_ptr = (char *) call_address;


    int consume;
    __patch_point_assert
        ( __patch_point_find_cmp((unsigned char *)call_address, &consume));

    // Pointer to the jump
    int jmp_addr = consume;
    pp->jump_offset = jmp_addr;
    __patch_point_assert
        ( __patch_point_decode_jump(&call_address[jmp_addr], pp,
                                    &consume));

    // Search for movl $(TEXT) %e[cd]x
    // before call address, in order to remove them, they are used
    // within the fast call of __patch_point
    int movl_length = 0;
    unsigned char *movl_ptr = call_address;
    while (movl_length < 2) {
        movl_ptr -= 5;
        if (movl_ptr[0] == 0xb9) { // movl ecx
            if (*((int*)&movl_ptr[1]) != (int) ppl) {
                break;
            }
        } else if (movl_ptr[0] == 0xba) { // movl edx
            if (*((int*)&movl_ptr[1]) != (int) name) {
                break;
            }
        } else {
            break;
        }
        movl_length ++;
    }
    // Consume the movls before out call
    pp->jump_offset += (movl_length * 5);
    pp->jump_ptr -= (movl_length * 5);
    consume += (movl_length * 5);

    pp->jump_space = consume;

    // Make code writable
    __patch_point_writable(pp->jump_ptr, true);
    int i;
    for (i = 0; i < consume; i++)
        pp->jump_ptr[i] = 0x90;

    // make code not writable again
    __patch_point_writable(pp->jump_ptr, false);

    __patch_point_set_jump(pp, patch_point_get(ppl, name));

    // Save name of patch_point
    pp->name = name;

    // Insert into list of patch points
    pp->next = ppl->points;
    ppl->points = pp;

    return 0;
}
