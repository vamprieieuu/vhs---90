# Camera pickup tubes

By default the video that goes into the recorder is taken as it is. With a camera tube on,
it is first treated as what a three-tube Plumbicon camera made of the scene: each channel's
target holds charge across frames, so moving highlights leave lag and, when they are bright
enough, comet tails. The tube physics is in `tube.rs`; the scanned target stage is in
`camera.rs`. See `docs/design/CAMERA_SCAN.md` for the design contract and
`docs/CAMERA_RESEARCH.md` for the underlying data and measurements.

## Chain

The input pictures are the camera's gamma-corrected output, held over their own intervals
(for `push(f)`, both fields see `f`; for `push_fields(f0, f1)`, field 0's half-frame
sees `f0` and field 1's half-frame sees `f1`). Per channel:

1. `v = rgb / 255`; scene light `L = oetf_inverse(v) * 2^iris_stops` (BT.709 transfer, so
   `L = 1` gives peak white at an iris of 0), giving photocurrent `peak * L^gamma` (the
   light transfer exponent of the data sheet).
2. Each target is divided into `ACTIVE_W` columns by `4 * ACTIVE_H` sub-rows (`SUB_ROWS = 4`),
   so the beam overlaps adjacent lines of the other field. Between beam visits, each
   sub-element integrates `(j_photo + j_dark + j_bias) * dt` over the held pictures covering
   its interval, up to the signal-plate limit `C * V_target`.
3. Rows are read in raster order - field 0 top-down, then field 1 top-down - with sample `x`
   of active row `r` in frame `n` read at its own time `read_time_s(n, r, x)`. Each row pass
   runs up to seven target-side steps: ACT flyback over a span of lines ahead of the read
   beam (`plumbicon-act`), dynamic beam control bisection per column (`plumbicon-dbc`),
   integration to the read instant, capture of the aim by the nearest element above the
   beam-bending onset when `blooming > 0`, a single landing per element across the row, sharing
   landed charge back to the aims' columns, and recording the column read current. When
   `blooming == 0` and neither ACT nor DBC is active, the column-by-column path produces
   identical output.
4. The charge landed across the pass's sub-rows is summed and divided by `SUB_ROWS * T_frame`
   to give the row signal current in nA, then convolved along the line with a horizontal
   Gaussian beam spot (`sigma_h = 40 ns`, renormalised inside the active line at the ends).
5. The line signal passes through the head amplifier and the processing amplifier
   (`camera_processing.rs`), then the three clipped gamma-corrected signals are encoded directly
   to baseband Y / C (`encode_from_rgb`).

The stage keeps the charge and last-read time of every sub-element between frames. It begins
with zero charge, then scans a dark column for 10 seconds before frame 0. It is built only when
a tube is selected, so with `tube = "off"` (the default) the picture is untouched and the
golden renders do not change. The same code runs at 525/60 and 625/50: read times, frame
period and sample rate come from the engine's raster constants.

Because the tube's exponent is 0.95 and not 1, and the beam spot has finite width, a still
scene is not returned identically even with the lag at rest: mid greys shift by a couple
of levels and fine spatial detail is softened slightly by the spot.

## Physics

Each element of the target is a small capacitor. Light discharges it through the
photoconductor between scans (in the model: adds charge, counted positive), and the scanning
beam recharges it to the cathode potential. The charge the beam delivers is the signal. Two
limits of the beam make the picture remember the past.

**Discharge (capacitive) lag.** Near the cathode potential only the faster electrons of the
beam can land, because their energies are spread thermally. The landing current falls
exponentially as the element approaches the cathode potential: `i = I_b exp(u / V_T)` for
`u <= 0`, and `i = I_b` above. An element left slightly charged after a scan gives up the
rest over the next scans. In target-equivalent charge `q = C u` (nA s), one scan of one
element is exact:

1. above the cathode potential the whole beam lands: `r1 = min(q, I_b)`, `q -= r1`;
2. with the beam charge `I_b - r1` left, below it: `exp(-q'/K) = exp(-q/K) + (I_b - r1)/K`.

`K = C V_T` is the charge constant in nA s and the signal is the charge removed. A scan leaves
the element slightly below the cathode potential, so a tube at rest holds negative charge;
the first rows of a new light are weak while that charge is worked off. A light bias adds a
steady charge that keeps elements off the slow part of the curve, which is how the XQ1410 cuts
the lag.

**Comet tails.** The beam is set to twice what peak white needs. A highlight beyond that
(the sun on chrome, a lamp, an overexposed face) leaves charge the scan cannot remove. It
accumulates until the element reaches the signal-plate voltage and photoconduction stops
(`C * 45 V`, the target plate limit), and after the highlight has gone it drains at the beam
current, reading above peak white (clipped white) until it is gone. A
longer or brighter highlight stores more; a stronger beam (`beam_ratio`) shortens it.

**Anti-comet-tail (ACT) gun.** During each line flyback (`ACT_FLYBACK_S = 8.0 us`), the ACT
flood beam covers a vertical band spanning `ACT_SPAN_PH = 1/30` of picture height
(`h_act = 8` active lines per field at 525, `9.6` at 625) with `ACT_BEAM_NA = 100 000 nA`
(`24.0 nA s` per flyback pass). Every sub-element in the band above the channel's ACT
pedestal (`ACT_LEVEL_V = [3.5, 7.0, 3.5] V`, or `4.83 / 8.05 / 4.62 nA s` for R / G / B) is
discharged down toward that pedestal at the flyback instant `t_fb`. Interior lines receive
multiple overlapping flybacks across adjacent field lines (`96 nA s` total over 4 passes,
clearing full target-plate charge above the pedestal), whereas the very first line of a field
receives only one flyback pass (`24.0 nA s`) unless already discharged earlier in the scan.

**Dynamic beam control (`plumbicon-dbc`).** A diode-gun Plumbicon (`V_T = 0.10 V`,
capacitance `1.7x` triode Plumbicon -> `2.346 / 1.955 / 2.244 nF` for R / G / B) adds
feedback that boosts the read beam on a column above the fixed quiescent `beam_ratio` when
stored charge in the scan footprint exceeds the nominal beam headroom (`DBC.headroom = 0.5`),
solving per column by bisection to `DBC_TOLERANCE_NA = 0.01 nA` up to `DBC.max_ratio = 17.0`
times peak signal current (`plate_nas = [105.57, 87.975, 100.98] nA s`). Above `17x`
highlights, the beam hits the diode-gun limit and leaves a comet tail.

**Beam bending and blooming.** When `blooming > 0`, an element at target potential
`V = q / C` above `V_BEND_ONSET_V = 10.0 V` deflects nearby scanning beams toward itself
within radius `rho(V) = min(blooming * (V - V_BEND_ONSET_V) / V_BEND_ONSET_V, BEND_RADIUS_MAX_LINES)`
(`3.5` frame lines at the `45 V` plate limit for `blooming = 1`, capped at
`BEND_RADIUS_MAX_LINES = 4.0` frame lines, metric aspect `TARGET_ASPECT = 4/3`), bounded
to sub-rows inside the read beam's vertical support and columns scaled by `sample_pitch_lines`
(`0.849` at 525, `1.038` at 625). Each column's beam lands at most once on the capturing
element (nearest in metric distance, ties broken by highest potential then upper-left), and
the resulting signal current is returned to every aimed column that was captured. Highlights
below `V_BEND_ONSET_V` (including white at normal exposure) do not bend the beam, while bright
overexposed spots pull charge from dark neighbours so the bloom grows with highlight level.

## Tube constants in seconds

The beam lands with an exponential energy distribution. Its temperature voltage is
`V_T = k T_b / e`: `0.17 V` for the triode guns with crossover (1900 to 2350 K, US
4,388,556), and `0.10 V` for a diode gun (about 1200 K, the same patent). All three currents
here use the triode value; the ACT gun is a triode. The fitted target capacitance is shared by
the XQ1020, XQ1410 and XQ1520 of each colour.

The scan charge constant is `K = C V_T` in nA s. The plate limit is `C * 45 V`; neither value
depends on frame rate. Capacitance was fitted on the 625/50 scanned target using the XQ1520 red
decay family against bias (least squares on log ratios) together with the 40 to 80 ms build-up,
and the 60 and 200 ms decay plus 60 ms build-up table figures for green and blue. The data-sheet
read is a uniformly lit column at `ACTIVE_W / 2`, with each active row's target-equivalent
current measured in nA by the same sub-row scan as the camera. See
[`docs/design/CAMERA_SCAN.md`](design/CAMERA_SCAN.md), “Tube constants in seconds”.

| Channel | `C` (nF) | `K = C V_T` (nA s) | `V_T` (V) |
|---|---:|---:|---:|
| R | 1.38 | 0.235 | 0.17 |
| G | 1.15 | 0.196 | 0.17 |
| B | 1.32 | 0.224 | 0.17 |

PbO blue's capacitance sits within 15 % of the red/green mean (1.265 nF); its extra lag comes from
two trap populations in the blue layer (see "Target materials and traps"), and a larger capacitance
plays no part. `plumbicon-dbc` uses the diode-gun value (`V_T = 0.10 V`) with `1.7x` larger
capacitance (`2.346 / 1.955 / 2.244 nF` for R / G / B) so `K = C * V_T` stays the same.

The following table records the lag checks from `tests/camera_lag.rs`; percentages are measured
at the indicated time after a light transition. Decay is relative to the signal above dark and
bias; build-up is relative to the final signal. Values are reported in measured (handbook) form.

| Check | Measure | Data (%) | Scanned model (%) |
|---|---|---:|---:|
| XQ1520 R, 0 nA bias | decay 60 / 80 / 120 / 200 ms | 13 / 9 / 5.3 / 3.3 | 14.67 / 10.83 / 6.80 / 3.46 |
| XQ1520 R, 1 nA bias | decay 60 / 80 / 120 / 200 ms | 10 / 7.3 / 3.2 / 1.5 | 12.61 / 8.84 / 4.99 / 2.02 |
| XQ1520 R, 3 nA bias | decay 60 / 80 / 120 / 200 ms | 8.3 / 5.5 / 2.4 / 0.6 | 9.18 / 5.73 / 2.53 / 0.59 |
| XQ1520 R, 8 nA bias | decay 60 / 80 / 120 ms | 3 / 1.9 / 0.25 | 3.84 / 1.69 / 0.35 |
| XQ1520 R, 0 nA bias | build-up 40 / 60 / 80 ms | 52 / 85 / 96 | 56.53 / 85.96 / 97.11 |
| XQ1020 / XQ1520 R | decay 60 / 200 ms; build-up 60 ms | 12.5 / 3.75; 85 | 14.67 / 3.46; 85.96 |
| XQ1020 / XQ1520 G | decay 60 / 200 ms; build-up 60 ms | 9.5 / 3.25; 95 | 12.31 / 2.65; 93.12 |
| XQ1020 / XQ1520 B | decay 60 / 200 ms; build-up 60 ms | 15 / 5.25; 70 | 18.60 / 5.82; 77.42 |
| XQ1410 R / G / B | decay 60 ms | 8 / 7 / 11 | 9.18 / 8.54 / 13.24 |
| XQ1410 G, 40/600 nA | decay 60 ms, no bias / 10 nA bias | 8 / 1–1.7 | 6.84 / 0.98 |
| XQ1020 G, 300/600 nA | decay 60 / 200 ms | 1.2 / 0.4 | 1.04 / 0.19 |
| 525/60 vs 625/50 | XQ1520 R decay 60 / 80 / 120 / 200 ms; build-up 60 ms | same-time curves within 12%; build-up within 10 points | passed within both limits |

The XQ1410 and high-key figures are predictions: the fit never saw them. The 525-line engine uses the
same constants and is checked at the same elapsed times.

## Target materials and traps

Four target materials join lead oxide (`plumbicon`, `plumbicon-bias`, `plumbicon-act`,
`plumbicon-dbc`): `vidicon` (Sb2S3), `newvicon` (ZnSe/ZnCdTe), `saticon` (Se-As-Te) and
`chalnicon` (CdSe). A camera with one of them carries three identical tubes behind the prism,
each at 200 nA peak white with a triode gun (`V_T = 0.17 V`). See
[`docs/design/CAMERA_SCAN.md`](design/CAMERA_SCAN.md), "Target materials and traps", and
[`docs/CAMERA_RESEARCH.md`](CAMERA_RESEARCH.md), "Six photoconductors differ in gamma, dark
current and lag mechanism" and "Traps add an exposure-dependent tail".

| Kind | `gamma` | Linearise | Peak (nA) | `V_target` (V) | `dark_25` (nA) | Doubling (deg C) | `C` (nF) | Sources |
|---|---:|---|---:|---:|---:|---:|---:|---|
| `plumbicon` (R / G / B) | 0.95 | no | 150 / 300 / 150 | 45 | 1.0 | none | 1.38 / 1.15 / 1.32 | Philips T10, Mullard 1987; no temperature law published |
| `vidicon` (Sb2S3) | 0.65 | yes | 200 | 30 `[est]` | 20 | 7.5 | 9.9 | Philips T12 / XQ1271, Panasonic WV-3110; `V_target` 10-60 V, doubling 7-8 deg C |
| `newvicon` (ZnSe/ZnCdTe) | 1.0 | no | 200 | 25 `[est]` | 5 | 7.5 | 7.6 | Philips T12, Matsushita S4075; `V_target` 10-35 V, doubling 7-8 deg C |
| `saticon` (Se-As-Te) | 1.0 | no | 200 | 50 | 0.215 | 9.6 | 0.80 | Hitachi H8397A (0.15 nA at 20 C, 2.7 nA at 60 C); layer 0.58-1.16 nF (Hitachi Hyoron 1976); `C` fitted to the BC4908 lag family |
| `chalnicon` (CdSe) | 0.925 | no | 200 `[est]` | 35 | 0.681 | 9.0 | 6.4 | Toshiba 1973 (gamma 0.90-0.95; dark 0.1 / 1 / 10 nA at 0 / 30 / 60 C) |

`dark_25` and the doubling step are set so the law `dark(T) = dark_25 * 2^((T - 25) / doubling)`
passes through the published points; `T` is `CameraParams::temperature_c` (deg C, default 25).
Dark current acts on the target like bias light, so a warm tube lags less. Every capacitance
except PbO red and green is fitted on the scanned target together with the traps; the Newvicon's
is more than twice PbO green's ("much larger target capacitance"). For `vidicon` the processing
amplifier raises the clamped, gained signal to `1 / gamma` (keeping its sign) before the knee, as
a vidicon camera's gamma corrector was set for its tube, so the overall law stays the 0.45 one;
lead oxide and the Chalnicon keep their small departure from 1.

**Traps.** Some layers hold part of the light they absorb in traps and give it back later. A
channel has up to `MAX_TRAPS = 2` trap populations on every sub-element, each with a capture
share `f` (0 to 1), a capacity `N` (nA s, counted over the whole target like the stored charge)
and a release time `tau` (s). Light, meaning photocurrent plus bias light but not dark current,
of current `j` fills a population as `dn/dt = f j (1 - n / N) - n / tau`: a share of the light is
caught while there is room, and what is caught leaks out at its own rate. Over each stretch in
which the light is constant (the same field pieces the light is integrated in) this has the
closed form `n(t) = n_inf + (n_0 - n_inf) exp(-b t)` with `b = f j / N + 1 / tau` and
`n_inf = f j / b`, and the element gains `j dt` less what the traps took, `sum(n_end - n_start)`.
Nothing is lost: everything trapped reaches the element later, which is the lag. Under steady
light a population holds `N j / (j + N / (f tau))`, so a small population is filled by a little
bias light and then adds no lag, while a large one stays proportional to the light and bias light
does not touch it. The traps settle with the target in the dark scan before frame 0, with any
bias light on. Only PbO blue, the Saticon and the vidicon have traps; PbO red and green
("negligible"), the Newvicon ("free of photoconductive lag") and the Chalnicon ("mainly
capacitive") have none.

| Channel | Population | `f` | `N` (nA s) | `tau` (s) |
|---|---|---:|---:|---:|
| PbO blue | fast | 0.12 | 0.37 | 0.066 |
| PbO blue | slow | 0.035 | 3.9 | 0.45 |
| Saticon | fast | 0.95 | 0.14 | 0.047 |
| Saticon | proportional | 0.027 | 20 `[est]` | 0.10 |
| Vidicon | one | 0.95 | 5.0 | 0.25 |

The blue tube's fast population fills within a few fields and gives the 50 ms excess of short
exposures; the slow one fills over about a second and keeps the 200 ms floor near 2 % that bias
light does not remove. The Saticon's fast population takes nearly all the light while it has room,
so a few nA of bias keep it full and its share of the lag is what bias light removes; its second
population is so large that it stays proportional to the light, and gives the tail of about 1.5 %
at 50 ms that the BC4908 keeps at 200 nA however much bias it gets. The vidicon's single strong
population carries most of its lag, and bias light cuts its 200 ms tail more than its first fields.

The measured lag beside each figure of `tests/camera_materials.rs` (lag in percent of the signal
above dark and bias, middle rows of the scanned target). The RCA exposure families run on a dark
target lit for the given number of fields at 30 nA with 5 nA bias and a 100 nA beam at 60 Hz;
the XQ1427B is a 2/3-inch tube, so for it blue's capacitance and trap capacities are scaled to
that scanned area.

| Check | Measure | Data (%) | Scanned model (%) |
|---|---|---:|---:|
| XQ1410B blue (20/300 nA, 50 Hz) | decay 200 ms at 0 / 1 / 2 / 3 / 4 / 5 / 8 / 12 nA bias | 5.2 / 3.6 / 2.9 / 2.6 / 2.4 / 2.2 / 2.1 / 2.0 | 5.82 / 4.64 / 3.87 / 3.37 / 3.05 / 2.84 / 2.49 / 2.28 |
| XQ1427B blue | decay 50, 200 ms after 2 / 4 / 16 / 64 / 256 fields | 1.9, 0.3 / 2.4, 0.45 / 3.4, 1.25 / 4.5, 1.95 / 4.7, 2.25 | 1.60, 0.21 / 1.96, 0.36 / 2.80, 0.93 / 3.56, 1.48 / 3.61, 1.52 |
| Saticon BC4391 (shape only) | decay 50, 200 ms after 2 / 4 / 16 / 64 / 256 fields; as shares of the 256-field reading | 1.5, 0.2 / 1.8, 0.25 / 2.3, 0.45 / 2.45, 0.5 / 2.55, 0.5; 0.59, 0.4 / 0.71, 0.5 / 0.90, 0.9 / 0.96, 1 / 1, 1 | 4.42, 0.15 / 4.75, 0.25 / 5.45, 0.47 / 5.56, 0.50 / 5.56, 0.50; 0.79, 0.30 / 0.85, 0.50 / 0.98, 0.94 / 1, 1 / 1, 1 |
| Saticon BC4908 (800 nA beam, 30 C, 60 Hz) | decay 50 ms at 30 / 50 / 200 nA with 0 nA; 5 nA; 10 nA bias | 12.4 / 8.7 / 3.6; 6.1 / 4.3 / 2.4; 2.9 / 2.2 / 1.85 | 12.08 / 8.00 / 3.06; 5.48 / 4.07 / 2.25; 2.96 / 2.54 / 1.93 |
| Saticon H8397A (200 nA, 3.5x beam, 30 C, 60 Hz), for reference | decay 50 ms at 0 / 5 / 10 / 16 nA bias | 6.7 / 3.9 / 2.0 / 0.8 | 3.06 / 2.25 / 1.94 / 1.80 |
| Newvicon S4075 / T12 (200 nA, 25 C) | decay 50 ms (60 Hz) / 60 ms (50 Hz) | 10 / 8-13 | 10.93 / 9.17 |
| Newvicon at 45 C | decay 50 ms (60 Hz) | below 25 C by 1.2x | 6.05 (1.81x) |
| Chalnicon (200 nA, 25 C, 60 Hz) `[est]` | decay 50 ms | 10 | 10.28 |
| Vidicon (200 nA, 20 nA dark, 60 Hz) | decay 50 / 200 ms | 20 / 8 (5-12) | 15.98 / 5.32 |
| Vidicon, 20 nA bias | cut of the 50 / 200 ms lag | 13 to 9.2 (1.4x) / 3.3 to 1.2 (2.8x) | 1.30x / 2.15x |

The Saticon data do not agree with each other. In both the capacitive law and the traps, lag in
percent of the signal can only fall as the signal rises, but the H8397A at 200 nA lags more than
the BC4391 at 30 nA, and Hitachi's own 0.05 uA figure (2-3 %) is below its 0.2 uA one. RCA's
BC4908, the H8397A's equivalent, falls with signal as the law does (12.4 / 8.7 / 3.6 % at 30 / 50 /
200 nA), and so do three other measurements of the kind
([`docs/design/CAMERA_RETENTION.md`](design/CAMERA_RETENTION.md), "The Saticon's lag does not rise
with signal"). The Saticon is therefore fitted to the whole BC4908 family, every point within a
factor 1.18 (`C` 0.80 nF, inside the layer's 0.58-1.16 nF). The BC4391 family is judged by its
shape only, because its levels (about 2 % at 50 ms at 30 nA with 5 nA bias) are less than half the
BC4908's and its tube's size and gun are not known; the H8397A Fig. 6 levels are not judged. The
fit also meets the BC4390's limits it was not made on (2.20 % at 50 ms and 0.72 % at 150 ms with
6 nA bias, against at most 3 % and 1 %).

## Electronics

After the beam reads each row's current in nA, the line signal passes through the head
amplifier and processing amplifier (`camera_processing.rs`) in the order of US 4032974: head
amplifier (noise and band) -> clamp -> gain -> linearise (`vidicon` only) -> colour matrix (when
`spectral` is enabled on a profile with `matrix = true`) -> knee -> gamma -> aperture correction
with coring -> clip. See `docs/design/CAMERA_SCAN.md` ("Camera electronics" and "Spectral taking
and the colour matrix") for the contract and `docs/CAMERA_RESEARCH.md` ("Head-amplifier noise and
processing lie mostly above 2.5 MHz") for the measurements and sources. Flare and shading with
their corrections wait for the optics and geometry work (without the error they cancel, adding them
would only add a fault).

**Head amplifier.** The tube acts as a current source into the input capacitance `C_in` of
tube, coil, FET and strays. The amplifier adds a noise current of one-sided spectral density
`S(f) = 2 q I + 4 k T / R_f + (2 pi f C_in)^2 4 k T R_n` (A^2/Hz): white shot noise of the
sample's own current `I` (clamped at zero), white thermal noise of the feedback resistor `R_f`,
and the FET voltage noise (`R_n`) differentiated across `C_in`. Because the FET term rises in
amplitude with frequency (the "triangular" spectrum, US 4355335), 87.5 % of the FET noise
power in a flat 0 to 5 MHz band lies above 2.5 MHz; across 15 to 30 pF over 5 MHz this gives
0.55 to 1.6 nA rms, bracketing the "about 1 nA, as low as 0.5 nA" of good preamplifiers
(US 4704635). Noise is deterministic and independent per `(seed, frame, row, channel)` via
`rng::Rng` stream `STREAM_CAMERA = 61`. Signal plus noise then pass the amplifier's band
(`band_hz = 5.5 MHz`, flat to about 5 MHz, US 3743766): the magnitude of an analog second-order
Butterworth low-pass, `|H(f)| = 1 / sqrt(1 + (f / band_hz)^4)`, delay-compensated and realised as
a 21-tap symmetric linear-phase FIR whose line ends repeat their end sample.

**Clamp and gain.** Each channel subtracts its settled dark-equilibrium current (`black`) so a
capped camera averages zero, then divides by `peak_signal_na` (150 / 300 / 150 nA for R / G / B
in the 30 mm Plumbicon) so peak white is 1.0 in linear light `L`.

**Knee.** Applied per channel on the linear signal when a knee range `R` is set (US 4470067).
Values `L <= 1` are unchanged, and highlights above `1` follow `1 + (L - 1) s` with
`s = (gamma_inverse(WHITE_CLIP) - 1) / (R - 1)` so `L = R` reaches the white clip. Without a
knee, highlights pass unchanged to gamma and clip there.

**Gamma.** Each channel passes through the unclamped 0.45 law (`gamma_correct(L)`): `4.5 L`
below `0.018` (negative excursions included, multiplying low-key noise by 4.5) and
`1.099 L^0.45 - 0.099` above (EBU Tech 3335; equal to `tube::oetf` on `[0, 1]`).

**Aperture correction and coring.** Detail is extracted from the gamma-corrected green channel
and added equally to all three channels so edges receive no colour fringes. Horizontal detail
uses delay `T` along the line, `d_h(t) = s(t) - (s(t - T) + s(t + T)) / 2`, with fractional
delays interpolated by cubic spline and end samples repeated at the line ends (BBC Engineering
91). Vertical detail uses 1H delays within the same field,
`d_v(r) = s(r) - (s(r - 2) + s(r + 2)) / 2`, with rows that lack a neighbour using their own
value (BBC Monograph 47). Coring suppresses small detail below threshold `c`,
`core(d) = sign(d) max(|d| - c, 0)`, before adding `k_h core(d_h) + k_v core(d_v)`. The
horizontal gain `k_h` restores the green channel's combined horizontal spot `M(f)` and
head-amplifier response `|H(f)|` at `f_p = 1 / (2 T)`:
`k_h = (1 / (M(f_p) |H(f_p)|) - 1) / 2`.

**Clip.** Each channel is clamped to `[BLACK_CLIP, WHITE_CLIP] = [-0.03, 1.05]` of peak white
after the detail is added, so highlight overshoots are clipped at white while dark undershoots
remain down to `-0.03`.

**Profiles.** Selected by `CameraParams::processing` (config key `processing`, default
`studio`):

| Parameter | `studio` (three-tube broadcast) | `consumer` (home camera) | Source / notes |
|---|---|---|---|
| `C_in`, `R_n`, `R_f`, `T` | 20 pF, 75 ohm, 1 Mohm, 300 K | 30 pF, 100 ohm, 470 kohm, 300 K | Published ranges 15-30 pF, 50-100 ohm, 0.3-1 Mohm (`docs/CAMERA_RESEARCH.md`); consumer at the poor end (~45 dB at 300 nA, matching PK-958 "> 45 dB") |
| Band (-3 dB) | 5.5 MHz | 5.5 MHz | Flat to about 5 MHz (US 3743766) |
| Colour matrix | `true` (`T^-1`) | `false` `[est]` | US 4032974, Jones 1968; WV-3110 has no linear matrix |
| Knee range `R` | 2.3 | none `[est]` | 1984 studio practice: linear to 100 %, 230 % to white clip (US 4470067) |
| Aperture `T` | 150 ns | 200 ns | 150 ns (BBC Engineering 91); 0.2 us delay line (Panasonic) |
| Vertical gain `k_v` | 0.5 | 0 | `g = 0.25` (BBC Monograph 47) |
| Coring `c` | 0.008 `[est]` | 0.015 `[est]` | Threshold in gamma-corrected units |

## Spectral taking and the colour matrix

With `CameraParams::spectral = true` (config key `spectral`, default `false`) in three-tube mode,
each channel sees the scene's light through the lens IR-cut filter, its dichroic prism arm and its
photoconductor's spectral response (`spectral.rs`), and the processing amplifier applies the inverse
3x3 linear matrix (`camera_processing.rs`) when the profile enables it (`studio` yes, `consumer`
no). With `spectral = false`, both the taking matrix and the colour matrix are identity (and the
matrix multiply is skipped), so existing output is bit-identical. Stripe-layout spectra are out of
scope for this stage.

**Scene basis spectra and observer.** All spectra live on an 81-point wavelength grid from
`380 nm` to `780 nm` in `5 nm` steps (`N_LAMBDA = 81`, `LAMBDA_MIN_NM = 380.0`,
`LAMBDA_STEP_NM = 5.0`); light outside `380-780 nm` is not invented `[est]`. The scene light is a
sum of three non-negative primary spectra `E(l) = sum_c L_c E_c(l)`, formed from smooth shapes
`S_R = 0.0118 + 0.9882 long_pass(l, 619.0, 15.0)`, `S_G = exp(-((l - 539.1) / 33.2)^2 / 2)` and
`S_B = short_pass(l, 474.2, 18.8)` (`[DER]`, where `long_pass(l, e, w) = 1 / (1 + exp(-(l - e) / w))`
and `short_pass = 1 - long_pass`), scaled by `E_c(l) = Y_c S_c(l) / sum_i ybar(l_i) S_c(l_i)` with
`BT709_LUMINANCE = [0.2126, 0.7152, 0.0722]` `[DS]` and the Wyman, Sloan and Shirley 2013 (JCGT
2(2)) multi-lobe CIE 1931 2-degree fit `[DS]`. Their chromaticities sit at `(0.6400, 0.3300)`,
`(0.3002, 0.6001)` and `(0.1500, 0.0601)` against BT.709 `(0.64, 0.33)`, `(0.30, 0.60)` and
`(0.15, 0.06)`, summing to D65 white at `(0.3129, 0.3292)` against `(0.3127, 0.3290)`.

**Fitted prism and IR-cut `[est, fitted to DS]`.** The dichroic prism splits at `480 nm` and
`580 nm` with `5 nm` edges (`prism_R = long_pass(l, 580, 5)`, `prism_G = long_pass(l, 480, 5) *
short_pass(l, 580, 5)`, `prism_B = short_pass(l, 480, 5)`), and the lens IR-cut is
`short_pass(l, 680, 10)` (`680 nm` edge, `10 nm` wide). These widths and edges are fitted so the
Plumbicon camera's matrix comes within `0.075` in every term of the BBC optimum Plumbicon matrix
(`[[1.14, -0.18, 0.04], [-0.06, 1.23, -0.17], [-0.03, 0.02, 1.01]]`, Jones 1968 `[DS]`).

**Material responses.** Each material's relative spectral response is piecewise linear between the
`(nm, response)` knots below and `0` outside them; `[FIG]` points come from the published data-sheet
figures (`docs/CAMERA_RESEARCH.md`), and remaining points are `[est]` fills of the curve shapes:

| Material | Knots `(nm, response)` | Sources and notes |
|---|---|---|
| `Pbo` (standard PbO) | `380 0.55 [est], 400 0.70 [est], 450 0.92 [est], 490 1.00 [FIG], 550 0.72 [est], 600 0.35 [est], 630 0.10 [est], 650 0.00 [FIG]` | Philips T10 Fig. 1 (peak 485-500 nm, dark by 650 nm) |
| `PboExtendedRed` | `380 0.55 [est], 400 0.70 [est], 450 0.92 [est], 490 1.00 [est], 550 0.80 [est], 600 0.55 [est], 650 0.38 [est], 700 0.25 [est], 780 0.10 [FIG]` | Philips T10 Fig. 1 (sulphur-doped layer tailing to 850-950 nm); red channel of all Plumbicon kinds `[DER]` |
| `Sb2s3` (`vidicon`) | `380 0.40 [est], 400 0.55 [est], 450 0.80 [est], 550 1.00 [FIG], 600 0.88 [est], 675 0.50 [FIG], 700 0.36 [est], 750 0.12 [est], 780 0.05 [est]` | `docs/CAMERA_RESEARCH.md` (peaks near 550 nm, half peak near 675 nm) |
| `Newvicon` | `380 0.36 [est], 400 0.38 [FIG], 500 0.55 [est], 600 0.78 [est], 700 0.96 [est], 735 1.00 [FIG], 780 0.92 [est]` | Philips T12 Fig. 1 (220 of 575 mA/W at 400 nm, peak 730-740 nm) |
| `Saticon` | `380 0.65 [est], 400 0.80 [est], 430 1.00 [FIG], 450 1.00 [FIG], 500 0.82 [est], 550 0.62 [est], 595 0.40 [FIG], 650 0.12 [est], 700 0.03 [FIG], 750 0.00 [est]` | Hitachi H8397A Fig. 3 (100 % at 430-450 nm, ~40 % at 595 nm, ~3 % at 700 nm) |
| `Chalnicon` | `380 0.60 [est], 400 0.64 [FIG], 500 0.75 [est], 600 0.87 [est], 710 1.00 [FIG], 740 0.50 [est], 770 0.02 [FIG], 780 0.00 [est]` | Toshiba 1973 Fig. 6 (0.30 of 0.47 uA/uW at 400 nm, peak ~710 nm, near zero by 770 nm) |

`TubeKind::materials()` assigns `[PboExtendedRed, Pbo, Pbo]` to every Plumbicon kind, `[Sb2s3; 3]`
to `Vidicon`, `[Newvicon; 3]` to `Newvicon`, `[Saticon; 3]` to `Saticon`, `[Chalnicon; 3]` to
`Chalnicon`, and `None` to `Off`.

**Taking and colour matrices.** Channel `ch` weights wavelength `l` by
`tau_ch(l) = ir_cut(l) * prism_ch(l) * response_m(l)`. Integrating against each scene primary and
normalising each row to sum to `1` (white balance on `L = (1, 1, 1)`) gives the taking matrix `T`,
so the linear light on target `ch` is `l_ch = sum_c T[ch][c] L_c` before `photo_current`. The
processing amplifier applies `M = T^-1` (whose rows also sum to `1`) after clamp, gain and
vidicon linearising (`1 / gamma`) and before knee and gamma (US 4032974). Recorded contract values
(`docs/design/CAMERA_SCAN.md`):

- Plumbicon (`[PboExtendedRed, Pbo, Pbo]`):
  `T = [[0.846, 0.153, 0.001], [0.032, 0.803, 0.165], [0.016, 0.043, 0.940]]`,
  `M = [[1.191, -0.229, 0.039], [-0.044, 1.265, -0.222], [-0.019, -0.054, 1.073]]`.
  Without the red-extended layer (`[Pbo; 3]`), the red arm sees only 580-650 nm and requires a red
  row `[1.69, -0.83, 0.14]` `[DER]`.
- Red primary read on its own channel without a matrix (`T[R][R]`): Saticon `0.766`, Plumbicon
  `0.846`, Vidicon `0.854`, Chalnicon `0.898`, Newvicon `0.903`.
- The other materials, the same material on all three targets (`taking_matrix` / `colour_matrix`
  evaluated on the knots above, rows R / G / B) `[DER]`:

  | Tube | Taking matrix `T` | Colour matrix `M = T^-1` |
  |---|---|---|
  | `vidicon` (`Sb2s3`) | `[[0.854, 0.145, 0.001], [0.038, 0.830, 0.133], [0.016, 0.046, 0.938]]` | `[[1.179, -0.207, 0.028], [-0.051, 1.224, -0.173], [-0.018, -0.056, 1.074]]` |
  | `newvicon` | `[[0.903, 0.096, 0.001], [0.041, 0.835, 0.124], [0.016, 0.042, 0.941]]` | `[[1.113, -0.129, 0.016], [-0.051, 1.211, -0.160], [-0.017, -0.052, 1.069]]` |
  | `saticon` | `[[0.766, 0.232, 0.002], [0.033, 0.801, 0.166], [0.016, 0.037, 0.947]]` | `[[1.320, -0.385, 0.065], [-0.050, 1.273, -0.223], [-0.021, -0.044, 1.064]]` |
  | `chalnicon` | `[[0.898, 0.101, 0.001], [0.039, 0.828, 0.133], [0.016, 0.039, 0.945]]` | `[[1.120, -0.138, 0.018], [-0.050, 1.222, -0.172], [-0.017, -0.048, 1.065]]` |

  The Saticon's weak red (its response falls to about 40 % at 595 nm) needs the largest
  correction: its red row takes 0.385 of green back out.

**Checks (`tests/camera_colour.rs`).**
- Scene primaries: non-negative across `380-780 nm`, BT.709 `(x, y)` within `0.003`, luminance
  shares within `0.002`, and D65 white within `0.003`.
- Material curves: peak `1.0` (`<= 1e-9`), peak wavelengths within `+-10 nm`, data-sheet points
  within `0.03`, `Chalnicon` non-decreasing over `400-710 nm`, `PboExtendedRed >= Pbo` everywhere,
  prism isolation `<= 1e-6` at `400 nm` (red) and `700 nm` (blue).
- White balance: `T` and `M` row sums `= 1.0` and `M * T = I` within `1e-9`; `photo_currents` on
  white and grey (`0.5`) match `photo_current` within `1e-9`; `VideoProcessor` with matrix reads
  grey within `0.002`.
- Matrix necessity and mismatch: Plumbicon `M` within `0.10` of `BBC_PLUMBICON` and `T` within
  `0.07` of `BBC_PLUMBICON^-1`; standard `[Pbo; 3]` `M[R][G] <= -0.5`; every material's `M` has an
  off-diagonal term `>= 0.10`; without a matrix `T[c][c] <= 0.97` with primaries turning toward
  their spectral neighbours (red and green toward yellow, blue toward cyan); behind the PbO matrix,
  Newvicon red over-saturates by `>= 0.03` and Saticon red under-corrects by `>= 0.05`.
- Whole-camera bars (`BAR_LIGHT = 0.5`, 8 frames, inner rows, `BAR_MARGIN = 24`): spectral `read`
  agrees with ideal `read_currents` fed `T * L` within `1e-5` relative; Vidicon `VideoProcessor`
  restores saturated red within `0.002`; Studio Plumbicon restores colour bars within `0.02` of
  expected video and `0.03` linear light of the ideal camera, while Consumer leaves the unmatrixed
  taking mixture within `0.02`.

## Highlights

Contract and acceptance test bounds from `docs/design/CAMERA_SCAN.md` ("Highlights") and
`crates/vhsphys-core/tests/camera_highlights.rs`:

* `plumbicon-act`: `ACT_LEVEL_V = [3.5, 7.0, 3.5] V`, `ACT_BEAM_NA = 100 000 nA`,
  `ACT_FLYBACK_S = 8.0 us`, `ACT_SPAN_PH = 1/30` (`24.0 nA s` per flyback, `8` / `9.6` field
  lines at 525 / 625). Rated `32x` highlights (`ACT_RATED = 32.0`) leave no comet tail in
  interior rows (`< LAG_ONLY = 0.25` of peak white on the first dark frame), while low-key
  lag (`20 / 300 nA` at `50 Hz`) agrees with `plumbicon` within `SAME_LAG = 0.01`.
* `plumbicon-dbc`: diode gun `V_T = 0.10 V`, capacitance `2.346 / 1.955 / 2.244 nF` (R / G / B),
  `DBC.headroom = 0.5`, `DBC.max_ratio = 17.0`, `DBC_TOLERANCE_NA = 0.01 nA`. Matches plain
  Plumbicon linearity within `LINEAR_TOLERANCE = 0.01` up to rated exposure and low-key lag
  within `SAME_LAG = 0.01`, discharges a `16x` highlight (`DBC_RATED = 16.0`), leaves a comet
  tail at `32x` (`DBC_BEYOND = 32.0`), and keeps read current bounded within `DBC_MAX_TOLERANCE = 0.03`
  of `17.0x`.
* `blooming = 1.0`: `V_BEND_ONSET_V = 10.0 V`, `BEND_RADIUS_MAX_LINES = 4.0`, `TARGET_ASPECT = 4/3`.
  Peak white (`L = 1`) does not spread beyond `WHITE_SPREAD = 0.002`; a `16x` bar
  (`BENT_LIGHT = 16.0`) produces bloom `>= BLOOM_MIN = 0.25` of peak white at `BEYOND_SPOT = 3`
  samples outside the bar edge (inside the 3.5-line plate radius) while `BEYOND_RADIUS = 6`
  samples stays `< DARK_MAX = 0.02`; increasing a horizontal band from `MILD_LIGHT = 4.0` to
  `BENT_LIGHT = 16.0` expands outer-row bloom (`BLOOM_ROWS = 6`) by at least `BLOOM_GROWTH = 1.3`.
* Trail colour: under a `4x` highlight (`TRAIL_LIGHT = 4.0`), `plumbicon`'s clipped red tail
  outlasts green by at least `RED_OVER_GREEN_FRAMES = 2` frames; `saticon` at
  `beam_ratio = 3.5` (`SATICON_BEAM_RATIO`) and `3x` light (`SATICON_LIGHT`) for `2.0 s`
  (`SATICON_ON_S`) drains within `SATICON_GONE = 0.005` of baseline after `2.0 s` (`SATICON_GONE_S`).

## Single-tube stripe camera

With `CameraParams::layout = "stripe"` (default `"three-tube"`), a single pickup tube sees the
scene through an optical low-pass and cyan/yellow stripe filter on its faceplate, matching the
frequency-separation system of the Panasonic WV-3110 (`docs/design/CAMERA_STRIPE.md`). Any
`TubeKind` other than `off` may carry stripes; the one target is the kind's green channel
(`channels_at(beam_ratio, temperature_c)[1]`, with `bending` from `blooming`), held in slot
`STRIPE_SLOT = 1` of `TargetSignal` while slots 0 and 2 stay empty and are never scanned.

**Chain.** Each stage acts on the scene light, the target charge or the line signal (`stripe.rs`,
`camera.rs`, `camera_processing.rs`):

1. **Optical low-pass and stripes.** Linear scene light `L_c = oetf_inverse(v / 255) * 2^iris_stops`
   passes per active row through a birefringent plate (`optical_lowpass`), which averages two
   copies shifted along the line by half a carrier period `d = 1 / (2 f_c)` (`cos(pi f d)` is zero
   at `STRIPE_CARRIER_HZ = 3.58e6` Hz, from `w = 7.49 mm`, `p = 40 um`, `t = 52.3 us`). Each
   sub-row takes its row's light through cyan and yellow stripes of opposite slant (`T_G = 1`,
   `T_R = (1 + cos th_R) / 2`, `T_B = (1 + cos th_B) / 2` with `th = 2 pi f_c x / FS_BB + phi(r)`,
   `phi_R(r) = +pi r / 4`, `phi_B(r) = -pi r / 4`), summed by `target_light`.
2. **Photocurrent and scan.** `photo_current(ch, target_light / STRIPE_WHITE_LIGHT)` with
   `STRIPE_WHITE_LIGHT = 2.0` (the mean of `T_R + T_G + T_B`), so white averages the tube's peak
   signal current while its stripe peaks reach 1.5 times peak current. The scan engine (lag, traps,
   ACT flyback, DBC, beam bending and plate limit) runs unchanged on slot `STRIPE_SLOT = 1`.
3. **Head amplifier, clamp and scale.** `HeadAmplifier::amplify` adds shot, feedback and FET
   noise plus the 5.5 MHz Butterworth band to the one line signal; noise near 3.58 MHz becomes
   colour noise after decoding. Clamp and scale then subtract the dark equilibrium reading and
   restore the decoder scale: `s = (i - black) * STRIPE_WHITE_LIGHT / peak_signal_na`.
4. **Decoder.** `StripeDecoder` extracts chroma with a Gaussian band-pass around
   `STRIPE_CARRIER_HZ` (`CHROMA_HALF_WIDTH_HZ = 0.5e6` Hz at -3 dB) and trapped luminance
   (`LUMA_BAND_HZ = 2.8e6` Hz at -3 dB), separates red and blue carriers with a 1H delay (`r - 2`)
   and quarter-carrier delay `1 / (4 f_c)`, takes each envelope from its analytic signal (Hilbert
   FIR) times `carrier_gain` followed by a `COLOUR_BAND_HZ = 0.5e6` Hz Gaussian low-pass, and
   forms green and mixed highs (`G_c = y_c - (R_c + B_c) / 2`, `h = (y - y_c) / 2`).
5. **Processing amplifier.** `VideoProcessor::with_levels(processing, &[ch; 3], floor, white)`
   clamps each channel at the capped-lens `floor`, gains to the white card's `white`, and runs
   linearise (`vidicon`), knee, gamma, aperture correction from G and clip before encoding.

**Alignment.** The horizontal spot (`0.67` at `40 ns`), head-amplifier band (`0.92` at
`5.5 MHz`), vertical beam overlap of adjacent slanted lines (`~0.88`) and target transfer
exponent weaken the carrier. At construction, after dark settle and without changing the camera's
own state, a clone at `iris_stops = 0.0` reads `ALIGN_FRAMES = 8` frames of a flat 255 white card
(last read amplified as `ALIGN_WHITE_FRAME = -1`, bringing a vidicon's carrier response within
0.2 % of its 40-frame value), and a `TargetSignal` of `black` in slot `STRIPE_SLOT` is amplified as
`ALIGN_CAPPED_FRAME = -2`. Both are clamped, scaled and decoded with gain 1 over the middle half
(`ACTIVE_H/4 .. 3 ACTIVE_H/4`, `ACTIVE_W/4 .. 3 ACTIVE_W/4`) to give means `W1` and `C1`, setting
`carrier_gain = 1 / (((W1_R - C1_R) + (W1_B - C1_B)) / 2)`. Decoding both again with
`carrier_gain` gives `floor = C` and `white = W - C` per channel. Clamping R and B at `floor`
removes the envelope detector's rectified noise floor (~0.03 of white with `consumer`, which would
otherwise shift black toward magenta by about 30 levels of 255); as a consequence, weak carriers
lose chroma near the noise floor and dim greys lean green (~0.05 greener at 18 % grey, ~0.012 at
50 % grey, 0 at white).

**Carrier-loss green in highlights and trails.**
- **Highlights:** Two stops over white (`BEYOND_BEAM_STOPS = 2.0`) put every element past the
  beam (`2.0x` peak current), flattening the line signal at the beam current with no carrier: red
  and blue decode near 0, green decodes near 4, and the gamma-corrected picture turns green. One
  stop over (`PEAKS_BEYOND_STOPS = 1.0`) clips only the stripe peaks, stopping red and blue near 1
  where they would be 2.
- **Trails:** Because red and blue share one carrier on one target, their lag trails keep the same
  share of their lit reading (unlike three PbO tubes, where blue's traps hold longer). Because
  capacitive lag is longer at lower current, the stripe troughs linger more than the peaks, so the
  carrier decays before the luminance and a trail turns greener than the picture it follows (on a
  `vidicon`'s second dark frame, ~0.42 of its lit green against ~0.02 of its lit red and blue).

**Measured figures (`docs/design/CAMERA_STRIPE.md` and `tests/stripe_tube.rs`).**
- Alignment at 525 lines (`consumer`): `saticon` `carrier_gain` ~1.83 (`white` within 0.3 % of 1,
  `floor` ~0.028 in R and B and ~-0.028 in G; `tests/stripe_tube.rs` checks `white` within `0.03`
  of 1.0 and `floor[R/B]` in `(0, 0.1)`); `vidicon` `carrier_gain` ~2.8.
- Line-signal carrier on a `saticon` red field: weakened to `0.61` against a `0.67` spot response
  (checked within `0.75..=1.02` of spot response), with off-carrier (`+- 1.0 MHz`) `<= 0.03`.
- Grey, luminance and colour bars (`saticon`, `consumer`): white within `0.015`; 50 % grey channel
  spread `<= 0.025` and within `0.03` of `oetf(0.5)`; stripe vs three-tube luminance on 18 % and
  50 % grey within `0.01`; 75 % colour bars keep hue within `5.0 deg`, saturation ratio in
  `0.9..=1.05`, and linear luminance within `0.015`.
- Bandwidth (`saticon`, `stripe_colour`): 10-90 % colour step `0.4..=1.3 us`, luminance step
  `0.05..=0.35 us`, colour at least `2.5x` wider than luminance.
- Lag (`vidicon` dark frames 1 and 2 after 12 frames of 0.5 grey): `|R - B| <= 0.02` and
  `G - max(R, B) >= 0.1` (contrast: three-tube `plumbicon` has `B >= 1.3 * R`).
- Highlights (`saticon`): at 2 stops over white, decoded `R, B <= 0.1`, `G >= 3.0`, and
  gamma-corrected `G - R >= 0.5`, `G - B >= 0.5`; at 1 stop over, decoded `R, B <= 1.3`,
  `G >= 2.2`.
- The 625-line PAL engine (`4.95` samples per carrier cycle) runs the same alignment and meets the
  same white and 50 % grey bounds.

## Geometry

With `CameraParams::geometry = true` (default `false`; config key `geometry`), each frame runs
the profile's optical, deflection, shading, corner-focus and corrector parameters
(`Processing::geometry(kind)` in `geometry.rs`, `camera.rs` and `camera_processing.rs`; contract
in `docs/design/CAMERA_SCAN.md`, section "Geometry"; sources in `docs/CAMERA_RESEARCH.md`). When
`geometry = false`, `GeometrySpec::NONE` is used and the engine produces bit-identical output to
the non-geometry path. Positions `(x, y)` are measured from picture centre in picture heights
(`PH`), with `X_EDGE_PH = TARGET_ASPECT / 2 = 2/3`, `RHO_CORNER_PH = 5/6`, normalised axes
`u = x / X_EDGE_PH`, `v = 2 y`, and `picture_position_ph(r, s)`.

**Effects and sources.**
- **Deflection and three-tube registration.** A beam aimed at nominal `(x, y)` lands at
  `(x + dx_ch, y + dy_ch)` with `dx_ch = kappa (x^2 - X_EDGE_PH^2) / (2 X_EDGE_PH) + p x rho^2 + s_ch x + q_ch x rho^2`
  and `dy_ch = p y rho^2 + s_ch y + q_ch y rho^2` (green `s_G = q_G = 0`, red `(+s, +q)`, blue
  `(-s, -q)` `[est: the signs]`; scan-line timing scale `PH_PER_NS = 2.56e-5`). Target coordinates
  extend beyond the active picture on all four sides (`UniformColumn` has no geometry), with
  out-of-picture elements lit from their nearest active-picture sample `[est]`. For target column
  `j` on row `r`, the beam read time solves `x(s_j) + dx_ch(x(s_j), y_r) = x(j)` (to `<= 1e-6`
  sample), passes are centred at `c_r + dy_ch(x(s_j), y_r) ACTIVE_H` with per-column normalised
  vertical shares `w_rk(j)`, and the horizontal spot reads at continuous column
  `X(s) = s + dx_ch(x(s), y_r) ACTIVE_W / TARGET_ASPECT` scaled by speed `X'(s)`. Thus the line
  signal follows local scan gain `J_ch = (1 + d(dx_ch)/dx) (1 + d(dy_ch)/dy)` `[DS: Philips T10 4.2]`.
  Zones are defined as A (`rho <= 0.5`), B (`rho <= 2/3`), and C (beyond `2/3`) `[est: the research
  gives the registration bands by zone but not the zones]`.
- **Black shading.** Target dark and bias currents at `(x, y)` multiply by `1 + b(u, v)`, where
  `b(u, v) = h_saw u + h_par (u^2 - 1/3) + v_saw v + v_par (v^2 - 1/3)` has zero picture mean. Both
  profiles use `h_saw = v_saw = 0.0125`, `h_par = v_par = 0.075` (`+12.5 %` bottom-right, `+7.5 %`
  top-left, `-5 %` centre) `[DS: up to 12.5 % at corners, split est]`. The target starts at
  uniform dark equilibrium and settles over the first frames as a switched-on camera.
- **White shading.** Sensitivity before transfer multiplies target light by
  `1 + w_ch = 1 - fall_ch (rho / RHO_CORNER_PH)^2` (centre-high): Plumbicon kinds use
  `fall = [0.03, 0.03, 0.0]` `[DS: "a few per cent" in red and green, blue flat; 3 est]`; Saticon
  uses `0.15` `[DS: up to 15 %]`; vidicon, Newvicon and Chalnicon use `0.0` `[no data]`.
- **Veiling glare.** Light reaching target `ch` is `(1 - f_ch) l + f_ch (K * l)` with exponential
  kernel `K(d) = exp(-d / a_ch) / (2 pi a_ch^2)` over the active picture only. Fitted to the
  `BLACK_SQUARE_PH = 0.1` black-square chart (`1 % / 0.5 %` H8397A; `5 % / 1 %` T10) `[DER on DS]`:
  Saticon `f = 0.0215`, `a = 0.40 PH` (also used for PbO, Newvicon and Chalnicon `[est: no data]`);
  vidicon `f = 0.0116`, `a = 0.15 PH` `[DER]` (with transfer `0.65`, a shift-invariant veil gives a
  corner reading `>= 0.51` of the centre reading, leaving the vidicon's `1 %` corner open).
- **Halation (PbO kinds).** Total internal reflection from the faceplate front face adds
  `h_ch (K_h * l)`, where `h_ch = reflectance_ch (1 - 1 / n^2)`, `n = GLASS_INDEX = 1.5` `[est]`,
  `reflectance = [0.15, 0.08, 0.05]` `[est]`, `faceplate_mm = 1.5` `[est]`,
  `disc_mm = ANTI_HALATION_DISC_MM = 6.0` `[DS]`, `target_height_mm = 12.8` `[DS]`, and
  `K_h(d) = (1 + tan^2 theta_c) / (pi (2 t)^2) / (1 + (d / 2t)^2)^2` for `d >= 2 t tan theta_c`, 0
  inside `[DER]`. Without the 6 mm disc the ring starts at `0.21 PH`; the disc moves it to
  `1.05 PH` (past the `0.833 PH` corner). Optics act on each held field picture in order: taking,
  veil, halation, white shading, transfer (evaluated via padded 2D FFT on a `171 x 128` grid to
  well within `1e-4` of peak light).
- **Corner focus.** Both spot sigmas multiply by `F_ch = 1 + focus_ch (rho / RHO_CORNER_PH)^2`:
  vidicon and Newvicon `0.45` (`31 %` corner loss `[DS: 25-35 %]`); Chalnicon `0.25` (`20 %`
  `[DS]`); PbO kinds and Saticon `0.25` (`20 %` `[est: no corner figure; low end for studio
  tubes]`).
- **Processing amplifier and correctors.** After clamp and gain `L1 = (I - black) / peak`,
  correctors enabled by `ProcessingSpec::corrections` run before `linearise -> matrix -> knee ->
  gamma -> aperture -> clip`:
  1. Flare (`US 4001502`): `L2 = L1 - c_ch APL_ch`, where `APL_ch` is the frame mean of `L1` `[est]`
     and `c_ch` is `flare_reading[ch] / (1 - BLACK_SQUARE_PH^2 / TARGET_ASPECT)` measured at
     construction on the centre black-square chart at unit exposure.
  2. Black shading (`US 4032974`): `L3 = L2 - black_reference_na[ch] (J_ch (1 + b) - 1) / peak_ch`
     using the `25 deg C` dark-plus-bias reference, so temperature drift from `25 deg C` remains.
  3. White shading (`US 4032974`): `L4 = L3 / (J_ch (1 + w_ch)^gamma_ch)`.
- **Single-tube stripe camera.** `Layout::Stripe` forces `[Registration::NONE; 3]` (one target).
  Veil acts on scene RGB light before `optical_lowpass` and stripes; deflection, shading,
  halation and corner focus act on the target, lowering carrier modulation toward the corners.

**Profiles.**

| Parameter | Studio | Consumer |
|---|---|---|
| Deflection `kappa`, `p` | `-0.012`, `0.0155` (<= `0.48 %` in H circle, <= `0.90 %` outside; `[DS]` bounds `0.5 %`, `1 %`) | `-0.045`, `0.031` (`1.6 %`, `1.8 %`; `[DS]` bound `2 %`) |
| Registration `s`, `q` | `8.7e-4`, `3.2e-3` (<= `33 / 60 / 101 ns` in zones A / B / C; `[DS]` `25-40 / 40-80 / 80-120 ns`) | `1.12e-3`, `3.69e-3` (`40 / 72 / 120 ns` `[est]`) |
| Black shading | `h_saw = v_saw = 0.0125`, `h_par = v_par = 0.075` `[DS]` | `h_saw = v_saw = 0.0125`, `h_par = v_par = 0.075` `[DS]` |
| White shading, flare, halation, focus | tube kind's | tube kind's |
| Correctors | flare, black shading, white shading (`US 4032974`, `US 4001502`) | none `[est]` |
| Colour matrix | yes (with `spectral`) | no |

**Checks and bounds (`tests/camera_geometry.rs`).**
- Geometry off matches `GeometrySpec::NONE` bit for bit.
- Studio deflection stays within `0.005` inside `rho <= 0.5` and `0.01` outside (each >= half bound);
  consumer stays within `0.02` (corners >= `0.01`). Zone registration peaks sit inside
  `[25, 40]`, `[40, 80]`, `[80, 120] ns` for studio (`Consumer` at `<= 120 ns` in C).
- Beam landing placement matches `picture_position_ph` to `<= 0.1` sample horizontally and
  `<= 0.3` line vertically; non-linear sweep signal current tracks `1 + kappa x / X_EDGE_PH`
  (`kappa = -0.2`) to `0.5 %`, spot width scales inversely with speed to `0.02` of peak (`SAME_CHARGE = 0.01`),
  and R/B registration edges sit within `0.07` sample.
- Black shading reaches `+12.5 %` / `+7.5 %` / `-5 %` (within `0.01` after `15` frames); studio
  corrector flattens `25 deg C` dark shading within `1e-3` while temperature drift stays visible.
  White shading lowers corners by `2-5 %` (PbO R/G) and `10-15 %` (Saticon); studio corrector
  flattens white within `1e-3`.
- Flare black-square checks: Saticon `0.010` centre / `0.005` corner (within `0.0012`); vidicon
  `0.05` centre (within `0.005`, corner below `0.6x` centre); studio flare corrector brings centre
  black square within `0.0015` of black (corner `-0.009..-0.002`) and matches `c_ch` within `12 %`.
- PbO halation (without disc) produces a ring on `[0.23, 0.27] PH` matching derivation within `10 %`
  with red/green ratio matching reflectances within `5 %`; the `6 mm` anti-halation disc drops
  inner/ring flare below `1e-9`.
- Corner focus second-moment spot sigma matches the `F_ch`-broadened moving Gaussian within `2 %`,
  growing by at least `1.3x` at the corner (`Newvicon` `0.45`, corner limiting-resolution losses:
  vidicon/Newvicon `25-35 %`, Chalnicon `20 % +- 2 %`, studio `15-35 %`).
- Whole camera on a `0.5` linear grey field (`8` frames): studio corners match centre within
  `FLAT = 0.01` linear light; consumer left corners sit at least `0.03` above centre.

## Retention

Target retention, reversible burn, beam switching, lens-capped idle time and cold warm-up all
act per sub-element inside `integrate_sub_row` (`tube.rs`, `camera.rs`; contract in
`docs/design/CAMERA_RETENTION.md`, checks in `tests/camera_retention.rs`). Nothing is applied
to the picture after scanning. Channels with `Collection::FULL`, `Burn::NONE` and
`cold_offset_c == 0` (`newvicon` and `chalnicon` by default) run the unsplit rule bit-for-bit;
otherwise, when collection is not `FULL` and either held charge `h > 0` or
`q + (j + dark) dt > q_knee` (`q_knee = C (target_voltage_v - knee_v)`), each held piece of
length `dt` is cut into `n = min(max(1, ceil((j + dark) dt / (C HELD_STEP_V))),
ceil(dt / RETENTION_STEP_S))` equal sub-steps (`HELD_STEP_V = 0.5` V `[DER]`,
`RETENTION_STEP_S = 0.5e-3` s `[DER]`), advancing held charge by an exponential midpoint step
with collection taken at the charge predicted for the middle of the sub-step; `n = 1`
otherwise. Checked against a converged `1/16` ms reference (`tests/camera_retention_step.rs`),
readings stay within `3.0e-5` of peak white without bias light and `1.1e-4` with it (against
`1.9e-3` and `7.4e-3` for the old 0.5 ms step, and closer than the old step in every case
measured), at 11 to 21 sub-steps a frame for a lit lead-oxide element at peak white (against
about 70) and about 2.4 fps instead of 0.85 on the default camera `[DER]`. When geometry black
shading is enabled, `bias_na` and `dark_na_at(t)` both carry the element's `shade` factor.

**Held charge (`Collection`).** For lead oxide (all four Plumbicon kinds) and the Saticon,
the layer voltage `V_l(q) = target_voltage_v - q / (capacitance_f * 1e9)` (`layer_voltage_v`)
sets the carrier collection efficiency `eta(V)` (`0` at or below `zero_v`, `1` at or above
`knee_v`, `(1 - exp(-(V - zero_v) / scale_v)) / (1 - exp(-(knee_v - zero_v) / scale_v))`
between them). The uncollected share `(1 - eta) j` fills a held-charge population `h` (`0 <= h <= H`)
via `dh/dt = (1 - eta) j (1 - h / H) - h / tau_h` with `j = (p + bias_na * shade) * s`, and
what `h` takes is released back onto the element so total charge is conserved and passes through
the beam's bottleneck. `vidicon`, `newvicon` and `chalnicon` use `Collection::FULL`
(`knee_v = -inf`, `eta = 1` everywhere).

| Constant | `zero_v` (V) | `scale_v` (V) | `knee_v` (V) | `capacity_nas` (nA s) | `release_s` (s) | Sources |
|---|---:|---:|---:|---:|---:|---|
| `PBO_COLLECTION[R/G]` | 5 | 10 | 45 | 10 `[est]` | 2 `[est]` | RCA AN-5042 Fig. 2 red; XQ1020 high key (Philips 5b 1975) |
| `PBO_COLLECTION[B]` | 3 | 6 | 45 | 1 `[est]` | 2 `[est]` | RCA AN-5042 Fig. 2 blue ("almost negligible in the blue") |
| `SATICON_COLLECTION` | 8 | 8 | 38 | 14 `[est]` | 0.25 `[est]` | Hitachi Hyoron 1976 Fig. 8; US 4,330,733 Fig. 6 |

**Lead-oxide high key, highlight burn and erasure.** At low key (`20/300` and `40/600` nA,
50 Hz), `V_l` stays above `45 V` so lag matches `Collection::FULL` within `1e-3` relative
(`< 1e-4` on prototype). At high key (`300/600` nA, XQ1020), green decay at `60 / 200 ms`
rises from `1.04 / 0.19 %` to `1.23 / 0.42 %` (Philips 5b 1975 data `1.2 / 0.4 %`, checked
within factor `1.3`). A `20x` peak-white spot held `5 s` leaves a positive burn `3 s` later
primarily in red and green (`0.78 %` and `0.39 %` of peak; bound `>= 0.2 %` `[est]`) and less
in blue (`0.09 %`; bound `<= 1/3` of green). Exposing to `1x` white for `3 s` with the beam off
takes all elements to the `45 V` plate and saturates `h` uniformly, erasing the contrast
(`0.000 %` against `0.18 %`; bound `<= 10 %` `[est]`, RCA AN-5042).

**Saticon sticking and beam-off burn.** Normal `0.2 uA` white reaches `41.7 V`, above the
`38 V` knee, leaving under `2 %` (`0.99 %`) at the 10th field (`3.5x` beam, `30 C`).
After `20x` peak white (`4 uA` for `10 s`), the field collapses and the 10th-field residual is
`21.9 %` of `0.2 uA` (inside the `8` to `27 %` band of US 4,330,733 Fig. 6; ratio `>= 8`).
With the beam on, a `1x` spot for `10 s` leaves `0.16 %` of peak (`< 0.5 %`) `0.5 s` later;
with the beam off, elements climb to `50 V` and fill `h`, leaving `2.95 %` (`>= 1 %` and `>= 5x`
beam-on `[est]`, RCA 1978, US 4,492,982). Nothing makes lag rise with signal: at a fixed
`800 nA` beam (BC4908, Neuhauser 1978 Fig. 12), `50 ms` lag falls across `30 / 50 / 200 nA`
(`12.08 / 8.00 / 3.06 %`, data `12.4 / 8.7 / 3.6 %`; ratio `>= 1.7`, got `3.9`).

**Antimony trisulphide burn (`VIDICON_BURN`).** Only `vidicon` carries reversible burn
(`rate_per_s = 5.5e-6`, `recovery_s = 600.0` s, `dark_gain = 0.75`; `Burn::NONE` for all
other kinds). Burn `b` (`0 <= b < 1`) evolves per sub-step under photocurrent only via
`db/dt = r (p / peak_signal_na) (1 - b) - b / tau_b`, scaling light sensitivity by `1 - b_m`
and dark current by `1 + dark_gain * b_m` (midpoint `b_m = (b0 + b1) / 2`). Exposed to a
`3500x` `[est]` lamp (`60 W` frosted lamp at `152 cm`, `f/1.5`, US 3,546,520) for `120 s`,
sensitivity drops to `0.888` (`0.875 +- 0.03`) and dark current rises `+8.4 %` (`5` to `12 %`);
after `1800 s` (`30 min`) idle, both recover within `1 %` (`0.995` sensitivity, `+0.4 %` dark).

**Beam off, idle and warm-up.** `UniformColumn::set_beam_on` and `CameraTube::set_beam_on`
turn the beam off: read beam, ACT flyback, DBC bisection and beam bending all become zero
(`TargetSignal` and `UniformColumn` return `0` on every row) while elements still integrate
light and dark current up to `plate_nas`. `idle(seconds)` caps the lens for
`round(seconds * FRAME_RATE)` whole frames (`UniformColumn::frame` advances by `n`): with the
beam on, traps, `h` and `b` evolve with `p = 0` and charge/timestamps shift to dark equilibrium;
with the beam off, elements integrate dark current (in sub-steps of at most `1 s` when warming,
midpoint dark current, with held charge taking the same half-volt midpoint step) up to the plate. `CameraParams::cold_start_c: Option<f64>` (default
`None`) sets `cold_offset_c = cold_start_c - temperature_c` on every channel in
`CameraTube::seeded`. Session time `t` relaxes the faceplate offset as
`T(t) - T_op = cold_offset_c * exp(-max(t, 0) / WARM_UP_S)` with `WARM_UP_S = 600.0` s `[est]`,
evaluated by `TubeChannel::dark_na_at(t)` at each sub-step midpoint while the black clamp keeps
the switch-on (`t <= 0`) dark level. Checked on a `Newvicon` (`dark_doubling_c = 7.5 C`)
switched on at `10 C` in a `35 C` camera: dark readings match `dark_at(Newvicon, T(t))`
within `3 %` at `0`, `WARM_UP_S` and `2 * WARM_UP_S`, and `50 ms` `200 nA` lag stays within
`10 %` of the steady tube at `T(t)` with cold lag `>= 1.2x` warm lag.

The same sequence is available as settings (`tests/camera_switch_on.rs`; `docs/design/CAMERA_RETENTION.md`,
"Switching on as camera settings"). `CameraParams::idle_s` (config key `idle_s`, flag `--camera-idle`, default `0`) is the
lens-capped time with the beam on before the first frame: `CameraTube::seeded` ends with `idle(idle_s)`,
after the dark settle and the cold offset, so a cold camera warms under the cap. `beam_off_s` (config key
`beam_off_s`, flag `--camera-beam-off`, default `0`) holds the beam off with the lens open for the first
`round(beam_off_s * FRAME_RATE)` frames of the shot: the targets store the scene unread, and the read of
the first frame with the beam on lands what built up. A negative or non-finite value of either counts as
`0`, and a hand call of `set_beam_on` cancels a pending switch-on.

## Known limits

* Beam bending models electrostatic attraction from stored target potential toward elements above
  `10 V` within `4.0` frame lines; out-of-scope highlight and retention effects are not modelled:
  DBC feedback loop delay, beam spot growth with beam current, dynamic beam-current deflection
  shift, landing-energy displacement (approach vs departure asymmetry), cross-channel bending
  across separate prism targets, permanent burn damage thresholds, and burn-in kinetics for
  non-Sb2S3 materials (sign of Saticon/Chalnicon retained image vs target voltage).
* The capacitances are fitted at 625/50 and used unchanged at 525/60; elapsed-time lag is
  checked on both raster engines.
* Spectral taking and the colour matrix cover the three-tube prism cameras behind the fitted
  dichroic prism and IR-cut filter; stripe-camera spectra, illuminant shifts and metameric
  residuals outside the three primary spectra are out of scope.
* Lag vs temperature beyond dark current and beam temperature vs beam current ("reduced beam")
  are not modelled.

## Why tails end magenta

Each colour has its own fitted capacitance and the same signal-plate voltage. Red and blue
carry half the peak current of green with half the beam on them. Their higher stored overload
and capacitance make their tails longer, and blue adds photoconductive trap lag in its layer.
Green runs out first and the tail ends in red plus blue: magenta.

For the uniform-column highlight checks in `tests/camera_tube.rs`, a 4x highlight leaves
clipped-white tails of 1 frame on green, 4 on red and 6 on blue; at 8x and 100x, green has
1 frame in both cases because the plate limit caps the stored charge. The ACT gun leaves no
clipped-white tail on any channel.

## Why there is no phosphor stage

An earlier version also smeared the decoded picture with a CRT phosphor afterglow of 40 % per
frame. The P22 phosphors of a colour tube decay in microseconds (blue and green) to about a
millisecond (red), far below the 20 ms of a field, so a display does not leave visible trails
and the stage had nothing to model. The trails on old recordings are the camera's, and they
are on the tape.

## Using it

    vhsphys run in.mp4 out.mp4 --preset worn --camera-tube plumbicon --iris-stops 2

Command-line flags:

* `--camera-tube`: `off` (default), `plumbicon` (XQ1020), `plumbicon-bias` (XQ1410),
  `plumbicon-act` (XQ1520), `plumbicon-dbc` (diode gun with dynamic beam control, XQ3440),
  `vidicon` (Sb2S3), `newvicon` (ZnSe/ZnCdTe), `saticon` (Se-As-Te, H8397A) and `chalnicon`
  (CdSe).
* `--iris-stops <N>`: exposure of the camera tubes in stops (`0` is the light that gives peak
  white; `2` overexposes by a factor of four, so bright parts of the picture are beyond what the
  beam can take off and leave tails).
* `--camera-temperature <DEG_C>`: target temperature of the camera tubes in degrees Celsius
  (default `25`); warmer tubes have more dark current.
* `--camera-cold-start <DEG_C>`: faceplate temperature of the camera tubes at switch-on in
  degrees Celsius; the black creeps as they warm to `--camera-temperature` (`WARM_UP_S = 600` s).
* `--camera-beam-ratio <N>`: beam current as a multiple of what peak white needs (default `2`,
  the data sheet's); it must be above `0`. A weaker beam leaves more lag and longer comet tails.
* `--camera-blooming <N>`: beam-bending strength (default `0`, off; `0` or more). No measurement
  sets its scale (`docs/MEASUREMENTS_WANTED.md`).
* `--camera-idle <SECONDS>`: time the camera stands with its lens capped and the beam on before
  the shot (default `0`); a cold camera (`--camera-cold-start`) warms up under the cap.
* `--camera-beam-off <SECONDS>`: time the beam is held off at the start of the shot with the lens
  open (default `0`). The targets store the scene unread; the picture comes up with the beam, a
  Saticon burns and a lead-oxide burn is erased.
* `--camera-processing <PROFILE>`: camera electronics profile, `studio` (three-tube broadcast,
  default) or `consumer` (home camera).
* `--camera-layout three-tube|stripe`: how the camera sees colour, `three-tube` (a prism and a
  tube per colour, default) or `stripe` (one tube behind a colour stripe filter).
* `--camera-spectral`: take the scene through the camera's prism, infrared filter and target
  spectral responses, with its colour matrix, instead of ideal colour.
* `--camera-geometry`: give the camera its scan geometry (deflection, registration, black and
  white shading, lens flare and corner focus).

In a config file:

    [camera]
    tube = "plumbicon"
    iris_stops = 2.0
    beam_ratio = 2.0
    blooming = 0.0
    temperature_c = 25.0
    processing = "studio"
    layout = "three-tube"
    spectral = false
    geometry = false
    # cold_start_c = 10.0
    # idle_s = 0.0
    # beam_off_s = 0.0

In the desktop front end (`python -m vhsgui --page camera`), the **Camera** button in schematic
mode opens the camera page (`docs/design/SCHEMATIC.md`, "Page 3: The camera"). It draws the
camera chain ahead of the recorder (`lens` -> `split` -> `tubes` -> `head_amp` -> `processing`
-> `encoder` -> `tv`) with all thirteen camera controls (`IRIS`, `LENS CAP`, `LAYOUT`,
`SPECTRAL COLOUR`, `TUBE`, `TUBE TEMP`, `SCAN GEOMETRY`, `BEAM`, `BLOOMING`, `COLD START`,
`START TEMP`, `BEAM OFF`, `ELECTRONICS`). Switching `LAYOUT` switches the `split` and `tubes`
drawings between a prism with three tubes (`three-tube`) and a stripe filter with one tube
(`stripe`); while `TUBE` is `off`, the twelve other camera lines dim and are inert, and `START TEMP`
works only while `COLD START` is on.

The tube's memory starts after its dark settling scan when a session starts. A render that starts
part-way into a clip (`--start`) with a tube on therefore needs a few frames of pre-roll
before its first output frame to match a render of the whole clip; the lag of a long tail is
a few tens of frames.
