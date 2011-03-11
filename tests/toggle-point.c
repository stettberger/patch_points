#include <patch_point.h>
#include <stdio.h>
patch_point_list ppl;

int main() {
    volatile int i;
    for (i = 0; i < 5; i++) {
        patch_point_set(&ppl, "pp", (i % 2));
        patch_point(&ppl, "pp") {
            printf("enabled\n");
        } else {
            printf("disabled\n");
        }
    }
}

/*
 * check-name: loop with toggling of patch point
 * check-output-start
disabled
enabled
disabled
enabled
disabled
 * check-output-end
 */
