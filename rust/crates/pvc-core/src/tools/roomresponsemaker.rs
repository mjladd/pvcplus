//! Ports `roomresponsemaker.c` (9318 lines, the largest and most
//! structurally distinct tool in this project): a recursive image-source
//! polygonal-room acoustics engine, not a phase-vocoder filter/resynthesis
//! tool like every other Phase 5 tool. **This module is Phase 1 of a
//! multi-phase port**: the room/speaker/listener geometry layer only - room
//! polygon construction (synthesized or file-read), coordinate transforms,
//! listener/source/speaker position resolution, and the small segment/angle
//! utilities everything else builds on. It does not yet cover the recursive
//! image-source reflection-path algorithm (`mirrorPolygonCoordinatesAroundAllSides`,
//! `writeReflectionPulsesIntoImpulseResponse`) or any impulse-response
//! audio/convolution/filtering code - those are later phases, not started
//! here. No CLI wiring yet either.
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
//! Also read, but deliberately deferred to a later phase (not needed by
//! any function in this phase's scope, confirmed by grepping call sites):
//! `mirrorPointAroundLineSegment` (only used by the reflection-path
//! algorithm), `findIntersectionOfLinesContainingSegments` (only used by
//! `makeSourceToThresholdProximityDistance`), `rotatePointToAngle` and
//! `valueIsBetweenTheseTwo` (only used by the direct-sound amplitude and
//! source-threshold-proximity functions), and `isPolygonConcave`'s reflex-
//! vertex-flagging sub-step (depends on `pointToLinePosition`, itself out
//! of scope) - [`polygon_is_concave`] here only reproduces the sign-change
//! convex/concave *boolean*, not the per-vertex reflex-angle flags.
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

#[cfg(test)]
mod tests {
    use super::*;

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
}
