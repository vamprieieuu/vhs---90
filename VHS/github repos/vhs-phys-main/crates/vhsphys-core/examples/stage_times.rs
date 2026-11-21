//! Where the time of a render goes: each step of the chain timed on one thread, per field.
//!
//! The steps are the ones `Session` runs (record, head, deck, decode, picture tube), called
//! here one after another on a moving test clip so that each can be timed alone. Run with
//! `cargo run --release -p vhsphys-core --example stage_times -- [preset]` (default `worn`).

use std::collections::HashMap;
use std::time::Instant;
use vhsphys_core::config;
use vhsphys_core::consts::*;
use vhsphys_core::crt;
use vhsphys_core::decode::{decode_baseband, StreamView};
use vhsphys_core::electronics::{discriminate, process_segment};
use vhsphys_core::head::{azimuth_smooth, head_front_end, render_pulses, Controls};
use vhsphys_core::mech::Mech;
use vhsphys_core::pipeline::calibrate;
use vhsphys_core::record::{record_track, FrameStore, Transition};
use vhsphys_core::rng::Rng;
use vhsphys_core::tape::Tape;
use vhsphys_core::tracking::{lateral_weights, NEIGHBOURS};
use vhsphys_core::video::{encode_frame, Frame};

/// Frames in the clip, and the fields timed (those with every neighbour inside the clip).
const FRAMES: usize = 6;
const FIRST_FIELD: i64 = 3;
const LAST_FIELD: i64 = 8;

/// The head's noise stream, as in the session.
const STREAM_HEAD: u64 = 1;

fn moving_frame(i: usize) -> Frame {
    let mut f = Frame::new(ACTIVE_W, ACTIVE_H);
    for y in 0..ACTIVE_H {
        for x in 0..ACTIVE_W {
            let xs = (x + 29 * i) % ACTIVE_W;
            let p = (y * ACTIVE_W + x) * 3;
            f.rgb[p] = (xs * 255 / (ACTIVE_W - 1)) as u8;
            f.rgb[p + 1] = ((y + 13 * i) % 256) as u8;
            f.rgb[p + 2] = if (xs / 40 + y / 40) % 2 == 0 { 230 } else { 20 };
        }
    }
    f
}

fn track_window(n: i64) -> (i64, usize) {
    (switch_time(n) - TRACK_MARGIN_RF as i64, FIELD_RF + 2 * TRACK_MARGIN_RF)
}

/// Seconds spent in each named step, in the order first seen.
#[derive(Default)]
struct Times {
    names: Vec<&'static str>,
    secs: HashMap<&'static str, f64>,
}

impl Times {
    fn time<T>(&mut self, name: &'static str, f: impl FnOnce() -> T) -> T {
        let t = Instant::now();
        let out = f();
        if !self.secs.contains_key(name) {
            self.names.push(name);
        }
        *self.secs.entry(name).or_insert(0.0) += t.elapsed().as_secs_f64();
        out
    }
}

fn main() {
    let preset = std::env::args().nth(1).unwrap_or_else(|| "worn".into());
    let mut p = config::build(Some(&preset), "", 42).unwrap();
    let cal = calibrate(&p);
    p.deck.fm_gain = cal.fm_gain;
    p.decode.burst_nominal = cal.burst_nominal;
    p.decode.burst_boost = p.record.burst_boost as f64;
    p.decode.chroma_delay = cal.chroma_delay;
    let mut tape_p = p.tape.clone();
    tape_p.speed = p.speed;
    let (mech, tape) = (Mech::new(p.mech.clone(), p.seed), Tape::new(tape_p, p.seed));

    let mut t = Times::default();
    let frames: Vec<Frame> = (0..FRAMES).map(moving_frame).collect();
    let store = FrameStore::new(frames.iter().map(|f| t.time("source: encode frame (per frame)", || encode_frame(f))).collect());

    // record: two tracks beyond each end of the fields that are played
    let mut trans: HashMap<i64, Vec<Transition>> = HashMap::new();
    for n in FIRST_FIELD - 3..=LAST_FIELD + 3 {
        let (g0, len) = track_window(n);
        let tr = t.time("record: record_track", || record_track(&store, &p.record, n.rem_euclid(2) as usize, g0, len));
        trans.insert(n, tr);
    }
    let recorded = (LAST_FIELD + 3 - (FIRST_FIELD - 3) + 1) as f64;

    // head: the steps of the session's RF render, without automation or outside fields
    let mut rf: HashMap<i64, Vec<f32>> = HashMap::new();
    for n in FIRST_FIELD - 1..=LAST_FIELD + 1 {
        let (g0, len) = track_window(n);
        let hp = p.heads[n.rem_euclid(2) as usize].clone();
        let mut ctl = Controls::new(len);
        t.time("head: transport warp (fill_warp)", || mech.fill_warp(n, g0, &mut ctl));
        t.time("head: tape defects and spacing (paint)", || tape.paint(n, g0, hp.width_um, &mut ctl));
        if hp.extra_spacing_um != 0.0 {
            ctl.dd.iter_mut().for_each(|v| *v += hp.extra_spacing_um as f32);
        }
        let w = t.time("head: lateral weights", || lateral_weights(&p.tracking, p.speed, hp.width_um, p.seed, n, len, g0));
        let h0 = hp.h0_samples();
        let mut buf = vec![0.0f32; len];
        t.time("head: own-track pulses", || render_pulses(&trans[&n], 0.0, &ctl, Some(&w[2]), h0, &mut buf));
        if hp.azimuth_error_deg > 0.0 {
            let mean_w = w[2].iter().sum::<f32>() as f64 / w[2].len() as f64;
            t.time("head: azimuth smoothing", || azimuth_smooth(&mut buf, hp.width_um * mean_w, hp.azimuth_error_deg));
        }
        let mut nb_ctl = ctl.clone();
        nb_ctl.mag.iter_mut().for_each(|v| *v = 1.0);
        for (k, &j) in NEIGHBOURS.iter().enumerate() {
            let wmax = w[k].iter().cloned().fold(0.0f32, f32::max);
            if j == 0 || wmax < 0.003 {
                continue;
            }
            let mut nb = vec![0.0f32; len];
            t.time("head: neighbour pulses", || {
                render_pulses(&trans[&(n + j)], -(j as f64) * p.speed.adjacent_track_offset_rf(), &nb_ctl, Some(&w[k]), h0, &mut nb)
            });
            if j.abs() % 2 == 1 {
                t.time("head: azimuth smoothing", || azimuth_smooth(&mut nb, hp.width_um * wmax as f64, 2.0 * AZIMUTH_DEG));
            }
            t.time("head: neighbour sum", || buf.iter_mut().zip(nb.iter()).for_each(|(a, b)| *a += *b));
        }
        let mut rng = Rng::new(p.seed, STREAM_HEAD, n);
        t.time("head: noise and resonance (head_front_end)", || head_front_end(&mut buf, &hp, &mut rng));
        rf.insert(n, buf);
    }
    let rendered = (LAST_FIELD - FIRST_FIELD + 3) as f64;

    // deck: the head-switched segment of each field
    let warm = SEGMENT_WARM_RF as i64;
    let mut segs: HashMap<i64, (i64, Vec<f32>, Vec<f32>)> = HashMap::new();
    for n in FIRST_FIELD..=LAST_FIELD {
        let (g_a, g_b) = (switch_time(n), switch_time(n + 1));
        let (s0, s1) = (g_a - warm, g_b + warm);
        let mut buf = Vec::with_capacity((s1 - s0) as usize);
        for (m, lo, hi) in [(n - 1, s0, g_a), (n, g_a, g_b), (n + 1, g_b, s1)] {
            let gr = track_window(m).0;
            buf.extend_from_slice(&rf[&m][(lo - gr) as usize..(hi - gr) as usize]);
        }
        let out = t.time("deck: process_segment (all of it)", || process_segment(&buf, &p.deck, s0));
        // the limiter and discriminator alone, on the raw RF (inside the figure above as well)
        t.time("deck:   of which discriminator (about)", || discriminate(&buf, p.deck.limiter_threshold as f32, p.deck.demod_delay));
        let (a, b) = (warm as usize / OS, (warm as usize + FIELD_RF) / OS);
        segs.insert(n, (g_a / OS as i64, out.luma[a..b].to_vec(), out.cunder[a..b].to_vec()));
    }
    let played = (LAST_FIELD - FIRST_FIELD + 1) as f64;

    // decode and picture tube, per frame
    let mut decoded = 0.0;
    for k in (FIRST_FIELD + 1) / 2..=(LAST_FIELD - 2) / 2 {
        let parts = [&segs[&(2 * k)], &segs[&(2 * k + 1)], &segs[&(2 * k + 2)]];
        let luma: Vec<f32> = parts.iter().flat_map(|s| s.1.iter().copied()).collect();
        let cu: Vec<f32> = parts.iter().flat_map(|s| s.2.iter().copied()).collect();
        let view = StreamView { bb0: parts[0].0, luma: &luma, cunder: &cu };
        let bb = t.time("decode: decode_baseband (per frame)", || decode_baseband(k, &view, &p.decode, &p.composite, &p.interference));
        t.time("tube: scan_to_frame (per frame)", || crt::scan_to_frame(&bb, k, &p.interference));
        decoded += 1.0;
    }

    println!("preset {preset}: milliseconds on one thread, per field (or per frame where it says so)");
    let mut frame_ms = 0.0;
    for name in &t.names {
        let (count, per_frame) = match name.split(':').next().unwrap() {
            "source" => (FRAMES as f64, 1.0),
            "record" => (recorded, 2.0),
            "head" => (rendered, 2.0),
            "deck" => (played, 2.0),
            _ => (decoded, 1.0),
        };
        let ms = t.secs[name] * 1e3 / count;
        if !name.contains("of which") {
            frame_ms += ms * per_frame;
        }
        println!("  {ms:8.2}  {name}");
    }
    println!("  {frame_ms:8.2}  sum for one frame (two fields)");
}
