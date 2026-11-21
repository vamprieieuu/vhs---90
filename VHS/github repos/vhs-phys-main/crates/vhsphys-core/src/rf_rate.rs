//! How finely the tape signal is sampled.
//!
//! Everything between the record head and the deck's demodulator (the FM carrier, the
//! magnetisation reversals, the head's pulses, the limiter and the discriminator) is worked out
//! on a sample grid locked to the line: a whole number of RF samples to each sample of the
//! baseband grid (910 samples a line, 4 x fsc at 525 lines). The rates are named by their
//! multiple of the 525-line colour subcarrier; the 625-line engine uses the same number of
//! samples to each line, so its rates are 0.7 % lower. 12 x fsc is the reference the model's
//! figures were set at. A coarser grid is quicker and a little less exact (fewer samples to each
//! carrier cycle, coarser edges of the head's pulses and the tape's defects); a finer one is
//! slower and closer to the continuous signal. The baseband grid, the picture and the camera do
//! not change with it.
//!
//! The rate is a property of an engine: each engine is compiled for one rate, as it is for one
//! raster (`lib.rs`).

use serde::{Deserialize, Serialize};

/// Baseband samples per period of the 525-line colour subcarrier (the baseband grid is 4 x fsc).
const BB_PER_FSC: usize = 4;

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum RfRate {
    /// 8 x fsc: two RF samples to each baseband sample.
    #[serde(rename = "8x")]
    X8,
    /// 12 x fsc, the reference: three RF samples to each baseband sample.
    #[default]
    #[serde(rename = "12x")]
    X12,
    /// 16 x fsc: four RF samples to each baseband sample.
    #[serde(rename = "16x")]
    X16,
}

impl RfRate {
    pub const ALL: [RfRate; 3] = [RfRate::X8, RfRate::X12, RfRate::X16];

    /// The rate the model's figures were set at.
    pub const REFERENCE: RfRate = RfRate::X12;

    /// The name used on the command line and in config files.
    pub const fn name(self) -> &'static str {
        match self {
            RfRate::X8 => "8x",
            RfRate::X12 => "12x",
            RfRate::X16 => "16x",
        }
    }

    /// The RF rate as a multiple of the colour subcarrier.
    pub const fn fsc_multiple(self) -> usize {
        match self {
            RfRate::X8 => 8,
            RfRate::X12 => 12,
            RfRate::X16 => 16,
        }
    }

    /// RF samples to each baseband sample.
    pub const fn os(self) -> usize {
        self.fsc_multiple() / BB_PER_FSC
    }

    /// The rate named `name` (`8x`, `12x` or `16x`).
    pub fn from_name(name: &str) -> Option<RfRate> {
        RfRate::ALL.into_iter().find(|r| r.name() == name)
    }
}
