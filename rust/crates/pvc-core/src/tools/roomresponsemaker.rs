//! Ports `roomresponsemaker.c` (9318 lines, the largest and most
//! structurally distinct tool in this project): a recursive image-source
//! polygonal-room acoustics engine, not a phase-vocoder filter/resynthesis
//! tool like every other Phase 5 tool. **This module is now through Phase 3
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
//! **What Phase 3 deliberately does *not* cover, and why**: reading
//! `writeReflectionPulsesIntoImpulseResponse()` in full (lines ~4404-5062)
//! showed that the overwhelming majority of that function - everything
//! feeding `impulseResponseNow` before the gain computed here ever gets
//! multiplied onto it - is a wall/reflection-order impulse-response *file*
//! cache-and-convolve engine (`testSequence`/`recallIR`/`addToIRfileCodes`/
//! `convolveTwoArrays`/`makeReflectionOrderImpulseResponseNow`/the
//! `filterAndNormalize*` family), not distance-driven synthetic pulse
//! placement as the function's own name suggests. That engine reads actual
//! wall/reflection-order impulse-response audio files, is a substantial and
//! distinct later phase on its own, and is not started here.
//! `writeDirectSourcePulsesIntoImpulseResponse()` and its whole
//! speaker-dispersion/source-threshold-proximity dependency chain
//! (`makeSourceToSpeakerDistancesAndAngles`,
//! `findSourceToSpeakerAngleDifferencesFromSourceToListenerAngle`,
//! `makeSourceToThresholdProximityProportion`/`Distance`,
//! `isSourceBehindOrInFrontOfSpeakerThreshold`) remains fully deferred, same
//! as Phase 2 left it - direct-sound pulses are a distinct concern from the
//! reflection math this phase covers. Plotting-file output and CLI wiring
//! are also still not started, so `crack()` flag cross-referencing is still
//! deferred to that later phase, same as Phases 1-2.
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
//! **Worth the next phase double-checking** (found while reading `main()`'s
//! control flow around this phase's own setup calls, but not itself part
//! of this phase's scope): `rotatedSource` (the effective source rotation
//! used later by the dispersion-pattern/reflection code) is computed in
//! `main()` at lines 1209-1219, *before* `getSourceCoordinates()`/
//! `getListenerCoordinates()` are ever called (lines 1322/1366) - and
//! `sourceToListenerAngle` (which `-q1`, "orient source to listener,"
//! reads at line 1214) is only ever assigned inside
//! `findListenerToSourceSegmentLengthAndAngle()`, itself not called until
//! much later. At the point `rotatedSource` is computed,
//! `sourceToListenerAngle` still holds its zero-initialized default, so
//! `-q1` mode appears to always behave identically to `-q0` ("orient to
//! room") - `rotatedSource` is set once and never recomputed afterward.
//! Confirmed by grepping every reference to both variables; needs
//! re-verification once the phase that ports this control flow is
//! underway, since it directly affects source-orientation semantics.

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
}
