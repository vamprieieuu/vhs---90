//! Physical model of VHS record and playback.
//!
//! Signal flow (see `docs/ARCHITECTURE.md`):
//! frame -> raster -> record electronics -> record head/tape (relay model)
//! -> tape with defects and head/tape spacing -> playback head (Lorentzian
//! pulses, azimuth crosstalk) -> mechanics (timebase) -> deck electronics
//! (equaliser, dropout compensator, limiter, FM discriminator, emphasis) ->
//! sync/colour decoding -> frame.
//!
//! The raster (525 or 625 lines) is a set of compile-time constants, so the engine is written
//! once and compiled twice: `ntsc` against `consts.rs`, `pal` against `consts_pal.rs`. The
//! crate root re-exports the 525-line engine, so `vhsphys_core::pipeline::Session` is
//! `vhsphys_core::ntsc::pipeline::Session`. What needs no raster constant is compiled once
//! and shared by both engines: `dsp`, `frame`, `geometry`, `interference_params`, `rf_rate`, `rng`,
//! `spectral`, `standard` and `tube`.
//!
//! Decks of the two engines are joined by `rack`: tagged signals (`signal`), explicit links and
//! a standards converter (`convert`, `aperture`); see `docs/BOUNCE.md`.

// Shared by both engines: nothing in them depends on the raster.
pub mod automation;
pub mod dsp;
pub mod frame;
pub mod geometry;
pub mod interference_params;
pub mod rf_rate;
pub mod rng;
pub mod spectral;
pub mod standard;
pub mod tube;

// What joins decks of either engine. These modules name both engines (`crate::ntsc`,
// `crate::pal`) and hold no raster constant of their own.
pub mod aperture;
pub mod convert;
pub mod rack;
pub mod signal;

/// Declares every engine module, each from the one source file, inside the module that calls
/// it. The arguments are the constants file the engine is built against and its RF sample rate. Engine files reach each
/// other as `super::x`, so each copy of the engine sees its own constants; the shared modules
/// above are reached as `crate::x`.
macro_rules! engine {
    ($consts:literal, $rate:expr) => {
        /// The RF sample rate this engine was built for (`consts::RF_RATE` reads it).
        pub const ENGINE_RF_RATE: crate::rf_rate::RfRate = $rate;
        #[path = $consts]
        pub mod consts;
        #[path = "audio.rs"]
        pub mod audio;
        #[path = "camera.rs"]
        pub mod camera;
        #[path = "camera_processing.rs"]
        pub mod camera_processing;
        #[path = "composite.rs"]
        pub mod composite;
        #[path = "condition.rs"]
        pub mod condition;
        #[path = "config.rs"]
        pub mod config;
        #[path = "crt.rs"]
        pub mod crt;
        #[path = "decode.rs"]
        pub mod decode;
        #[path = "electronics.rs"]
        pub mod electronics;
        #[path = "emphasis.rs"]
        pub mod emphasis;
        #[path = "format.rs"]
        pub mod format;
        #[path = "head.rs"]
        pub mod head;
        #[path = "interference.rs"]
        pub mod interference;
        #[path = "mech.rs"]
        pub mod mech;
        #[path = "pipeline.rs"]
        pub mod pipeline;
        #[path = "record.rs"]
        pub mod record;
        #[path = "stripe.rs"]
        pub mod stripe;
        #[path = "tape.rs"]
        pub mod tape;
        #[path = "tracking.rs"]
        pub mod tracking;
        #[path = "video.rs"]
        pub mod video;
    };
}

/// The 525-line engine (NTSC raster, `consts.rs`).
pub mod ntsc;
/// The 625-line engine (PAL raster, `consts_pal.rs`).
pub mod pal;

// The same two engines with the tape signal sampled more coarsely or more finely than the
// reference 12 x fsc (`rf_rate`). The unit tests inside the engine files pin figures of the
// reference rate, so these copies are left out of the unit-test build; `tests/rf_rate.rs`
// checks them.
/// The 525-line engine at 8 x fsc.
#[cfg(not(test))]
pub mod ntsc_rf8;
/// The 625-line engine at 8 x fsc.
#[cfg(not(test))]
pub mod pal_rf8;
/// The 525-line engine at 16 x fsc.
#[cfg(not(test))]
pub mod ntsc_rf16;
/// The 625-line engine at 16 x fsc.
#[cfg(not(test))]
pub mod pal_rf16;

// The root is the 525-line engine, so every existing path keeps working.
pub use ntsc::*;
