//! Acceptance test for the scanned camera tube (docs/design/CAMERA_SCAN.md): the target is read
//! line by line, field by field, at each element's own time, by a beam with a Gaussian profile.

use std::f64::consts::PI;
use vhsphys_core::camera::{read_time_s, CameraParams, CameraTube, TargetSignal};
use vhsphys_core::consts::{ACTIVE_H, ACTIVE_W, FH, FRAME_RATE, FS_BB};
use vhsphys_core::frame::Frame;
use vhsphys_core::tube::TubeKind;

const R: usize = 0;
const G: usize = 1;

fn camera(kind: TubeKind) -> CameraTube {
    CameraTube::new(&CameraParams { tube: kind, ..CameraParams::default() }).unwrap()
}

/// Mean of channel `ch` over the rows that satisfy `keep`, every sample.
fn mean_rows(s: &TargetSignal, ch: usize, keep: impl Fn(usize) -> bool) -> f64 {
    let (mut sum, mut n) = (0.0, 0usize);
    for (r, row) in s.rows.iter().enumerate() {
        if keep(r) {
            sum += row[ch].iter().map(|v| *v as f64).sum::<f64>();
            n += row[ch].len();
        }
    }
    sum / n as f64
}

/// Read `frames` frames of a light that is `level(t)` nA everywhere.
fn run_uniform(cam: &mut CameraTube, frames: usize, level: impl Fn(f64) -> f64 + Sync) -> Vec<TargetSignal> {
    (0..frames).map(|_| cam.read_currents(&|_, _, _, t| level(t))).collect()
}

fn dark_and_bias(cam: &CameraTube, ch: usize) -> f64 {
    cam.channels()[ch].dark_na + cam.channels()[ch].bias_na
}

#[test]
fn a_steady_uniform_light_reads_back_photo_plus_dark_and_bias() {
    for (light, kind) in [(100.0, TubeKind::PlumbiconBias), (0.0, TubeKind::PlumbiconBias), (100.0, TubeKind::Plumbicon)] {
        let mut cam = camera(kind);
        let s = run_uniform(&mut cam, 40, |_| light).pop().unwrap();
        for ch in 0..3 {
            let want = light + dark_and_bias(&cam, ch);
            let got = mean_rows(&s, ch, |_| true);
            assert!((got - want).abs() < 0.01 * want.max(1.0), "{kind:?} ch{ch}, light {light}: read {got:.3}, want {want:.3}");
        }
    }
}

#[test]
fn rows_are_read_in_raster_order_one_field_after_the_other() {
    let t_frame = 1.0 / FRAME_RATE;
    for n in [0i64, 7] {
        // field 0 (even rows) is read entirely before field 1 (odd rows)
        let last_field0 = (0..ACTIVE_H).step_by(2).map(|r| read_time_s(n, r, ACTIVE_W - 1)).fold(f64::MIN, f64::max);
        let first_field1 = (1..ACTIVE_H).step_by(2).map(|r| read_time_s(n, r, 0)).fold(f64::MAX, f64::min);
        assert!(last_field0 < first_field1, "frame {n}: {last_field0} vs {first_field1}");
        // within a field, one line period between rows, top down
        for r in [0usize, 1, 100, 101] {
            let dt = read_time_s(n, r + 2, 0) - read_time_s(n, r, 0);
            assert!((dt - 1.0 / FH).abs() < 1e-12, "row {r}: {dt}");
        }
        // along the line, one baseband sample at a time
        let dx = read_time_s(n, 10, 1) - read_time_s(n, 10, 0);
        assert!((dx - 1.0 / FS_BB).abs() < 1e-15);
        // the whole frame lies inside its own frame period
        let t0 = n as f64 * t_frame;
        for r in 0..ACTIVE_H {
            let t = read_time_s(n, r, 0);
            assert!(t >= t0 && t < t0 + t_frame, "frame {n} row {r}: {t}");
        }
    }
}

#[test]
fn a_light_switched_on_between_fields_builds_up_down_the_field() {
    // Each row integrates from its previous read to its own read, so a light that comes on at the
    // start of field 1 is read in full only by rows near the bottom of that field: the rolling
    // skew of a tube camera. Field 0 of the same frame, read before it, sees none of it.
    let mut cam = camera(TubeKind::Plumbicon);
    run_uniform(&mut cam, 20, |_| 0.0);
    let n = 20i64;
    let t_on = (n as f64 + 0.5) / FRAME_RATE;
    let s = cam.read_currents(&|_, _, _, t| if t >= t_on { 150.0 } else { 0.0 });
    let base = dark_and_bias(&cam, G);
    let row_mean = |r: usize| s.rows[r][G].iter().map(|v| *v as f64).sum::<f64>() / ACTIVE_W as f64 - base;
    for r in (0..ACTIVE_H).step_by(2) {
        assert!(row_mean(r) < 1.0, "field 0 row {r} was read before the light came on: {}", row_mean(r));
    }
    let top = (1..41).step_by(2).map(row_mean).sum::<f64>() / 20.0;
    let bottom = (ACTIVE_H - 41..ACTIVE_H).step_by(2).map(row_mean).sum::<f64>() / 20.0;
    assert!(bottom > 10.0, "rows at the bottom of field 1 integrated the light: {bottom}");
    assert!(bottom > 3.0 * top, "the light builds up down the field: top {top:.2}, bottom {bottom:.2}");
}

#[test]
fn a_picture_change_between_fields_reaches_field_one_only() {
    let mut cam = camera(TubeKind::Plumbicon);
    let black = Frame::new(ACTIVE_W, ACTIVE_H);
    let mut white = Frame::new(ACTIVE_W, ACTIVE_H);
    white.rgb.fill(255);
    for _ in 0..10 {
        cam.read(&black, &black);
    }
    let s = cam.read(&black, &white);
    let base = dark_and_bias(&cam, G);
    let peak = cam.channels()[G].peak_signal_na;
    let f0 = mean_rows(&s, G, |r| r % 2 == 0);
    let f1 = mean_rows(&s, G, |r| r % 2 == 1);
    assert!(f0 < base + 0.01 * peak, "field 0 saw only the black picture: {f0}");
    assert!(f1 > base + 0.3 * peak, "field 1 integrated the white picture: {f1}");
}

#[test]
fn the_scene_before_the_first_picture_is_that_picture() {
    // The top rows of the first frame integrate mostly the time before the session; that light is
    // the first picture's, so a camera starting on a still grey reads it down the whole frame.
    let mut cam = camera(TubeKind::Plumbicon);
    let mut grey = Frame::new(ACTIVE_W, ACTIVE_H);
    grey.rgb.fill(128);
    let s = cam.read(&grey, &grey);
    let base = dark_and_bias(&cam, G);
    let top = mean_rows(&s, G, |r| r < 40) - base;
    let bottom = mean_rows(&s, G, |r| r >= ACTIVE_H - 40) - base;
    assert!(bottom > 1.0, "the first frame reads the grey: {bottom}");
    assert!(top > 0.9 * bottom, "top rows of the first frame: {top:.2}, bottom rows {bottom:.2}");
}

/// (lit - unlit) / (lit + unlit) of the signal above dark, over the rows `rows`.
fn contrast(s: &TargetSignal, cam: &CameraTube, lit: impl Fn(usize) -> bool, rows: impl Fn(usize) -> bool) -> f64 {
    let base = dark_and_bias(cam, G);
    let on = mean_rows(s, G, |r| rows(r) && lit(r)) - base;
    let off = mean_rows(s, G, |r| rows(r) && !lit(r)) - base;
    (on - off) / (on + off)
}

#[test]
fn the_beam_overlaps_the_other_fields_lines() {
    // frame Nyquist: every other row lit. The beam's wings read the other field's lines, so
    // little of the pattern survives (sigma about 0.65 of the frame-line pitch).
    let mut cam = camera(TubeKind::Plumbicon);
    let lit_frame = |r: usize| r % 2 == 0;
    let s = (0..30).map(|_| cam.read_currents(&|_, r, _, _| if lit_frame(r) { 30.0 } else { 0.0 })).last().unwrap();
    let c_frame = contrast(&s, &cam, lit_frame, |_| true);
    assert!(c_frame < 0.35, "frame-Nyquist contrast {c_frame:.3}");
    // field Nyquist: pairs of rows lit, so within one field lit and dark rows alternate. Much more
    // of this pattern survives.
    let mut cam = camera(TubeKind::Plumbicon);
    let lit_field = |r: usize| (r / 2) % 2 == 0;
    let s = (0..30).map(|_| cam.read_currents(&|_, r, _, _| if lit_field(r) { 30.0 } else { 0.0 })).last().unwrap();
    let c_field = contrast(&s, &cam, lit_field, |r| r % 2 == 0);
    assert!(c_field > 0.4, "field-Nyquist contrast {c_field:.3}");
    assert!(c_field > c_frame + 0.2);
}

/// Amplitude of the component at `f` Hz of a row signal sampled at the baseband rate.
fn amplitude(row: &[f32], f: f64) -> f64 {
    let (mut re, mut im) = (0.0, 0.0);
    for (x, v) in row.iter().enumerate() {
        let w = 2.0 * PI * f * x as f64 / FS_BB;
        re += *v as f64 * w.cos();
        im += *v as f64 * w.sin();
    }
    2.0 * (re * re + im * im).sqrt() / row.len() as f64
}

#[test]
fn the_horizontal_spot_is_a_gaussian_in_time() {
    for f in [1.0e6, 3.0e6] {
        let mut cam = camera(TubeKind::Plumbicon);
        // a whole number of cycles over the active line
        let cycles = (f * ACTIVE_W as f64 / FS_BB).round();
        let f = cycles * FS_BB / ACTIVE_W as f64;
        let light = move |_: usize, _: usize, x: usize, _: f64| 60.0 + 40.0 * (2.0 * PI * f * x as f64 / FS_BB).sin();
        let s = (0..20).map(|_| cam.read_currents(&light)).last().unwrap();
        let sigma = cam.channels()[G].sigma_h_s;
        let want = (-2.0 * PI * PI * sigma * sigma * f * f).exp();
        // middle rows, away from the line ends where the spot runs off the picture
        let got = (100..110).map(|r| amplitude(&s.rows[r][G], f)).sum::<f64>() / 10.0 / 40.0;
        assert!((got - want).abs() < 0.05 * want, "{:.1} MHz: response {got:.3}, Gaussian {want:.3}", f / 1e6);
    }
}

/// Decay after `on_s` seconds of `level` nA: (seconds after the light went out, residual above
/// dark as a percentage of the signal) for each field read within `upto_s`, from the mean read
/// time of the field's rows.
fn decay(cam: &mut CameraTube, ch: usize, level: f64, on_s: f64, upto_s: f64) -> Vec<(f64, f64)> {
    let t_frame = 1.0 / FRAME_RATE;
    let on_frames = (on_s / t_frame).ceil() as usize;
    let t_off = on_frames as f64 * t_frame;
    let lit = run_uniform(cam, on_frames, |_| level).pop().unwrap();
    let base = dark_and_bias(cam, ch);
    let span = mean_rows(&lit, ch, |_| true) - base;
    let mut out = vec![];
    let mut n = on_frames as i64;
    while (n as f64 * t_frame) - t_off < upto_s {
        let s = cam.read_currents(&|_, _, _, _| 0.0);
        for p in 0..2 {
            let rows: Vec<usize> = (p..ACTIVE_H).step_by(2).collect();
            let t = rows.iter().map(|r| read_time_s(n, *r, ACTIVE_W / 2)).sum::<f64>() / rows.len() as f64;
            let v = mean_rows(&s, ch, |r| r % 2 == p);
            out.push((t - t_off, 100.0 * (v - base) / span));
        }
        n += 1;
    }
    out
}

/// The closed form (Philips 1963, eq. 6): landing current after a step from i1 to i2, K = C V_T.
fn logistic_residual_pct(t: f64, signal: f64, dark: f64, k: f64) -> f64 {
    let (i1, i2) = (signal + dark, dark);
    let i = i2 / (1.0 - (1.0 - i2 / i1) * (-t * i2 / k).exp());
    100.0 * (i - i2) / signal
}

#[test]
fn decay_follows_the_logistic_law_in_time() {
    // the handbook's low-key condition: 20 nA of signal on a red tube with a 300 nA beam
    let mut cam = camera(TubeKind::PlumbiconAct);
    let ch = cam.channels()[R];
    let readings = decay(&mut cam, R, 20.0, 1.0, 0.22);
    for (t, got) in readings.into_iter().filter(|(t, _)| *t > 0.04) {
        let want = logistic_residual_pct(t, 20.0, ch.dark_na + ch.bias_na, ch.k_nas);
        assert!((got - want).abs() < 0.25 * want, "{:.0} ms: {got:.2} % against the law's {want:.2} %", t * 1e3);
    }
}

/// Log-linear interpolation of decay readings at `t`.
fn at(readings: &[(f64, f64)], t: f64) -> f64 {
    let i = readings.iter().position(|(ti, _)| *ti >= t).expect("reading after t");
    let ((t0, v0), (t1, v1)) = (readings[i - 1], readings[i]);
    let f = (t - t0) / (t1 - t0);
    (v0.ln() * (1.0 - f) + v1.ln() * f).exp()
}

#[test]
fn lag_runs_on_time_whatever_the_field_rate() {
    use vhsphys_core::pal;
    let mut ntsc = camera(TubeKind::Plumbicon);
    let a = decay(&mut ntsc, R, 20.0, 1.0, 0.25);
    let mut pal_cam = pal::camera::CameraTube::new(&pal::camera::CameraParams { tube: TubeKind::Plumbicon, ..Default::default() }).unwrap();
    let t_frame = 1.0 / pal::consts::FRAME_RATE;
    let on_frames = (1.0 / t_frame).ceil() as usize;
    let t_off = on_frames as f64 * t_frame;
    let base = pal_cam.channels()[R].dark_na + pal_cam.channels()[R].bias_na;
    let lit = (0..on_frames).map(|_| pal_cam.read_currents(&|_, _, _, _| 20.0)).last().unwrap();
    let span = lit.rows.iter().map(|r| r[R].iter().map(|v| *v as f64).sum::<f64>()).sum::<f64>() / (lit.rows.len() * lit.rows[0][R].len()) as f64 - base;
    let mut b = vec![];
    for n in on_frames as i64..on_frames as i64 + 8 {
        let s = pal_cam.read_currents(&|_, _, _, _| 0.0);
        for p in 0..2 {
            let rows: Vec<usize> = (p..s.rows.len()).step_by(2).collect();
            let t = rows.iter().map(|r| pal::camera::read_time_s(n, *r, s.rows[0][R].len() / 2)).sum::<f64>() / rows.len() as f64;
            let v = rows.iter().map(|r| s.rows[*r][R].iter().map(|v| *v as f64).sum::<f64>()).sum::<f64>() / (rows.len() * s.rows[0][R].len()) as f64;
            b.push((t - t_off, 100.0 * (v - base) / span));
        }
    }
    for t in [0.1, 0.2] {
        let (x, y) = (at(&a, t), at(&b, t));
        assert!(x / y > 0.85 && x / y < 1.18, "{:.0} ms: 525 lines {x:.2} %, 625 lines {y:.2} %", t * 1e3);
    }
}

#[test]
fn the_same_input_gives_the_same_signal() {
    let light = |ch: usize, r: usize, x: usize, t: f64| ((ch * 7 + r * 3 + x) % 50) as f64 * (1.0 + t);
    let run = || {
        let mut cam = camera(TubeKind::PlumbiconAct);
        (0..3).map(|_| cam.read_currents(&light)).last().unwrap().rows
    };
    assert!(run() == run());
}
