#include <patch_point.h>
#include <stdio.h>
patch_point_list ppl;

int main() {
    patch_point(&ppl, "pp") {
        printf("enabled\n");
    }

    patch_point_disable(&ppl, "pp");

    patch_point(&ppl, "pp") {
        printf("disabled\n");
    }
}

/*
 * check-name: simple patch point
 * check-output-start
enabled
 * check-output-end
 */
