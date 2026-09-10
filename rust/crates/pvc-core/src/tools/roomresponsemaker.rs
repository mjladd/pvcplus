//! Ports `roomresponsemaker.c` (9318 lines, the largest and most
//! structurally distinct tool in this project): a recursive image-source
//! polygonal-room acoustics engine, not a phase-vocoder filter/resynthesis
//! tool like every other Phase 5 tool. **This module is now through Phase 5
//! of a multi-phase port.**
//!
//! **Phase 1** covered the room/speaker/listener geometry layer: room
//! polygon construction (synthesized or file-read), coordinate transforms,
//! listener/source/speaker position resolution, and the small segment/angle
//! utilities everything else builds on.
//!
//! **Phase 2** covered the recursive image-source reflection-path algorithm
//! itself: [`mirror_point`] (`mirrorPointAroundLineSegment`),
//! [`polygon_reflex_vertex_flags`]/[`point_to_line_position`] (the
//! reflex-vertex-flagging half of `isPolygonConcave()` that Phase 1
//! deliberately left out), and [`find_reflection_paths`]
//! (`mirrorPolygonCoordinatesAroundAllSides`), the recursive search itself,
//! restructured around an explicit [`RoomAcousticsInput`]/internal
//! search-trail pair instead of the C's own order-indexed global arrays,
//! but reproducing the same recursion structure, angle-window prefilter,
//! accept/reject tests, and (bugs included) formulas.
//!
//! **Phase 3** (this update) covers the *pulse-gain and delay-index* math
//! inside `writeReflectionPulsesIntoImpulseResponse()` that turns one
//! accepted [`ReflectionPath`] into a scalar amplitude and an output-sample
//! delay index: [`source_orientation_to_reflection_angle_difference`],
//! [`dispersion_pattern_amplitude`], [`air_absorption_multiplier`],
//! [`wall_gainscale_amplitude`], [`reflection_order_gainscale_amplitude`],
//! their combination in [`reflection_pulse_gain`],
//! [`reflection_delay_sample_index`], and
//! [`reflection_pulse_passes_inclusion_threshold`] - plus
//! [`front_source_head_room_scalar`]/[`pre_echo_time_seconds`] from
//! `makePreEchoValues()`, a small piece of setup math both the reflection
//! and (out of scope here) direct-sound pulse paths depend on.
//!
//! **Phase 4** (this update) covers the *pure signal-processing* pieces of
//! the wall/reflection-order impulse-response engine Phase 3 identified but
//! deferred: [`convolve_two_arrays`] (`convolveTwoArrays`),
//! [`crop_end_for_silence`]/[`crop_ir_data_end_for_silence`]
//! (`cropEndForSilence`/`cropIR_DataEndForSilence`), [`find_peak_amp`],
//! [`filter_fft`]/[`filter_audio_array`] (`filterFFT`/`filterAudioArray`),
//! [`smooth_release_of_cropped_end`], the wall-IR-sequence selection inside
//! `createAndReorderWallReflectionSequence()`
//! ([`wall_sequence_for_reflection`], [`compare_wall_sequences`],
//! [`select_and_reorder_wall_reflection_sequences`]), and
//! [`average_reflection_order_delay_times`]. Everything here operates on
//! plain slices/`Vec`s and already-available [`ReflectionPath`] data - no
//! file I/O.
//!
//! **Phase 5** (this update) covers the direct (unreflected) source-to-
//! speaker pulse path: [`listener_to_speaker_angle_bounds`] (the min/max/
//! straddle-flag half of `makeListenerToSpeakerAngles()` Phase 1 deferred -
//! turns out the reflection recursion never needed it, only this phase
//! does), [`find_crossfade_speaker_pair`]/[`make_source_to_threshold_proximity_distance`]
//! (`makeSourceToThresholdProximityDistance()`),
//! [`is_source_behind_or_in_front_of_speaker_threshold`],
//! [`find_listener_to_source_segment_length_and_angle`],
//! [`direct_sound_speaker_distances_for_delays`]
//! (`makeDirectSoundSpeakerDelayTimes()`),
//! [`direct_sound_delay_sample_index`] and [`make_direct_sound_speaker_amplitudes`]
//! (`makeDirectSoundSpeakerAmplitudes()`, the "BETWEEN SPEAKERS"/"NOT IN
//! CROSSFADE" branches of the real, live
//! `writeDirectSourcePulsesIntoImpulseResponse()` - not its dead `OLD`
//! twin, confirmed unused by grepping every call site). Two whole
//! functions this dependency chain touches turned out to be entirely dead
//! and are not ported at all - see findings 21-22.
//!
//! **What's still not covered, and why**: the actual wall/reflection-order
//! impulse-response *file* cache-and-convolve engine, namely
//! `readInWallImpulseResponses`/`readInReflectionOrderImpulseResponses`,
//! `recallIR`/`addToIRfileCodes` (the file-based memoization these
//! functions' own callers use `compare_wall_sequences`'s sort order to
//! drive), `filterAndNormalizeImpulseResponseNow`/`filterAndNormalize*`
//! (the very code Phase 4's finding 14 says needs to renormalize
//! [`convolve_two_arrays`]'s `1/N`-scaled output), and `getWallImpulseResponseChannelAssignments`/
//! `getWallDecibelGainscaleLevels`/`getReflectionOrderDecibelGainscaleLevels`
//! (all three genuinely file-driven, `fopen`/`fscanf` in the C, unlike
//! everything else read so far), is still not started; it's real file I/O
//! and belongs with `pvc-cli`/`pvc-io`. `makeSpaceReflectionCoordinates` is
//! pure geometry with no I/O, but its only consumer
//! (`reflectionSoundPathCoordinates`, confirmed via its single call site at
//! line ~3119 inside `mirrorPolygonCoordinatesAroundAllSides()`) is
//! plot-file output, not audio math - deferred to whichever later phase
//! handles plotting. CLI wiring (`main()`'s own control flow) is also
//! still not started, so `crack()` flag cross-referencing remains deferred
//! to that later phase - see finding 19 for a real bug in that control
//! flow, confirmed this phase by finally reading `main()`'s actual
//! statement order around it.
//!
//! **`pvc-core` does no I/O of its own** (matching `tools::chordmapperplus`'s
//! established convention) - every function here takes already-read file
//! *content* as `&str`, not a path. Real file reads belong in `pvc-cli`/`pvc-io`.
//!
//! ## Real C bugs/quirks found while reading (confirmed against the source,
//! not guessed)
//!
//! 1. **`getListenerCoordinates()`/`getSourceCoordinates()` (lines 3724,
//!    3814) hard-`exit(EXIT_FAILURE)` if their coordinates file supplies
//!    more than one X/Y pair** - directly contradicting `usage()`'s claim
//!    that "Multiple listener/source pairs are applied in sequence to
//!    successive channels, looping as needed." Only `getSpeakerCoordinates()`
//!    (line 3899) actually allows multiple positions; the "looping"
//!    behaviour `usage()` describes for `-L`/`-i` does not exist in the
//!    real C. This port's [`resolve_single_point`] enforces the real
//!    (single-point) constraint, not the documented one.
//!
//! 2. **`pointInPolygonTest()` (line 3221) computes a fresh bisector-ray
//!    "inside" test once per polygon side inside its `sideSegment` loop,
//!    but the `thisPointIsInPolygon` result it computes is only ever
//!    checked *after* that loop exits** (line 3378) - every iteration's
//!    result except the last (`sideSegment == numberOfWalls - 1`) is
//!    silently discarded. The function is therefore equivalent to running
//!    the bisector-ray test exactly once, using only the last wall's
//!    bisector - not "does the point pass every side's ray test," as the
//!    loop structure implies. [`point_in_polygon`] reproduces this exact
//!    (reduced) behaviour rather than the apparently-intended per-side
//!    aggregate.
//!
//! 3. **A second, independent bug in that same function** (lines
//!    3309-3312): when adjusting bisector-ray endpoint angles that
//!    straddle the +/-PI boundary, the C loops `sideSegmentEnd` over `0,
//!    1` but always adds `TWOPI` to index `[0]` regardless of which
//!    index's angle was actually negative - so `angles[1]`'s own
//!    straddle case is silently never corrected, and `angles[0]` can be
//!    corrected twice (once per loop pass) if both angles are negative.
//!    Reproduced exactly in [`point_in_polygon`].
//!
//! 4. **`findMinMaxValues()` (line 4172) is dead code** - never called
//!    anywhere in the file (confirmed by `grep`) - and is also broken in
//!    isolation (`*minVal` is updated with the *greater-than* test, the
//!    same condition as `*maxVal`, so it converges to the same value as
//!    the max instead of the true min). Not ported.
//!
//! 5. **`sameCoordinatesTest()` (line 5639) and `areCoordinatesTheSame()`
//!    (line 5750) are byte-identical duplicate functions.** Ported once,
//!    as [`same_point`].
//!
//! 6. **The room-synthesis default mode (`-f0`, `makeCorneredSpace()`,
//!    line 2298) is driven entirely by `randf()`/glibc `random()`**,
//!    matching `tools::ring`'s already-established `randf()` precedent
//!    (see e.g. `tools::inharmonator`, `tools::chordmapperplus`) - this
//!    project does not attempt bit-exact glibc `random()` reproduction.
//!    [`make_cornered_space`] takes a caller-supplied uniform-random
//!    closure instead of a hardcoded RNG, so golden-test design (a later
//!    phase) can either supply a fixed deterministic sequence for unit
//!    tests or route golden coverage through the file-based coordinate
//!    modes (`-f1`/`-f2`) instead, sidestepping RNG matching entirely -
//!    the same strategy already used elsewhere in this project to avoid
//!    randomized golden cases.
//!
//! 7. **`makeCorneredSpace()` never validates or retries** - the
//!    synthesized room is checked exactly once, in `main()`, via
//!    `polygonTest()`/`isPolygonConcave()`, and a failing `polygonTest()`
//!    calls `exit(EXIT_FAILURE)` with no retry loop. [`make_cornered_space`]
//!    matches this: it always returns a room from its inputs, with
//!    validation left to a separate [`polygon_is_valid`] call.
//!
//! 8. **Minor, not functionally significant**: `getPolygonCoordinates()`'s
//!    (line 3580) own "missing coordinates file" guard compares
//!    `strcasecmp(...) == 1`, but `strcasecmp` is only guaranteed to
//!    return zero-vs-nonzero, not a specific nonzero value - so this
//!    check effectively never fires, and a genuinely-missing file is
//!    instead caught later by the real `fopen()` NULL check with a
//!    slightly different error message. `getListenerCoordinates()`'s
//!    equivalent guard (`== 0`, checking equality with the `"EMPTY"`
//!    sentinel) is the correct, portable form and is what this port's
//!    file-selection logic actually follows.
//!
//! Also read in Phase 1, but deferred there and now ported in Phase 2:
//! `mirrorPointAroundLineSegment` (now [`mirror_point`]/[`mirror_point_f64`])
//! and `isPolygonConcave()`'s reflex-vertex-flagging sub-step (now
//! [`polygon_reflex_vertex_flags`]/[`point_to_line_position`]).
//!
//! Still deliberately deferred past Phase 2 (confirmed by grepping call
//! sites - each is only used by direct-sound/dispersion code, not the
//! reflection recursion): `findIntersectionOfLinesContainingSegments`
//! (only used by `makeSourceToThresholdProximityDistance`),
//! `rotatePointToAngle` and `valueIsBetweenTheseTwo` (only used by the
//! direct-sound amplitude and source-threshold-proximity functions).
//!
//! 9. **The angle-window "straddles +/-PI" correction in
//!    `mirrorPolygonCoordinatesAroundAllSides()` (lines 2696-2728) is dead
//!    code**, found only by tracing which variables the accept test and the
//!    recursive narrowing step actually read afterward. When a candidate
//!    mirror side's parent angle window has differently-signed bounds more
//!    than PI apart (i.e. straddles the +/-PI wraparound), the C computes a
//!    "rotate negative angles into `[0, 2*PI)`" correction - but stores it
//!    into `mirrorSegAngleLimitsLow`/`High`, its own by-value parameters,
//!    which are never read again in this branch. The values the accept test
//!    (lines 2733-2744) and the recursive narrowing step (lines 3170-3187)
//!    actually use - `mirrorSegAngleLimitsTempLow`/`High` and the raw
//!    parameters, respectively - were already captured from the *unrotated*
//!    window one statement earlier, in both the straddling and
//!    non-straddling branches alike. So `mirrorSegAngleLimitsTempLow`/`High`
//!    equal the raw parent window in every case, straddling or not - the
//!    dedicated correction has no observable effect (a second, narrower bug
//!    lives inside that same dead block: its final "swap" reassigns
//!    `mirrorSegAngleLimitsLow` twice instead of ever writing `...High`, and
//!    its second `< 0.0` guard checks `mirrorSegAngleLimitsLow` a second
//!    time instead of `...High` - moot either way, since the block is dead).
//!    Net effect: whenever a candidate's parent window straddles +/-PI, only
//!    the candidate's *own* two endpoint angles get rotated into `[0, 2*PI)`
//!    before the accept test - the window bounds they're compared against
//!    stay in the original, possibly-inverted (`low > high`) representation.
//!    [`mirror_segment_angle_window_test`] reproduces this exactly: it does
//!    not implement the dead rotation at all, since skipping it is
//!    numerically identical to running it and discarding the result.
//!    Confirmed by reading the data flow, not by an instrumented rebuild -
//!    this phase's algorithmic core has no golden test yet to verify
//!    against (see the module's scope note above).
//!
//! 10. **`pointToLinePosition()` can never actually return its own
//!     documented "0 = 180 degrees" case.** The global array it feeds,
//!     `polygonReflexVertexAngleFlags`, is commented as `-1 = reflex, 1
//!     non-reflex, 0 = 180 degrees` (line 260) - but the function computes
//!     its sign via `copysign(1., position)`, and C's `copysign` never
//!     returns `0.0`: a perfectly collinear triple (`position == 0.0`)
//!     still yields `+1`, not `0`. The three-state comment describes a
//!     value the function cannot produce. [`point_to_line_position`]
//!     reproduces the real (always +/-1) behavior via `f64::copysign`
//!     rather than `.signum()` (which *would* introduce a real `0` case for
//!     an exact-zero input, diverging from the C).
//!
//! 11. **The "wall gainscale" mode-select in `writeReflectionPulsesIntoImpulseResponse()`
//!     (lines ~4919-4932) has a dead `mode < 0` branch, so the tool's own
//!     *default* silently uses every wall instead of just the last one.**
//!     The C writes the "FROM LAST" branch as
//!     `else if( wall_impulse_and_gainscale_response_mode > 0 )` - an exact
//!     duplicate of the `if` condition immediately above it, making it
//!     permanently unreachable. A second, correct copy of this same
//!     `> 0`/`< 0` branch pair exists elsewhere in the file (lines
//!     ~7408-7417), confirming this isn't "the pattern always works this
//!     way" but a one-off copy-paste error at this call site. Since
//!     `usage()`'s own documented default is `-1` ("last wall"), every real
//!     run that doesn't pass `-t` explicitly falls through to the harmless-
//!     looking final `else` and multiplies gainscale across the *entire*
//!     wall sequence for that reflection, not just its last bounce.
//!     [`wall_gainscale_amplitude`] reproduces this exactly.
//!
//! 12. **One gain term in that same function's amplitude accumulation
//!     bypasses the shared `dB_to_amp()` lookup table.** Every other term
//!     (wall gainscale, reflection-order gainscale, the flat `-k`
//!     reflected-sound gain) converts its own dB value via `dB_to_amp()` -
//!     this project's [`crate::units::DbToAmp`], itself a deliberately
//!     approximate 1990s lookup table (see `units.rs`). The
//!     source-dispersion-pattern term alone (line ~4894) computes
//!     `pow(10.0, dB/20.)` directly instead - a commented-out earlier
//!     version of that same line (`thisProportionOfDispersionDecibelsAsAmp
//!     = dB_to_amp(...)`, lines ~4892-4893) shows the table-based form was
//!     the original intent, later replaced. [`dispersion_pattern_amplitude`]
//!     reproduces the real (exact-formula) live line, not the commented-out
//!     alternative.
//!
//! 19. **Confirmed (Phase 1/2 only suspected this): `-q1` ("orient source
//!     to listener") always behaves identically to `-q0` ("orient to
//!     room").** `rotatedSource` is computed in `main()` at lines
//!     1209-1219, *before* `getSourceCoordinates()`/`getListenerCoordinates()`
//!     are ever called (lines 1322/1366) - and `sourceToListenerAngle`
//!     (which `-q1` reads at line 1214) is only ever assigned inside
//!     `findListenerToSourceSegmentLengthAndAngle()`, itself not called
//!     until much later. Reading `main()`'s actual statement order (not
//!     just grepping references, as Phase 1/2 had) confirms
//!     `sourceToListenerAngle` still holds its zero-initialized default
//!     (`0.0`) at the point `rotatedSource` is computed, and `rotatedSource`
//!     is set once and never recomputed afterward - so `-q1` and `-q0`
//!     are unconditionally the same real behavior. This port does not
//!     invent a "fixed" `-q1`; a future CLI-wiring phase should pass
//!     `orient_source_to_listener` through as documented anyway (matching
//!     the C's own observable behavior, bug included) rather than silently
//!     "correcting" it.
//!
//! 20. See [`DirectSoundAmplitudeInput::source_to_listener_angle_plus_rotation`]'s
//!     own doc comment: `sourceToListenerAnglePlusRotation` is declared but
//!     never assigned anywhere in the file, an incomplete rename left
//!     behind by a comment that says as much.
//!
//! 21. **`makeSourceToSpeakerDistancesAndAngles()` and
//!     `findSourceToSpeakerAngleDifferencesFromSourceToListenerAngle()` are
//!     both entirely dead with respect to program behavior.** Every
//!     value they write - `sourceToSpeakerDistances[]`,
//!     `sourceToSpeakerAngles[]`, `minSourceToSpeakerDistance`,
//!     `maxSourceToSpeakerDistance`, `sourceToSpeakerAngleDifferences[]` -
//!     is read only by their own `fprintf`/`prf` debug output, never by
//!     any other function (confirmed by grepping every reference). Their
//!     min/max bookkeeping (lines 5700-5708) also repeats finding 4's
//!     exact "wrong comparison direction" bug (`findMinMaxValues()`): the
//!     max-tracking `if` uses `<` - the *min* condition - instead of `>`,
//!     so it converges toward the true minimum instead of the maximum.
//!     Since both functions are dead, neither is ported at all, matching
//!     `findMinMaxValues()`'s own treatment in finding 4.
//!
//! 22. **`makeSourceToThresholdProximityProportion()` is also dead.** Its
//!     only output, `sourceToThresholdProximityDistanceProportion`, is
//!     read only inside `writeDirectSourcePulsesIntoImpulseResponseOLD()`
//!     (confirmed dead itself - see the Phase 5 module-doc paragraph
//!     above), never by the real, live
//!     `writeDirectSourcePulsesIntoImpulseResponse()`. Not ported.

/// A 2D point in feet (this tool's native unit - see `usage()`'s "All
/// distances are expressed in feet").
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Point {
    pub x: f32,
    pub y: f32,
}

impl Point {
    pub const ORIGIN: Point = Point { x: 0., y: 0. };

    pub fn new(x: f32, y: f32) -> Self {
        Point { x, y }
    }
}

/// A line segment between two points - mirrors the C's flat `float[4]`
/// segment arrays (`[x0, y0, x1, y1]`).
#[derive(Debug, Clone, Copy)]
pub struct Segment {
    pub a: Point,
    pub b: Point,
}

impl Segment {
    pub fn new(a: Point, b: Point) -> Self {
        Segment { a, b }
    }
}

/// Ports `findSegmentLength()`.
pub fn segment_length(seg: Segment) -> f32 {
    (seg.a.x - seg.b.x).hypot(seg.a.y - seg.b.y)
}

/// Ports `findSegmentAngle()`.
pub fn segment_angle(seg: Segment) -> f32 {
    (seg.b.y - seg.a.y).atan2(seg.b.x - seg.a.x)
}

/// Ports `piToDegrees()`.
pub fn pi_to_degrees(v: f32) -> f32 {
    360.0 * v / std::f32::consts::TAU
}

/// Ports `degreesToPi()`.
pub fn degrees_to_pi(v: f32) -> f32 {
    std::f32::consts::TAU * v / 360.0
}

/// Ports both `sameCoordinatesTest()` and `areCoordinatesTheSame()` - the
/// C's own byte-identical duplicate functions (finding 5).
pub fn same_point(a: Point, b: Point) -> bool {
    a.x == b.x && a.y == b.y
}

/// Ports `adjacentSegmentsTest()`: true if the two segments share any
/// endpoint (compared exactly, matching the C's `==` float comparison).
pub fn segments_share_endpoint(s0: Segment, s1: Segment) -> bool {
    same_point(s0.a, s1.a)
        || same_point(s0.a, s1.b)
        || same_point(s0.b, s1.a)
        || same_point(s0.b, s1.b)
}

/// Ports `examineSegmentsForIntersection()`. Returns the intersection
/// point of the two segments' *containing lines* when it falls within
/// both segments' bounding boxes; `None` otherwise (including the
/// parallel and collinear-overlapping cases, which the C also treats as
/// "no intersection" - a real quirk of this bounding-box-based test, not
/// a full general-position segment-intersection algorithm, faithfully
/// reproduced here).
pub fn segments_intersect(w: Segment, p: Segment) -> Option<Point> {
    let (wx0, wy0, wx1, wy1) = (w.a.x, w.a.y, w.b.x, w.b.y);
    let (px0, py0, px1, py1) = (p.a.x, p.a.y, p.b.x, p.b.y);

    let w_vertical = (wx1 - wx0) == 0.0;
    let p_vertical = (px1 - px0) == 0.0;

    let (x, y) = if w_vertical && p_vertical {
        return None;
    } else if w_vertical {
        let mp = (py1 - py0) / (px1 - px0);
        let bp = py0 - (mp * px0);
        let x = wx0;
        let y = (mp * x) + bp;
        if !((y >= wy0 && y <= wy1) || (y >= wy1 && y <= wy0)) {
            return None;
        }
        if !((x >= px0 && x <= px1) || (x >= px1 && x <= px0)) {
            return None;
        }
        (x, y)
    } else if p_vertical {
        let mw = (wy1 - wy0) / (wx1 - wx0);
        let bw = wy0 - (mw * wx0);
        let x = px0;
        let y = (mw * x) + bw;
        if !((y >= py0 && y <= py1) || (y >= py1 && y <= py0)) {
            return None;
        }
        if !((x >= wx0 && x <= wx1) || (x >= wx1 && x <= wx0)) {
            return None;
        }
        (x, y)
    } else {
        let mw = (wy1 - wy0) / (wx1 - wx0);
        let bw = wy0 - (mw * wx0);
        let mp = (py1 - py0) / (px1 - px0);
        let bp = py0 - (mp * px0);
        if mp == mw {
            return None;
        }
        let x = (bw - bp) / (mp - mw);
        let y = (mp * x) + bp;

        let (wxlow, wxhigh) = if wx0 <= wx1 { (wx0, wx1) } else { (wx1, wx0) };
        let (wylow, wyhigh) = if wy0 <= wy1 { (wy0, wy1) } else { (wy1, wy0) };
        let (pxlow, pxhigh) = if px0 <= px1 { (px0, px1) } else { (px1, px0) };
        let (pylow, pyhigh) = if py0 <= py1 { (py0, py1) } else { (py1, py0) };

        if !(x >= wxlow
            && x <= wxhigh
            && y >= wylow
            && y <= wyhigh
            && x >= pxlow
            && x <= pxhigh
            && y >= pylow
            && y <= pyhigh)
        {
            return None;
        }
        (x, y)
    };

    Some(Point::new(x, y))
}

/// Ports `pointInPolygonTest()` for a single point, reproducing findings
/// 2 and 3 above: only the *last* wall's bisector ray actually determines
/// the result (every earlier wall's own ray test is computed and then
/// discarded), and the straddle-angle correction has a real
/// always-index-`0` bug. `polygon` must have at least one vertex.
pub fn point_in_polygon(polygon: &[Point], point: Point) -> bool {
    let n = polygon.len();
    debug_assert!(n > 0, "polygon must have at least one vertex");
    let side_segment = n - 1; // only the last loop iteration's result survives in the C (finding 2)

    let mut end_angles = [0.0f32; 2];
    let mut end_lengths = [0.0f32; 2];
    for (end, entry) in end_angles.iter_mut().enumerate() {
        let corner = polygon[(side_segment + end) % n];
        let seg = Segment::new(point, corner);
        end_lengths[end] = segment_length(seg);
        *entry = segment_angle(seg);
    }
    let greater_distance = end_lengths[0].max(end_lengths[1]);

    let same_sign = end_angles[0].signum() == end_angles[1].signum();
    let straddles = !(same_sign || (end_angles[1] - end_angles[0]).abs() < std::f32::consts::PI);
    if straddles {
        // Finding 3: the C always corrects index 0, regardless of which
        // end's angle is actually negative.
        for a in end_angles {
            if a < 0.0 {
                end_angles[0] += std::f32::consts::TAU;
            }
        }
    }

    let average_angle = 0.5 * (end_angles[0] + end_angles[1]);
    let ray_end = Point::new(
        point.x + (greater_distance * 2.0) * average_angle.cos(),
        point.y + (greater_distance * 2.0) * average_angle.sin(),
    );
    let test_seg = Segment::new(point, ray_end);

    let mut intersect_count = 0;
    for corner in 0..n {
        let poly_seg = Segment::new(polygon[corner], polygon[(corner + 1) % n]);
        if segments_intersect(test_seg, poly_seg).is_some() {
            intersect_count += 1;
        }
    }

    intersect_count % 2 == 1
}

/// Ports `polygonTest()`: `Ok(())` if the polygon has 3+ vertices, no
/// duplicated coordinates, and no self-intersecting (non-adjacent) sides;
/// otherwise `Err` with a description of the first-found problem class
/// (duplicate vertices are checked before self-intersection, matching the
/// C's own early return at that stage).
pub fn polygon_is_valid(polygon: &[Point]) -> Result<(), String> {
    if polygon.len() <= 2 {
        return Err(format!(
            "polygon must have three or more vertices, got {}",
            polygon.len()
        ));
    }

    for i in 0..polygon.len() - 1 {
        for j in (i + 1)..polygon.len() {
            if same_point(polygon[i], polygon[j]) {
                return Err(format!("duplicate coordinates at vertices {i} and {j}"));
            }
        }
    }

    let n = polygon.len();
    for v0 in 0..n {
        let seg0 = Segment::new(polygon[v0], polygon[(v0 + 1) % n]);
        for v1 in 0..n {
            if v0 == v1 {
                continue;
            }
            let seg1 = Segment::new(polygon[v1], polygon[(v1 + 1) % n]);
            if !segments_share_endpoint(seg0, seg1) && segments_intersect(seg0, seg1).is_some() {
                return Err(format!("sides {v0} and {v1} intersect"));
            }
        }
    }

    Ok(())
}

/// Ports the sign-change-counting portion of `isPolygonConcave()`: `true`
/// if the polygon boundary's x or y coordinate changes sign-of-delta more
/// than twice while walking the vertices. The C's further reflex-vertex
/// flagging (`polygonReflexVertexAngleFlags`, depends on the out-of-scope
/// `pointToLinePosition`) is not ported here - see the module doc comment.
pub fn polygon_is_concave(polygon: &[Point]) -> bool {
    let n = polygon.len();
    if n < 2 {
        return false;
    }

    let mut x_sign_last = (polygon[1].x - polygon[0].x).signum();
    let mut y_sign_last = (polygon[1].y - polygon[0].y).signum();
    let mut x_changes = 0;
    let mut y_changes = 0;

    for c0 in 1..n {
        let v0 = c0;
        let v1 = (c0 + 1) % n;
        let x_sign_now = (polygon[v1].x - polygon[v0].x).signum();
        let y_sign_now = (polygon[v1].y - polygon[v0].y).signum();
        if x_sign_now != x_sign_last {
            x_changes += 1;
        }
        if y_sign_now != y_sign_last {
            y_changes += 1;
        }
        x_sign_last = x_sign_now;
        y_sign_last = y_sign_now;
    }

    x_changes > 2 || y_changes > 2
}

/// Ports `makeCorneredSpace()`: synthesizes a `num_corners`-sided room
/// polygon from a regularity proportion (0 = maximally irregular corner
/// angles, 1 = perfectly regular), a corner-distance range, and a whole-
/// room rotation. `uniform` must return a uniformly-distributed value in
/// `[lo, hi]` each call, matching the real C's own `randf(lo, hi)` calls
/// in the same order (proportions first, one per corner, then one radius
/// per corner) - see finding 6 for why this takes a pluggable source
/// instead of a hardcoded RNG.
pub fn make_cornered_space(
    num_corners: usize,
    min_max_distance: (f32, f32),
    regularity_proportion: f32,
    rotation_degrees: f32,
    mut uniform: impl FnMut(f32, f32) -> f32,
) -> Vec<Point> {
    assert!(num_corners > 0);
    let n = num_corners as f32;

    let mut random_proportions: Vec<f32> = (0..num_corners).map(|_| uniform(0., 1.)).collect();
    let sum: f32 = random_proportions.iter().sum();
    for p in &mut random_proportions {
        *p = (*p / sum) * (1.0 - regularity_proportion) * n;
    }

    let base_angle = std::f32::consts::TAU / n;
    let angles: Vec<f32> = random_proportions
        .iter()
        .map(|&p| (regularity_proportion * base_angle) + (base_angle * p))
        .collect();

    let mut origin_to_corner_angles = vec![0.0f32; num_corners];
    for corner in 1..num_corners {
        origin_to_corner_angles[corner] = origin_to_corner_angles[corner - 1] + angles[corner - 1];
    }
    for a in &mut origin_to_corner_angles {
        *a += degrees_to_pi(rotation_degrees);
    }

    origin_to_corner_angles
        .iter()
        .map(|&angle| {
            let radius = uniform(min_max_distance.0, min_max_distance.1);
            Point::new(radius * angle.cos(), radius * angle.sin())
        })
        .collect()
}

/// Room modification parameters shared by both the room (`-X/-Y/-b/-B/-e/-E/-A/-M`)
/// and speaker (`-F/-g/-h/-H/-I/-j/-J/-k`) transforms - `scaleAndRotateCoordinates()`
/// is one function applied twice in the C with different arguments.
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct CoordinateTransform {
    pub x_translation: f32,
    pub y_translation: f32,
    pub neg_x_scale: f32,
    pub pos_x_scale: f32,
    pub neg_y_scale: f32,
    pub pos_y_scale: f32,
    pub rotation_degrees: f32,
    pub scale: f32,
}

impl Default for CoordinateTransform {
    fn default() -> Self {
        CoordinateTransform {
            x_translation: 0.,
            y_translation: 0.,
            neg_x_scale: 1.,
            pos_x_scale: 1.,
            neg_y_scale: 1.,
            pos_y_scale: 1.,
            rotation_degrees: 0.,
            scale: 1.,
        }
    }
}

impl CoordinateTransform {
    /// True when every field is at its identity value - the C skips the
    /// whole transform (and re-deriving angle/distance) in this case, a
    /// pure optimization with no behavioral difference (see the module's
    /// own reading notes); this port always recomputes instead, which is
    /// numerically identical.
    fn is_identity(&self) -> bool {
        *self == CoordinateTransform::default()
    }
}

/// Ports `scaleAndRotateCoordinates()`: translate, then quadrant-signed
/// per-axis scale (based on the *post-translation* coordinate sign), then
/// uniform scale, then (if rotating) reconstruct from the newly-scaled
/// origin-distance at the rotated angle. Returns the transformed points
/// paired with their final origin-to-point angle and distance (the C's
/// own `originToPointAngles`/`originToPointDistances` out-parameters).
pub fn apply_coordinate_transform(
    points: &[Point],
    t: &CoordinateTransform,
) -> Vec<(Point, f32, f32)> {
    points
        .iter()
        .map(|&p| {
            if t.is_identity() {
                let angle = segment_angle(Segment::new(Point::ORIGIN, p));
                let distance = segment_length(Segment::new(Point::ORIGIN, p));
                return (p, angle, distance);
            }

            let mut x = p.x + t.x_translation;
            let mut y = p.y + t.y_translation;

            if x < 0. {
                x *= t.neg_x_scale;
            } else if x > 0. {
                x *= t.pos_x_scale;
            }
            if y < 0. {
                y *= t.neg_y_scale;
            } else if y > 0. {
                y *= t.pos_y_scale;
            }

            x *= t.scale;
            y *= t.scale;

            let mut angle = y.atan2(x);
            let distance = x.hypot(y);

            if t.rotation_degrees != 0. {
                angle += degrees_to_pi(t.rotation_degrees);
                x = distance * angle.cos();
                y = distance * angle.sin();
            }

            (Point::new(x, y), angle, distance)
        })
        .collect()
}

/// Ports `cut_data_lines()` (`legacy/pvc_lib/cut_data_lines.c`), the
/// shared coordinate/table-file comment-stripping preprocessor also used
/// by `harmonizer`/`inharmonator`/`groupdelaymaker`: strips `{...}`
/// comments, then applies "solo" mode (keep only `!`-marked records,
/// `field_count` whitespace tokens each) if any `!` marker exists
/// anywhere in the file, else "mute" mode (drop `m`-marked records). A
/// marker only counts when the *previous* character isn't alphabetic and
/// isn't `/`. This is a local, `field_count`-parameterized twin of
/// `tools::chordmapperplus::cut_data_lines` (that one hardcodes its own
/// tone-record field count) - not a call to it, to avoid touching already
/// -shipped code for an unrelated tool; a future cleanup could unify both
/// behind one shared parameterized function.
pub fn cut_data_lines(text: &str, field_count: usize) -> Result<String, String> {
    let mut stage1 = String::with_capacity(text.len());
    let mut brace_open = false;
    for c in text.chars() {
        match c {
            '{' => {
                if brace_open {
                    return Err("two open braces in a row".to_string());
                }
                brace_open = true;
            }
            '}' => {
                if !brace_open {
                    return Err("brace not open".to_string());
                }
                brace_open = false;
            }
            _ => {
                if !brace_open {
                    stage1.push(c);
                }
            }
        }
    }
    if brace_open {
        return Err("open curly brace at end of file".to_string());
    }

    let chars: Vec<char> = stage1.chars().collect();
    let is_marker = |chars: &[char], i: usize, marker: char| -> bool {
        if chars[i] != marker {
            return false;
        }
        let last_c = if i == 0 { ' ' } else { chars[i - 1] };
        !last_c.is_alphabetic() && last_c != '/'
    };

    let solo_on = (0..chars.len()).any(|i| is_marker(&chars, i, '!'));

    let mut out = String::with_capacity(stage1.len());
    let mut i = 0usize;
    if solo_on {
        while i < chars.len() {
            if is_marker(&chars, i, '!') {
                i += 1;
                for field in 0..field_count {
                    let (tok, next) = take_token(&chars, i);
                    i = next;
                    if field > 0 {
                        out.push(' ');
                    }
                    out.push_str(&tok);
                }
                out.push(' ');
            } else {
                i += 1;
            }
        }
    } else {
        while i < chars.len() {
            if is_marker(&chars, i, 'm') {
                i += 1;
                for _ in 0..field_count {
                    let (_, next) = take_token(&chars, i);
                    i = next;
                }
            } else {
                out.push(chars[i]);
                i += 1;
            }
        }
    }
    Ok(out)
}

fn take_token(chars: &[char], mut i: usize) -> (String, usize) {
    while i < chars.len() && chars[i].is_whitespace() {
        i += 1;
    }
    let mut tok = String::new();
    while i < chars.len() && !chars[i].is_whitespace() {
        tok.push(chars[i]);
        i += 1;
    }
    (tok, i)
}

/// Parses whitespace-separated float pairs (matching the C's
/// `fscanf(data, " %f ", &temp)` counting loop), erroring if the count is
/// odd - the real "coordinates file must contain X/Y (or angle/radius)
/// pairs" check every one of `getPolygonCoordinates`/`getListenerCoordinates`/
/// `getSourceCoordinates`/`getSpeakerCoordinates` performs.
pub fn parse_coordinate_pairs(text: &str) -> Result<Vec<Point>, String> {
    let values: Result<Vec<f32>, String> = text
        .split_whitespace()
        .map(|tok| {
            tok.parse::<f32>()
                .map_err(|e| format!("could not parse {tok:?}: {e}"))
        })
        .collect();
    let values = values?;
    if values.len() % 2 != 0 {
        return Err(format!(
            "coordinates file has an odd number of values ({}); must contain X/Y pairs",
            values.len()
        ));
    }
    Ok(values
        .chunks_exact(2)
        .map(|c| Point::new(c[0], c[1]))
        .collect())
}

/// How room polygon coordinates are sourced - `usage()`'s `-f` flag.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PolygonCoordinateSource {
    /// `-f0` (the default): synthesized via [`make_cornered_space`].
    Synthesized,
    /// `-f1`: Cartesian (X-Y) pairs from a file.
    Cartesian,
    /// `-f2`: Polar (angle-in-degrees, radius) pairs from a file, converted
    /// to Cartesian.
    Polar,
}

/// Converts polar (angle-in-degrees, radius) pairs to Cartesian, matching
/// `getPolygonCoordinates()`'s own `-f2` branch.
pub fn polar_to_cartesian(pairs: &[Point]) -> Vec<Point> {
    pairs
        .iter()
        .map(|p| {
            let (angle, radius) = (p.x, p.y);
            let rad = degrees_to_pi(angle);
            Point::new(radius * rad.cos(), radius * rad.sin())
        })
        .collect()
}

/// Ports the file-based branch of `getListenerCoordinates()`/
/// `getSourceCoordinates()`: exactly one coordinate pair is required - see
/// finding 1. `field_text` is `None` for "use the default (0, 0) position"
/// (the C's own behavior when the coordinates file flag is omitted).
pub fn resolve_single_point(field_text: Option<&str>) -> Result<Point, String> {
    match field_text {
        None => Ok(Point::ORIGIN),
        Some(text) => {
            let cut = cut_data_lines(text, 2)?;
            let points = parse_coordinate_pairs(&cut)?;
            match points.len() {
                1 => Ok(points[0]),
                n => Err(format!(
                    "routine is designed for use with only one pair of coordinates, got {n}"
                )),
            }
        }
    }
}

/// Ports the file-based branch of `getSpeakerCoordinates()`: one or more
/// positions allowed (unlike listener/source), defaulting to a single
/// speaker at the origin when no file is supplied.
pub fn resolve_speaker_points(field_text: Option<&str>) -> Result<Vec<Point>, String> {
    match field_text {
        None => Ok(vec![Point::ORIGIN]),
        Some(text) => {
            let cut = cut_data_lines(text, 2)?;
            parse_coordinate_pairs(&cut)
        }
    }
}

/// Ports `makeListenerToSpeakerAngles()`'s angle computation (the min/max
/// index and straddle-flag bookkeeping is downstream reflection-algorithm
/// setup, out of this phase's scope). Since `getListenerCoordinates()`
/// only ever resolves a single listener position (finding 1), the C's own
/// `position % numberOfListenerPositions` indexing is always `% 1` (a
/// no-op) - this port takes a single [`Point`] rather than reproducing
/// that dead modulo.
pub fn listener_to_speaker_angles(listener: Point, speakers: &[Point]) -> Vec<f32> {
    speakers
        .iter()
        .map(|&speaker| segment_angle(Segment::new(listener, speaker)))
        .collect()
}

/// Ports `orderOfAnglesTest()`: with 3 or more speakers, checks that
/// consecutive angle deltas are consistently signed (allowing at most one
/// sign flip in each direction); 2 or fewer speakers always pass (matching
/// the C's own `numberOfSpeakerPositions > 2` guard).
pub fn angles_in_order(angles: &[f32]) -> bool {
    if angles.len() <= 2 {
        return true;
    }
    let n = angles.len();
    let mut positive = 0;
    let mut negative = 0;
    for i in 0..n {
        if angles[(i + 1) % n] - angles[i] > 0. {
            positive += 1;
        } else {
            negative += 1;
        }
    }
    if positive > negative {
        negative <= 1
    } else {
        positive <= 1
    }
}

/// Ports `mirrorPointAroundLineSegment()` (byte-identical at both of its
/// definitions, lines ~510 and ~2382): reflects `point` across the
/// (infinite) line containing `line`, in `f64` - see the module doc
/// comment's precision note. `line`'s four components are `(x0, y0, x1,
/// y1)`, matching the C's flat `line[4]`.
pub fn mirror_point_f64(point: (f64, f64), line: (f64, f64, f64, f64)) -> (f64, f64) {
    let (px, py) = point;
    let (lx0, ly0, lx1, ly1) = line;

    if (lx0 - lx1).abs() < 0.000_000_001 {
        // VERTICAL
        (lx0 + (lx0 - px), py)
    } else if (ly0 - ly1).abs() < 0.000_000_001 {
        // HORIZONTAL
        (px, ly0 + (ly0 - py))
    } else {
        // DIAGONAL: reflect through the perpendicular intersection of
        // `point` with `line`.
        let line_slope = (ly1 - ly0) / (lx1 - lx0);
        let intercept_y = ly0 - (line_slope * lx0);
        let out_line_slope = -1.0 / line_slope;
        let out_intercept_y = py - (out_line_slope * px);
        let ix = (out_intercept_y - intercept_y) / (line_slope - out_line_slope);
        let iy = (out_line_slope * ix) + out_intercept_y;
        (ix + (ix - px), iy + (iy - py))
    }
}

/// `f32` convenience wrapper matching every real call site of
/// `mirrorPointAroundLineSegment()`: convert to `f64`, mirror, truncate
/// back to `f32` (see [`mirror_point_f64`] and the module doc comment's
/// precision note).
pub fn mirror_point(point: Point, line: Segment) -> Point {
    let (x, y) = mirror_point_f64(
        (point.x as f64, point.y as f64),
        (
            line.a.x as f64,
            line.a.y as f64,
            line.b.x as f64,
            line.b.y as f64,
        ),
    );
    Point::new(x as f32, y as f32)
}

/// Ports `pointToLinePosition()`: the sign of the 2D cross product of `(B -
/// A)` and `(P - A)`, computed in `double` then reduced with `copysign` -
/// see finding 10 for why this uses `f64::copysign` rather than
/// `.signum()`. Returns `1` or `-1`, matching the C's actual (never-zero)
/// range.
pub fn point_to_line_position(a: Point, b: Point, p: Point) -> i8 {
    let position = ((b.x as f64 - a.x as f64) * (p.y as f64 - a.y as f64))
        - ((b.y as f64 - a.y as f64) * (p.x as f64 - a.x as f64));
    1.0f64.copysign(position) as i8
}

/// Ports the reflex-vertex-flagging sub-step of `isPolygonConcave()` (run
/// only when the polygon is concave - see [`polygon_is_concave`] for the
/// sign-change test that decides that, ported separately in Phase 1).
/// Returns one flag per vertex, indexed the same as a wall/mirror-side
/// index (wall `i` runs from vertex `i` to vertex `i + 1`): `-1` = reflex,
/// `1` = non-reflex (see finding 10 for why the C's own documented `0`
/// case is unreachable - this port only ever produces `-1`/`1` too, except
/// for the all-zero convex case below). For a convex polygon the C never
/// populates this array (it stays zero-filled from `calloc`); this port
/// matches that by returning all zeros without walking the loop at all.
pub fn polygon_reflex_vertex_flags(polygon: &[Point], is_concave: bool) -> Vec<i8> {
    let n = polygon.len();
    if !is_concave {
        return vec![0; n];
    }

    let mut flags = vec![0i8; n];
    let mut side_count = [0i32; 3]; // bucket index = flag + 1, i.e. covers [-1, 0, 1]

    for c0 in 0..n {
        let v0 = c0;
        let v1 = (c0 + 1) % n;
        let v2 = (c0 + 2) % n;
        let flag = point_to_line_position(polygon[v0], polygon[v1], polygon[v2]);
        flags[(c0 + 1) % n] = flag;
        side_count[(flag + 1) as usize] += 1;
    }

    if side_count[0] > side_count[2] {
        for f in &mut flags {
            *f *= -1;
        }
    }

    flags
}

/// Ports the per-candidate-mirror-side angle-window prefilter inside
/// `mirrorPolygonCoordinatesAroundAllSides()` (lines ~2653-2747): whether a
/// candidate mirror side is even worth exploring at this recursion order, a
/// cheap check run before the far more expensive segment-intersection
/// chain test in [`find_reflection_paths`].
///
/// `endpoint_angles_from_speaker` are the mirror segment's two endpoint
/// angles as seen from the speaker (unsorted, as `atan2` naturally
/// produces them - this function sorts them, matching the C's own sort at
/// the top of the loop body). `parent_window` is `None` for the first
/// recursion order (which always passes, with no window yet established),
/// `Some((low, high))` afterward.
///
/// Returns `(passes, sorted_endpoint_angles)`; the sorted angles feed
/// [`narrow_mirror_segment_angle_window`] for any recursive call this
/// candidate goes on to make. See finding 9 for why this function performs
/// no "rotate past +/-PI" correction on the *window*, even though the C
/// appears to attempt one - only the candidate's own two endpoint angles
/// get that treatment, matching the C's real (buggy) behavior.
pub fn mirror_segment_angle_window_test(
    endpoint_angles_from_speaker: (f32, f32),
    parent_window: Option<(f32, f32)>,
) -> (bool, (f32, f32)) {
    let mut angles = [
        endpoint_angles_from_speaker.0,
        endpoint_angles_from_speaker.1,
    ];
    if angles[0] > angles[1] {
        angles.swap(0, 1);
    }

    let Some((low, high)) = parent_window else {
        return (true, (angles[0], angles[1])); // order 1: always passes, no window yet
    };

    // "SAME SIGN OR STRADDLING ZERO" vs "STRADDLES +/-PI" (see finding 9):
    // only the straddling branch rotates the candidate's own angles.
    let same_sign = low.is_sign_negative() == high.is_sign_negative();
    let short_window = (high - low).abs() < std::f32::consts::PI;
    let candidate_angles = if same_sign || short_window {
        angles
    } else {
        let mut rotated = angles;
        if rotated[0] < 0.0 {
            rotated[0] += std::f32::consts::TAU;
        }
        if rotated[1] < 0.0 {
            rotated[1] += std::f32::consts::TAU;
        }
        if rotated[0] > rotated[1] {
            rotated.swap(0, 1);
        }
        rotated
    };

    let window_low = low - (std::f32::consts::PI / 100.0);
    let window_high = high + (std::f32::consts::PI / 100.0);

    let mut passes = false;
    if candidate_angles[0] >= window_low && candidate_angles[0] <= window_high {
        passes = true;
    }
    if candidate_angles[1] >= window_low && candidate_angles[1] <= window_high {
        passes = true;
    }
    if candidate_angles[0] < window_low && candidate_angles[1] > window_high {
        passes = true;
    }

    (passes, (angles[0], angles[1]))
}

/// Ports the recursive-narrowing step run right before recursing to the
/// next order (lines ~3170-3187): the intersection of the candidate's own
/// sorted endpoint angles (from [`mirror_segment_angle_window_test`]) and
/// the parent window, re-padded by the same `+/-PI/100` margin used by the
/// accept test.
pub fn narrow_mirror_segment_angle_window(
    this_order: usize,
    sorted_endpoint_angles: (f32, f32),
    parent_window: Option<(f32, f32)>,
) -> (f32, f32) {
    let (mut low, mut high) = if this_order == 1 {
        sorted_endpoint_angles
    } else {
        parent_window.expect("parent window is required when narrowing past order 1")
    };
    if sorted_endpoint_angles.0 > low {
        low = sorted_endpoint_angles.0;
    }
    if sorted_endpoint_angles.1 < high {
        high = sorted_endpoint_angles.1;
    }
    (
        low - (std::f32::consts::PI / 100.0),
        high + (std::f32::consts::PI / 100.0),
    )
}

/// One accepted image-source reflection path found by
/// [`find_reflection_paths`] - ports the "viable reflection" bookkeeping at
/// lines ~3046-3128 of `mirrorPolygonCoordinatesAroundAllSides()`.
#[derive(Debug, Clone, PartialEq)]
pub struct ReflectionPath {
    pub order: usize,
    pub distance: f32,
    pub time_seconds: f32,
    /// Mirror-side indices in the C's own `reflectionWalls_lastToFirst`
    /// order: index 0 is the wall mirrored at recursion order 1, which is
    /// physically the *last* wall the sound bounces off before reaching
    /// the speaker; the last entry is physically the *first* bounce from
    /// the source.
    pub mirror_wall_sequence: Vec<usize>,
    pub image_source: Point,
    /// Where the direct segment from speaker to the final image source
    /// crosses the order-1 mirror segment - feeds the source-orientation-
    /// to-reflection angle-difference calculation in a later phase (not
    /// computed here since it needs `rotatedSource`, itself dependent on
    /// out-of-scope CLI/`main()` control flow - see Phase 1's own "worth
    /// the next phase double-checking" note above).
    pub source_to_first_mirror_segment_intersection: Point,
}

/// Aggregate counts [`find_reflection_paths`] returns alongside its
/// [`ReflectionPath`] list - ports `totalExaminedReflections`,
/// `viableReflectionCount`, and `viableReflectionCountByOrder` (all
/// per-output-channel in the C; this port covers one channel's search per
/// call, matching one `outputFileChannelNumber`).
#[derive(Debug, Clone, PartialEq)]
pub struct ReflectionSearchStats {
    pub total_examined: u32,
    pub viable_count: u32,
    pub viable_count_by_order: Vec<u32>,
}

/// Immutable per-search configuration for [`find_reflection_paths`] -
/// groups the geometry/parameters the C reads from its own globals
/// throughout `mirrorPolygonCoordinatesAroundAllSides()`.
pub struct RoomAcousticsInput<'a> {
    pub room: &'a [Point],
    pub polygon_is_concave: bool,
    /// From [`polygon_reflex_vertex_flags`]; pass all-zero for a convex
    /// room (matching the C's own never-populated-when-convex array).
    pub reflex_flags: &'a [i8],
    pub listener: Point,
    pub speaker: Point,
    pub source: Point,
    pub high_order_limit: usize,
    /// `usage()`'s `-o`; the C's default is `1` (test applied). Ports
    /// `listener_space_cross_reflections__include_0__exclude_1 == 1`.
    pub exclude_listener_space_cross_reflections: bool,
    pub speed_of_sound_feet_per_second: f32,
}

/// Per-recursion-level scratch state ported from the C's own order-indexed
/// global arrays (`polygonCoordinates`, `sourceCoordinatesForThisPolygon`,
/// `mirrorSegments`, `mirrorSideNumber`) - a stack here instead, since the
/// C's arrays are safe only because the recursion is synchronous
/// depth-first (each order's slot is written immediately before use and
/// consumed only by that same call's own recursive child, never by a
/// sibling). `polygons`/`sources` are indexed by order directly (`[0]` is
/// the unmirrored room/source); `mirror_segments`/`mirror_sides` are
/// indexed by `order - 1`.
struct SearchTrail {
    polygons: Vec<Vec<Point>>,
    sources: Vec<Point>,
    mirror_segments: Vec<Segment>,
    mirror_sides: Vec<usize>,
}

/// Ports `mirrorPolygonCoordinatesAroundAllSides()`: the recursive
/// image-source search for every valid reflection path (of any order up to
/// `input.high_order_limit`) from `input.source` to `input.speaker` inside
/// `input.room`. See the module doc comment for what this covers versus
/// what's deferred, and finding 9 for a real bug reproduced inside
/// [`mirror_segment_angle_window_test`].
pub fn find_reflection_paths(
    input: &RoomAcousticsInput,
) -> (Vec<ReflectionPath>, ReflectionSearchStats) {
    let mut results = Vec::new();
    let mut stats = ReflectionSearchStats {
        total_examined: 0,
        viable_count: 0,
        viable_count_by_order: vec![0; input.high_order_limit],
    };
    let mut trail = SearchTrail {
        polygons: vec![input.room.to_vec()],
        sources: vec![input.source],
        mirror_segments: Vec::new(),
        mirror_sides: Vec::new(),
    };
    mirror_polygon_around_all_sides(input, 1, None, None, &mut trail, &mut results, &mut stats);
    (results, stats)
}

fn mirror_polygon_around_all_sides(
    input: &RoomAcousticsInput,
    this_order: usize,
    mirror_side_to_skip: Option<usize>,
    parent_window: Option<(f32, f32)>,
    trail: &mut SearchTrail,
    results: &mut Vec<ReflectionPath>,
    stats: &mut ReflectionSearchStats,
) {
    if this_order > input.high_order_limit {
        return;
    }
    let num_walls = input.room.len();
    let mut viable_this_order = 0u32;

    for mirror_side in 0..num_walls {
        if Some(mirror_side) == mirror_side_to_skip {
            // Don't re-mirror by the same wall that produced the parent
            // order's own mirror polygon (lines 2626-2634).
            continue;
        }

        let parent_polygon = &trail.polygons[this_order - 1];
        let mirror_segment = Segment::new(
            parent_polygon[mirror_side],
            parent_polygon[(mirror_side + 1) % num_walls],
        );

        let angle0 = segment_angle(Segment::new(input.speaker, mirror_segment.a));
        let angle1 = segment_angle(Segment::new(input.speaker, mirror_segment.b));
        let (passes, sorted_angles) =
            mirror_segment_angle_window_test((angle0, angle1), parent_window);
        if !passes {
            continue;
        }

        let mirrored_polygon: Vec<Point> = parent_polygon
            .iter()
            .map(|&p| mirror_point(p, mirror_segment))
            .collect();
        let image_source = mirror_point(trail.sources[this_order - 1], mirror_segment);

        trail.polygons.push(mirrored_polygon);
        trail.sources.push(image_source);
        trail.mirror_segments.push(mirror_segment);
        trail.mirror_sides.push(mirror_side);

        let reflection_segment = Segment::new(input.speaker, image_source);
        let (reflection_ok, first_intersection) =
            evaluate_reflection_candidate(input, trail, this_order, reflection_segment);

        stats.total_examined += 1;

        if reflection_ok {
            stats.viable_count += 1;
            viable_this_order += 1;
            let distance = segment_length(reflection_segment);
            results.push(ReflectionPath {
                order: this_order,
                distance,
                time_seconds: distance / input.speed_of_sound_feet_per_second,
                mirror_wall_sequence: trail.mirror_sides[..this_order].to_vec(),
                image_source,
                source_to_first_mirror_segment_intersection: first_intersection,
            });
        }

        if this_order < input.high_order_limit {
            let new_window =
                narrow_mirror_segment_angle_window(this_order, sorted_angles, parent_window);
            mirror_polygon_around_all_sides(
                input,
                this_order + 1,
                Some(mirror_side),
                Some(new_window),
                trail,
                results,
                stats,
            );
        }

        trail.polygons.pop();
        trail.sources.pop();
        trail.mirror_segments.pop();
        trail.mirror_sides.pop();
    }

    stats.viable_count_by_order[this_order - 1] += viable_this_order;
}

/// Ports the accept/reject test chain applied to one already-mirrored
/// candidate (lines ~2812-3044): the mirror-segment intersection chain,
/// the optional listener-proximity exclusion, the reflex-adjacent-wall
/// test, and (for a concave room) the wall-containment test. `trail` must
/// already have this candidate's own mirror segment/side/polygon pushed
/// (matching the C writing `mirrorSegments[thisOrderMinusOne]`/
/// `mirrorSideNumber[thisOrderMinusOne]` before this same loop at lines
/// 2804-2808). Returns `(accepted, source_to_first_mirror_segment_intersection)`.
fn evaluate_reflection_candidate(
    input: &RoomAcousticsInput,
    trail: &SearchTrail,
    this_order: usize,
    reflection_segment: Segment,
) -> (bool, Point) {
    let num_walls = input.room.len();
    let mut first_intersection = Point::ORIGIN;

    for order_index in 0..this_order {
        let Some(intersection) =
            segments_intersect(trail.mirror_segments[order_index], reflection_segment)
        else {
            return (false, first_intersection);
        };

        if order_index == 0 {
            first_intersection = intersection;
            if input.exclude_listener_space_cross_reflections {
                let to_speaker = segment_length(Segment::new(intersection, input.speaker));
                let to_listener = segment_length(Segment::new(intersection, input.listener));
                if to_speaker > to_listener {
                    return (false, first_intersection);
                }
            }
        }

        if order_index > 0 {
            let (higher, lower) =
                if trail.mirror_sides[order_index] > trail.mirror_sides[order_index - 1] {
                    (
                        trail.mirror_sides[order_index],
                        trail.mirror_sides[order_index - 1],
                    )
                } else {
                    (
                        trail.mirror_sides[order_index - 1],
                        trail.mirror_sides[order_index],
                    )
                };
            let blocked_by_reflex_corner = if higher - lower == 1 {
                input.reflex_flags[higher] == -1
            } else if higher == num_walls - 1 && lower == 0 {
                input.reflex_flags[lower] == -1
            } else {
                false
            };
            if blocked_by_reflex_corner {
                return (false, first_intersection);
            }
        }
    }

    if input.polygon_is_concave
        && !reflection_path_is_unobstructed(input, trail, this_order, reflection_segment)
    {
        return (false, first_intersection);
    }

    (true, first_intersection)
}

/// Ports the concave-room wall-containment test (lines ~2919-3044): the
/// reflection segment (speaker to final image source) must not cross any
/// non-mirror wall of the real room polygon, nor any non-mirror side of
/// any intermediate mirrored polygon along this candidate's own chain.
fn reflection_path_is_unobstructed(
    input: &RoomAcousticsInput,
    trail: &SearchTrail,
    this_order: usize,
    reflection_segment: Segment,
) -> bool {
    let num_walls = input.room.len();

    let skip_next = trail.mirror_sides[0];
    for side in 0..num_walls {
        if side == skip_next {
            continue;
        }
        let wall = Segment::new(input.room[side], input.room[(side + 1) % num_walls]);
        if segments_intersect(wall, reflection_segment).is_some() {
            return false;
        }
    }

    for order_index in 0..this_order {
        let previous_skip = trail.mirror_sides[order_index];
        let next_skip = if order_index == this_order - 1 {
            None
        } else {
            Some(trail.mirror_sides[order_index + 1])
        };
        let polygon = &trail.polygons[order_index + 1];
        for side in 0..num_walls {
            if side == previous_skip || Some(side) == next_skip {
                continue;
            }
            let wall = Segment::new(polygon[side], polygon[(side + 1) % num_walls]);
            if segments_intersect(wall, reflection_segment).is_some() {
                return false;
            }
        }
    }

    true
}

/// Ports `makePreEchoValues()`'s `frontSourceHeadRoomScalar` computation
/// (lines ~9249-9256): a headroom multiplier applied to every reflection's
/// (and every direct-sound pulse's, out of this phase's scope) amplitude,
/// derived from how far the *farthest* forward source position could be
/// from the listener versus a minimum reference distance. Returns `1.0`
/// unclamped whenever that farthest-distance limit doesn't exceed the
/// reference distance (the C's own guard against a negative or
/// zero/negative-exponent-blowup ratio), matching the C's own `float`
/// storage (the `pow()` result narrows to `float` immediately on
/// assignment, not carried in `double` any further - see the module's
/// established precision-cascade convention in `units.rs`).
pub fn front_source_head_room_scalar(
    max_speaker_to_listener_distance: f32,
    source_minimum_distance_from_listener: f32,
    minimum_reference_distance_feet: f32,
    air_absorption_exponent_for_real_space_source: f32,
) -> f32 {
    let limit = max_speaker_to_listener_distance - source_minimum_distance_from_listener;
    if limit <= minimum_reference_distance_feet {
        1.0
    } else {
        let ratio = minimum_reference_distance_feet / limit; // float division, as in the C
        (ratio as f64).powf(air_absorption_exponent_for_real_space_source as f64) as f32
    }
}

/// Ports `preEchoTime`'s computation in `makePreEchoValues()` (lines
/// ~9235-9236): the larger of the two worst-case delay times (speaker-to-
/// speaker and speaker-to-listener), used as a fixed head-start offset so
/// no pulse's delay index ever goes negative. Computing the two delay-time
/// inputs themselves (maximum pairwise distances across every resolved
/// speaker/listener position) is plain geometry over [`Point`]s already
/// available from Phase 1 - left to the caller rather than re-derived here,
/// to avoid duplicating that iteration ahead of the phase that actually
/// wires multi-speaker output-channel setup together.
pub fn pre_echo_time_seconds(
    max_speaker_to_speaker_distance_delay_time: f32,
    max_speaker_to_listener_distance_delay_time: f32,
) -> f32 {
    max_speaker_to_speaker_distance_delay_time.max(max_speaker_to_listener_distance_delay_time)
}

/// Ports the per-viable-reflection angle-difference computation at lines
/// ~3099-3105 of `mirrorPolygonCoordinatesAroundAllSides()`: how far
/// (absolute, wrapped into `[0, PI]`) the *source's own facing angle*
/// diverges from the direction of the segment connecting the source to
/// where its sound path first crosses the order-1 mirror segment - i.e.
/// [`ReflectionPath::source_to_first_mirror_segment_intersection`], which
/// Phase 2 stored specifically to feed this later computation.
/// `rotated_source_angle` is the C's own `rotatedSource` - computed by
/// `main()`'s CLI/source-orientation control flow, out of this phase's
/// scope (see Phase 1's "worth the next phase double-checking" note: that
/// control flow has its own suspected bug making `-q1` behave like `-q0`,
/// still unverified).
pub fn source_orientation_to_reflection_angle_difference(
    rotated_source_angle: f32,
    source: Point,
    source_to_first_mirror_segment_intersection: Point,
) -> f32 {
    let intersection_angle = segment_angle(Segment::new(
        source,
        source_to_first_mirror_segment_intersection,
    ));
    let mut angle_diff = rotated_source_angle - intersection_angle;
    while angle_diff > std::f32::consts::PI {
        angle_diff -= std::f32::consts::TAU;
    }
    while angle_diff < -std::f32::consts::PI {
        angle_diff += std::f32::consts::TAU;
    }
    angle_diff.abs()
}

/// Ports the source-dispersion-pattern amplitude term (lines ~4883-4894):
/// how much a reflection's amplitude rolls off as the source's own facing
/// angle diverges from the reflection's direction (see
/// [`source_orientation_to_reflection_angle_difference`]). **A real, minor
/// quirk found while reading (finding 12)**: every *other* gain term in
/// this same accumulation (see [`reflection_pulse_gain`]) converts its own
/// dB value via the shared `dB_to_amp()` lookup table (this project's
/// [`crate::units::DbToAmp`], with its own deliberate table-interpolation
/// error, see `units.rs`); a commented-out earlier version of *this* line
/// did too (`thisProportionOfDispersionDecibelsAsAmp = dB_to_amp(...)`), but
/// the real, live C computes this one term via the *exact*
/// `pow(10.0, dB/20.)` formula directly instead, bypassing the table
/// entirely. Confirmed by reading both the live line and the commented-out
/// alternative immediately above it. Reproduced as the exact formula, not
/// the table, to match the real (live) C.
pub fn dispersion_pattern_amplitude(
    angle_difference: f32,
    source_dispersion_pattern_rolloff_decibels: f32,
) -> f32 {
    let proportion = angle_difference / std::f32::consts::PI;
    let db = proportion * source_dispersion_pattern_rolloff_decibels;
    10.0f64.powf(db as f64 / 20.0) as f32
}

/// Ports the air-absorption amplitude term (lines ~4908-4915), shared in
/// form with [`front_source_head_room_scalar`] (a `pow` of a
/// distance-based ratio clamped to at most `1.0`) but over a *reflection's*
/// own path distance rather than the farthest forward-source limit.
pub fn air_absorption_multiplier(
    reflection_distance: f32,
    minimum_reference_distance_feet: f32,
    air_absorption_exponent_for_reflections: f32,
) -> f32 {
    let ratio = minimum_reference_distance_feet / reflection_distance; // float division, as in the C
    let clamped = (ratio as f64).min(1.0);
    clamped.powf(air_absorption_exponent_for_reflections as f64) as f32
}

/// Ports the "wall gainscale" product loop (lines ~4919-4942): one
/// `dB_to_amp` factor per wall in `wall_reflection_sequence` (in the same
/// last-to-first order as [`ReflectionPath::mirror_wall_sequence`] - index
/// `0` is the most recent bounce), selected by `mode` (`usage()`'s `-t`,
/// default `-1`, "last wall").
///
/// **A real, confirmed bug (finding 11)**: the C's own `mode < 0` ("FROM
/// LAST") branch is written as `else if( wall_impulse_and_gainscale_response_mode > 0 )`,
/// an exact duplicate of the *first* branch's condition a few lines above,
/// making it permanently unreachable (confirmed by reading; a second,
/// correct copy of this same `> 0`/`< 0` branch pair exists elsewhere in
/// the file, at lines ~7408-7417, ruling out "the whole pattern is meant to
/// work this way"). Every negative-or-zero `mode`, including the tool's own
/// **default** of `-1`, therefore falls through to the final catch-all
/// `else` and multiplies gainscale across *every* wall in the sequence, not
/// just the last `|mode|`. Reproduced exactly: only `mode > 0` ("first
/// `mode` walls") behaves as documented; anything else uses the whole
/// sequence. A `mode` whose magnitude reaches or exceeds the sequence's own
/// length reads past the end of the C's fixed-size array (undefined
/// behavior, not a reproducible value, per the already-established
/// `tools::ratechanger` precedent for undefined C behavior), so this port
/// clamps to the sequence's own last index instead of reading out of
/// bounds.
pub fn wall_gainscale_amplitude(
    wall_reflection_sequence: &[usize],
    mode: i32,
    wall_decibel_gainscale_levels: &[f32],
    db_to_amp: &crate::units::DbToAmp,
) -> f32 {
    let order = wall_reflection_sequence.len();
    debug_assert!(order > 0, "a reflection path always has order >= 1");
    debug_assert!(
        !wall_decibel_gainscale_levels.is_empty(),
        "wall gainscale levels must not be empty"
    );
    let last_n = if mode > 0 {
        ((mode as usize).saturating_sub(1)).min(order - 1)
    } else {
        order - 1
    };
    wall_reflection_sequence[0..=last_n]
        .iter()
        .map(|&wall| {
            db_to_amp
                .convert(wall_decibel_gainscale_levels[wall % wall_decibel_gainscale_levels.len()])
        })
        .product()
}

/// Ports the reflection-order gainscale term (lines ~4947-4951): a single
/// `dB_to_amp` factor selected by this reflection's own order, clamped to
/// the gainscale-level table's own length (the C's `fmin`).
pub fn reflection_order_gainscale_amplitude(
    order: usize,
    reflection_order_decibel_gainscale_levels: &[f32],
    db_to_amp: &crate::units::DbToAmp,
) -> f32 {
    debug_assert!(order > 0, "a reflection path always has order >= 1");
    debug_assert!(
        !reflection_order_decibel_gainscale_levels.is_empty(),
        "reflection-order gainscale levels must not be empty"
    );
    let k = order.min(reflection_order_decibel_gainscale_levels.len());
    db_to_amp.convert(reflection_order_decibel_gainscale_levels[k - 1])
}

/// Every input [`reflection_pulse_gain`] needs to compute one reflection's
/// final pulse amplitude - groups the per-reflection values (from a
/// [`ReflectionPath`] and [`source_orientation_to_reflection_angle_difference`])
/// alongside the scalar tool parameters the C reads from its own globals.
pub struct ReflectionPulseGainInput<'a> {
    pub angle_difference: f32,
    pub source_dispersion_pattern_rolloff_decibels: f32,
    pub reflection_distance: f32,
    pub minimum_reference_distance_feet: f32,
    pub air_absorption_exponent_for_reflections: f32,
    pub wall_reflection_sequence: &'a [usize],
    pub wall_impulse_and_gainscale_response_mode: i32,
    pub wall_decibel_gainscale_levels: &'a [f32],
    pub reflection_order_decibel_gainscale_levels: &'a [f32],
    pub reflected_sound_gain_decibels: f32,
    pub front_source_head_room_scalar: f32,
}

/// Ports the full per-reflection amplitude accumulation (lines ~4886-4956):
/// dispersion-pattern rolloff, then air absorption, then wall gainscale,
/// then reflection-order gainscale, then the flat `-k` reflected-sound gain
/// and [`front_source_head_room_scalar`] - each term multiplied on in the
/// same order the C does, since `dB_to_amp`'s table interpolation makes
/// this genuinely not associativity-free to more than float rounding noise.
/// See finding 12 for why the *first* term alone bypasses the shared
/// `dB_to_amp` table.
pub fn reflection_pulse_gain(
    input: &ReflectionPulseGainInput,
    db_to_amp: &crate::units::DbToAmp,
) -> f32 {
    let mut amp = dispersion_pattern_amplitude(
        input.angle_difference,
        input.source_dispersion_pattern_rolloff_decibels,
    );
    amp *= air_absorption_multiplier(
        input.reflection_distance,
        input.minimum_reference_distance_feet,
        input.air_absorption_exponent_for_reflections,
    );
    amp *= wall_gainscale_amplitude(
        input.wall_reflection_sequence,
        input.wall_impulse_and_gainscale_response_mode,
        input.wall_decibel_gainscale_levels,
        db_to_amp,
    );
    amp *= reflection_order_gainscale_amplitude(
        input.wall_reflection_sequence.len(),
        input.reflection_order_decibel_gainscale_levels,
        db_to_amp,
    );
    amp *= db_to_amp.convert(input.reflected_sound_gain_decibels);
    amp *= input.front_source_head_room_scalar;
    amp
}

/// Ports the reflection pulse's own output-sample delay index (lines
/// ~4863-4867): `j = (int)(preEchoTime + (reflectionTime *
/// reflections_time_scaler * osr) + 0.5)`. Every term up through the
/// product is plain `f32` arithmetic, but the `+ 0.5` is a bare (double)
/// literal in the C, promoting the whole sum to `double` one step before
/// the final truncating `(int)` cast - reproduced by adding it in `f64`,
/// matching this project's established precision-cascade convention (see
/// `units.rs`'s module doc comment). Callers must ensure the sum is
/// non-negative (always true for real room/speed-of-sound inputs) since
/// C's truncating `(int)` cast and Rust's `as i64` only agree for
/// non-negative values.
pub fn reflection_delay_sample_index(
    pre_echo_time_seconds: f32,
    reflection_time_seconds: f32,
    reflections_time_scaler: f32,
    output_sample_rate: f32,
) -> i64 {
    let sum = pre_echo_time_seconds
        + (reflection_time_seconds * reflections_time_scaler * output_sample_rate);
    (sum as f64 + 0.5) as i64
}

/// Ports the reflection pulse's write-vs-skip test (line ~4962): a
/// reflection whose final [`reflection_pulse_gain`] falls below this
/// threshold (`usage()`'s `-3`, default `-96` dB) is never mixed into the
/// output impulse response at all.
pub fn reflection_pulse_passes_inclusion_threshold(
    amp: f32,
    impulse_inclusion_threshold_decibels: f32,
    db_to_amp: &crate::units::DbToAmp,
) -> bool {
    amp >= db_to_amp.convert(impulse_inclusion_threshold_decibels)
}

// ---------------------------------------------------------------------
// Phase 4: wall/reflection-order impulse-response signal-processing math
// ---------------------------------------------------------------------

/// Ports `convolveTwoArrays()`'s actual signal-processing algorithm (block
/// overlap-add FFT convolution via [`crate::fft::rfft`], whose packed
/// layout this function relies on being bit-for-bit the C's own `rfft` -
/// see `fft.rs`'s module doc comment): the full linear convolution of
/// `array0` (treated as the fixed "filter", `Lh0` taps) against `array1`
/// (the "signal", processed in `Lh0`-sample blocks), returning a buffer of
/// length `array0.len() + array1.len() - 1`. The C's own memory
/// caching/reuse across repeated calls (`previousN`/`convolveTwoArraysReset`
/// static state, all pure allocation bookkeeping) is not reproduced - this
/// port allocates fresh scratch space per call, which is numerically
/// identical.
///
/// **Finding 13: the raw result is scaled by `1 / N`** relative to the
/// mathematically "true" linear convolution of `array0`/`array1`, where
/// `N = (2 * array0.len() - 1).next_power_of_two()` is this call's own
/// internal per-block FFT size. This isn't a port bug - it falls straight
/// out of `crate::fft::rfft`'s own forward-transform normalization
/// convention (confirmed by `fft.rs`'s own `impulse_has_flat_magnitude_spectrum`/
/// `forward_then_inverse_round_trips` tests: a bare forward+inverse round
/// trip is unity-gain, but multiplying *two* forward-transformed spectra
/// together before a single inverse leaves one un-cancelled factor of the
/// forward transform's own `1/N` scale in the result), and the real C's
/// own `rfft.c` has the identical convention - so the real
/// `convolveTwoArrays()` produces exactly this same scaled-down result.
/// Whatever later phase actually calls this is expected to renormalize
/// (most likely `filterAndNormalizeImpulseResponseNow()`/
/// `filterAndNormalizeWallImpulseResponses()`, both still deferred),
/// matching the precedent already established in `tools::impulseresponse`
/// (whose own explicit post-`rfft` peak-normalization step exists for the
/// same underlying reason: raw `rfft` output isn't unity-scaled).
///
/// **Finding 14, a genuine algorithmic bug (not a port artifact) - confirmed
/// by direct derivation and a hand-checked test case, faithfully
/// reproduced**: whenever `array1.len()` is *not* an exact multiple of the
/// block size `array0.len()` (`Lh0`), the final block is "under-full"
/// (`sampsToRead < Lh0`), and this algorithm silently drops
/// `Lh0 - sampsToRead` samples of otherwise-correct convolution data from
/// the output's tail, while also mis-placing the final overlap-add carry
/// (`BthisB`) at the wrong global offset. The carry is captured from each
/// block's own *local* indices `[Lh0, Lh0 + Lh0m1)` - correct only when
/// every block reads a full `Lh0` real samples - but the flush step then
/// writes that carry starting at `array1Index` (the real, possibly-short
/// count of samples actually consumed), not at `block_start + Lh0`. The
/// local convolution values at indices `[sampsToRead, Lh0)` - which *are*
/// valid, correct output for the corresponding global positions (a
/// zero-padded FIR block's filter taps still reach back into real input
/// samples there) - are read by neither the per-block output-copy loop
/// (bounded by `sampsToRead`) nor the carry capture (starting at `Lh0`),
/// and are lost outright; every value from that point on is then shifted
/// one slot earlier than it should be. Since `array1.len()` being an exact
/// multiple of `array0.len()` is the unusual case for real audio-length
/// inputs, this bug is expected to fire on almost every real call to the
/// real C's own `convolveTwoArrays()` where the two arrays' lengths don't
/// happen to divide evenly - corrupting the last `Lh0` or more samples of
/// its output. Reproduced exactly (not worked around), since this port's
/// purpose is to match the real, verifiable-by-oracle C - `pvc-core`
/// callers of this function should be aware the last several samples of
/// its output can be wrong whenever `array1.len() % array0.len() != 0`.
pub fn convolve_two_arrays(array0: &[f32], array1: &[f32]) -> Vec<f32> {
    let lh0 = array0.len();
    let lh1 = array1.len();
    debug_assert!(lh0 > 0 && lh1 > 0, "both arrays must be non-empty");

    let l0 = 2 * lh0 - 1;
    let n = l0.next_power_of_two();
    let n2 = n >> 1;
    let lh0m1 = lh0 - 1;

    let mut cv_buffer = vec![0.0f32; n];
    cv_buffer[..lh0].copy_from_slice(array0);
    crate::fft::rfft(&mut cv_buffer, n2, true);

    let mut b_this_b = vec![0.0f32; lh0m1];
    let mut output = vec![0.0f32; lh1 + lh0m1];

    let mut array1_index = 0usize;
    while array1_index < lh1 {
        let mut cv_in_buffer = vec![0.0f32; n];
        let samps_to_read = (lh1 - array1_index).min(lh0);
        cv_in_buffer[..samps_to_read]
            .copy_from_slice(&array1[array1_index..array1_index + samps_to_read]);

        crate::fft::rfft(&mut cv_in_buffer, n2, true);

        let mut cv_out_buffer = vec![0.0f32; n];
        cv_out_buffer[0] = cv_in_buffer[0] * cv_buffer[0];
        cv_out_buffer[1] = cv_in_buffer[1] * cv_buffer[1];
        let mut i = 2usize;
        while i < n {
            let j = i + 1;
            let real = (cv_in_buffer[i] * cv_buffer[i]) - (cv_in_buffer[j] * cv_buffer[j]);
            let imag = (cv_in_buffer[i] * cv_buffer[j]) + (cv_in_buffer[j] * cv_buffer[i]);
            cv_out_buffer[i] = real;
            cv_out_buffer[j] = imag;
            i += 2;
        }

        crate::fft::rfft(&mut cv_out_buffer, n2, false);

        for i in 0..lh0m1 {
            cv_out_buffer[i] += b_this_b[i];
        }
        b_this_b.copy_from_slice(&cv_out_buffer[lh0..lh0 + lh0m1]);

        output[array1_index..array1_index + samps_to_read]
            .copy_from_slice(&cv_out_buffer[..samps_to_read]);
        array1_index += samps_to_read;
    }

    output[array1_index..array1_index + lh0m1].copy_from_slice(&b_this_b);
    output
}

/// Ports `cropEndForSilence()`'s actual index math: the length `array`
/// would be trimmed to after dropping every trailing sample below
/// `threshold`. The C's two return paths (`index + 1` when cropping
/// happened, `sameSize` when the last sample already meets `threshold`)
/// both reduce to the same `index + 1` value, so this port has only one
/// path; the C's own `shortenMemory`-gated realloc is pure memory
/// bookkeeping, not reproduced (matching this module's established
/// convention of returning lengths/values rather than replicating C's
/// alloc/free calls). **Not reproduced**: the C's boundary condition
/// (`(array[index] < threshold) && (index >= 0)`) checks the array
/// element *before* the bounds guard, so an empty `array` would read one
/// element past the start in the C - undefined behaviour, not a value-level
/// bug, so this port simply guards `index >= 0` first instead (returning
/// `0` for an empty array, the well-defined intent).
pub fn crop_end_for_silence(array: &[f32], threshold: f32) -> usize {
    if array.is_empty() {
        return 0;
    }
    let mut index = array.len() as isize - 1;
    while index >= 0 && array[index as usize] < threshold {
        index -= 1;
    }
    (index + 1) as usize
}

/// Ports `cropIR_DataEndForSilence()`: like [`crop_end_for_silence`], but
/// returns the *last-loud* sample index itself (no `+ 1`) plus a release
/// tail of `output_sample_rate * end_crop_release_time_seconds` samples,
/// capped at `array.len()` - a deliberately different contract from
/// [`crop_end_for_silence`] (this one exists to leave room for
/// [`smooth_release_of_cropped_end`]'s fade, not to crop tight), not a
/// bug despite the near-identical name/purpose. Returns `i64` rather than
/// `usize` because the C's own clamp is one-sided (only an *upper* bound
/// against `array.len()`): a fully-silent `array` with a release time too
/// short to push the sum positive can make the C return a small negative
/// `int`, which this port preserves rather than silently clamping to `0`.
pub fn crop_ir_data_end_for_silence(
    array: &[f32],
    threshold: f32,
    output_sample_rate: f32,
    end_crop_release_time_seconds: f32,
) -> i64 {
    let mut index: i64 = array.len() as i64 - 1;
    while index >= 0 && array[index as usize] < threshold {
        index -= 1;
    }
    index += (output_sample_rate * end_crop_release_time_seconds) as i64;
    index.min(array.len() as i64)
}

/// Ports `findPeakAmp()`. The C's own "peak amp found is 0" `stderr`
/// warning on an all-zero (or empty) `array` is diagnostic-only, not
/// reproduced (matching this project's convention of not porting `stderr`
/// logging - see e.g. `tools::spectrummapper`'s own findings).
pub fn find_peak_amp(array: &[f32]) -> f32 {
    array.iter().fold(0.0f32, |peak, &x| peak.max(x.abs()))
}

/// Ports `filterFFT()`'s per-bin low/high dB-per-octave rolloff shaping of
/// an already-`rfft`-forward-transformed buffer (`fft_array`, packed per
/// `crate::fft`'s layout: slots `[0]`/`[1]` hold DC/Nyquist, every other
/// pair is a `(re, im)` bin). `n` is the transform's own real-sample size
/// (`2 * n2`); the loop below walks `n / 2` "bins" via `fundamental`.
///
/// **Finding 15, confirmed by the `rfft` packing** (not present in a
/// standard bin-per-slot FFT layout): the loop's `k == 0` iteration reads
/// slots `[0]`/`[1]`, which the packed layout uses for *DC and Nyquist
/// together*, not for bin 0's own (real, imaginary) pair - bin 0 has no
/// imaginary part, so the layout reuses slot `[1]` for the *Nyquist* bin's
/// (also purely real) amplitude instead. Whenever `low_freq > 0.` (the
/// tool's own common case), `binFreq(0) = 0. < low_freq` is always true,
/// and the `k == 0` branch hard-zeros *both* slots - meaning the Nyquist
/// bin's amplitude is unconditionally destroyed by the *low*-frequency
/// rolloff, regardless of `high_freq`, and is never evaluated against the
/// `high_freq`/`high_rolloff` branch at all (no other `k` value ever maps
/// to slot `[1]`). Reproduced exactly here - not a guess, confirmed by
/// tracing which slots each `k` actually touches against `fft.rs`'s
/// documented packing. This is now the *second* independent confirmation
/// of this exact rfft-packing gotcha in this codebase: `tools::irconvolver`'s
/// own `apply_bandpass_rolloff` (its module doc comment) already documents
/// the identical DC/Nyquist-slot-sharing asymmetry in its own,
/// differently-shaped rolloff loop.
/// Rolloff shape shared by [`filter_fft`]/[`filter_audio_array`] - the C's
/// own `lowFreq`/`highFreq`/`lowRolloffInDBperOctave`/`highRolloffInDBperOctave`/
/// `compoundLevels` parameter group, bundled to keep both functions' own
/// argument count reasonable.
#[derive(Debug, Clone, Copy)]
pub struct BandpassRolloff {
    pub low_freq: f32,
    pub high_freq: f32,
    pub low_rolloff_db_per_octave: f32,
    pub high_rolloff_db_per_octave: f32,
    pub compound_levels: i32,
}

pub fn filter_fft(
    fft_array: &mut [f32],
    n: usize,
    fundamental: f32,
    rolloff: &BandpassRolloff,
    db_to_amp: &crate::units::DbToAmp,
) {
    for k in 0..(n / 2) {
        let i = 2 * k;
        let j = i + 1;
        let bin_freq = k as f32 * fundamental;

        if bin_freq < rolloff.low_freq {
            if k == 0 {
                fft_array[i] = 0.0;
                fft_array[j] = 0.0;
            } else {
                let rolloff_db = ((crate::response::hz_to_midi(rolloff.low_freq)
                    - crate::response::hz_to_midi(bin_freq))
                    / 12.0)
                    * rolloff.low_rolloff_db_per_octave;
                let rolloff_amp = db_to_amp.convert(rolloff_db);
                for _ in 0..rolloff.compound_levels.max(0) {
                    fft_array[i] *= rolloff_amp;
                    fft_array[j] *= rolloff_amp;
                }
            }
        } else if bin_freq > rolloff.high_freq {
            let rolloff_db = ((crate::response::hz_to_midi(bin_freq)
                - crate::response::hz_to_midi(rolloff.high_freq))
                / 12.0)
                * rolloff.high_rolloff_db_per_octave;
            let rolloff_amp = db_to_amp.convert(rolloff_db);
            for _ in 0..rolloff.compound_levels.max(0) {
                fft_array[i] *= rolloff_amp;
                fft_array[j] *= rolloff_amp;
            }
        }
    }
}

/// Ports `filterAudioArray()`: zero-pads `signal` to the next power of two
/// past `2 * signal.len() - 1`, forward-`rfft`s it, applies [`filter_fft`],
/// then inverse-`rfft`s - returning the *full* padded-length buffer.
///
/// **Finding 16**: the C writes its own zero-padded, now-N-samples-long
/// result back into `audioArrayForFilter` but leaves
/// `lengthOfAudioArrayForFilter` (the length every caller actually reads)
/// at its original, pre-filter value - the line that would update it
/// (`lengthOfAudioArrayForFilter = N`) is commented out in the source.
/// Every real caller therefore only ever reads the first
/// `signal.len()` samples of this function's output and silently discards
/// the rest - including whatever time-domain "ringing" tail a steep
/// rolloff spread past the original length. This port returns the full
/// `N`-sample buffer rather than guessing at the truncation itself; a
/// later phase reproducing real oracle output must truncate this
/// function's return value back to `signal.len()` to match.
pub fn filter_audio_array(
    signal: &[f32],
    sample_rate: f32,
    rolloff: &BandpassRolloff,
    db_to_amp: &crate::units::DbToAmp,
) -> Vec<f32> {
    debug_assert!(!signal.is_empty());
    let l = 2 * signal.len() - 1;
    let n = l.next_power_of_two();
    let n2 = n >> 1;
    let fundamental = sample_rate / n as f32;

    let mut buf = vec![0.0f32; n];
    buf[..signal.len()].copy_from_slice(signal);

    crate::fft::rfft(&mut buf, n2, true);
    filter_fft(&mut buf, n, fundamental, rolloff, db_to_amp);
    crate::fft::rfft(&mut buf, n2, false);

    buf
}

/// Ports `smoothReleaseOfCroppedEnd()`: fades the last portion of `array`
/// (a release-time proportion of its own duration, capped at 33%) linearly
/// to silence via [`crate::warp::curve`] (`curve(1., 0., n, 0.)`, the same
/// primitive `tools::spectwarper` already reuses).
///
/// **Finding 17, faithfully reproduced**: when the computed fade length
/// rounds to exactly `1` sample, the C's own `(float) i / (float)
/// (numSamples - 1)` divides `0. / 0.` - `NaN`, not `0.` - so that single
/// sample is multiplied by `NaN` (corrupted) rather than left alone or
/// silenced. This port does not special-case it away.
pub fn smooth_release_of_cropped_end(
    array: &mut [f32],
    release_time_seconds: f32,
    sample_rate: f32,
) {
    let size = array.len();
    let dur = size as f32 / sample_rate;
    let release_time_proportion = release_time_seconds / dur;

    let num_samples = if release_time_proportion > 0.33 {
        (0.5 + size as f32 * 0.33) as i64
    } else {
        (0.5 + size as f32 * release_time_proportion) as i64
    };

    for i in 0..num_samples {
        let j = size as i64 - num_samples + i;
        if j < 0 || j as usize >= size {
            continue;
        }
        let n = i as f32 / (num_samples - 1) as f32;
        array[j as usize] *= crate::warp::curve(1.0, 0.0, n, 0.0);
    }
}

/// Ports the wall-IR-sequence selection inside
/// `createAndReorderWallReflectionSequence()` (lines ~7407-7449) - a
/// *different* wall-count-based mode select from [`wall_gainscale_amplitude`]'s
/// own (buggy, see finding 11): this one is the "correct" reference copy
/// finding 11's write-up points to. Given one [`ReflectionPath`]'s own
/// `mirror_wall_sequence` (last-to-first order), returns the subset of
/// walls this reflection's *impulse-response file lookup* actually uses,
/// in first-to-last physical bounce order, each index taken modulo
/// `wall_num_input_channels` (mapping a wall number to an available IR
/// input channel): `mode > 0` keeps the first `mode` walls (closest to the
/// source), `mode < 0` keeps the last `mode.abs()` walls (closest to the
/// speaker), `mode == 0` keeps every wall.
pub fn wall_sequence_for_reflection(
    mirror_wall_sequence_last_to_first: &[usize],
    mode: i32,
    wall_num_input_channels: usize,
) -> Vec<usize> {
    let order = mirror_wall_sequence_last_to_first.len();
    debug_assert!(order > 0, "a reflection path always has order >= 1");
    debug_assert!(wall_num_input_channels > 0);

    let (start_n, num_walls) = if mode > 0 {
        let num_walls = (mode as usize).min(order);
        (order - 1, num_walls)
    } else if mode < 0 {
        let num_walls = (mode.unsigned_abs() as usize).min(order);
        (num_walls - 1, num_walls)
    } else {
        (order - 1, order)
    };

    (0..num_walls)
        .map(|i| mirror_wall_sequence_last_to_first[start_n - i] % wall_num_input_channels)
        .collect()
}

/// Ports `isFirstSequenceGreaterLesserOrEqualToSecond()`'s comparison
/// logic: compares `seq0`/`seq1` element-by-element up to `compare_length`
/// entries, out-of-range entries (past each `Vec`'s own real length)
/// treated as `0` - the first differing position decides the order (ties
/// on that position resolved `>=` toward `Greater`); if every position
/// through `compare_length` matches, the *longer* sequence sorts greater.
///
/// **Finding 19, not reproduced as-is**: the C always passes its own
/// `highOrderLimit` as `compare_length`, relying on the shared backing
/// array's out-of-range tail being `0` - true only because that array is
/// `calloc`'d once, at start-up, and *never re-zeroed between calls*. A
/// later reflection's shorter sequence can therefore read a stale,
/// leftover value from an earlier (longer) sequence that previously
/// occupied the same slot, not a real zero - a genuine cross-call memory-
/// reuse hazard, not a fixed value-level bug (same class this project
/// already declines to reproduce for uninitialized-default parameters,
/// e.g. `tools::inharmonator`'s finding 2). This port always compares
/// against a *virtual* zero-padding out to `compare_length`, matching the
/// C's well-defined first-use behavior rather than its undefined
/// memory-reuse hazard.
pub fn compare_wall_sequences(
    seq0: &[usize],
    seq1: &[usize],
    compare_length: usize,
) -> std::cmp::Ordering {
    use std::cmp::Ordering;

    let at = |seq: &[usize], i: usize| -> usize { seq.get(i).copied().unwrap_or(0) };

    for i in 0..compare_length {
        let (a, b) = (at(seq0, i), at(seq1, i));
        if a != b {
            return if a >= b {
                Ordering::Greater
            } else {
                Ordering::Less
            };
        }
    }
    seq0.len().cmp(&seq1.len())
}

/// Ports `createAndReorderWallReflectionSequence()`'s full per-channel
/// flow: select every `reflections` entry whose own `order` falls within
/// `[low_order_limit, high_order_limit]`, build each one's wall-IR
/// sequence via [`wall_sequence_for_reflection`], then stable-sort the
/// selected set via [`compare_wall_sequences`] (matching the C's own
/// bubble sort, which - since it only ever swaps on a strict `Greater` -
/// is itself stable; Rust's `sort_by` preserves the same relative order
/// among ties). Each returned entry's `usize` is the reflection's own
/// index into `reflections`, matching the C's own stored `thisReflection`
/// pointer (later phases doing IR-file-cache lookups need it to trace a
/// sorted sequence back to its originating reflection).
///
/// **Finding 18**: `sortSequence()` - a separate, simpler ascending-order
/// bubble sort over a single flat array - is dead code (confirmed via
/// `grep`: declared and defined, never called anywhere in the file) and
/// is not ported.
pub fn select_and_reorder_wall_reflection_sequences(
    reflections: &[ReflectionPath],
    mode: i32,
    wall_num_input_channels: usize,
    low_order_limit: usize,
    high_order_limit: usize,
) -> Vec<(usize, Vec<usize>)> {
    let mut selected: Vec<(usize, Vec<usize>)> = reflections
        .iter()
        .enumerate()
        .filter(|(_, r)| r.order >= low_order_limit && r.order <= high_order_limit)
        .map(|(i, r)| {
            (
                i,
                wall_sequence_for_reflection(
                    &r.mirror_wall_sequence,
                    mode,
                    wall_num_input_channels,
                ),
            )
        })
        .collect();

    selected.sort_by(|a, b| compare_wall_sequences(&a.1, &b.1, high_order_limit));
    selected
}

/// Ports `findAverageReflectionOrderDelayTimes()`: for each reflection
/// order from `1` to `high_order_limit`, the mean [`ReflectionPath::time_seconds`]
/// across every `reflections` entry of that order - index `0` of the
/// returned `Vec` is order `1`, matching the C's own `orderAverageReflectionTimes[order - 1]`.
/// An order with no reflections at all divides by zero, producing `NaN`
/// (the C's own `stderr`-printed diagnostic behavior, faithfully
/// reproduced via the same `0. / 0.` float division - not a crash, and
/// this function is itself diagnostic-only in the C, feeding only a
/// `stderr` printout, not further audio math).
pub fn average_reflection_order_delay_times(
    reflections: &[ReflectionPath],
    high_order_limit: usize,
) -> Vec<f32> {
    (1..=high_order_limit)
        .map(|order| {
            let (sum, count) = reflections
                .iter()
                .filter(|r| r.order == order)
                .fold((0.0f32, 0u32), |(sum, count), r| {
                    (sum + r.time_seconds, count + 1)
                });
            sum / count as f32
        })
        .collect()
}

// ---------------------------------------------------------------------
// Phase 5: the direct-sound (unreflected) pulse path
// ---------------------------------------------------------------------

/// Ports the min/max/straddle-flag bookkeeping half of
/// `makeListenerToSpeakerAngles()` (lines ~4009-4089) that Phase 1's
/// [`listener_to_speaker_angles`] deliberately left for "downstream
/// reflection-algorithm setup" - it turns out this bookkeeping is never
/// actually used by the reflection recursion (Phase 2), only by this
/// phase's own [`find_crossfade_speaker_pair`]/[`make_source_to_threshold_proximity_distance`].
pub struct ListenerToSpeakerAngleBounds {
    pub minimum: f32,
    pub minimum_index: usize,
    pub maximum: f32,
    pub maximum_index: usize,
    /// `true` at index `i` when the wrap segment from speaker `i` to
    /// speaker `i + 1` (mod count) is the one whose two endpoint angles
    /// straddle the +/-PI boundary - at most one `true` entry for a
    /// well-formed speaker fan.
    pub straddle_flags: Vec<bool>,
}

/// Ports the second half of `makeListenerToSpeakerAngles()`.
/// `speaker_configuration_is_polygon` is `usage()`'s speaker-configuration
/// flag (`sequence` = false, `polygon` = true): a 2-speaker "stereo pair"
/// always uses the stereo-specific straddle test regardless of this flag
/// (matching the C's own `numberOfSpeakerPositions == 2` special case);
/// 3-or-more speakers scan one fewer wrap segment in `sequence` mode (the
/// last speaker doesn't wrap back to the first).
pub fn listener_to_speaker_angle_bounds(
    angles: &[f32],
    speaker_configuration_is_polygon: bool,
) -> ListenerToSpeakerAngleBounds {
    let n = angles.len();
    debug_assert!(n > 0, "at least one speaker position is required");
    let mut minimum = std::f32::consts::TAU;
    let mut minimum_index = 0;
    let mut maximum = -std::f32::consts::TAU;
    let mut maximum_index = 0;
    for (i, &a) in angles.iter().enumerate() {
        if a < minimum {
            minimum = a;
            minimum_index = i;
        }
        if a > maximum {
            maximum = a;
            maximum_index = i;
        }
    }

    let mut straddle_flags = vec![false; n];
    if n == 2 {
        if (maximum - minimum) > std::f32::consts::PI {
            straddle_flags[0] = true;
        }
    } else {
        let last = if speaker_configuration_is_polygon {
            n
        } else {
            n - 1
        };
        for (position, flag) in straddle_flags.iter_mut().enumerate().take(last) {
            let p1 = (position + 1) % n;
            if (position == minimum_index && p1 == maximum_index)
                || (position == maximum_index && p1 == minimum_index)
            {
                *flag = true;
            }
        }
    }

    ListenerToSpeakerAngleBounds {
        minimum,
        minimum_index,
        maximum,
        maximum_index,
        straddle_flags,
    }
}

/// Ports `valueIsBetweenTheseTwo()`: `true` if `v` falls within `[b0, b1]`
/// regardless of which bound is numerically smaller.
pub fn value_is_between_these_two(v: f32, b0: f32, b1: f32) -> bool {
    (v >= b0 && v <= b1) || (v <= b0 && v >= b1)
}

/// Ports `rotatePointToAngle()`: keeps `point_to_rotate`'s own distance
/// from `origin` but re-derives its position at `new_angle` instead. The
/// C promotes `cos`/`sin`'s argument (and result) through `double` before
/// narrowing back to `float`; reproduced the same way (this project's
/// established precision-cascade convention, see `units.rs`).
pub fn rotate_point_to_angle(point_to_rotate: Point, origin: Point, new_angle: f32) -> Point {
    let length = segment_length(Segment::new(origin, point_to_rotate)) as f64;
    let angle = new_angle as f64;
    Point::new(
        (length * angle.cos()) as f32 + origin.x,
        (length * angle.sin()) as f32 + origin.y,
    )
}

/// Ports `findIntersectionOfLinesContainingSegments()`.
///
/// **Finding 23**: unlike its sibling [`segments_intersect`]
/// (`examineSegmentsForIntersection`), every bounding-box containment
/// check here is commented out in the real C (three whole `if` blocks,
/// each replaced by an unconditional `segmentsIntersect = true;`) - so
/// despite taking the same `float[4]`-style segment arguments as
/// `segments_intersect`, this function actually treats `w`/`p` as
/// *infinite lines*, not segments: it reports an intersection for any two
/// non-parallel lines, regardless of whether the computed point falls
/// within either input segment's own extent. Reproduced exactly - `None`
/// only for the parallel (including "both vertical") cases.
pub fn find_intersection_of_lines_containing_segments(w: Segment, p: Segment) -> Option<Point> {
    let (wx0, wy0, wx1, wy1) = (w.a.x, w.a.y, w.b.x, w.b.y);
    let (px0, py0, px1, py1) = (p.a.x, p.a.y, p.b.x, p.b.y);

    let w_vertical = (wx1 - wx0) == 0.0;
    let p_vertical = (px1 - px0) == 0.0;

    if w_vertical && p_vertical {
        return None;
    }
    if w_vertical {
        let mp = (py1 - py0) / (px1 - px0);
        let bp = py0 - (mp * px0);
        let x = wx0;
        return Some(Point::new(x, (mp * x) + bp));
    }
    if p_vertical {
        let mw = (wy1 - wy0) / (wx1 - wx0);
        let bw = wy0 - (mw * wx0);
        let x = px0;
        return Some(Point::new(x, (mw * x) + bw));
    }

    let mw = (wy1 - wy0) / (wx1 - wx0);
    let bw = wy0 - (mw * wx0);
    let mp = (py1 - py0) / (px1 - px0);
    let bp = py0 - (mp * px0);
    if mp == mw {
        return None;
    }
    let x = (bw - bp) / (mp - mw);
    Some(Point::new(x, (mp * x) + bp))
}

/// Ports the crossfade-speaker-pair search shared (in effect, though
/// duplicated rather than factored out in the C) by
/// `makeSourceToThresholdProximityDistance()` (lines ~6147-6193) and the
/// dead `writeDirectSourcePulsesIntoImpulseResponseOLD()`'s own copy of the
/// same loop - this port implements only the live version. Returns the
/// `(speaker0, speaker1)` pair the given angle falls between, or `None` if
/// no such pair exists (matching the C's `sourceIsBetweenTwoSpeakers ==
/// false` outcome).
pub fn find_crossfade_speaker_pair(
    listener_to_source_angle: f32,
    listener_to_speaker_angles: &[f32],
    bounds: &ListenerToSpeakerAngleBounds,
    speaker_configuration_is_polygon: bool,
) -> Option<(usize, usize)> {
    let n = listener_to_speaker_angles.len();
    let limit = (n - 1) + usize::from(speaker_configuration_is_polygon);
    let mut speaker0 = 0usize;
    while speaker0 < limit {
        let speaker1 = (speaker0 + 1) % n;
        let is_between = if bounds.straddle_flags[speaker0] {
            listener_to_source_angle > bounds.maximum || listener_to_source_angle < bounds.minimum
        } else {
            value_is_between_these_two(
                listener_to_source_angle,
                listener_to_speaker_angles[speaker0],
                listener_to_speaker_angles[speaker1],
            )
        };
        if is_between {
            return Some((speaker0, speaker1));
        }
        speaker0 += 1;
    }
    None
}

/// The distance/intersection-point result [`make_source_to_threshold_proximity_distance`]
/// computes - ports `sourceToThresholdProximityDistance`/
/// `thresholdAndListenerToSourceIntersection`.
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct ThresholdProximityResult {
    pub source_to_threshold_proximity_distance: f32,
    pub threshold_and_listener_to_source_intersection: Point,
}

/// Ports `makeSourceToThresholdProximityDistance()`'s two live branches
/// (lines ~6195-6287): when the source's own listener-relative angle falls
/// between two speakers (`crossfade` is `Some`, from
/// [`find_crossfade_speaker_pair`]), the threshold point is the real
/// intersection of the listener-source segment with the speaker-to-speaker
/// segment between them (reusing [`segments_intersect`] - the C calls the
/// exact same `examineSegmentsForIntersection()` here as the reflection
/// path's own accept tests do); otherwise, in `sequence` speaker
/// configuration only, it extrapolates from whichever end of the speaker
/// sequence is nearest the source, using the *unbounded* line intersection
/// [`find_intersection_of_lines_containing_segments`] instead (matching the
/// real C's own choice of function at this call site).
///
/// Returns `Ok(None)` for the remaining case the C itself leaves
/// unhandled: `polygon` speaker configuration with no crossfade pair found.
/// A well-formed polygon speaker fan gives
/// [`find_crossfade_speaker_pair`] full 360-degree coverage, so this should
/// not arise for valid room/speaker geometry - the C simply leaves
/// `sourceToThresholdProximityDistance`/`thresholdAndListenerToSourceIntersection`
/// at whatever value they last held (their own zero-initialized default on
/// a fresh run) in this case, which this port surfaces as `None` rather
/// than silently fabricating a stale value.
///
/// Returns `Err` only when the `sequence`-mode extrapolation's two nearest
/// speakers are collinear with the listener-source line (parallel lines),
/// matching the C's own `exit(EXIT_FAILURE)` at line ~6275 for this case.
pub fn make_source_to_threshold_proximity_distance(
    speakers: &[Point],
    listener: Point,
    source: Point,
    crossfade: Option<(usize, usize)>,
    speaker_configuration_is_polygon: bool,
) -> Result<Option<ThresholdProximityResult>, String> {
    if let Some((s0, s1)) = crossfade {
        let threshold_segment = Segment::new(speakers[s0], speakers[s1]);
        let listener_to_source = Segment::new(listener, source);
        let Some(intersection) = segments_intersect(threshold_segment, listener_to_source) else {
            return Err(
                "listener-to-source segment does not cross the speaker threshold segment"
                    .to_string(),
            );
        };
        let distance = segment_length(Segment::new(intersection, source));
        return Ok(Some(ThresholdProximityResult {
            source_to_threshold_proximity_distance: distance,
            threshold_and_listener_to_source_intersection: intersection,
        }));
    }

    if speaker_configuration_is_polygon {
        return Ok(None);
    }

    let n = speakers.len();
    debug_assert!(n >= 2, "sequence mode needs at least two speakers");
    let (near0, near1) = if segment_length(Segment::new(source, speakers[0]))
        < segment_length(Segment::new(source, speakers[n - 1]))
    {
        (speakers[0], speakers[1])
    } else {
        (speakers[n - 1], speakers[n - 2])
    };

    let threshold_line = Segment::new(near0, near1);
    let listener_to_source = Segment::new(listener, source);
    let Some(intersection) =
        find_intersection_of_lines_containing_segments(threshold_line, listener_to_source)
    else {
        return Err(
            "PROBLEM WITH INTERSECTION WITH PROJECTED THRESHOLD BEYOND SPEAKER SEQUENCE."
                .to_string(),
        );
    };
    let distance = segment_length(Segment::new(intersection, source));
    Ok(Some(ThresholdProximityResult {
        source_to_threshold_proximity_distance: distance,
        threshold_and_listener_to_source_intersection: intersection,
    }))
}

/// Ports `isSourceBehindOrInFrontOfSpeakerThreshold()`: the source is
/// "behind" the speaker threshold line when it's farther from the listener
/// than the threshold crossing point is.
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct SourceThresholdOrientation {
    pub source_is_behind_speakers: bool,
    /// `1.0` when behind, `-1.0` when in front - ports
    /// `sourceSpeakerOrientationSign`.
    pub source_speaker_orientation_sign: f32,
}

pub fn is_source_behind_or_in_front_of_speaker_threshold(
    listener: Point,
    source: Point,
    threshold_and_listener_to_source_intersection: Point,
) -> SourceThresholdOrientation {
    let listener_to_source = segment_length(Segment::new(listener, source));
    let listener_to_threshold = segment_length(Segment::new(
        listener,
        threshold_and_listener_to_source_intersection,
    ));
    let source_is_behind_speakers = listener_to_source > listener_to_threshold;
    SourceThresholdOrientation {
        source_is_behind_speakers,
        source_speaker_orientation_sign: if source_is_behind_speakers { 1.0 } else { -1.0 },
    }
}

/// Ports `findListenerToSourceSegmentLengthAndAngle()`. The C corrects
/// `sourceToListenerAngle` with a single `if` (not a `while` loop, unlike
/// most other angle-wrap sites in this file) - safe here specifically
/// because `listenerToSourceAngle` is an `atan2` result confined to `(-PI,
/// PI]`, so adding PI can only ever push the sum into `(0, 2*PI]`, at most
/// one `TWOPI` subtraction away from the target range; reproduced with the
/// same single correction, not a loop.
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct ListenerToSourceGeometry {
    pub length: f32,
    pub listener_to_source_angle: f32,
    pub source_to_listener_angle: f32,
}

pub fn find_listener_to_source_segment_length_and_angle(
    listener: Point,
    source: Point,
) -> ListenerToSourceGeometry {
    let seg = Segment::new(listener, source);
    let listener_to_source_angle = segment_angle(seg);
    let mut source_to_listener_angle = listener_to_source_angle + std::f32::consts::PI;
    if source_to_listener_angle > std::f32::consts::PI {
        source_to_listener_angle -= std::f32::consts::TAU;
    }
    ListenerToSourceGeometry {
        length: segment_length(seg),
        listener_to_source_angle,
        source_to_listener_angle,
    }
}

/// Ports `makeDirectSoundSpeakerDelayTimes()`. Since `getSourceCoordinates()`
/// only ever resolves a single source position (Phase 1's finding 1), the
/// C's own `sourceCoordinates[(channel % numberOfSourcePositions) * 2]`
/// indexing is always the same point - this port takes a single `source:
/// Point` rather than reproducing that dead modulo (same reasoning Phase 1
/// already applied to [`listener_to_speaker_angles`]).
pub fn direct_sound_speaker_distances_for_delays(speakers: &[Point], source: Point) -> Vec<f32> {
    speakers
        .iter()
        .map(|&speaker| segment_length(Segment::new(source, speaker)))
        .collect()
}

fn wrap_angle_to_pi_range(mut angle: f32) -> f32 {
    while angle > std::f32::consts::PI {
        angle -= std::f32::consts::TAU;
    }
    while angle < -std::f32::consts::PI {
        angle += std::f32::consts::TAU;
    }
    angle
}

/// Ports the direct-sound pulse's own output-sample delay index (the live
/// `writeDirectSourcePulsesIntoImpulseResponse()`, lines ~5081-5087).
///
/// **Finding 24**: this formula's own parenthesization multiplies
/// `output_sample_rate` across the *entire* sum, including
/// `pre_echo_time_seconds` - genuinely different from the structurally
/// similar-looking reflection-pulse formula
/// ([`reflection_delay_sample_index`], Phase 3), whose C source (line
/// ~4863: `j = (int)( preEchoTime + (reflectionTimes[...] *
/// reflections_time_scaler * (float) osr) + 0.5)`) multiplies `osr` only
/// across the *reflection*-time term, leaving `preEchoTime` added in
/// unscaled. Cross-checked against `makePreEchoValues()`'s own
/// `preEchoDistance = preEchoTime * speedOfSoundInFeetPerSecond` (line
/// 9240): this only makes sense if `preEchoTime` is in *seconds* (a
/// distance requires seconds times feet-per-second) - meaning the
/// reflection-pulse formula adds a small seconds-valued quantity directly
/// into a samples-valued sum with no unit conversion, while this
/// direct-sound formula (reproduced here) is the one that treats
/// `preEchoTime` consistently, converting the whole seconds-valued sum to
/// samples via one shared `* osr`. Both are reproduced exactly as each own
/// real C computes them - Phase 3's [`reflection_delay_sample_index`] is
/// not revisited by this phase (out of scope, per this project's
/// established practice of not reopening already-shipped ports), but this
/// finding is recorded here since it was only discoverable by reading both
/// formulas side by side.
pub fn direct_sound_delay_sample_index(
    pre_echo_time_seconds: f32,
    source_speaker_orientation_sign: f32,
    source_to_speaker_distance_for_delay: f32,
    speed_of_sound_feet_per_second: f32,
    output_sample_rate: f32,
) -> i64 {
    let orientation_term = source_speaker_orientation_sign * source_to_speaker_distance_for_delay
        / speed_of_sound_feet_per_second;
    let sum = (pre_echo_time_seconds + orientation_term) * output_sample_rate;
    (sum as f64 + 0.5) as i64
}

/// Ports the small air-absorption-exponent selector shared by
/// `makeDirectSoundSpeakerAmplitudes()`/`writeDirectSourcePulsesIntoImpulseResponseOLD()`
/// (e.g. line ~5817): the direct-sound path's own exponent choice depends
/// on which side of the speaker threshold the source sits, unlike the
/// reflection path's fixed `airAbsorptionExponentForReflections` (Phase 3).
pub fn direct_sound_air_absorption_exponent(
    source_is_behind_speakers: bool,
    air_absorption_exponent_for_real_space_source: f32,
    air_absorption_exponent_for_virtual_space_source: f32,
) -> f32 {
    if source_is_behind_speakers {
        air_absorption_exponent_for_virtual_space_source
    } else {
        air_absorption_exponent_for_real_space_source
    }
}

/// Ports the `sourceInFrontProximityGain` computation at the top of
/// `makeDirectSoundSpeakerAmplitudes()` (lines ~5820-5824): a source in
/// front of the speaker threshold gets boosted by the *inverse* of the air
/// -absorption falloff it would otherwise suffer at the threshold distance
/// (compensating for the threshold-proximity blending applied elsewhere);
/// a source behind the threshold gets no such compensation (`1.0`).
pub fn source_in_front_proximity_gain(
    source_is_behind_speakers: bool,
    minimum_reference_distance_feet: f32,
    source_to_threshold_proximity_distance: f32,
    this_air_absorption_exponent: f32,
) -> f32 {
    if source_is_behind_speakers {
        1.0
    } else {
        let ratio =
            (minimum_reference_distance_feet / source_to_threshold_proximity_distance) as f64;
        let falloff = ratio.min(1.0).powf(this_air_absorption_exponent as f64);
        (1.0 / falloff) as f32
    }
}

/// Every input [`make_direct_sound_speaker_amplitudes`] needs - ports the
/// scalar globals `makeDirectSoundSpeakerAmplitudes()` reads (lines
/// ~5780-6133).
pub struct DirectSoundAmplitudeInput<'a> {
    pub speakers: &'a [Point],
    pub source: Point,
    pub listener: Point,
    pub listener_to_speaker_angles: &'a [f32],
    /// `Some((speaker0, speaker1))` when
    /// [`find_crossfade_speaker_pair`] found a straddling pair
    /// (`sourceIsBetweenTwoSpeakers`); `None` selects the C's own
    /// "NON-CROSSFADE" branch.
    pub crossfade: Option<(usize, usize)>,
    pub listener_to_source_angle: f32,
    pub source_to_listener_angle: f32,
    /// Ports `sourceToListenerAnglePlusRotation`. **Finding 20**: this
    /// global is declared (line 376) but *never assigned anywhere in the
    /// file* - a comment immediately above its one read site (line 1210,
    /// "CHANGE THIS sourceToListenerAnglePlusRotation to rotatedSource")
    /// shows an intended rename/refactor that was only ever completed for
    /// this function's *crossfade* branch (which correctly recomputes its
    /// own `thisRotatedSource` locally), not its non-crossfade branch
    /// (line 6072), which still reads this always-zero global. Every real
    /// run should pass `0.0` here to reproduce the bug faithfully, not the
    /// clearly-intended `rotated_source_angle`.
    pub source_to_listener_angle_plus_rotation: f32,
    pub source_is_behind_speakers: bool,
    pub orient_source_to_listener: bool,
    pub source_rotation: f32,
    pub minimum_reference_distance_feet: f32,
    pub air_absorption_exponent_for_real_space_source: f32,
    pub air_absorption_exponent_for_virtual_space_source: f32,
    pub source_dispersion_pattern_rolloff_decibels: f32,
    pub threshold_proximity_scalar_switch: bool,
    pub source_to_threshold_proximity_distance: f32,
    pub front_source_head_room_scalar: f32,
    pub direct_sound_gain_decibels: f32,
}

/// Ports `makeDirectSoundSpeakerAmplitudes()`'s two live branches in full
/// (lines ~5832-6113): the "BETWEEN SPEAKERS" crossfade branch (rotating
/// the source position to each of the two bracketing speaker angles and
/// blending their independent amplitude terms by
/// `crossFadeProportion`) and the "NOT IN CROSSFADE" branch (a single
/// per-speaker computation using the source's real position directly).
/// See [`DirectSoundAmplitudeInput::source_to_listener_angle_plus_rotation`]
/// (finding 20) for a real bug reproduced in the non-crossfade branch.
pub fn make_direct_sound_speaker_amplitudes(
    input: &DirectSoundAmplitudeInput,
    db_to_amp: &crate::units::DbToAmp,
) -> Vec<f32> {
    let n = input.speakers.len();
    let this_air_absorption_exponent = direct_sound_air_absorption_exponent(
        input.source_is_behind_speakers,
        input.air_absorption_exponent_for_real_space_source,
        input.air_absorption_exponent_for_virtual_space_source,
    );
    let source_in_front_gain = source_in_front_proximity_gain(
        input.source_is_behind_speakers,
        input.minimum_reference_distance_feet,
        input.source_to_threshold_proximity_distance,
        this_air_absorption_exponent,
    );

    if let Some((speaker0, speaker1)) = input.crossfade {
        let mut temp_angles = [
            input.listener_to_speaker_angles[speaker0],
            input.listener_to_speaker_angles[speaker1],
        ];
        let mut listener_to_source_angle_temp = input.listener_to_source_angle;
        if (temp_angles[1] - temp_angles[0]).abs() > std::f32::consts::PI {
            if temp_angles[0] < 0.0 {
                temp_angles[0] += std::f32::consts::TAU;
            }
            if temp_angles[1] < 0.0 {
                temp_angles[1] += std::f32::consts::TAU;
            }
            if listener_to_source_angle_temp < 0.0 {
                listener_to_source_angle_temp += std::f32::consts::TAU;
            }
        }

        let cross_fade_proportion = 1.0
            - ((listener_to_source_angle_temp - temp_angles[0])
                / (temp_angles[1] - temp_angles[0]));

        let branch =
            |crossfade_speaker: usize, base_angle_raw: f32| -> (Vec<f32>, Vec<f32>, Vec<f32>) {
                let mut base_angle = base_angle_raw;
                if input.source_is_behind_speakers {
                    base_angle += std::f32::consts::PI;
                    if base_angle > std::f32::consts::PI {
                        base_angle -= std::f32::consts::TAU;
                    }
                }
                let mut this_rotated_source = if input.orient_source_to_listener {
                    base_angle + input.source_rotation
                } else {
                    input.source_rotation
                };
                this_rotated_source = wrap_angle_to_pi_range(this_rotated_source);

                // Ports `rotatePointToAngle(sourceCoordinates, listenerCoordinates,
                // tempAngles[k], ...)`: rotated around the *listener*, using the
                // raw (pre-behind-speakers-adjustment) angle - not `base_angle`.
                let rotated_source =
                    rotate_point_to_angle(input.source, input.listener, base_angle_raw);

                let mut angle_amp_scalars = vec![0.0f32; n];
                let mut distance_amp_scalars = vec![0.0f32; n];
                let mut rolloff_decibels = vec![0.0f32; n];
                for speaker in 0..n {
                    let segment = Segment::new(rotated_source, input.speakers[speaker]);
                    let (angle, limited_diff_angle) = if speaker == crossfade_speaker {
                        (base_angle, 0.0f32)
                    } else {
                        let a = segment_angle(segment);
                        (a, (std::f32::consts::FRAC_PI_2).min((a - base_angle).abs()))
                    };
                    angle_amp_scalars[speaker] =
                        1.0 - (limited_diff_angle / std::f32::consts::FRAC_PI_2);

                    let distance = segment_length(segment);
                    let ratio = (input.minimum_reference_distance_feet / distance) as f64;
                    distance_amp_scalars[speaker] =
                        (ratio.min(1.0).powf(this_air_absorption_exponent as f64)) as f32;

                    let angle_diff = wrap_angle_to_pi_range(this_rotated_source - angle).abs();
                    rolloff_decibels[speaker] = input.source_dispersion_pattern_rolloff_decibels
                        * (angle_diff / std::f32::consts::PI);
                }
                (angle_amp_scalars, distance_amp_scalars, rolloff_decibels)
            };

        let (angle_amp0, distance_amp0, rolloff0) = branch(speaker0, temp_angles[0]);
        let (angle_amp1, distance_amp1, rolloff1) = branch(speaker1, temp_angles[1]);

        (0..n)
            .map(|speaker| {
                let threshold_proximity_scalar = if input.threshold_proximity_scalar_switch {
                    (cross_fade_proportion * angle_amp0[speaker])
                        + ((1.0 - cross_fade_proportion) * angle_amp1[speaker])
                } else {
                    1.0
                };
                threshold_proximity_scalar
                    * ((cross_fade_proportion * distance_amp0[speaker])
                        + ((1.0 - cross_fade_proportion) * distance_amp1[speaker]))
                    * ((cross_fade_proportion * db_to_amp.convert(rolloff0[speaker]))
                        + ((1.0 - cross_fade_proportion) * db_to_amp.convert(rolloff1[speaker])))
                    * source_in_front_gain
                    * input.front_source_head_room_scalar
                    * db_to_amp.convert(input.direct_sound_gain_decibels)
            })
            .collect()
    } else {
        (0..n)
            .map(|speaker| {
                let segment = Segment::new(input.source, input.speakers[speaker]);
                let angle = segment_angle(segment);
                let distance = segment_length(segment);

                let angle_diff =
                    wrap_angle_to_pi_range(input.source_to_listener_angle_plus_rotation - angle)
                        .abs();
                let rolloff_decibels = input.source_dispersion_pattern_rolloff_decibels
                    * (angle_diff / std::f32::consts::PI);

                let threshold_proximity_scalar = if input.threshold_proximity_scalar_switch {
                    let diff = (angle - input.source_to_listener_angle).abs();
                    (diff.min(std::f32::consts::FRAC_PI_2) as f64).cos() as f32
                } else {
                    1.0
                };

                let ratio = (input.minimum_reference_distance_feet / distance) as f64;
                let air_absorption =
                    (ratio.min(1.0).powf(this_air_absorption_exponent as f64)) as f32;

                threshold_proximity_scalar
                    * air_absorption
                    * source_in_front_gain
                    * input.front_source_head_room_scalar
                    * db_to_amp.convert(rolloff_decibels)
                    * db_to_amp.convert(input.direct_sound_gain_decibels)
            })
            .collect()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::units::DbToAmp;

    fn square(side: f32) -> Vec<Point> {
        vec![
            Point::new(0., 0.),
            Point::new(side, 0.),
            Point::new(side, side),
            Point::new(0., side),
        ]
    }

    #[test]
    fn segment_length_matches_hypot() {
        let seg = Segment::new(Point::new(0., 0.), Point::new(3., 4.));
        assert_eq!(segment_length(seg), 5.0);
    }

    #[test]
    fn segment_angle_matches_atan2() {
        let seg = Segment::new(Point::new(0., 0.), Point::new(1., 1.));
        assert!((segment_angle(seg) - std::f32::consts::FRAC_PI_4).abs() < 1e-6);
    }

    #[test]
    fn degrees_pi_round_trip() {
        assert!((degrees_to_pi(180.) - std::f32::consts::PI).abs() < 1e-6);
        assert!((pi_to_degrees(std::f32::consts::PI) - 180.).abs() < 1e-4);
    }

    #[test]
    fn same_point_exact_equality() {
        assert!(same_point(Point::new(1., 2.), Point::new(1., 2.)));
        assert!(!same_point(Point::new(1., 2.), Point::new(1., 2.0001)));
    }

    #[test]
    fn adjacent_segments_share_an_endpoint() {
        let s0 = Segment::new(Point::new(0., 0.), Point::new(1., 0.));
        let s1 = Segment::new(Point::new(1., 0.), Point::new(1., 1.));
        assert!(segments_share_endpoint(s0, s1));
        let s2 = Segment::new(Point::new(5., 5.), Point::new(6., 6.));
        assert!(!segments_share_endpoint(s0, s2));
    }

    #[test]
    fn crossing_segments_intersect_at_midpoint() {
        let w = Segment::new(Point::new(-1., 0.), Point::new(1., 0.));
        let p = Segment::new(Point::new(0., -1.), Point::new(0., 1.));
        let hit = segments_intersect(w, p).expect("should intersect");
        assert!((hit.x).abs() < 1e-6 && (hit.y).abs() < 1e-6);
    }

    #[test]
    fn parallel_segments_do_not_intersect() {
        let w = Segment::new(Point::new(0., 0.), Point::new(1., 0.));
        let p = Segment::new(Point::new(0., 1.), Point::new(1., 1.));
        assert!(segments_intersect(w, p).is_none());
    }

    #[test]
    fn non_overlapping_collinear_segments_report_no_intersection() {
        // The C's bounding-box test treats any parallel pair (mp == mw),
        // including collinear ones, as non-intersecting - a real quirk,
        // reproduced here (see segments_intersect's doc comment).
        let w = Segment::new(Point::new(0., 0.), Point::new(1., 0.));
        let p = Segment::new(Point::new(2., 0.), Point::new(3., 0.));
        assert!(segments_intersect(w, p).is_none());
    }

    #[test]
    fn valid_square_polygon_passes() {
        assert!(polygon_is_valid(&square(10.)).is_ok());
    }

    #[test]
    fn polygon_needs_three_vertices() {
        let two = vec![Point::new(0., 0.), Point::new(1., 1.)];
        assert!(polygon_is_valid(&two).is_err());
    }

    #[test]
    fn polygon_with_duplicate_vertex_fails() {
        let mut p = square(10.);
        p[2] = p[0];
        assert!(polygon_is_valid(&p).is_err());
    }

    #[test]
    fn self_intersecting_bowtie_fails() {
        // Crossed quadrilateral: 0-1-2-3 with sides (0,1)-(2,3) and (1,2)-(3,0)
        // arranged to cross.
        let bowtie = vec![
            Point::new(0., 0.),
            Point::new(10., 10.),
            Point::new(10., 0.),
            Point::new(0., 10.),
        ];
        assert!(polygon_is_valid(&bowtie).is_err());
    }

    #[test]
    fn square_is_convex() {
        assert!(!polygon_is_concave(&square(10.)));
    }

    #[test]
    fn l_shape_is_concave() {
        let l_shape = vec![
            Point::new(0., 0.),
            Point::new(10., 0.),
            Point::new(10., 5.),
            Point::new(5., 5.),
            Point::new(5., 10.),
            Point::new(0., 10.),
        ];
        assert!(polygon_is_concave(&l_shape));
    }

    #[test]
    fn point_clearly_inside_square_is_inside() {
        let sq = square(10.);
        assert!(point_in_polygon(&sq, Point::new(5., 5.)));
    }

    #[test]
    fn point_clearly_outside_square_is_outside() {
        let sq = square(10.);
        assert!(!point_in_polygon(&sq, Point::new(50., 50.)));
    }

    #[test]
    fn make_cornered_space_produces_requested_corner_count() {
        // Fixed sequence RNG: deterministic constant midpoint, so the
        // polygon is well-formed and reproducible for the test.
        let uniform = move |lo: f32, hi: f32| lo + (hi - lo) * 0.5;
        let room = make_cornered_space(6, (20., 40.), 1.0, 0., uniform);
        assert_eq!(room.len(), 6);
        // Fully regular (regularity_proportion = 1.0) with a constant
        // radius should form a regular hexagon - opposite corners should
        // be equidistant from the origin.
        for p in &room {
            let dist = segment_length(Segment::new(Point::ORIGIN, *p));
            assert!((dist - 30.0).abs() < 1e-3, "distance {dist} should be ~30");
        }
    }

    #[test]
    fn cut_data_lines_strips_brace_comments() {
        let text = "1.0 2.0 {this is a comment} 3.0 4.0";
        let cut = cut_data_lines(text, 2).unwrap();
        let points = parse_coordinate_pairs(&cut).unwrap();
        assert_eq!(points, vec![Point::new(1.0, 2.0), Point::new(3.0, 4.0)]);
    }

    #[test]
    fn cut_data_lines_mismatched_brace_errors() {
        assert!(cut_data_lines("1.0 { 2.0", 2).is_err());
        assert!(cut_data_lines("1.0 } 2.0", 2).is_err());
    }

    #[test]
    fn cut_data_lines_mute_mode_drops_marked_record() {
        let text = "1.0 2.0 m 3.0 4.0 5.0 6.0";
        let cut = cut_data_lines(text, 2).unwrap();
        let points = parse_coordinate_pairs(&cut).unwrap();
        assert_eq!(points, vec![Point::new(1.0, 2.0), Point::new(5.0, 6.0)]);
    }

    #[test]
    fn cut_data_lines_solo_mode_keeps_only_marked_record() {
        let text = "1.0 2.0 ! 3.0 4.0 5.0 6.0";
        let cut = cut_data_lines(text, 2).unwrap();
        let points = parse_coordinate_pairs(&cut).unwrap();
        assert_eq!(points, vec![Point::new(3.0, 4.0)]);
    }

    #[test]
    fn parse_coordinate_pairs_rejects_odd_count() {
        assert!(parse_coordinate_pairs("1.0 2.0 3.0").is_err());
    }

    #[test]
    fn polar_to_cartesian_converts_correctly() {
        let polar = vec![Point::new(0., 10.), Point::new(90., 10.)];
        let cart = polar_to_cartesian(&polar);
        assert!((cart[0].x - 10.).abs() < 1e-4 && cart[0].y.abs() < 1e-4);
        assert!(cart[1].x.abs() < 1e-4 && (cart[1].y - 10.).abs() < 1e-4);
    }

    #[test]
    fn resolve_single_point_defaults_to_origin() {
        assert_eq!(resolve_single_point(None).unwrap(), Point::ORIGIN);
    }

    #[test]
    fn resolve_single_point_rejects_multiple_pairs() {
        // Real C behavior (finding 1): more than one pair is a hard error,
        // despite usage() claiming multi-position looping is supported.
        assert!(resolve_single_point(Some("1.0 2.0 3.0 4.0")).is_err());
    }

    #[test]
    fn resolve_single_point_reads_one_pair() {
        assert_eq!(
            resolve_single_point(Some("1.5 2.5")).unwrap(),
            Point::new(1.5, 2.5)
        );
    }

    #[test]
    fn resolve_speaker_points_allows_multiple_pairs() {
        let speakers = resolve_speaker_points(Some("0 0 10 0 10 10")).unwrap();
        assert_eq!(speakers.len(), 3);
    }

    #[test]
    fn resolve_speaker_points_defaults_to_single_origin() {
        assert_eq!(resolve_speaker_points(None).unwrap(), vec![Point::ORIGIN]);
    }

    #[test]
    fn coordinate_transform_identity_preserves_points() {
        let t = CoordinateTransform::default();
        let points = vec![Point::new(3., 4.), Point::new(-1., -2.)];
        let out = apply_coordinate_transform(&points, &t);
        assert_eq!(out[0].0, points[0]);
        assert_eq!(out[1].0, points[1]);
    }

    #[test]
    fn coordinate_transform_translates() {
        let t = CoordinateTransform {
            x_translation: 5.,
            y_translation: -2.,
            ..Default::default()
        };
        let out = apply_coordinate_transform(&[Point::new(0., 0.)], &t);
        assert_eq!(out[0].0, Point::new(5., -2.));
    }

    #[test]
    fn coordinate_transform_rotates_90_degrees() {
        let t = CoordinateTransform {
            rotation_degrees: 90.,
            ..Default::default()
        };
        let out = apply_coordinate_transform(&[Point::new(1., 0.)], &t);
        assert!((out[0].0.x).abs() < 1e-5);
        assert!((out[0].0.y - 1.).abs() < 1e-5);
    }

    #[test]
    fn coordinate_transform_quadrant_scale_applies_by_sign() {
        let t = CoordinateTransform {
            neg_x_scale: 2.,
            pos_x_scale: 3.,
            ..Default::default()
        };
        let out = apply_coordinate_transform(&[Point::new(-1., 0.), Point::new(1., 0.)], &t);
        assert_eq!(out[0].0.x, -2.);
        assert_eq!(out[1].0.x, 3.);
    }

    #[test]
    fn listener_to_speaker_angles_matches_segment_angle() {
        let listener = Point::ORIGIN;
        let speakers = vec![Point::new(1., 0.), Point::new(0., 1.)];
        let angles = listener_to_speaker_angles(listener, &speakers);
        assert!((angles[0] - 0.0).abs() < 1e-6);
        assert!((angles[1] - std::f32::consts::FRAC_PI_2).abs() < 1e-6);
    }

    #[test]
    fn angles_in_order_passes_for_two_or_fewer() {
        assert!(angles_in_order(&[0.1]));
        assert!(angles_in_order(&[0.1, 5.0]));
    }

    #[test]
    fn angles_in_order_detects_consistent_ascending_order() {
        assert!(angles_in_order(&[0.0, 1.0, 2.0, 3.0]));
    }

    #[test]
    fn mirror_point_across_horizontal_line() {
        let (x, y) = mirror_point_f64((2.0, 3.0), (0.0, 0.0, 5.0, 0.0));
        assert!((x - 2.0).abs() < 1e-9 && (y - -3.0).abs() < 1e-9);
    }

    #[test]
    fn mirror_point_across_vertical_line() {
        let (x, y) = mirror_point_f64((2.0, 3.0), (0.0, 0.0, 0.0, 5.0));
        assert!((x - -2.0).abs() < 1e-9 && (y - 3.0).abs() < 1e-9);
    }

    #[test]
    fn mirror_point_across_diagonal_line() {
        // Reflect (2, 3) across y = x: swaps to (3, 2).
        let (x, y) = mirror_point_f64((2.0, 3.0), (0.0, 0.0, 5.0, 5.0));
        assert!((x - 3.0).abs() < 1e-9 && (y - 2.0).abs() < 1e-9);
    }

    #[test]
    fn mirror_point_f32_wrapper_matches_f64() {
        let p = mirror_point(
            Point::new(2., 3.),
            Segment::new(Point::ORIGIN, Point::new(5., 0.)),
        );
        assert!((p.x - 2.0).abs() < 1e-5 && (p.y - -3.0).abs() < 1e-5);
    }

    #[test]
    fn point_to_line_position_opposite_sides_have_opposite_sign() {
        let a = Point::new(0., 0.);
        let b = Point::new(1., 0.);
        let above = point_to_line_position(a, b, Point::new(0., 1.));
        let below = point_to_line_position(a, b, Point::new(0., -1.));
        assert_eq!(above, -below);
    }

    #[test]
    fn reflex_vertex_flags_all_zero_for_convex_room() {
        let flags = polygon_reflex_vertex_flags(&square(10.), false);
        assert_eq!(flags, vec![0, 0, 0, 0]);
    }

    #[test]
    fn reflex_vertex_flags_finds_the_one_reflex_corner_of_an_l_shape() {
        // Same L-shape as `l_shape_is_concave` above; (5, 5) at index 3 is
        // the one interior (reflex) corner - hand-verified via the cross
        // products `pointToLinePosition` computes (see this test's own
        // commit for the by-hand derivation).
        let l_shape = vec![
            Point::new(0., 0.),
            Point::new(10., 0.),
            Point::new(10., 5.),
            Point::new(5., 5.),
            Point::new(5., 10.),
            Point::new(0., 10.),
        ];
        let flags = polygon_reflex_vertex_flags(&l_shape, true);
        assert_eq!(flags, vec![1, 1, 1, -1, 1, 1]);
    }

    #[test]
    fn angle_window_order_one_always_passes() {
        let (passes, sorted) = mirror_segment_angle_window_test((0.2, 0.1), None);
        assert!(passes);
        assert_eq!(sorted, (0.1, 0.2));
    }

    #[test]
    fn angle_window_rejects_candidate_entirely_outside_window() {
        let (passes, _) = mirror_segment_angle_window_test((2.0, 2.5), Some((0.0, 1.0)));
        assert!(!passes);
    }

    #[test]
    fn angle_window_accepts_candidate_endpoint_inside_window() {
        let (passes, _) = mirror_segment_angle_window_test((0.5, 0.6), Some((0.0, 1.0)));
        assert!(passes);
    }

    #[test]
    fn angle_window_accepts_when_window_is_swallowed_by_candidate_span() {
        let (passes, _) = mirror_segment_angle_window_test((0.1, 0.9), Some((0.4, 0.6)));
        assert!(passes);
    }

    #[test]
    fn narrow_angle_window_order_one_pads_its_own_sorted_angles() {
        let (low, high) = narrow_mirror_segment_angle_window(1, (0.2, 0.8), None);
        assert!((low - (0.2 - std::f32::consts::PI / 100.0)).abs() < 1e-6);
        assert!((high - (0.8 + std::f32::consts::PI / 100.0)).abs() < 1e-6);
    }

    #[test]
    fn narrow_angle_window_intersects_with_parent() {
        let (low, high) = narrow_mirror_segment_angle_window(2, (0.3, 0.5), Some((0.1, 0.6)));
        assert!((low - (0.3 - std::f32::consts::PI / 100.0)).abs() < 1e-6);
        assert!((high - (0.5 + std::f32::consts::PI / 100.0)).abs() < 1e-6);
    }

    #[test]
    fn find_reflection_paths_finds_expected_first_order_left_wall_reflection() {
        // 10x10 square room, walls in order bottom/right/top/left (index 3
        // is the left wall, x=0). Source above, speaker below, both on the
        // vertical center line - the left-wall image-source reflection is
        // hand-computable: mirroring source (5, 8) across x=0 gives image
        // (-5, 8); the segment from speaker (5, 2) to that image crosses
        // x=0 at y=5 (within the wall's [0, 10] span), at distance
        // sqrt(10^2 + 6^2) = sqrt(136).
        let room = square(10.);
        let listener = Point::new(5., 2.); // == speaker, so the proximity
                                           // exclusion test never rejects (see the test's own doc comment above).
        let input = RoomAcousticsInput {
            room: &room,
            polygon_is_concave: false,
            reflex_flags: &[0, 0, 0, 0],
            listener,
            speaker: Point::new(5., 2.),
            source: Point::new(5., 8.),
            high_order_limit: 1,
            exclude_listener_space_cross_reflections: true,
            speed_of_sound_feet_per_second: 1125.,
        };

        let (paths, stats) = find_reflection_paths(&input);

        let left_wall = paths
            .iter()
            .find(|p| p.mirror_wall_sequence == vec![3])
            .expect("left-wall (index 3) reflection should be found");
        assert_eq!(left_wall.order, 1);
        assert!((left_wall.distance - 136f32.sqrt()).abs() < 1e-3);
        assert!((left_wall.time_seconds - (136f32.sqrt() / 1125.)).abs() < 1e-6);

        assert_eq!(paths.len() as u32, stats.viable_count);
        assert_eq!(stats.viable_count_by_order.len(), 1);
        assert_eq!(stats.viable_count_by_order[0], stats.viable_count);
        assert!(stats.total_examined >= stats.viable_count);
    }

    #[test]
    fn find_reflection_paths_respects_high_order_limit() {
        let room = square(10.);
        let input = RoomAcousticsInput {
            room: &room,
            polygon_is_concave: false,
            reflex_flags: &[0, 0, 0, 0],
            listener: Point::new(5., 2.),
            speaker: Point::new(5., 2.),
            source: Point::new(5., 8.),
            high_order_limit: 2,
            exclude_listener_space_cross_reflections: true,
            speed_of_sound_feet_per_second: 1125.,
        };

        let (paths, stats) = find_reflection_paths(&input);
        assert!(paths.iter().all(|p| p.order <= 2));
        assert_eq!(stats.viable_count_by_order.len(), 2);
        assert!(
            paths.iter().any(|p| p.order == 2),
            "expected at least one second-order reflection"
        );
    }

    #[test]
    fn front_source_head_room_scalar_is_unity_within_reference_distance() {
        assert_eq!(front_source_head_room_scalar(10., 9., 1., 2.), 1.0);
        assert_eq!(front_source_head_room_scalar(10., 9., 5., 2.), 1.0);
    }

    #[test]
    fn front_source_head_room_scalar_rolls_off_past_reference_distance() {
        // limit = 10 - 0 = 10, ratio = 1/10, ^2 = 0.01
        let got = front_source_head_room_scalar(10., 0., 1., 2.);
        assert!((got - 0.01).abs() < 1e-6, "got {got}");
    }

    #[test]
    fn pre_echo_time_seconds_picks_the_larger_delay() {
        assert_eq!(pre_echo_time_seconds(0.05, 0.08), 0.08);
        assert_eq!(pre_echo_time_seconds(0.08, 0.05), 0.08);
    }

    #[test]
    fn source_orientation_angle_difference_is_zero_when_facing_the_reflection() {
        // Source at origin facing due east (angle 0); the reflection path's
        // first intersection is also due east - no divergence.
        let diff = source_orientation_to_reflection_angle_difference(
            0.0,
            Point::ORIGIN,
            Point::new(10., 0.),
        );
        assert!(diff.abs() < 1e-6, "diff {diff}");
    }

    #[test]
    fn source_orientation_angle_difference_wraps_to_at_most_pi() {
        // Source facing due west (PI); intersection due east (angle 0) -
        // the raw difference is PI either way, never more.
        let diff = source_orientation_to_reflection_angle_difference(
            std::f32::consts::PI,
            Point::ORIGIN,
            Point::new(10., 0.),
        );
        assert!((diff - std::f32::consts::PI).abs() < 1e-5, "diff {diff}");
    }

    #[test]
    fn dispersion_pattern_amplitude_is_unity_when_facing_the_reflection() {
        assert_eq!(dispersion_pattern_amplitude(0.0, -20.0), 1.0);
    }

    #[test]
    fn dispersion_pattern_amplitude_matches_exact_formula_not_the_table() {
        // Finding 12: this one term uses pow(10, dB/20) directly, not the
        // DbToAmp lookup table - at full rolloff (angle_difference == PI,
        // proportion 1.0) the dB value passed through is exactly
        // `source_dispersion_pattern_rolloff_decibels` itself.
        let got = dispersion_pattern_amplitude(std::f32::consts::PI, -6.0);
        let want = 10.0f64.powf(-6.0 / 20.0) as f32;
        assert!((got - want).abs() < 1e-6, "got {got}, want {want}");
    }

    #[test]
    fn air_absorption_multiplier_is_unity_within_reference_distance() {
        assert_eq!(air_absorption_multiplier(1.0, 1.0, 2.0), 1.0);
        assert_eq!(air_absorption_multiplier(0.5, 1.0, 2.0), 1.0);
    }

    #[test]
    fn air_absorption_multiplier_rolls_off_past_reference_distance() {
        // ratio = 1/10, ^2 = 0.01
        let got = air_absorption_multiplier(10.0, 1.0, 2.0);
        assert!((got - 0.01).abs() < 1e-6, "got {got}");
    }

    #[test]
    fn wall_gainscale_default_mode_uses_every_wall_not_just_the_last() {
        // Finding 11: mode -1 (the tool's own default, documented "last
        // wall") actually falls through to using the WHOLE sequence.
        let db_to_amp = DbToAmp::new();
        let sequence = [0usize, 1, 2];
        let levels = [-6.0f32];
        let got = wall_gainscale_amplitude(&sequence, -1, &levels, &db_to_amp);
        let want = db_to_amp.convert(-6.0).powi(3);
        assert!((got - want).abs() < 1e-5, "got {got}, want {want}");
    }

    #[test]
    fn wall_gainscale_positive_mode_uses_only_the_first_n_walls() {
        let db_to_amp = DbToAmp::new();
        let sequence = [0usize, 1, 2];
        let levels = [-6.0f32, 0.0, 0.0];
        // mode 1: only wall index 0 (level -6dB) contributes.
        let got = wall_gainscale_amplitude(&sequence, 1, &levels, &db_to_amp);
        let want = db_to_amp.convert(-6.0);
        assert!((got - want).abs() < 1e-6, "got {got}, want {want}");
    }

    #[test]
    fn wall_gainscale_mode_past_sequence_length_clamps_instead_of_panicking() {
        let db_to_amp = DbToAmp::new();
        let sequence = [0usize, 1];
        let levels = [0.0f32, 0.0];
        let got = wall_gainscale_amplitude(&sequence, 10, &levels, &db_to_amp);
        let want = db_to_amp.convert(0.0).powi(2);
        assert!((got - want).abs() < 1e-5, "got {got}, want {want}");
    }

    #[test]
    fn reflection_order_gainscale_clamps_to_table_length() {
        let db_to_amp = DbToAmp::new();
        let levels = [-3.0f32, -6.0];
        // order 5 exceeds the table's own length (2) - clamps to the last entry.
        let got = reflection_order_gainscale_amplitude(5, &levels, &db_to_amp);
        let want = db_to_amp.convert(-6.0);
        assert_eq!(got, want);
    }

    #[test]
    fn reflection_delay_sample_index_rounds_half_up() {
        // 1.0 + (0.01 * 1.0 * 100.0) = 2.0 exactly -> +0.5 -> 2 (truncated)
        assert_eq!(reflection_delay_sample_index(1.0, 0.01, 1.0, 100.0), 2);
        // 0.004 * 1.0 * 100.0 = 0.4 -> +0.5 = 0.9 -> truncates to 0
        assert_eq!(reflection_delay_sample_index(0.0, 0.004, 1.0, 100.0), 0);
        // 0.006 * 1.0 * 100.0 = 0.6 -> +0.5 = 1.1 -> truncates to 1
        assert_eq!(reflection_delay_sample_index(0.0, 0.006, 1.0, 100.0), 1);
    }

    #[test]
    fn reflection_pulse_inclusion_threshold_boundary() {
        let db_to_amp = DbToAmp::new();
        let threshold_amp = db_to_amp.convert(-96.0);
        assert!(reflection_pulse_passes_inclusion_threshold(
            threshold_amp,
            -96.0,
            &db_to_amp
        ));
        assert!(!reflection_pulse_passes_inclusion_threshold(
            threshold_amp * 0.5,
            -96.0,
            &db_to_amp
        ));
    }

    #[test]
    fn reflection_pulse_gain_combines_every_term_in_order() {
        let db_to_amp = DbToAmp::new();
        let sequence = [0usize];
        let input = ReflectionPulseGainInput {
            angle_difference: 0.0, // dispersion amplitude forced to 1.0
            source_dispersion_pattern_rolloff_decibels: -20.0,
            reflection_distance: 1.0,
            minimum_reference_distance_feet: 1.0, // air absorption forced to 1.0
            air_absorption_exponent_for_reflections: 2.0,
            wall_reflection_sequence: &sequence,
            wall_impulse_and_gainscale_response_mode: -1,
            wall_decibel_gainscale_levels: &[0.0],
            reflection_order_decibel_gainscale_levels: &[0.0],
            reflected_sound_gain_decibels: 0.0,
            front_source_head_room_scalar: 1.0,
        };
        let got = reflection_pulse_gain(&input, &db_to_amp);
        // Every 0dB term still passes through DbToAmp's own table
        // approximation (0.997791529, not exactly 1.0 - see units.rs) three
        // times: wall gainscale, reflection-order gainscale, reflected-
        // sound gain.
        let want = db_to_amp.convert(0.0).powi(3);
        assert!((got - want).abs() < 1e-5, "got {got}, want {want}");
    }

    // ---- Phase 4 ----

    #[test]
    fn convolve_two_arrays_matches_hand_computed_two_tap_kernel_scaled_by_1_over_n() {
        // A 2-tap kernel [1, 0.5] convolved with an impulse train: linear
        // FIR filtering, hand-verifiable up to convolve_two_arrays' own
        // 1/N scale factor (see the function's doc comment / finding 13) -
        // here Lh0=2, L0=3, N=next_power_of_two(3)=4.
        let kernel = [1.0f32, 0.5];
        let signal = [1.0f32, 0.0, 0.0, 1.0, 0.0, 0.0];
        let out = convolve_two_arrays(&kernel, &signal);
        assert_eq!(out.len(), kernel.len() + signal.len() - 1);
        let true_convolution = [1.0, 0.5, 0.0, 1.0, 0.5, 0.0, 0.0];
        let n = 4.0f32;
        for (i, (&got, &true_v)) in out.iter().zip(true_convolution.iter()).enumerate() {
            assert!(
                (got - true_v / n).abs() < 1e-4,
                "index {i}: got {got}, want {}",
                true_v / n
            );
        }
    }

    #[test]
    fn convolve_two_arrays_drops_and_misplaces_tail_samples_on_an_under_full_final_block() {
        // Finding 14: a = [1,2,3] (Lh0=3), b = [0.5,-1,0.25,2,1] (Lh1=5).
        // Block size is Lh0=3, so block 0 reads 3 real samples (full) and
        // block 1 reads only 2 (5 - 3): an under-full final block. The
        // true linear convolution (hand-computed) is:
        //   [0.5, 0, -0.25, -0.5, 5.75, 8, 3]
        // but true[5] (= 8) is never written anywhere - it falls in the
        // gap between the under-full block's own sampsToRead(2) and the
        // fixed carry-capture offset Lh0(3) - and the final flush then
        // writes the *next* value (true[6] = 3) one slot too early, at
        // global position 5 instead of 6, leaving position 6 as 0.
        let a = [1.0f32, 2.0, 3.0];
        let b = [0.5f32, -1.0, 0.25, 2.0, 1.0];
        let out = convolve_two_arrays(&a, &b);
        let n = (2 * a.len() - 1).next_power_of_two() as f32; // Lh0=3 -> L0=5 -> N=8

        let true_convolution = [0.5, 0.0, -0.25, -0.5, 5.75, 8.0, 3.0];
        let scaled: Vec<f32> = out.iter().map(|&v| v * n).collect();

        // Positions 0-4 (untouched by the under-full block's own carry
        // boundary) match the true convolution exactly.
        for i in 0..5 {
            assert!(
                (scaled[i] - true_convolution[i]).abs() < 1e-3,
                "index {i}: got {}, want {}",
                scaled[i],
                true_convolution[i]
            );
        }
        // Position 5 should be true_convolution[5] (8.0) but instead holds
        // true_convolution[6] (3.0) - shifted one slot early.
        assert!((scaled[5] - 3.0).abs() < 1e-3, "got {}", scaled[5]);
        // Position 6 should be true_convolution[6] (3.0) but is left at 0
        // - the true value 8.0 is dropped entirely, never written.
        assert!(scaled[6].abs() < 1e-3, "got {}", scaled[6]);
    }

    #[test]
    fn crop_end_for_silence_trims_trailing_below_threshold_samples() {
        let array = [1.0, 1.0, 0.5, 0.01, 0.0, 0.0];
        assert_eq!(crop_end_for_silence(&array, 0.1), 3);
    }

    #[test]
    fn crop_end_for_silence_keeps_full_length_when_last_sample_is_loud() {
        let array = [0.0, 0.0, 1.0];
        assert_eq!(crop_end_for_silence(&array, 0.1), 3);
    }

    #[test]
    fn crop_end_for_silence_empty_array_returns_zero() {
        assert_eq!(crop_end_for_silence(&[], 0.1), 0);
    }

    #[test]
    fn crop_ir_data_end_for_silence_adds_release_tail_with_no_plus_one() {
        // Last loud sample is index 1 (value 1.0); with 0 release time the
        // C returns the raw last-loud index, not index+1.
        let array = [0.0, 1.0, 0.0, 0.0, 0.0];
        assert_eq!(crop_ir_data_end_for_silence(&array, 0.5, 10.0, 0.0), 1);
    }

    #[test]
    fn crop_ir_data_end_for_silence_adds_sample_rate_scaled_release_time() {
        let array = [0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0];
        // last loud index 1, + (10 * 0.2) = 1 + 2 = 3
        assert_eq!(crop_ir_data_end_for_silence(&array, 0.5, 10.0, 0.2), 3);
    }

    #[test]
    fn crop_ir_data_end_for_silence_clamps_to_array_length_only_on_the_high_side() {
        let array = [0.0, 1.0, 0.0];
        assert_eq!(crop_ir_data_end_for_silence(&array, 0.5, 1000.0, 1.0), 3);
    }

    #[test]
    fn find_peak_amp_finds_largest_absolute_value() {
        assert_eq!(find_peak_amp(&[0.1, -5.0, 3.0, -2.0]), 5.0);
        assert_eq!(find_peak_amp(&[]), 0.0);
    }

    #[test]
    fn filter_fft_zeroes_dc_and_nyquist_together_below_low_freq() {
        // Finding: k == 0 shares slots [0]/[1] with DC and Nyquist in the
        // rfft packing, so a low-frequency rolloff with low_freq > 0 zeros
        // both, regardless of high_freq.
        let db_to_amp = crate::units::DbToAmp::new();
        let mut fft_array = vec![1.0f32; 8]; // N = 8, N/2 = 4 bins
        let rolloff = BandpassRolloff {
            low_freq: 50.0,
            high_freq: 1_000_000.0,
            low_rolloff_db_per_octave: -96.0,
            high_rolloff_db_per_octave: -96.0,
            compound_levels: 1,
        };
        filter_fft(&mut fft_array, 8, 100.0, &rolloff, &db_to_amp);
        assert_eq!(fft_array[0], 0.0, "DC should be zeroed (below low_freq)");
        assert_eq!(
            fft_array[1], 0.0,
            "Nyquist shares slot [1] with DC in this packing, so it is zeroed too, \
             even though high_freq is effectively infinite"
        );
    }

    #[test]
    fn filter_fft_leaves_in_band_bins_untouched() {
        let db_to_amp = crate::units::DbToAmp::new();
        let mut fft_array = vec![2.0f32; 8];
        let rolloff = BandpassRolloff {
            low_freq: 50.0,
            high_freq: 1_000_000.0,
            low_rolloff_db_per_octave: -96.0,
            high_rolloff_db_per_octave: -96.0,
            compound_levels: 1,
        };
        filter_fft(&mut fft_array, 8, 100.0, &rolloff, &db_to_amp);
        // bin 1 (fundamental=100Hz => bin1=100Hz) is >= low_freq(50) and <= high_freq: untouched.
        assert_eq!(fft_array[2], 2.0);
        assert_eq!(fft_array[3], 2.0);
    }

    #[test]
    fn filter_audio_array_returns_full_padded_length_not_original() {
        let db_to_amp = crate::units::DbToAmp::new();
        let signal = [1.0f32, 0.5, -0.5, 0.25, -0.25];
        let rolloff = BandpassRolloff {
            low_freq: 0.0,
            high_freq: 1_000_000.0,
            low_rolloff_db_per_octave: 0.0,
            high_rolloff_db_per_octave: 0.0,
            compound_levels: 1,
        };
        let out = filter_audio_array(&signal, 100.0, &rolloff, &db_to_amp);
        // L = 2*5-1 = 9, N = next_power_of_two(9) = 16.
        assert_eq!(out.len(), 16);
    }

    #[test]
    fn smooth_release_of_cropped_end_fades_tail_to_zero() {
        let mut array = vec![1.0f32; 100];
        // release_time_seconds / (100/1000 = 0.1s duration) = 0.5 -> capped
        // at 0.33 -> numSamples = round(100*0.33) = 33.
        smooth_release_of_cropped_end(&mut array, 0.05, 1000.0);
        assert!(
            (array[array.len() - 1]).abs() < 1e-4,
            "last sample should fade near 0"
        );
        assert_eq!(array[0], 1.0, "untouched head sample stays at 1.0");
    }

    #[test]
    fn smooth_release_of_cropped_end_single_sample_fade_produces_nan() {
        // A tiny release time on a short array can round numSamples down
        // to exactly 1, at which point the C's own i/(numSamples-1) is
        // 0./0. = NaN - faithfully reproduced, not special-cased away.
        let mut array = vec![1.0f32; 4];
        smooth_release_of_cropped_end(&mut array, 0.001, 1000.0);
        assert!(array[array.len() - 1].is_nan());
    }

    #[test]
    fn wall_sequence_for_reflection_mode_positive_keeps_first_walls_from_source() {
        // last-to-first: index 0 = last bounce (near speaker) = wall 5,
        // index 3 = first bounce (near source) = wall 2.
        let seq = [5, 4, 3, 2];
        let out = wall_sequence_for_reflection(&seq, 2, 100);
        // mode=2: first 2 walls from the source side, in first-to-last order.
        assert_eq!(out, vec![2, 3]);
    }

    #[test]
    fn wall_sequence_for_reflection_mode_negative_keeps_last_walls_near_speaker() {
        let seq = [5, 4, 3, 2];
        let out = wall_sequence_for_reflection(&seq, -2, 100);
        // mode=-2: last 2 walls near the speaker, in first-to-last order.
        assert_eq!(out, vec![4, 5]);
    }

    #[test]
    fn wall_sequence_for_reflection_mode_zero_keeps_all_walls_reversed() {
        let seq = [5, 4, 3, 2];
        let out = wall_sequence_for_reflection(&seq, 0, 100);
        assert_eq!(out, vec![2, 3, 4, 5]);
    }

    #[test]
    fn wall_sequence_for_reflection_applies_channel_modulo() {
        let seq = [7, 3];
        let out = wall_sequence_for_reflection(&seq, 0, 4);
        assert_eq!(out, vec![3, 3]); // 3 % 4 == 3, 7 % 4 == 3
    }

    #[test]
    fn compare_wall_sequences_orders_by_first_difference() {
        assert_eq!(
            compare_wall_sequences(&[1, 2], &[1, 3], 2),
            std::cmp::Ordering::Less
        );
        assert_eq!(
            compare_wall_sequences(&[1, 5], &[1, 3], 2),
            std::cmp::Ordering::Greater
        );
        assert_eq!(
            compare_wall_sequences(&[1, 2], &[1, 2], 2),
            std::cmp::Ordering::Equal
        );
    }

    #[test]
    fn compare_wall_sequences_treats_missing_tail_as_zero() {
        // seq0 is shorter; positions past its length compare as 0, so
        // every position through compare_length ties - the final
        // length-based tie-break then makes the shorter one Less.
        assert_eq!(
            compare_wall_sequences(&[1], &[1, 0, 0], 3),
            std::cmp::Ordering::Less
        );
        assert_eq!(
            compare_wall_sequences(&[1], &[1, 1], 3),
            std::cmp::Ordering::Less
        );
    }

    #[test]
    fn compare_wall_sequences_falls_back_to_length_when_fully_equal() {
        assert_eq!(
            compare_wall_sequences(&[1, 2], &[1, 2, 3], 2),
            std::cmp::Ordering::Less
        );
    }

    fn make_reflection(
        order: usize,
        mirror_wall_sequence: Vec<usize>,
        time_seconds: f32,
    ) -> ReflectionPath {
        ReflectionPath {
            order,
            distance: 0.0,
            time_seconds,
            mirror_wall_sequence,
            image_source: Point::ORIGIN,
            source_to_first_mirror_segment_intersection: Point::ORIGIN,
        }
    }

    #[test]
    fn select_and_reorder_wall_reflection_sequences_filters_by_order_and_sorts() {
        let reflections = vec![
            make_reflection(1, vec![3], 0.01),
            make_reflection(2, vec![1, 0], 0.02),
            make_reflection(3, vec![9, 9, 9], 0.03), // out of order-range, excluded
            make_reflection(1, vec![1], 0.04),
        ];
        let selected = select_and_reorder_wall_reflection_sequences(&reflections, 0, 100, 1, 2);
        // Order-3 reflection (index 2) is excluded; remaining sorted by
        // wall_sequence_for_reflection's own output.
        assert_eq!(selected.len(), 3);
        assert!(selected.iter().all(|(i, _)| *i != 2));
        // Sorted ascending by sequence content.
        let seqs: Vec<&Vec<usize>> = selected.iter().map(|(_, s)| s).collect();
        for w in seqs.windows(2) {
            assert_ne!(
                compare_wall_sequences(w[0], w[1], 2),
                std::cmp::Ordering::Greater
            );
        }
    }

    #[test]
    fn average_reflection_order_delay_times_averages_per_order() {
        let reflections = vec![
            make_reflection(1, vec![0], 0.10),
            make_reflection(1, vec![0], 0.20),
            make_reflection(2, vec![0, 0], 0.50),
        ];
        let averages = average_reflection_order_delay_times(&reflections, 2);
        assert!((averages[0] - 0.15).abs() < 1e-6, "order 1 average");
        assert!((averages[1] - 0.50).abs() < 1e-6, "order 2 average");
    }

    #[test]
    fn average_reflection_order_delay_times_nan_for_order_with_no_reflections() {
        let reflections = vec![make_reflection(1, vec![0], 0.10)];
        let averages = average_reflection_order_delay_times(&reflections, 2);
        assert!(
            averages[1].is_nan(),
            "order 2 has no reflections: 0./0. = NaN, matching the C"
        );
    }

    // -------------------------------------------------------------
    // Phase 5: direct-sound pulse path
    // -------------------------------------------------------------

    #[test]
    fn listener_to_speaker_angle_bounds_stereo_pair_straddles() {
        // Two speakers whose angles are close to +/-PI - more than PI apart,
        // so the stereo-specific straddle test (finding: only fires for
        // exactly 2 speakers) should flag index 0.
        let angles = [3.0, -3.0];
        let bounds = listener_to_speaker_angle_bounds(&angles, false);
        assert_eq!(bounds.minimum_index, 1);
        assert_eq!(bounds.maximum_index, 0);
        assert_eq!(bounds.straddle_flags, vec![true, false]);
    }

    #[test]
    fn listener_to_speaker_angle_bounds_stereo_pair_no_straddle() {
        let angles = [-0.5, 0.5];
        let bounds = listener_to_speaker_angle_bounds(&angles, false);
        assert_eq!(bounds.straddle_flags, vec![false, false]);
    }

    #[test]
    fn listener_to_speaker_angle_bounds_fan_straddles_between_min_and_max() {
        // Four speakers fanned from -2.0 to 2.0 radians in increasing order:
        // min is index 0, max is index 3, and they're adjacent only via the
        // wrap segment 3->0 - which sequence mode excludes (n - 1 = 3
        // segments scanned, 0..3) so nothing straddles; polygon mode
        // includes segment 3->0 and should flag it.
        let angles = [-2.0, -0.6, 0.6, 2.0];
        let sequence = listener_to_speaker_angle_bounds(&angles, false);
        assert!(sequence.straddle_flags.iter().all(|&f| !f));

        let polygon = listener_to_speaker_angle_bounds(&angles, true);
        assert_eq!(polygon.straddle_flags, vec![false, false, false, true]);
    }

    #[test]
    fn value_is_between_these_two_works_either_order() {
        assert!(value_is_between_these_two(0.5, 0.0, 1.0));
        assert!(value_is_between_these_two(0.5, 1.0, 0.0));
        assert!(!value_is_between_these_two(1.5, 0.0, 1.0));
    }

    #[test]
    fn find_crossfade_speaker_pair_finds_the_bracketing_pair() {
        let angles = [-1.0, 0.0, 1.0];
        let bounds = listener_to_speaker_angle_bounds(&angles, false);
        let pair = find_crossfade_speaker_pair(0.5, &angles, &bounds, false);
        assert_eq!(pair, Some((1, 2)));
    }

    #[test]
    fn find_crossfade_speaker_pair_none_outside_sequence_range() {
        let angles = [-1.0, 0.0, 1.0];
        let bounds = listener_to_speaker_angle_bounds(&angles, false);
        // Sequence mode never wraps past the last speaker.
        let pair = find_crossfade_speaker_pair(-2.5, &angles, &bounds, false);
        assert_eq!(pair, None);
    }

    #[test]
    fn rotate_point_to_angle_preserves_distance() {
        let origin = Point::new(0., 0.);
        let point = Point::new(5., 0.);
        let rotated = rotate_point_to_angle(point, origin, std::f32::consts::FRAC_PI_2);
        assert!(
            (rotated.x).abs() < 1e-4,
            "x should be ~0, got {}",
            rotated.x
        );
        assert!(
            (rotated.y - 5.0).abs() < 1e-4,
            "y should be ~5, got {}",
            rotated.y
        );
    }

    #[test]
    fn rotate_point_to_angle_around_nonzero_origin() {
        let origin = Point::new(10., 10.);
        let point = Point::new(13., 10.); // distance 3 from origin
        let rotated = rotate_point_to_angle(point, origin, 0.0);
        assert!((rotated.x - 13.0).abs() < 1e-4);
        assert!((rotated.y - 10.0).abs() < 1e-4);
    }

    #[test]
    fn find_intersection_of_lines_containing_segments_is_unbounded() {
        // Two short, non-overlapping segments whose *containing lines*
        // cross far outside both segments' own extents - segments_intersect
        // (bounded) reports no intersection, but this unbounded sibling
        // (finding 23) must still find the line intersection.
        let w = Segment::new(Point::new(0., 0.), Point::new(1., 0.));
        let p = Segment::new(Point::new(5., 1.), Point::new(5., 2.));
        assert!(segments_intersect(w, p).is_none());
        let hit = find_intersection_of_lines_containing_segments(w, p)
            .expect("infinite lines should intersect");
        assert!((hit.x - 5.0).abs() < 1e-4);
        assert!((hit.y).abs() < 1e-4);
    }

    #[test]
    fn find_intersection_of_lines_containing_segments_parallel_is_none() {
        let w = Segment::new(Point::new(0., 0.), Point::new(1., 0.));
        let p = Segment::new(Point::new(0., 1.), Point::new(1., 1.));
        assert!(find_intersection_of_lines_containing_segments(w, p).is_none());
    }

    #[test]
    fn make_source_to_threshold_proximity_distance_between_speakers() {
        let speakers = vec![Point::new(-5., 10.), Point::new(5., 10.)];
        let listener = Point::new(0., 0.);
        let source = Point::new(0., 20.);
        let result = make_source_to_threshold_proximity_distance(
            &speakers,
            listener,
            source,
            Some((0, 1)),
            false,
        )
        .expect("should not error")
        .expect("crossfade case always produces a result");
        assert!((result.threshold_and_listener_to_source_intersection.x).abs() < 1e-4);
        assert!((result.threshold_and_listener_to_source_intersection.y - 10.0).abs() < 1e-4);
        assert!((result.source_to_threshold_proximity_distance - 10.0).abs() < 1e-4);
    }

    #[test]
    fn make_source_to_threshold_proximity_distance_sequence_extrapolation() {
        // Source is beyond the near end of a 3-speaker sequence laid out
        // along y=10; no crossfade pair found, sequence mode extrapolates
        // from the two nearest (first two) speakers.
        let speakers = vec![
            Point::new(-10., 10.),
            Point::new(0., 10.),
            Point::new(10., 10.),
        ];
        let listener = Point::new(-20., 0.);
        let source = Point::new(-20., 20.);
        let result =
            make_source_to_threshold_proximity_distance(&speakers, listener, source, None, false)
                .expect("should not error")
                .expect("sequence mode always produces a result when not between speakers");
        assert!((result.threshold_and_listener_to_source_intersection.x - -20.0).abs() < 1e-3);
        assert!((result.threshold_and_listener_to_source_intersection.y - 10.0).abs() < 1e-3);
    }

    #[test]
    fn make_source_to_threshold_proximity_distance_polygon_no_pair_is_none() {
        let speakers = vec![Point::new(-5., 10.), Point::new(5., 10.)];
        let listener = Point::new(0., 0.);
        let source = Point::new(0., 20.);
        let result =
            make_source_to_threshold_proximity_distance(&speakers, listener, source, None, true)
                .expect("should not error");
        assert_eq!(result, None);
    }

    #[test]
    fn is_source_behind_or_in_front_of_speaker_threshold_behind() {
        let listener = Point::new(0., 0.);
        let threshold = Point::new(0., 10.);
        let source = Point::new(0., 20.); // farther than the threshold
        let orientation =
            is_source_behind_or_in_front_of_speaker_threshold(listener, source, threshold);
        assert!(orientation.source_is_behind_speakers);
        assert_eq!(orientation.source_speaker_orientation_sign, 1.0);
    }

    #[test]
    fn is_source_behind_or_in_front_of_speaker_threshold_in_front() {
        let listener = Point::new(0., 0.);
        let threshold = Point::new(0., 10.);
        let source = Point::new(0., 5.); // nearer than the threshold
        let orientation =
            is_source_behind_or_in_front_of_speaker_threshold(listener, source, threshold);
        assert!(!orientation.source_is_behind_speakers);
        assert_eq!(orientation.source_speaker_orientation_sign, -1.0);
    }

    #[test]
    fn find_listener_to_source_segment_length_and_angle_basic() {
        let listener = Point::new(0., 0.);
        let source = Point::new(10., 0.);
        let geometry = find_listener_to_source_segment_length_and_angle(listener, source);
        assert!((geometry.length - 10.0).abs() < 1e-4);
        assert!(geometry.listener_to_source_angle.abs() < 1e-4);
        assert!((geometry.source_to_listener_angle - std::f32::consts::PI).abs() < 1e-4);
    }

    #[test]
    fn direct_sound_speaker_distances_for_delays_matches_segment_length() {
        let source = Point::new(0., 0.);
        let speakers = vec![Point::new(3., 4.), Point::new(0., 10.)];
        let distances = direct_sound_speaker_distances_for_delays(&speakers, source);
        assert!((distances[0] - 5.0).abs() < 1e-4);
        assert!((distances[1] - 10.0).abs() < 1e-4);
    }

    #[test]
    fn direct_sound_delay_sample_index_scales_pre_echo_by_sample_rate() {
        // Finding 24: unlike reflection_delay_sample_index, pre-echo time
        // IS scaled by the sample rate here.
        let j = direct_sound_delay_sample_index(0.1, 1.0, 0.0, 1130.0, 44100.0);
        // (0.1 + 0.0) * 44100 + 0.5 truncated = 4410
        assert_eq!(j, 4410);
    }

    #[test]
    fn direct_sound_delay_sample_index_includes_orientation_term() {
        let j = direct_sound_delay_sample_index(0.0, -1.0, 1130.0, 1130.0, 44100.0);
        // (0.0 + (-1.0 * 1130/1130)) * 44100 + 0.5 = -44099.5 -> truncates toward 0 in f64->i64 cast...
        // but since the real formula only ever produces non-negative sums in practice,
        // just check the magnitude/sign behavior directly here.
        assert_eq!(j, -44099);
    }

    #[test]
    fn direct_sound_air_absorption_exponent_selects_by_orientation() {
        assert_eq!(
            direct_sound_air_absorption_exponent(true, 1.0, 2.0),
            2.0,
            "behind speakers uses the virtual-space exponent"
        );
        assert_eq!(
            direct_sound_air_absorption_exponent(false, 1.0, 2.0),
            1.0,
            "in front uses the real-space exponent"
        );
    }

    #[test]
    fn source_in_front_proximity_gain_is_one_when_behind() {
        assert_eq!(source_in_front_proximity_gain(true, 10.0, 5.0, 1.0), 1.0);
    }

    #[test]
    fn source_in_front_proximity_gain_boosts_when_in_front() {
        // ratio = min(1, 10/20) = 0.5; falloff = 0.5^1 = 0.5; gain = 1/0.5 = 2.0
        let gain = source_in_front_proximity_gain(false, 10.0, 20.0, 1.0);
        assert!((gain - 2.0).abs() < 1e-4);
    }

    #[test]
    fn make_direct_sound_speaker_amplitudes_non_crossfade_matches_hand_calc() {
        let db_to_amp = crate::units::DbToAmp::new();
        let speakers = vec![Point::new(10., 0.)];
        let input = DirectSoundAmplitudeInput {
            speakers: &speakers,
            source: Point::new(0., 0.),
            listener: Point::new(-10., 0.),
            listener_to_speaker_angles: &[0.0],
            crossfade: None,
            listener_to_source_angle: 0.0,
            source_to_listener_angle: 0.0,
            source_to_listener_angle_plus_rotation: 0.0,
            source_is_behind_speakers: false,
            orient_source_to_listener: false,
            source_rotation: 0.0,
            minimum_reference_distance_feet: 10.0, // == distance, so air absorption term is 1.0
            air_absorption_exponent_for_real_space_source: 1.0,
            air_absorption_exponent_for_virtual_space_source: 1.0,
            source_dispersion_pattern_rolloff_decibels: 0.0, // source aimed straight at the speaker: 0 angle diff regardless
            threshold_proximity_scalar_switch: false,        // pin the cos() term to 1.0
            source_to_threshold_proximity_distance: 10.0,
            front_source_head_room_scalar: 1.0,
            direct_sound_gain_decibels: 0.0,
        };
        let amps = make_direct_sound_speaker_amplitudes(&input, &db_to_amp);
        assert_eq!(amps.len(), 1);
        // threshold_proximity_scalar(1.0) * air_absorption(1.0) *
        // source_in_front_gain(1.0, since not behind and ratio==1) *
        // front_source_head_room_scalar(1.0) * dB_to_amp(0)^2 (rolloff + gain terms)
        let want = db_to_amp.convert(0.0) * db_to_amp.convert(0.0);
        assert!(
            (amps[0] - want).abs() < 1e-4,
            "got {}, want {}",
            amps[0],
            want
        );
    }

    #[test]
    fn make_direct_sound_speaker_amplitudes_crossfade_splits_between_two_speakers() {
        let db_to_amp = crate::units::DbToAmp::new();
        // Listener at origin, two speakers symmetric left/right, source
        // straight ahead exactly between them (on-axis) - crossfade
        // proportion should end up at the midpoint (0.5) and both speakers
        // should get equal amplitude by symmetry.
        let speakers = vec![Point::new(-5., 10.), Point::new(5., 10.)];
        let listener = Point::new(0., 0.);
        let source = Point::new(0., 20.);
        let angles = listener_to_speaker_angles(listener, &speakers);
        let listener_to_source = find_listener_to_source_segment_length_and_angle(listener, source);

        let input = DirectSoundAmplitudeInput {
            speakers: &speakers,
            source,
            listener,
            listener_to_speaker_angles: &angles,
            crossfade: Some((0, 1)),
            listener_to_source_angle: listener_to_source.listener_to_source_angle,
            source_to_listener_angle: listener_to_source.source_to_listener_angle,
            source_to_listener_angle_plus_rotation: 0.0,
            source_is_behind_speakers: false,
            orient_source_to_listener: false,
            source_rotation: 0.0,
            minimum_reference_distance_feet: 1.0,
            air_absorption_exponent_for_real_space_source: 1.0,
            air_absorption_exponent_for_virtual_space_source: 1.0,
            source_dispersion_pattern_rolloff_decibels: 0.0,
            threshold_proximity_scalar_switch: false,
            source_to_threshold_proximity_distance: 20.0,
            front_source_head_room_scalar: 1.0,
            direct_sound_gain_decibels: 0.0,
        };
        let amps = make_direct_sound_speaker_amplitudes(&input, &db_to_amp);
        assert_eq!(amps.len(), 2);
        assert!(
            (amps[0] - amps[1]).abs() < 1e-4,
            "symmetric setup should give equal amplitudes, got {:?}",
            amps
        );
    }
}
