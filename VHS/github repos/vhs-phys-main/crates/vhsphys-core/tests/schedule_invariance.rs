//! The output must not depend on how the work is cut up: on the number of frames asked for
//! at a time, on the number of threads, or on where in the clip a session starts. The tape,
//! the transport and every noise source are functions of the absolute position alone.

use vhsphys_core::config;
use vhsphys_core::consts::{ACTIVE_H, ACTIVE_W};
use vhsphys_core::frame::Frame;
use vhsphys_core::pipeline::{Chain, Params, Session};

/// Frames in the clip: several batches of the largest size tried, and an uneven remainder.
const FRAMES: usize = 11;

/// Frame `i` of a clip that changes from frame to frame.
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

fn clip() -> Vec<Frame> {
    (0..FRAMES).map(moving_frame).collect()
}

fn worn() -> Params {
    config::build(Some("worn"), "", 42).unwrap()
}

/// Run the clip, pushing `push` frames at a time and asking for `batch` frames at a time.
fn run_session(p: Params, input: &[Frame], push: usize, batch: usize) -> Vec<Frame> {
    let mut s = Session::new(p);
    let mut out = Vec::new();
    for chunk in input.chunks(push) {
        for f in chunk {
            s.push(f);
        }
        while s.ready() > 0 {
            out.extend(s.process(batch));
        }
    }
    s.finish();
    while s.ready() > 0 {
        out.extend(s.process(batch));
    }
    out
}

fn same(a: &[Frame], b: &[Frame], what: &str) {
    assert_eq!(a.len(), b.len(), "{what}: frame counts differ");
    for (k, (x, y)) in a.iter().zip(b.iter()).enumerate() {
        assert!(x.rgb == y.rgb, "{what}: frame {k} differs");
    }
}

#[test]
fn the_batch_size_does_not_change_the_picture() {
    let input = clip();
    let reference = run_session(worn(), &input, 1, 1);
    assert_eq!(reference.len(), FRAMES);
    for (push, batch) in [(1, 3), (4, 8), (FRAMES, 8), (FRAMES, 5), (3, 2)] {
        same(&reference, &run_session(worn(), &input, push, batch), &format!("push {push}, batch {batch}"));
    }
}

#[test]
fn the_number_of_threads_does_not_change_the_picture() {
    let input = clip();
    let reference = run_session(worn(), &input, FRAMES, 8);
    for threads in [1, 3] {
        let pool = rayon::ThreadPoolBuilder::new().num_threads(threads).build().unwrap();
        let got = pool.install(|| run_session(worn(), &input, 2, 5));
        same(&reference, &got, &format!("{threads} threads"));
    }
}

#[test]
fn the_batch_size_does_not_change_a_dub() {
    let input = clip();
    let stages = || config::generation_params(Some("worn"), "", 7, 2).unwrap();
    let run = |batch: usize| {
        let mut c = Chain::new(stages());
        let mut out = Vec::new();
        for f in &input {
            c.push(f);
            while c.ready() > 0 {
                out.extend(c.process(batch));
            }
        }
        c.finish();
        while c.ready() > 0 {
            out.extend(c.process(batch));
        }
        out
    };
    let reference = run(1);
    assert_eq!(reference.len(), FRAMES);
    same(&reference, &run(4), "dub, batch 4");
    same(&reference, &run(8), "dub, batch 8");
}

/// Frames a session started part-way into a clip needs before its picture is that of a run
/// of the whole clip: what was recorded before its first frame is blank to it, and the
/// tracks and segments of a frame reach back into the frame before.
const START_UP_FRAMES: usize = 2;

#[test]
fn a_session_started_part_way_gives_the_same_tape() {
    let input = clip();
    let whole = run_session(worn(), &input, FRAMES, 8);
    for start in [3usize, 4] {
        let mut s = Session::with_start(worn(), start as i64);
        let mut part = Vec::new();
        for f in &input[start..] {
            s.push(f);
        }
        s.finish();
        while s.ready() > 0 {
            part.extend(s.process(3));
        }
        assert_eq!(part.len(), FRAMES - start);
        for k in start + START_UP_FRAMES..FRAMES {
            assert!(whole[k].rgb == part[k - start].rgb, "start {start}: frame {k} differs from the run of the whole clip");
        }
    }
}
