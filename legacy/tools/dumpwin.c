// Dev-only tool (plan Task 2.4): dumps real makewindows()/rfft() output so
// the Rust port in rust/crates/pvc-core can be verified against actual C
// values rather than hand-derived expectations. Not part of the CMake
// build - compiled and linked directly against libpvoc.a for one-off
// verification runs. See docs/dev/rust-verification.md.
#include "../pvc_src/globals.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: dumpwin windows|fft\n");
        return 1;
    }

    // makewindows() reads TWOPI directly (no lazy-init like rfft has) - every
    // real tool's main() sets these before doing anything else; globals.h
    // leaves them BSS-zeroed otherwise.
    PI = 4. * atan(1.);
    TWOPI = 8. * atan(1.);

    if (strcmp(argv[1], "windows") == 0) {
        int nw = 1024, n = 1024, ifac = 220;
        float *h, *a, *s;
        fvec(h, nw);
        fvec(a, nw);
        fvec(s, nw);
        for (window_type = 0; window_type <= 16; window_type++) {
            makewindows(h, a, s, nw, n, ifac, 1);
            printf("# window_type=%d\n", window_type);
            for (int i = 0; i < nw; i++) {
                printf("%d %.9g %.9g %.9g\n", i, h[i], a[i], s[i]);
            }
        }
        return 0;
    }

    if (strcmp(argv[1], "fft") == 0) {
        int n = 256;
        float *x;
        fvec(x, 2 * n);
        // impulse
        x[0] = 1.0;
        rfft(x, n, 1);
        printf("# impulse forward\n");
        for (int i = 0; i < 2 * n; i++) printf("%.9g\n", x[i]);
        return 0;
    }

    fprintf(stderr, "unknown mode: %s\n", argv[1]);
    return 1;
}
