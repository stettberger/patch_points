#include <patch_point.h>
#include <stdio.h>
#include <assert.h>
patch_point_list ppl;

int main() {
    volatile int i;
    int *stack_addr = NULL;
    int *stack_addr_else = NULL;
    for (i = 0; i < 1000; i++) {
        patch_point_set(&ppl, "pp", (i % 2));
        patch_point(&ppl, "pp") {
            int Z = 0;
            assert (!stack_addr || &Z == stack_addr);
            stack_addr = &Z;
        } else {
            int Z = 0;
            assert (!stack_addr_else || &Z == stack_addr_else);
            stack_addr_else = &Z;

        }
    }
    printf("everything fine\n");
}

/*
 * check-name: check if we leak memory
 * check-output-start
everything fine
 * check-output-end
 */
