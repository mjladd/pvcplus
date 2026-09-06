/* Dev-only C verification aid, same idea as dumpwin.c/dumputils.c (see
 * docs/dev/rust-verification.md): links directly against libpvoc.a and
 * exercises findFilterTimeAndConstrainByWindow(), makeLoopSmoothTime(),
 * and makeInterpolatedFilterFrame() - twarp.c's time-navigation core -
 * with synthetic inputs across several frames, to verify pvc-core's
 * TimeNavigator/make_loop_smooth_time/interpolate_frame ports. Not part
 * of the CMake build, not a shipped tool.
 *
 * Build and run:
 *   cmake --build build --target pvoc -j
 *   gcc -Ilegacy/pvc_lib -Ilegacy/pvc_src -o /tmp/dumptwarp \
 *       legacy/tools/dumptwarp.c build/libpvoc.a -lsndfile -lm
 *   /tmp/dumptwarp
 */
#include <stdio.h>
#include <string.h>
#include "../pvc_lib/pv.h"

int frame_count = 0;
float t = 0.0;
float ringTime = 0.0;

static struct func const_func(float v) {
    struct func f;
    memset(&f, 0, sizeof(f));
    f.L = 1.0;
    f.n = 1.0;
    f.A[0] = v;
    return f;
}

static void run_scenario(const char *name, int autostop, int wrap_fold_clip,
                          int onset_release, float win_low, float win_hi,
                          float rate, float filttinc, int n_frames) {
    struct func origin = const_func(0.0);
    struct func rate_f = const_func(rate);
    struct func winlow_f = const_func(win_low);
    struct func winhi_f = const_func(win_hi);
    float filttnow = 0.0, oldfilttnow = 0.0;
    int autostopflag = 0;
    float dur = 100.0; /* large - not testing onset/release duration extension here */

    printf("--- %s ---\n", name);
    for (frame_count = 0; frame_count < n_frames; frame_count++) {
        t = (float) frame_count * filttinc;
        int reset = findFilterTimeAndConstrainByWindow(
            filttinc, &origin, &rate_f, onset_release, autostop, &autostopflag,
            wrap_fold_clip, &filttnow, &oldfilttnow, &winlow_f, &winhi_f, &dur,
            /*analysis_dur=*/1.0, 0, 1.0);
        printf("frame %d: filttnow=%.6f oldfilttnow=%.6f autostop=%d reset=%d\n",
               frame_count, filttnow, oldfilttnow, autostopflag, reset);
        if (autostopflag) break;
    }
}

int main(void) {
    /* WRAP mode: window [0, 0.5], filttinc=0.3, rate=1 -> forces
       multiple wraps quickly. */
    run_scenario("wrap", 0, 0, 0, 0.0, 0.5, 1.0, 0.3, 8);

    /* FOLD mode: same window/rate. */
    run_scenario("fold", 0, 1, 0, 0.0, 0.5, 1.0, 0.3, 8);

    /* CLIP mode: same window/rate. */
    run_scenario("clip", 0, 2, 0, 0.0, 0.5, 1.0, 0.3, 8);

    /* AUTOSTOP mode: window [0, 0.5], should stop once filttnow exceeds 0.5. */
    run_scenario("autostop", 1, 0, 0, 0.0, 0.5, 1.0, 0.3, 8);

    /* Reverse rate. */
    run_scenario("reverse", 0, 0, 0, 0.0, 1.0, -1.0, 0.3, 6);

    /* makeLoopSmoothTime across a few positions. */
    {
        struct func winlow_f = const_func(0.0);
        struct func winhi_f = const_func(1.0);
        struct func peak_f = const_func(0.2);
        float positions[] = {-0.05, 0.0, 0.05, 0.5, 0.95, 1.0, 1.05, 1.3};
        printf("--- loop_smooth_time ---\n");
        for (int i = 0; i < 8; i++) {
            frame_count = 1;
            float v = makeLoopSmoothTime(positions[i], 0, 0, 10.0, &winlow_f, &winhi_f, &peak_f);
            printf("filttnow=%.3f -> loopSmoothTime=%.6f\n", positions[i], v);
        }
    }

    /* makeInterpolatedFilterFrame: 4-frame, 1-channel, N=2 (Nplus2=4)
       synthetic analysis file. */
    {
        const char *path = "/tmp/dumptwarp_test.pva";
        FILE *fp = fopen(path, "wb");
        float header[32];
        memset(header, 0, sizeof(header));
        header[0] = 2;   /* N */
        header[1] = 100; /* D */
        header[2] = 1000; /* R */
        header[3] = 1;   /* chans */
        header[4] = 0;   /* window_type */
        fwrite(header, sizeof(float), 32, fp);
        /* 4 frames, Nplus2=4 floats each, values chosen to be easy to
           verify by hand: frame k = [k*10, k*10+1, k*10+2, k*10+3]. */
        for (int k = 0; k < 4; k++) {
            float frame[4] = {k * 10.0f, k * 10.0f + 1, k * 10.0f + 2, k * 10.0f + 3};
            fwrite(frame, sizeof(float), 4, fp);
        }
        fclose(fp);

        struct func data;
        memset(&data, 0, sizeof(data));
        data.fp = fopen(path, "rb");

        float F_lower[4], F_higher[4], channel[4];
        float iframes_per_sec = 10.0; /* R/D = 1000/100 */
        float timepoints[] = {0.0, 0.05, 0.1, 0.25, 0.3, 0.15};
        printf("--- interpolate_frame (iframes_per_sec=10) ---\n");
        for (int i = 0; i < 6; i++) {
            makeInterpolatedFilterFrame(&data, F_lower, F_higher, channel,
                                         iframes_per_sec, 4, timepoints[i], 0, 1);
            printf("t=%.3f -> [%.4f %.4f %.4f %.4f]  lower=[%.4f %.4f %.4f %.4f] higher=[%.4f %.4f %.4f %.4f]\n",
                   timepoints[i], channel[0], channel[1], channel[2], channel[3],
                   F_lower[0], F_lower[1], F_lower[2], F_lower[3],
                   F_higher[0], F_higher[1], F_higher[2], F_higher[3]);
        }
        fclose(data.fp);
    }

    return 0;
}
