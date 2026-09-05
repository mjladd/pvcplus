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

/* pv.h only declares this `extern` - the real definition normally comes
   from a tool's globals.h (via -fcommon tentative-definition merging).
   This standalone program needs its own. */
int frame_count = 0;

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

    /* curve(V1, V2, n, warp) */
    float curve_cases[][4] = {
        {0.0, 1.0, 0.5, 0.0},
        {0.0, 1.0, 0.5, 4.0},
        {0.0, 1.0, 0.5, -4.0},
        {0.0, 1.0, 0.25, 8.0},
        {10.0, 20.0, 0.75, -2.0},
    };
    for (int i = 0; i < 5; i++) {
        float *c = curve_cases[i];
        printf("curve(%g,%g,%g,%g) = %.9g\n", c[0], c[1], c[2], c[3],
               curve(c[0], c[1], c[2], c[3]));
    }

    /* spectmagwarp(SP, Nplus2, warpshape, normflag) on a small synthetic
       4-bin spectrum (mag/freq interleaved, Nplus2=10 for 5 bins). */
    {
        float sp[10] = {1.0, 100, 4.0, 200, 2.0, 300, 0.5, 400, 3.0, 500};
        spectmagwarp(sp, 10, 2.0, 0);
        printf("spectmagwarp(warp=2, norm=0) mags = %.9g %.9g %.9g %.9g %.9g\n",
               sp[0], sp[2], sp[4], sp[6], sp[8]);
    }
    {
        float sp[10] = {1.0, 100, 4.0, 200, 2.0, 300, 0.5, 400, 3.0, 500};
        spectmagwarp(sp, 10, -3.0, 1);
        printf("spectmagwarp(warp=-3, norm=1) mags = %.9g %.9g %.9g %.9g %.9g\n",
               sp[0], sp[2], sp[4], sp[6], sp[8]);
    }

    /* eq2(SP, N, dBlow, dBhi, freqlow, freqhi, fundamental, channel_freqdev, normflag) */
    {
        float sp[10] = {1.0, 100, 1.0, 200, 1.0, 300, 1.0, 400, 1.0, 500};
        float freqdev[10] = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
        frame_count = 0;
        eq2(sp, 10, -12.0, 6.0, 150.0, 350.0, 100.0, freqdev, 0);
        printf("eq2(shelf) mags = %.9g %.9g %.9g %.9g %.9g\n",
               sp[0], sp[2], sp[4], sp[6], sp[8]);
    }

    /* smooth(A, old_A, Nplus2, att, matt, rel, mrel) across 3 frames */
    {
        float a1[6] = {1.0, 0, 2.0, 0, 3.0, 0};
        float a2[6] = {0.5, 0, 5.0, 0, 1.0, 0};
        float a3[6] = {0.8, 0, 4.0, 0, 2.0, 0};
        float old_a[6] = {0, 0, 0, 0, 0, 0};
        float att = 0.3, matt = 0.7, rel = 0.6, mrel = 0.4;
        frame_count = 0;
        smooth(a1, old_a, 6, att, matt, rel, mrel);
        printf("smooth frame0 = %.9g %.9g %.9g\n", a1[0], a1[2], a1[4]);
        frame_count = 1;
        smooth(a2, old_a, 6, att, matt, rel, mrel);
        printf("smooth frame1 = %.9g %.9g %.9g\n", a2[0], a2[2], a2[4]);
        frame_count = 2;
        smooth(a3, old_a, 6, att, matt, rel, mrel);
        printf("smooth frame2 = %.9g %.9g %.9g\n", a3[0], a3[2], a3[4]);
    }

    /* smooth_setup(t, c, minusc, IR) */
    {
        float c, minusc;
        smooth_setup(0.05, &c, &minusc, 0.005);
        printf("smooth_setup(t=0.05, IR=0.005) c=%.9g minusc=%.9g\n", c, minusc);
        smooth_setup(0.0, &c, &minusc, 0.005);
        printf("smooth_setup(t=0, IR=0.005) c=%.9g minusc=%.9g\n", c, minusc);
    }

    /* getthresh(arr, Nplus2, tgen): peak amplitude among bins 1.. (bin 0
       excluded) times tgen. Not declared in pv.h - called in plainpv.c
       via a bare `float getthresh();` forward declaration (K&R-style
       "unspecified arguments", not "no arguments"), matching this old-
       style-defined function's actual calling convention. Declaring a
       *real* prototype here instead (`float getthresh(float*,int,float)`)
       was tried first and silently produced wrong values - it makes the
       caller pass `tgen` as a plain float, but the K&R definition's
       calling convention expects the default float->double promotion
       an unprototyped call applies. */
    {
        extern float getthresh();
        float sp[10] = {9.0, 0, 2.0, 0, 7.0, 0, 4.0, 0, 1.0, 0};
        printf("getthresh(peak-excl-bin0, tgen=0.5) = %.9g\n", getthresh(sp, 10, 0.5));
    }

    return 0;
}
