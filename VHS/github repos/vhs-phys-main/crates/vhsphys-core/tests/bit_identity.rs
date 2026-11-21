//! The engine's output, frozen over a spread of tapes, decks, speeds, rasters and outside
//! fields. A change that is meant to leave the model alone (a faster loop, a different order
//! of work, another schedule) must leave every figure in `FROZEN` as it is.
//!
//! Each case hashes two things: the pictures of a short moving clip, and the signals inside
//! the machine for two fields (the head's RF, the deck's luma and colour-under), bit for bit.
//! The pictures are 8-bit, so they alone would let a small error in the signal through.
//!
//! To freeze a deliberate change of the model, run
//! `cargo test --release -p vhsphys-core --test bit_identity -- --ignored --nocapture`
//! and paste the table it prints over `FROZEN`.

use vhsphys_core::automation::{AutomationSchedule, AutomationTrack};
use vhsphys_core::frame::Frame;

/// Frames in every clip: enough for the first frame clear of the start-up and one after it.
const FRAMES: usize = 4;

/// Fields whose inner signals are hashed, one per head.
const FIELDS: [i64; 2] = [2, 3];

const FNV_OFFSET: u64 = 0xcbf29ce484222325;
const FNV_PRIME: u64 = 0x100000001b3;

fn fnv_bytes(h: &mut u64, bytes: impl Iterator<Item = u8>) {
    for b in bytes {
        *h ^= b as u64;
        *h = h.wrapping_mul(FNV_PRIME);
    }
}

fn fnv_f32(h: &mut u64, v: &[f32]) {
    fnv_bytes(h, v.iter().flat_map(|x| x.to_bits().to_le_bytes()));
}

fn fnv_frames(h: &mut u64, frames: &[Frame]) {
    for f in frames {
        fnv_bytes(h, f.rgb.iter().copied());
    }
}

/// Frame `i` of a clip in which everything moves: bars that slide sideways, a ramp that
/// climbs, a bright block that crosses the picture and fine stripes at the bottom.
fn moving_frame(w: usize, h: usize, i: usize) -> Frame {
    let mut f = Frame::new(w, h);
    let bars: [[u8; 3]; 7] =
        [[191, 191, 191], [191, 191, 0], [0, 191, 191], [0, 191, 0], [191, 0, 191], [191, 0, 0], [0, 0, 191]];
    for y in 0..h {
        for x in 0..w {
            let xs = (x + 23 * i) % w;
            let c: [u8; 3] = if y < h / 2 {
                bars[xs * 7 / w]
            } else if y < h * 3 / 4 {
                let g = ((xs * 255 / (w - 1)) as u8).wrapping_add((y % 7) as u8);
                [g, g / 2 + 40, 255 - g]
            } else if (xs / 3) % 2 == 0 {
                [255, 255, 255]
            } else {
                [0, 0, 0]
            };
            let block = x >= 40 + 31 * i && x < 140 + 31 * i && y >= 60 + 17 * i && y < 130 + 17 * i;
            let p = (y * w + x) * 3;
            f.rgb[p..p + 3].copy_from_slice(if block { &[255, 250, 235] } else { &c });
        }
    }
    f
}

/// The hashing of one engine: `$engine` is `ntsc` or `pal`.
macro_rules! engine_hashes {
    ($engine:ident) => {
        pub mod $engine {
            use super::*;
            use vhsphys_core::$engine::consts::{ACTIVE_H, ACTIVE_W};
            use vhsphys_core::$engine::pipeline::{Chain, Params, Session};
            use vhsphys_core::$engine::config;

            pub fn clip() -> Vec<Frame> {
                (0..FRAMES).map(|i| moving_frame(ACTIVE_W, ACTIVE_H, i)).collect()
            }

            pub fn params(preset: &str, text: &str, seed: u64) -> Params {
                config::build(Some(preset), text, seed).unwrap()
            }

            fn signals(h: &mut u64, p: &Params, schedule: Option<&AutomationSchedule>) {
                let mut s = Session::new(p.clone());
                s.set_automation(schedule.cloned());
                for f in clip() {
                    s.push(&f);
                }
                s.finish();
                for n in FIELDS {
                    let d = s.debug_field(n);
                    fnv_f32(h, &d.rf);
                    fnv_f32(h, &d.luma);
                    fnv_f32(h, &d.cunder);
                    fnv_f32(h, &[d.doc_fraction, d.carrier_level]);
                    fnv_bytes(h, (d.n_transitions as u64).to_le_bytes().into_iter());
                }
            }

            /// Pictures and inner signals of one deck.
            pub fn deck(preset: &str, text: &str, seed: u64) -> u64 {
                let p = params(preset, text, seed);
                let mut h = FNV_OFFSET;
                fnv_frames(&mut h, &Session::run(p.clone(), &clip()));
                signals(&mut h, &p, None);
                h
            }

            /// As `deck`, with a schedule that moves the tracking, the head and the tape tension.
            pub fn automated(preset: &str, seed: u64) -> u64 {
                let p = params(preset, "", seed);
                let mut schedule = AutomationSchedule::new();
                schedule.set_track("tracking_offset_um", AutomationTrack::linear_ramp(0.0, 0.0, 3.0, 18.0));
                schedule.set_track("head_wear", AutomationTrack::smooth_ramp(0.0, 0.0, 3.0, 0.6));
                schedule.set_track("head_spacing_um", AutomationTrack::linear_ramp(0.0, 0.0, 3.0, 0.3));
                schedule.set_track("tension", AutomationTrack::linear_ramp(0.0, 0.0, 3.0, -0.8));
                let mut h = FNV_OFFSET;
                fnv_frames(&mut h, &Session::run_with_automation(p.clone(), &clip(), schedule.clone()));
                signals(&mut h, &p, Some(&schedule));
                h
            }

            /// Pictures of a dub through `n` decks.
            pub fn dub(preset: &str, seed: u64, n: usize) -> u64 {
                let stages = config::generation_params(Some(preset), "", seed, n).unwrap();
                let mut h = FNV_OFFSET;
                fnv_frames(&mut h, &Chain::run(stages, &clip()));
                h
            }
        }
    };
}

engine_hashes!(ntsc);
engine_hashes!(pal);

const LP: &str = "[condition]\ntape_speed = \"lp\"\n";
const EP: &str = "[condition]\ntape_speed = \"ep\"\n";
const COMPOSITE: &str = "[composite]\nenabled = true\ndecoder = \"comb1h\"\n";
const HEAD_BUMPS: &str = "[head]\ncontour_amp = 0.08\npole_length_um = 60.0\n";

/// Every case by name.
fn cases() -> Vec<(&'static str, Box<dyn Fn() -> u64 + Send + Sync>)> {
    let mut v: Vec<(&'static str, Box<dyn Fn() -> u64 + Send + Sync>)> = Vec::new();
    macro_rules! case {
        ($name:literal, $f:expr) => {
            v.push(($name, Box::new(|| $f)));
        };
    }
    case!("worn", ntsc::deck("worn", "", 42));
    case!("pristine", ntsc::deck("pristine", "", 1));
    case!("good", ntsc::deck("good", "", 8));
    case!("bad", ntsc::deck("bad", "", 7));
    case!("ruined", ntsc::deck("ruined", "", 3));
    case!("dry-caps", ntsc::deck("dry-caps", "", 5));
    case!("dirty-heads", ntsc::deck("dirty-heads", "", 9));
    case!("mistracked", ntsc::deck("mistracked", "", 2));
    case!("stretched", ntsc::deck("stretched", "", 4));
    case!("humid-attic", ntsc::deck("humid-attic", "", 6));
    case!("phone-buzz", ntsc::deck("phone-buzz", "", 11));
    case!("ham-radio", ntsc::deck("ham-radio", "", 12));
    case!("am-station", ntsc::deck("am-station", "", 13));
    case!("ground-loop", ntsc::deck("ground-loop", "", 14));
    case!("magnet-screen", ntsc::deck("magnet-screen", "", 15));
    case!("magnet-head", ntsc::deck("magnet-head", "", 16));
    case!("dimmer", ntsc::deck("dimmer", "", 17));
    case!("worn lp", ntsc::deck("worn", LP, 21));
    case!("worn ep", ntsc::deck("worn", EP, 22));
    case!("worn hq", ntsc::deck("worn", "hq = true\n", 23));
    case!("worn composite", ntsc::deck("worn", COMPOSITE, 24));
    case!("worn head bumps", ntsc::deck("worn", HEAD_BUMPS, 25));
    case!("bad pal60", ntsc::deck("bad", "standard = \"pal60\"\n", 26));
    case!("good automated", ntsc::automated("good", 27));
    case!("worn dub 2", ntsc::dub("worn", 28, 2));
    case!("pal worn", pal::deck("worn", "", 31));
    case!("pal pristine", pal::deck("pristine", "", 32));
    case!("pal bad lp", pal::deck("bad", LP, 33));
    case!("pal dry-caps", pal::deck("dry-caps", "", 34));
    case!("pal good automated", pal::automated("good", 35));
    case!("pal worn dub 2", pal::dub("worn", 36, 2));
    v
}

/// The output of every case, as the engine gave it when this table was last frozen.
const FROZEN: &[(&str, u64)] = &[
    ("worn", 0x2327a1ed32b84c5e),
    ("pristine", 0x4d15e41a2bfd0213),
    ("good", 0x0d20cbd08092531c),
    ("bad", 0x3d172e737ec48523),
    ("ruined", 0xc3672b0a361d05fa),
    ("dry-caps", 0xb35110a7adfdcf3b),
    ("dirty-heads", 0x700239127b3ca8da),
    ("mistracked", 0x2921d1c6d6f5bd57),
    ("stretched", 0x25abb9a87277203f),
    ("humid-attic", 0x166490ec303fb1b0),
    ("phone-buzz", 0x86d4f13c9689a540),
    ("ham-radio", 0x1945945e0717576f),
    ("am-station", 0x6f1bb66c3360790e),
    ("ground-loop", 0x714cba07cec85a55),
    ("magnet-screen", 0x61939d116cbe92fe),
    ("magnet-head", 0x9ee82a57723bfdc4),
    ("dimmer", 0x89618ebf24a4c885),
    ("worn lp", 0xcecea5407513fb7e),
    ("worn ep", 0x14a9c9d1d9320d93),
    ("worn hq", 0x10dec09d91060a2d),
    ("worn composite", 0x23eb78d1189762d1),
    ("worn head bumps", 0x1772689b8ec6c211),
    ("bad pal60", 0xcb8e3995317f4ff8),
    ("good automated", 0x9d16fca246588162),
    ("worn dub 2", 0xdaf192a3cede39ce),
    ("pal worn", 0xe2e13ebd99f9bb04),
    ("pal pristine", 0x60d343bcaa5022a4),
    ("pal bad lp", 0xf615e7a39dd58cf2),
    ("pal dry-caps", 0x9bfe7cc6b4a220dc),
    ("pal good automated", 0x870eff90a815c6f3),
    ("pal worn dub 2", 0x0208a3f87a6e2186),
];

fn run_all() -> Vec<(&'static str, u64)> {
    use std::sync::Mutex;
    // the cases are independent; run them side by side, each on its own thread
    let out = Mutex::new(Vec::new());
    std::thread::scope(|s| {
        for (name, f) in cases() {
            let out = &out;
            s.spawn(move || {
                let h = f();
                out.lock().unwrap().push((name, h));
            });
        }
    });
    let mut got = out.into_inner().unwrap();
    let order: Vec<&str> = cases().iter().map(|c| c.0).collect();
    got.sort_by_key(|(name, _)| order.iter().position(|n| n == name));
    got
}

#[test]
fn every_frozen_case_is_bit_identical() {
    let got = run_all();
    assert_eq!(got.len(), FROZEN.len(), "the table and the cases have different lengths; refreeze (see the file header)");
    let wrong: Vec<String> = got
        .iter()
        .zip(FROZEN.iter())
        .filter(|(g, f)| g != f)
        .map(|(g, f)| format!("{}: {:#018x}, frozen {:#018x} ({})", g.0, g.1, f.1, f.0))
        .collect();
    assert!(wrong.is_empty(), "the output changed in {} of {} cases:\n{}", wrong.len(), got.len(), wrong.join("\n"));
}

#[test]
#[ignore = "prints the table to paste over FROZEN"]
fn print_the_table() {
    println!("const FROZEN: &[(&str, u64)] = &[");
    for (name, h) in run_all() {
        println!("    ({name:?}, {h:#018x}),");
    }
    println!("];");
}
