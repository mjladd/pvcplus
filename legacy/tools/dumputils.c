/* Dev-only C verification aid, same idea as dumpwin.c (see
 * docs/dev/rust-verification.md): links directly against libpvoc.a and
 * dumps real dB_to_amp()/amp_to_dB()/semitones_to_mult() output. These are
 * lookup-table approximations (5000-entry tables, linearly interpolated),
 * not the exact math - not part of the CMake build, not a shipped tool.
 *
 * Build and run:
 *   cmake --build build --target pvoc -j
 *   gcc -Ilegacy/pvc_lib -Ilegacy/pvc_src -o /tmp/dumputils \
 *       legacy/tools/dumputils.c build/libpvoc.a -lsndfile -lm
 *   /tmp/dumputils
 */
#include <stdio.h>
#include "../pvc_lib/pv.h"

int main(void) {
    float dbs[] = {0.0, -6.0, -96.0, 96.0, -200.0, 200.0, 3.0, -40.0, -12.5, 48.0};
    for (int i = 0; i < 10; i++) {
        printf("dB_to_amp(%g) = %.9g\n", dbs[i], dB_to_amp(dbs[i]));
    }
    float amps[] = {1.0, 0.5, 2.0, 0.001, 100.0};
    for (int i = 0; i < 5; i++) {
        printf("amp_to_dB(%g) = %.9g\n", amps[i], amp_to_dB(amps[i]));
    }
    float semis[] = {0.0, 12.0, -12.0, 7.0, -7.0, 100.0, -100.0, 0.5};
    for (int i = 0; i < 8; i++) {
        printf("semitones_to_mult(%g) = %.9g\n", semis[i], semitones_to_mult(semis[i]));
    }
    return 0;
}
