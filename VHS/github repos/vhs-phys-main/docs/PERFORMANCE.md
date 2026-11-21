# Performance

Measured on a 754x480 clip of 240 frames (`--audio none --crf 0`, preset `worn` unless stated) on an 8-core Ryzen 7 5800X (16 threads) and a 4-core Intel Core i3-14100F (8 threads): 27.6 fps on 8 threads and 31.7 fps on 16 threads on the 5800X at the default 12x RF rate, reaching 38.3 fps with `--batch 24` (and 35.2 fps on `pristine`). Exact sampling (see "Exact sampling" below) costs about 8 % at 12x. On one thread of the 5800X, the same preset runs at 5.5 fps (183 ms per frame). Generations multiply the cost: each generation adds the cost of a deck. A camera tube is slower still (see "The camera" below): about 2 fps with a Plumbicon.

| machine, threads | before | after |
|---|---|---|
| 5800X, 8 threads | 22.7 fps, 87.0 CPU-s | 27.6 fps, 71.5 CPU-s |
| 5800X, 16 threads | 23.4 fps, 130.3 CPU-s | 31.7 fps, 93.2 CPU-s |
| 5800X, 16 threads, `--batch 24` (before exact sampling) | | 38.3 fps |
| 5800X, 16 threads, `--batch 16` (before exact sampling) | | 37.4 fps |
| 5800X, 16 threads, `pristine` | 24.2 fps | 35.2 fps |
| i3-14100F, 8 threads | 16.5 fps, 99.7 CPU-s | 78.3 CPU-s (wall time not measured on a quiet machine) |

CPU-seconds include the two ffmpeg processes (about 4 s per 100 frames). Every speed change left the output the same bit for bit; exact sampling (below) changed it on purpose, and the frozen output was frozen again with it: `crates/vhsphys-core/tests/bit_identity.rs` freezes 31 cases (both rasters, every preset family, LP, EP, HQ, composite, automation, dubs; pictures and the signals inside the deck) and `crates/vhsphys-core/tests/schedule_invariance.rs` checks that batch size, thread count and starting frame do not change the picture.

## Where the time goes

Measured on one thread of the 5800X with `cargo run --release -p vhsphys-core --example stage_times -- worn` (183 ms per frame, 5.5 fps: the deck 37 %, the head 31 %, the record stage 22 %, the decoder 7 %):

| stage / step | ms | unit | notes |
|---|---:|---|---|
| record | 19.8 | per field | |
| head: transport warp | 5.0 | per field | |
| head: tape defects | 0.6 | per field | |
| head: lateral weights | 0.3 | per field | |
| head: own-track pulses | 5.8 | per field | |
| head: azimuth smoothing | 4.6 | per field | |
| head: neighbour pulses | 5.7 | per field | |
| head: noise and resonance | 7.2 | per field | |
| deck | 34.0 | per field | limiter and discriminator about 10 ms |
| decode | 12.9 | per frame | |
| picture tube | 1.1 | per frame | |
| encoding the source | 3.0 | per frame | |

The work is a chain of recurrences, each sample waiting on the one before, so it does not vectorise along time, and there is no hot spot left above 6 % of the time. Eight threads gave 4.6x one thread before this work; eight copies of a one-thread render run side by side took 1.38x as long each as one alone, because every stage streamed field-sized arrays (3 to 12 MB) through memory. The second thread of each core adds little.

## The camera

A camera tube ahead of the recorder costs more than the recorder: each colour channel has a target scanned on four
sub-rows per line, read element by element at its own time, with the charge and the beam solved in many small steps per
frame. CPU-seconds per frame for 30 frames of a 960x720 clip with the `worn` preset, the recorder included (the cost
without a camera is the first row), measured on a machine shared with other work, so the figures are good to perhaps 20 %:

| camera | CPU-s per frame | times the recorder alone |
|---|---:|---:|
| none | 0.5 | 1 |
| `plumbicon` | 4.5 | 9 |
| `plumbicon-act` | 10.4 | 21 |
| `plumbicon-dbc` | 4.2 | 8 |
| `vidicon` | 2.4 | 5 |
| `saticon` | 2.8 | 6 |
| `plumbicon`, `--camera-layout stripe` | 3.2 | 6 |
| `plumbicon`, `--camera-geometry --camera-spectral` | 5.2 | 10 |

A Plumbicon camera therefore runs at about 2 fps on 8 cores (`docs/CAMERA.md` gives 2.4 fps for the default camera), the
ACT gun at under 1 fps, and a vidicon or Saticon at 2 to 3.

The ACT gun costs most: each line's flyback is a further pass over the band of lines it floods. The held charge of lead
oxide and the Saticon is advanced in half-volt midpoint steps, which took a lit lead-oxide element from about 70 to 11-21
sub-steps a frame with the readings kept within `3.0e-5` of peak white of a converged reference (`docs/CAMERA.md`,
"Retention"). `newvicon` and `chalnicon` have no held charge, burn or cold offset by default and run the unsplit rule.
The camera runs in the first generation only, so `--generations` adds the recorder's cost for each further deck and
nothing more.

## What helped

- A batch runs as one graph of tasks (record a track, play back its RF, run the deck over a segment, decode and scan a frame), each started as soon as its inputs exist, so no worker waits at a stage boundary; the source frames are encoded in parallel and the command line tool writes its output on a thread of its own. Larger batches now help: `--batch 24` is about 12 % faster than the default 8 at 16 threads.
- The deck and the record stage take their signal through all their steps a few lines at a time, every filter and the discriminator carrying their state from piece to piece, so the working buffers stay in the cache.
- Independent filters share one pass over the signal; the luma FIR computes several outputs side by side; the pulse sum runs over a slice; the colour-under carrier comes from a table by column; line and column counters replace a division per sample.

## What did not help

- `-C target-cpu=x86-64-v3` (AVX2): bit-identical and 2 % faster.
- A faster sine that gives the library's result bit for bit (a polynomial with a test that falls back on the library when rounding could differ): exact, and twice as slow as the system library's sine on the baseline target.
- Rendering the playback head a few lines at a time as well: identical output, no measurable gain; the head is limited by arithmetic.

## The RF sample rate

`--rf-rate 8x|12x|16x` (default `12x`) sets how finely the tape signal is sampled between the record head and the demodulator (2, 3 or 4 RF samples per baseband sample at 525 lines; the 625-line engine uses the same samples per line, so its rates are 0.7 % lower). Measured on the 754x480 240-frame `worn` clip on the 8-core Ryzen 7 5800X: 33.8 fps at `8x`, 27.6 at `12x`, 23.1 at `16x` on 8 threads; 39.0 fps at `8x`, 31.7 at `12x`, 27.0 at `16x` on 16 threads. So `8x` is about a quarter faster and `16x` about a sixth slower. On a quiet deck, a flat grey's blue rms is 8.5 levels at `8x`, 2.5 at `12x` (the same at the top and bottom of the picture) and 1.8 at `16x`. On `pristine`, `8x` has about a tenth more colour noise than `12x` (18.2 against 16.6), with about a quarter more luma noise at 625 lines (3.36 against 2.72); its colours stay within about a level of `12x`. `16x` looks the same as `12x` on a real tape. The deck's alignment level differs by up to 9 % at `8x`, and its AGC takes it up.

## Exact sampling

With every noise source off, a flat grey used to come out with colour noise made by the arithmetic alone: blue rms 7.6 levels at 12x, 5.1 at the top of the picture and 11.4 at the bottom. Three causes were found and fixed, and the output changes as a result:

1. The playback head summed each pulse around a centre held in single precision, which is good only to 1/16 of a sample late in a track. Pulses were therefore misplaced more the further along the track they lay. The centre is now split into a whole sample and a fraction.
2. Each pulse was cut off sharply at the edge of its window, so its area and centre depended on where it fell between two samples. The window now tapers smoothly over its outer quarter.
3. The record head's reversal times were placed on a straight line between two samples of the record current. They are now placed on the cubic through four samples.

The deck's colour-under is also low-passed before it is taken down to the baseband grid, as a real deck filters it before anything is sampled.

Blue rms on a quiet deck: 8x 21.7 to 8.5, 12x 7.6 to 2.5 (now the same at the top and bottom), 16x 4.5 to 1.8. On a clean tape at 12x the colour noise fell from 19.2 to 16.6, which is the tape's own, and 12x now agrees with 16x (16.5). The colour bars are nearer the source: the largest error, the blue of the cyan bar, fell from 13 to 7 levels.

The sampling costs about 8 % at 12x. The check is `crates/vhsphys-core/tests/colour_noise_floor.rs`; the frozen output and the golden hash were frozen again with it.

## Candidates, not done

Each candidate below would change the output and needs a check that the result stays as close to the physics, along with a new frozen table; the percentages are the share of one-thread time each could save at most:

1. A faster Gaussian generator for the head noise: the noise and resonance step is 8 %.
2. The transport warp on a coarser grid: 5.5 %.
3. Opposite-azimuth neighbour tracks at reduced bandwidth: about 9 %.
4. The supply hum by rotating phasors: a few percent with the `worn` preset.

Real time at 60 fps on an 8-core desktop needs about 1.6x to 2x more; on a 4-core machine it needs the GPU backend.
