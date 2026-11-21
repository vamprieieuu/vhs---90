# CONTRACT

# Retention on the scanned target

Design of the Retention stage of the scanned camera tube (`docs/design/CAMERA_SCAN.md`, "Stages":
"Retention: trap memory and burn-in, recovery, warm-up and temperature"). It builds on the engine as
merged with "Highlights": every element of a target is integrated by one rule
(`integrate_sub_row` in `camera.rs`) wherever it is touched, by the read pass, by the ACT flyback or by
the bending pad, and this stage adds state to that rule only. Everything here happens on the target,
in time, as light reaches it and the beam reads it: a retained image is charge the layer gives back
late, a burn is a change in what the layer does with later light and dark current. Nothing is applied
to the picture, and the frame is still written last, after the recorder.

Evidence labels as in `docs/CAMERA_RESEARCH.md`: **[DS]** printed number or statement, **[FIG]** read
from a printed graph, **[DER]** arithmetic or inference made here, **[est]** chosen where the sources
give nothing. Numbers marked [DER] against the model were checked on a prototype of this design built on
the merged engine (`dbe0a41`).

## Sources (new for this stage)

- **RCA, "Camera-ready broadcast vidicon series" (1977)** (the book cited as RCA 1977 in the research):
  AN-5042 on Vistacons (PbO), its Fig. 2 (signal current against target voltage for blue and red
  light), and Neuhauser, SMPTE Journal 87, March 1978, reprinted there, with Figs. 11 and 12 (lag of
  the BC4907/BC4908 18 mm Saticons at 30, 50 and 200 nA against bias light) and the BC4390 data sheet.
  <https://frank.pocnet.net/other/RCA/RCA_CameraReadyBoroadcastVidiconSeries_1977.pdf>
- **Hitachi Hyoron 58 (4), 1976**, "Development of the Saticon": Fig. 8 (photocurrent of the
  SnO2/Se-As-Te junction against applied voltage, with and without Te), Fig. 9 and its text (200 nA,
  third field after cut-off, about 3 %), Fig. 10 (layer capacitance independent of voltage).
  <https://www.hitachihyoron.com/jp/pdf/1976/04/1976_04_15.pdf>
- **US 4,330,733** (Hitachi, 1982): Saticon "high light sticking", the residual at the 10th field after a
  4 uA (20 times normal) signal is cut off, as a share of the 0.2 uA reference, Fig. 6; the cause is
  carriers held in levels of the Te-doped layer. <https://patents.google.com/patent/US4330733A/en>
- **Hitachi H8397A data sheet (1977)**, Fig. 6 (lag at 50 ms against bias light at 0.2 and 0.05 uA p-p).
  <https://frank.yueksel.org/other/Hitachi/Hitachi_Saticon_H8397A_1977.pdf>
- **RCA handbook ch. 11** (camera tube performance), Figs. 11-61/11-62 (exposure families, the BC4391
  Saticon and XQ1427B) and 11-63 (lag improvements of a 25 mm Saticon at 50 nA, including "reduced
  beam"). <http://www.r-type.org/pdfs/pctdh.pdf>
- **US 3,546,520** (Sb2S3 burn-in), **US 4,492,982** (burn worse with the beam off), Philips Tech. Rev.
  1963, Mullard 1987, Philips T10/T12, as in the research.

## What the evidence says

### The Saticon's lag does not rise with signal

The research left one Saticon anomaly open: the H8397A's Fig. 6 gives 2 to 3 % at 50 ms for 0.05 uA but
6.7 % for 0.2 uA, unbiased, while the BC4391 family gives about 2 % at 30 nA [FIG]. The model, fitted to
the BC4391, reads 1.4 % at 200 nA. Three independent measurements of the same kind of tube disagree with
that Fig. 6 curve, and agree with the capacitive law:

| Source | Tube | Condition (60 Hz, 50 ms / 3rd field) | Lag |
|---|---|---|---|
| RCA, Neuhauser 1978 Fig. 12 [FIG] | BC4908, RCA's equivalent of the H8397A (interchange table in the same book) | 30 / 50 / 200 nA, no bias | 12.4 / 8.7 / 3.6 % |
| same [FIG] | same | 5 nA bias | 6.1 / 4.3 / 2.4 % |
| same [FIG] | same | 10 nA bias | 2.9 / 2.2 / 1.85 % |
| Hitachi Hyoron 1976, text [DS] | 18 mm Saticon | 200 nA | about 3 % |
| US 4,330,733 Fig. 6 [FIG] | Se-As-Te target, any LiF content | 0.2 uA | about 2.4 % |
| RCA BC4390 data sheet [DS] | 2/3 in Saticon | 200 nA, 6 nA bias | at most 3 % (and 1 % at 150 ms) |
| Hitachi H8397A Fig. 6 [FIG] | H8397A | 0.05 / 0.2 uA, no bias | 2.0 / 6.7 % |

The BC4908 family falls with signal as a beam-limited capacitor does (the absolute residual is nearly
the same, 3.7 / 4.4 / 7.2 nA, so the percentage falls), and at 200 nA it keeps a part that bias light does
not remove (1.85 % with 10 nA), the sign of a trap tail proportional to the signal [DER]. Every mechanism
proposed for a rise was checked against the H8397A numbers [DER]:

- **A larger charge constant at the larger signal.** The 0.2 uA curve needs `K` about 0.4 to 0.7 nA s
  (logistic law fitted at each bias), the 0.05 uA curve about 0.05 nA s unbiased, a ratio of about 10 for
  a signal ratio of 4. The layer's capacitance does not depend on voltage (Hyoron Fig. 10), and a beam
  whose temperature rises with its current (space charge in the crossover: RCA 1978, Kurashige 1978; RCA
  Fig. 11-63 shows "reduced beam" lowering lag) would need `V_T` to grow as about `I_b^1.7` if the beam
  was set in proportion to the signal; the RCA figure's own cut (D to E) corresponds to `K` falling by
  about 1.4.
- **A trap filled faster than linearly** (field-dependent capture, Poole-Frenkel emission, space
  charge). Trap release does not pass through the beam's bottleneck, so bias light barely cuts it (the
  PbO blue floor); the H8397A 0.2 uA curve is cut eightfold by 16 nA of bias. A bias-proof part larger
  than about 0.15 % at 200 nA already breaks the H8397A cut at 16 nA within the existing factor 1.5.
- **High-light sticking** (US 4,330,733) is real and is built below, but it starts only where the layer
  field collapses, beyond about 2.4 times the H8397A's peak white, well above 0.2 uA.

Decision: no mechanism makes the lag rise with signal, and the H8397A's 0.2 uA absolute level is **not**
restored as a test. The test file instead pins what the H8397A's RCA equivalent shows: at a fixed beam
the 50 ms lag falls from 30 to 50 to 200 nA, by at least half the BC4908 ratio. The Saticon constants
fitted to the BC4391 family and the H8397A's relative cut stayed with this stage; the lead has since
had the Saticon refitted to the whole BC4908 family (see "Decisions for the lead", item 2).

### Lead oxide holds carriers where its layer field falls

RCA AN-5042 explains PbO highlight burn by the layer's current-voltage characteristic: "if the
photoconductor is not saturated at the operating target voltage, image burn-in problems may occur,
particularly for specular highlights"; carriers "not recovered as signal current ... may be trapped and
released at other times and thereby increase the lag"; red light, absorbed through the whole layer,
saturates later than blue, so burn is "primarily in the red and green channels, and almost negligible in
the blue" [DS]. Philips (1963) describes the same collapse for an unstabilised element: "the photocurrent
then falls as the field across the layer collapses" [DS]. The RCA Fig. 2 curves give the shape [FIG]:

| Target voltage (V) | 10 | 13 | 15 | 20 | 25 | 30 | 35 | 40 | 45 | 60 | 95 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| blue light, nA | 175 | 195 | 210 | 230 | 238 | 245 | 246 | 248 | 249 | 252 | 256 |
| red light, nA | - | 150 | 172 | 205 | 225 | 233 | 240 | 248 | 251 | 258 | 270 |

The element's layer voltage is the target voltage less its own charge, `V_l = V_target - q / C`. At low
key the element swings under 1.5 V and stays where the curves are flat. At high key a 300 nA green
element swings about 7 to 10 V between reads [DER], into the part of the curve where red and green have
not saturated; the carriers not swept out are held and come back over seconds. This is the slow tail the
research could not find at low key: Philips is right that PbO's photoconductive lag is negligible there,
and it appears with signal. On the prototype it lifts the XQ1020 high-key 200 ms point from 0.19 to
0.42 % (data 0.4 %) and the 60 ms point from 1.04 to 1.23 % (data 1.2 %), and changes no low-key reading
by more than 1e-4 of itself [DER].

### The XQ1410 "biased" 200 ms column

The known limit "with light bias the 200 ms tail is under 1 % where the XQ1410 table shows 2 to 3.5 %" is
not a missing mechanism. The Mullard curves for the same tubes with 3 and 8 nA of bias read 0.6 % at
200 ms and 0.25 % at 120 ms in red [FIG], which the model meets (0.59 and 0.35 %); a floor of 2 % in red
would break them by a factor of 3 to 8. The table's 200 ms values match the 0 nA curves (red 3 against
3.3 %), so that column is the unbiased tail (research, "What the evidence means"). The stage adds no
low-key floor to red or green; the blue floor exists already (traps).

### Saticon high-light sticking and the beam-off burn

In the Saticon the field must "flatten out" the saddle of the Te-doped layer; "if the target voltage (or
the field) is not high enough, carriers trapped in this saddle cause unwanted image retention and lower
sensitivity" (RCA 1978) [DS]. Hyoron Fig. 8 shows the Te-doped junction's photocurrent rising from a
threshold of about 8 V and levelling above about 25 to 40 V [FIG]: normalised to 50 V, 0.21 at 10 V, 0.43
at 12.5 V, 0.64 at 16 V, 0.85 at 25 V, 0.96 at 40 V. Hitachi's measure of the effect is high-light sticking:
the residual 10 fields after a 4 uA signal (20 times normal) is cut off, as a share of the 0.2 uA
reference, 8 % with fluoride doping and 27 % without (US 4,330,733 Fig. 6) [FIG], against about 2.4 % at
the 3rd field for 0.2 uA. The same held charge explains RCA's two Saticon statements: a camera left with
an image on the target and the beam off burns it in, while with the beam on the tube "is resistant to
image burn-in for many minutes" (RCA 1978; US 4,492,982) [DS]: only an element the beam does not hold
down drops its field.

### Antimony trisulphide burn

US 3,546,520 measured temporary burn on a commercial Sb2S3 vidicon: two minutes of a 60 W frosted lamp
at 152 cm imaged through f/1.5 lowered the photosensitivity from 0.080 to about 0.070 uA/uW (-12 %) and
raised the dark current from 10 to nearly 11 nA; both returned to normal within 30 minutes [DS]. The
cause is a change in the layer's electron-trap spectrum, driven by light beyond about 0.7 um; a photoflash
leaves permanent damage (about 100 nA dark current against 1 nA) [DS].

### Warm-up and temperature

Dark current already follows a temperature law per material; it acts on the target as bias light does,
so a warm tube lags less (Philips T12, for the Newvicon) [DS]. No warm-up transient was found in any
source; cameras guarded the first seconds by keeping the iris shut for 15 s (Panasonic WV-3110), and RCA
fitted "a low power (0.6 watt) dark heater for fast warm-up" that "minimizes the effect of temperature on
the photoconductor" [DS]. RCA AN-5042 says PbO lags more above room temperature [DS], without numbers.

## Decisions

- **Held charge, a trap whose capture is the layer's collection deficit.** For PbO (all three colours)
  and the Saticon, the share of the light the layer field does not sweep out, `1 - eta(V_l)`, goes into
  a population of held carriers with a capacity and a release time, exactly like the existing traps
  (Langmuir filling, exponential release, charge conserved). Light the held population cannot take is
  collected as now, so the element still charges to the plate and every highlight behaves as merged.
  Collection is complete above a knee voltage, so nothing changes where the field stays high.
- **Collection laws from the published curves.** PbO red from RCA Fig. 2's red curve, PbO blue from its
  blue curve, PbO green as red (RCA groups red and green for burn; green light is absorbed deep in the
  layer as red is) [DER]. The PbO knee is the target voltage, so the law is the curve normalised there.
  Saticon from Hyoron Fig. 8 with the knee at 38 V, just below the 39.6 V its element reached at 200 nA
  with the merged capacitance (41.7 V after the BC4908 refit), so the Saticon's normal readings are
  untouched.
- **Reversible burn for Sb2S3 only.** A slow per-element state, driven by the light on the element and
  recovering exponentially, that lowers the element's photosensitivity and raises its dark current, fitted
  to US 3,546,520. Other materials have no burn data and get none (`Burn::NONE`).
- **Beam off and idle time.** The beam can be switched off (the RCA erasure and the beam-off burn need
  it), and the camera can wait with the lens capped and the beam on for a time without reading frames
  (recovery over 30 minutes).
- **Warm-up as a faceplate temperature that relaxes to the operating one,** with an assumed time
  constant; dark current follows it through the existing law.
- **No superlinear lag** (see above).

## Equations

Per channel, per sub-element `(x, k)`, two new state variables beside the trap occupancies: the held
charge `h` (target-equivalent nA s, `0 <= h <= H`) and the burn `b` (dimensionless, `0 <= b < 1`). Both
start at 0 and are settled with the dark settle before frame 0 (light 0, so they stay 0 unless a test sets
otherwise). `CameraTube` copies the settled column to every column as it does for traps.

### Layer voltage and collection

```
V_l(q)   = target_voltage_v - q / (capacitance_f * 1e9)          (q in nA s; above the plate never)
eta(V)   = 1                                                      V >= knee_v
         = (1 - exp(-(V - zero_v) / scale_v)) / (1 - exp(-(knee_v - zero_v) / scale_v))   zero_v < V < knee_v
         = 0                                                      V <= zero_v
```

`Collection::FULL` has `knee_v = -inf`: `eta = 1` everywhere and no held charge.

### One held piece

Within a held piece of length `dt` (the field pieces of `integrate_sub_row`), with photo current `p` (nA,
evaluated once in the piece as now) and light `j = (p + bias_na) * s`, where `s` is the burn's
sensitivity factor below:

1. **Sub-steps.** If the channel's collection is `FULL` and its burn `NONE` and `cold_offset_c == 0`,
   the piece is integrated by the merged rule bit for bit. Otherwise it is cut into `n` equal sub-steps
   of `ds = dt / n`, with `n` from "The held-charge step" below (one sub-step per half volt the layer
   can swing in the piece, at least 1, at most the 0.5 ms grid). Each sub-step `i` (`0 <= i < n`) has the
   mid time `t_i = t0 + (i + 1/2) ds`, where `t0` is the piece's start.
2. **Burn** (if not `NONE`), exactly over the sub-step `ds` with constant drive:
   `db/dt = r (p / peak_signal_na) (1 - b) - b / tau_b`, so `b1 = b_inf + (b0 - b_inf) exp(-beta ds)`,
   `beta = r p / peak + 1 / tau_b`, `b_inf = (r p / peak) / beta`. The sub-step uses `b_m = (b0 + b1) / 2`:
   sensitivity factor `s = 1 - b_m`, dark-current factor `1 + dark_gain * b_m`. The drive is the photo
   current only (bias light is not the lamp that burns). Without burn, `s = 1` and the factor is 1.
3. **Light and dark of the sub-step.** `j = (p + bias_na) s` and `d = dark(t_i) (1 + dark_gain b_m)`,
   with `dark(t)` the channel's dark current at session time `t` (the warm-up law below). Both are
   constant over the sub-step and are the only currents onto the element besides what traps and the
   held charge take and give back. (With geometry, `bias_na` and `dark(t)` carry the element's black
   shading factor, as they do in every other rule of the engine.)
4. **Traps** fill from `j` exactly over `ds` as now (with `j` including `s`). Per trap `p` with
   capture `c > 0`, capacity `N`, release `tau`: `beta_p = c j / N + 1 / tau`, `n_inf = c j / beta_p`,
   `e_half = exp(-beta_p ds / 2)`, `n1 = n_inf + (n0 - n_inf) e_half^2`. The sub-step's trap change is
   `trap_dn = sum (n1 - n0)` and its change to the half-step is `trap_dn_half = sum (n_inf - n0)
   (1 - e_half)` (needed by step 5 only).
5. **Held charge** (if not `FULL`), `dh/dt = (1 - eta(V_l(q))) j (1 - h / H) - h / tau_h`, by an
   exponential midpoint step: the held population is advanced exactly with `eta` frozen, and `eta` is
   taken at the element's charge predicted for the middle of the sub-step.
   - If `j == 0` the capture term vanishes whatever `eta` is: `h1 = h0 exp(-ds / tau_h)`, no predictor.
   - Predictor, with `eta` at the start charge `q`: `a0 = (1 - eta(V_l(q))) j`,
     `beta0 = a0 / H + 1 / tau_h`, `h_inf0 = a0 / beta0`,
     `h_half = h_inf0 + (h0 - h_inf0) exp(-beta0 ds / 2)`,
     `q_half = min(q + (j + d) ds / 2 - trap_dn_half - (h_half - h0), plate_nas)`.
   - Step, with `eta` at the predicted charge: `a = (1 - eta(V_l(q_half))) j`, `beta = a / H + 1 / tau_h`,
     `h_inf = a / beta`, `h1 = h_inf + (h0 - h_inf) exp(-beta ds)`.
   - `held_dn = h1 - h0`. The predictor's `h_half` and `q_half` are discarded; only `h1` is stored.
6. **Element.** `q <- min(q + j ds - trap_dn - held_dn + d ds, plate_nas)`. Bias light is in `j`.

Burn and traps do not depend on the element's charge, so for a constant light they are exact at any
step; only the held charge couples to `q` through `eta`, and the midpoint makes that coupling second
order in `ds` (halving the swing per step cuts the error about four times, measured below).

Charge is conserved: what the held population takes it gives back (`-held_dn` turns positive as it
empties); what it cannot take (`h` near `H`) is collected at once, so an overloaded element still
reaches the plate. Released carriers go onto the element and are read by the beam like light: a retained
image passes through the beam's bottleneck, as lag does.

### The held-charge step

The number of sub-steps `n` of a piece of length `dt` that leaves the merged rule (step 1 above). At the
piece's start the element holds `q` and `h0`, its burn is `b0`; `t_mid` is the piece's mid time (where
`p` is evaluated), and the estimates use the start burn: `j_e = (p + bias_na) (1 - b0)`,
`d_e = dark(t_mid) (1 + dark_gain b0)` (`b0 = 0` without burn). `C = capacitance_f * 1e9` nA s per V.

1. **Fixed step (numerical reference).** If `retention_step_s > 0` (a `TubeChannel` field, `0.0` for
   every kind): `n = max(1, ceil(dt / retention_step_s))`, for every piece of the channel that leaves the
   merged rule, whatever its collection, burn or charge. Nothing else uses it.
2. **Held regime.** Otherwise, if the collection is not `FULL` and either `h0 > 0` or
   `q + (j_e + d_e) dt > q_knee` (`q_knee = C (target_voltage_v - knee_v)`, the charge at which the layer
   reaches the knee; the same trigger as before):
   `n = min(max(1, ceil((j_e + d_e) dt / (C HELD_STEP_V))), ceil(dt / RETENTION_STEP_S))`.
   `(j_e + d_e) dt / C` is how far light and dark current alone would move the layer voltage in the
   piece, so each sub-step moves it by at most `HELD_STEP_V = 0.5` V. The maximum keeps an overloaded
   element (which reaches the plate within the piece and then stays there) from taking more sub-steps
   than the 0.5 ms grid, `RETENTION_STEP_S = 0.5e-3` s; without it a hundred times peak white would take
   about 740 (red) to 1760 (green) sub-steps a lit frame against the grid's 67 [DER].
3. **Otherwise** `n = 1` (the layer stays above its knee, `eta = 1`, and `h` stays 0; burn, traps and
   the warm-up's dark current are evaluated once per piece as before).

**Why half a volt** [DER, numerical]. The project holds the step to accuracy first: the rule must
come closer to the converged target than the first rule (0.5 ms, `eta` at the start) at every point
measured. One step per volt does not: at peak white, where a red or blue element
swings under a volt in most pieces and so takes one midpoint step per piece, its reading errs by
1.1e-4 (R) and 8.6e-6 (B) of peak white against the first rule's 4.2e-5 and 3.6e-6, and its held
charge by 9.1e-4 against 3.9e-4 of the capacity. At half a volt every case below is closer than the
first rule in every quantity (or the same, where both take the same single step).

**Accuracy** [DER, numerical]. The reference is the same scheme at a fixed step (`retention_step_s`).
At 1/16 ms it is converged: 1/64 ms moves no row by more than 1e-7 of peak white, and the first rule's
scheme at a fixed 0.01 ms lies within 4e-5 of it (first order, as expected). A fixed 0.25 ms is within
1.2e-6 of peak white (rows) and 2e-5 of the capacity (held charge) of the 1/16 ms reference, so it
serves as the tests' reference. Against the 1/16 ms reference, over 40 frames of nine scenes on one
uniformly lit column (a tenth of peak white, peak white, twice peak white, 16 and 100 times peak white,
each for 20 frames (16 times: 10) then dark; a 20 times spot of a tenth of the height for 30 frames,
then dark; static bars; a
moving edge at 1.5 times peak white; a moving grating) for Plumbicon R, G and B, the Saticon, the ACT
Plumbicon (G, three scenes), Plumbicon with bias light (R, two scenes) and the vidicon (peak white and
its lamp):

| Quantity | This rule, worst | First rule, worst | One volt per step, worst |
|---|---|---|---|
| Row reading, share of peak white | 3.0e-5 plain (R, twice peak white); 1.1e-4 with bias light (R, twice peak white, the frame the light goes out) | 1.9e-3 plain; 7.4e-3 with bias light | 1.1e-4; 3.8e-4 |
| Held charge, share of its capacity | 3.0e-4 (G, bars) | 8.3e-3 | 1.5e-3 |
| Trap occupancy, share of its capacity | 5e-9 (vidicon) | the same | the same |
| Burn, absolute | 1.3e-12 (vidicon under the lamp, burn 0.025) | the same | the same |
| Stored charge, share of the plate | 1.3e-5 (Saticon, 16 times) | 8.1e-4 | 1.7e-5 |

Case by case the rule's error is below the first rule's in every scene with sub-steps; the closest
are the red grating (rows 2.4e-5 against 3.6e-5), red bias light at peak white (2.8e-5 against 4.5e-5)
and blue at peak white (1.7e-6 against 3.6e-6). Where neither rule sub-steps (low key; the Saticon at or
below peak white, whose layer stays above its knee; the vidicon, which has no held charge) both take
one step per piece and agree with the reference to rounding (3e-13 of peak white). The rule is second
order: on red with bias light at twice peak white, 1, 0.5 and 0.25 V per step give 3.8e-4, 1.1e-4 and
3.0e-5. The bounds of the tests (`tests/camera_retention_step.rs`) are, case by case, the rule's
measured error plus a quarter, each below the first rule's error in the same case.

**Cost** [DER]. Sub-steps per sub-element per frame on one column: one per piece (about 6 a frame) in
the dark or at low key, as before; at steady peak white 11.3 (R), 21.4 (G), 11.7 (B) against about 70
with the first rule (one volt per step: 7.8, 12.7, 7.9); in overload at most the 0.5 ms grid plus one
per piece (70.5), as before. A sub-step costs about 1.8 times an old one (four exponentials instead of
two). On the whole default three-tube Plumbicon camera (NTSC, 8 threads, a ramp scene with texture and a
moving highlight, measured on an implementation of this section): 8.5 s of CPU a frame becomes 3.2 s
(2.6 times less; one volt per step: 2.8 s), and a read takes 0.42 s instead of 1.1 to 1.2 s, 2.4 fps
instead of 0.85 (one volt: 2.6 fps). Another build ran on the machine at times; the CPU figures varied by
under 2 % between repeats.

### Warm-up

`TubeChannel.cold_offset_c` is the faceplate's temperature at switch-on less the operating temperature
(`0.0` for none). At session time `t` (s; before frame 0, `t` is taken as 0):

```
T(t) - T_op = cold_offset_c * exp(-max(t, 0) / WARM_UP_S)
dark(t)     = dark_na * 2^((T(t) - T_op) / dark_doubling_c)       (dark_na when dark_doubling_c is infinite)
```

`WARM_UP_S = 600` s [est: no warm-up transient is published; the order of a small camera head reaching
equilibrium with a heater]. Traps, held charge and burn have no temperature law (none is published). The
black clamp of the camera electronics keeps the reading of the target as settled at switch-on, so a
camera warming up shows its black rising by the change of dark current, as a camera without an
optical-black clamp did (Panasonic: dark shading changes with temperature [DS]).

### Beam off

With the beam off every pass lands nothing: the read beam, the ACT flyback and the dynamic beam control
are all zero, the frame's `TargetSignal` (or `UniformColumn` reading) is 0 on every row, and the target
integrates light and dark current to the plate. Bending captures nothing (there is no beam to bend).

### Idle

`idle(seconds)`: the lens is capped and the beam is on for `n = round(seconds * FRAME_RATE)` whole frames
that are not read out; the frame counter advances by `n`. Every element's slow states evolve exactly with
no light (`p = 0`): trap occupancies `n e^(-t / tau)`, held charge `h e^(-t / tau_h)`, burn
`b e^(-t / tau_b)`; what they release is read away by the beam. Each element's charge and last-landed time
are then those of the dark-settled column shifted by `n` frames (the beam has held it at dark equilibrium
at the temperature of switch-on; the first frames read after an idle settle it to the present dark
current). With the beam off, `idle` integrates as the skipped frames would, with no light: charge rises
by the dark current (sub-steps of at most 1 s when warming, midpoint dark current) up to the plate.
The idle time `dt` is one piece with `p = 0` (bias light only), integrated by the sub-steps of "One held
piece" (burn, light and dark, traps, held charge by the midpoint step, element), with `n`:
`max(1, ceil(dt / retention_step_s))` if `retention_step_s > 0`; otherwise `max(n_held, n_slow)`, where
`n_held = min(max(1, ceil((bias_na + dark(t0)) dt / (C HELD_STEP_V))), ceil(dt / RETENTION_STEP_S))` when
the collection is not `FULL`, `bias_na > 0` and either `h0 > 0` or `q + (bias_na + dark(t0)) dt > q_knee`
(`t0` the element's last-landed time), else 1; and `n_slow = ceil(dt / 1 s)` when warming
(`cold_offset_c != 0` with a finite `dark_doubling_c`) or burnt (`b0 > 0`), else 1. As before, once the
charge reaches the plate on a tube that is neither warming nor burnt, the rest of the time is one exact
step at the plate.

## Constants

In `tube.rs`:

| Constant | Value | Evidence |
|---|---|---|
| `PBO_COLLECTION[R]` | `zero_v 5, scale_v 10, knee_v 45, capacity_nas 10, release_s 2` | zero and scale through RCA AN-5042 Fig. 2 red (0.57 at 13 V, 0.89 at 25 V, 0.96 at 35 V against 0.60, 0.90, 0.96) [FIG]; knee at the 45 V target [DER]; capacity and release [est], fitted with the law to the XQ1020 high-key 1.2 / 0.4 % (Philips 5b 1975) [DS] |
| `PBO_COLLECTION[G]` | as red | [DER: RCA groups red and green; no green curve] |
| `PBO_COLLECTION[B]` | `zero_v 3, scale_v 6, knee_v 45, capacity_nas 1, release_s 2` | RCA Fig. 2 blue (0.70 at 10 V, 0.87 at 15 V, 0.94 at 20 V against 0.70, 0.84, 0.92) [FIG]; capacity a tenth of red's [est: "almost negligible in the blue", AN-5042] |
| `SATICON_COLLECTION` | `zero_v 8, scale_v 8, knee_v 38, capacity_nas 14, release_s 0.25` | Hyoron Fig. 8 Te-doped (0.22, 0.44, 0.64, 0.90 at 10, 12.5, 16, 25 V against 0.21, 0.43, 0.64, 0.85) [FIG]; knee below the 200 nA swing [DER]; capacity and release [est] fitted to US 4,330,733's 8 to 27 % (prototype: about 15 %; 21.9 % after the BC4908 refit) [FIG], release also within the RCA "about 2 s" for a residue [DS] |
| `VIDICON_BURN` | `rate_per_s 5.5e-6, recovery_s 600, dark_gain 0.75` | US 3,546,520: -12 % after 120 s, dark 10 to nearly 11 nA, normal within 30 min [DS]; recovery 600 s leaves 5 % after 30 min [DER]; rate for `b(120 s) = 0.12` at `BURN_LAMP_LIGHT` [DER]; `dark_gain = 0.09 / 0.12` [DER] |
| `WARM_UP_S` | 600 s | [est] |

In `camera.rs`: `HELD_STEP_V = 0.5` V, the layer swing per held-charge sub-step [DER, numerical: the
largest of 1, 0.5 and 0.25 V that is closer to the converged reference than the first rule everywhere,
"The held-charge step"]; `RETENTION_STEP_S = 0.5e-3` s, the
shortest sub-step the rule takes (the old fixed step, kept as the overload maximum) [DER, numerical].
Neither is a property of a tube, so neither is a measurement to be wanted.

Capacities are target-equivalent over the whole target, as trap capacities are: a target of another size
scales them with its area (the test helper `resized` in `camera_materials.rs` does so for traps).

The lamp of US 3,546,520 as a light on the model's target [est, DER]: a frosted 60 W lamp of about
5e4 cd/m^2 imaged at f/1.5 gives `pi L / (4 N^2)` about 1.7e4 lx on the faceplate; a home vidicon reaches
its 200 nA peak white near 5 lx (XQ1271, 344 uA/lm on 1.23 cm^2 [DER]), so the lamp is about 3500 times
peak white. The burn rate is fitted at that light; a scene's lamp burns at the rate its light gives.

## Interfaces

In `tube.rs`:

```rust
/// How completely the layer sweeps out its carriers as its field falls, and the carriers it holds
/// when it does not. `FULL`: complete collection, no held charge.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Collection {
    pub zero_v: f64,
    pub scale_v: f64,
    pub knee_v: f64,
    /// Held-charge capacity, target-equivalent nA s.
    pub capacity_nas: f64,
    /// Release time of the held charge, s.
    pub release_s: f64,
}
impl Collection {
    pub const FULL: Collection; // knee_v = f64::NEG_INFINITY, capacity 0, release 1
    pub fn is_full(&self) -> bool; // knee_v is not finite
    /// `eta(V)`, 0 to 1.
    pub fn efficiency(&self, layer_v: f64) -> f64;
}
pub const PBO_COLLECTION: [Collection; 3];
pub const SATICON_COLLECTION: Collection;

/// Reversible burn: drive per second per unit of `p / peak_signal_na`, recovery time, and the dark
/// current's relative rise per unit of burn.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Burn { pub rate_per_s: f64, pub recovery_s: f64, pub dark_gain: f64 }
impl Burn { pub const NONE: Burn; } // rate_per_s 0, recovery_s 1, dark_gain 0
pub const VIDICON_BURN: Burn;
pub const WARM_UP_S: f64 = 600.0;

// TubeChannel gains:
//     collection: Collection   // PBO_COLLECTION[k] for every Plumbicon kind (including -dbc),
//                              // SATICON_COLLECTION for Saticon, FULL otherwise
//     burn: Burn               // VIDICON_BURN for Vidicon, NONE otherwise
//     cold_offset_c: f64       // 0.0 from TubeKind; CameraTube sets it from CameraParams
//     retention_step_s: f64    // 0.0 from TubeKind (the held-charge step); > 0: fixed sub-steps
//                              // of that length, s (the numerical reference of the tests)
impl TubeChannel {
    /// `target_voltage_v - q / (capacitance_f * 1e9)`, V.
    pub fn layer_voltage_v(&self, q_nas: f64) -> f64;
    /// Dark current at session time `t_s` (the warm-up law), nA.
    pub fn dark_na_at(&self, t_s: f64) -> f64;
}
```

In `camera.rs` (both engines):

```rust
pub const RETENTION_STEP_S: f64 = 0.5e-3;
pub const HELD_STEP_V: f64 = 0.5;
/// State of one sub-element of a `UniformColumn`; 0 for the states its channel does not carry.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct ElementState {
    pub charge_nas: f64,
    pub traps_nas: [f64; MAX_TRAPS],
    pub held_nas: f64,
    pub burn: f64,
}
// CameraParams gains, with #[serde(default)] as the others:
//     cold_start_c: Option<f64>   // faceplate temperature at switch-on, deg C; None = at temperature_c
impl UniformColumn {
    /// Index of the next frame to be read.
    pub fn frame(&self) -> i64;
    pub fn set_beam_on(&mut self, on: bool);
    /// Lens capped, beam as set, for `seconds` (rounded to whole frames) without reading.
    pub fn idle(&mut self, seconds: f64);
    /// State of sub-row `k` (`0..SUB_ROWS * ACTIVE_H`) as last left by a read or idle.
    pub fn element(&self, k: usize) -> ElementState;
    /// Sub-steps taken by this column's reads in the pieces that leave the merged rule (the sum of
    /// every such piece's `n`) since construction, the dark settle excluded; 0 for a channel on the
    /// merged rule. `idle` does not count.
    pub fn held_steps(&self) -> u64;
}
impl CameraTube {
    pub fn set_beam_on(&mut self, on: bool);
    pub fn idle(&mut self, seconds: f64);
}
```

`CameraTube::seeded` sets `cold_offset_c = cold_start_c - temperature_c` on every channel when
`cold_start_c` is set.

### Switching on as camera settings

Two more `CameraParams` fields, `#[serde(default)]` as the others, make the switch-on sequence a setting
(`crates/vhsphys-core/tests/camera_switch_on.rs`):

- `idle_s: f64` (default `0`): lens capped, beam on, before the first frame. `CameraTube::seeded` ends with
  `idle(idle_s)`, after the dark settle and the cold offset, so a cold camera warms under the cap.
- `beam_off_s: f64` (default `0`): the beam is off, the lens open, for the first
  `n = round(beam_off_s * FRAME_RATE)` frames of the shot. The targets store the scene unread; the read of
  frame `n` is the first with the beam on, and it lands what built up (a Saticon burns, a lead-oxide burn
  is erased, as the beam-off tests above show).

A negative or non-finite value of either counts as `0`. Each is exactly the hand-driven sequence:
`idle(idle_s)` right after seeding, `set_beam_on(false)`, and `set_beam_on(true)` before reading frame
`n` of the shot. A hand call of `set_beam_on` cancels the pending switch-on. The per-element state may live in the trap-occupancy array widened to
`MAX_TRAPS + 2` or in its own arrays; either way it exists only for channels that need it (traps, a
collection that is not `FULL`, a burn), so the other kinds cost nothing.

## Defaults and what stays

- **Camera off** is untouched (golden test). `CameraParams::default()` has `cold_start_c: None`.
- **Newvicon and Chalnicon** have `FULL` collection, no burn and no warm-up by default: their pieces take
  the merged rule bit for bit.
- **Plumbicon kinds** take sub-steps only where an element's layer falls below 45 V within a piece. At the
  handbook's low-key conditions (20 to 40 nA, under 1.5 V of swing) the held charge stays below 1e-6 nA s
  and every reading of `camera_lag.rs` changes by under 1e-4 of itself [DER, prototype: the XQ1520 red
  family and green low key identical to four decimals]. The high-key test of `camera_lag.rs`
  (`lag_at_high_key_is_the_data_sheets`, factor 2.2 at 200 ms) moves from 0.19 to about 0.42 %, towards
  the data. Highlights: an overloaded element still reaches the plate; the held charge returns at most
  `capacity / release` (5 nA red and green, 0.5 nA blue) after it, below every threshold of
  `camera_tube.rs` and `camera_highlights.rs`; dynamic beam control's 16-times linearity is met within
  0.07 % after its 20 frames (the held population fills in a few frames and then gives back what it
  takes). The prototype ran `camera_tube`, `camera_lag`, `camera_materials`, `camera_highlights`,
  `camera_scan` and `camera_blooming` unchanged and they passed [DER].
- **Saticon** readings at or below 2.4 times its peak white do not reach the 38 V knee; the materials
  tests are unchanged. (After the BC4908 refit, 0.80 nF, peak white leaves the layer at 41.7 V.) Its 3-times highlight held 2 s (`camera_highlights.rs`) fills the held charge,
  which is gone (0.02 nA) 2 s later, as that test requires.
- **Vidicon** readings change by the burn of their own light: 5 s at peak white burns `2.8e-5`, a change of
  under 1e-4 in every existing reading [DER].
- No constant fitted before this stage changes: `TARGET_CAPACITANCE_F`, `PBO_BLUE_TRAPS`,
  `SATICON_CAPACITANCE_F`, `SATICON_TRAPS` and the rest stay as merged. (The two Saticon constants were
  refitted afterwards to the BC4908 family, decision 2.)
- Cost: sub-steps only on elements whose layer falls below the knee; burn adds one exponential per piece
  on vidicon elements. The expectation that a normal picture has few such elements was wrong for lead
  oxide: its knee is its target voltage, so every lit element, and every element that has ever held
  charge, takes the held-charge step. With the first fixed rule (0.5 ms sub-steps, `eta` at the start)
  that was about 70 sub-steps per lit element a frame and the default three-tube Plumbicon read about
  1 fps; the step of "The held-charge step" takes 11 to 21 at peak white and the camera reads about
  2.4 fps (see there).

## Tests (`tests/camera_retention.rs`)

| Test | Source | Tolerance | Prototype [DER] |
|---|---|---|---|
| `the_retention_constants_follow_the_published_curves` | RCA AN-5042 Fig. 2, Hyoron Fig. 8 [FIG] | the law within 0.06 of each normalised point; which kinds carry what | passes |
| `lead_oxide_holds_carriers_only_where_its_field_falls` | XQ1520 R family, XQ1020/1520 G low key (Mullard 1987) | identical to `FULL` within 1e-3 relative | identical to four decimals |
| `the_lead_oxide_high_key_tail_is_the_xq1020s` | Philips 5b 1975: 1.2 / 0.4 % at 60 / 200 ms, 300/600 nA [DS] | factor 1.3 each | 1.23 / 0.42 % |
| `a_saticon_sticks_after_twenty_times_peak_white` | US 4,330,733 Fig. 6: 8 to 27 % of 0.2 uA at the 10th field [FIG] | the band; 0.2 uA leaves under 2 % at the same time; ratio at least 8 [DER] | about 15 % against 0.5 % (refit: 21.9 % against 0.99 %) |
| `a_saticon_burns_with_the_beam_off_and_not_with_it_on` | RCA 1978, US 4,492,982 [DS] | beam on: under 0.5 % of peak 0.5 s after; beam off: at least 1 % and 5 times the beam-on value [est] | 0.17 % against 3.1 % (refit: 0.16 % against 2.95 %) |
| `the_saticon_lag_falls_with_signal_as_its_rca_equivalent_shows` | BC4908 Fig. 12, 30 / 50 / 200 nA at 800 nA beam [FIG] | strictly falling; 30 over 200 nA at least 1.7 (data 3.4) | 8.6 / 5.3 / 1.39 % (refit: 12.08 / 8.00 / 3.06 %, ratio 3.9; data 12.4 / 8.7 / 3.6) |
| `a_lead_oxide_highlight_burns_in_red_and_green_not_blue` | RCA AN-5042 [DS] | 3 s after a 20-times highlight of 5 s: red and green at least 0.2 % of peak [est, a few code values after gamma], blue at most a third of green's share | R / G / B 0.78 / 0.39 / 0.09 % |
| `white_light_with_the_beam_off_erases_a_lead_oxide_burn` | RCA AN-5042: "erased almost completely and almost instantly" [DS] | residual contrast under 10 % of the unerased one [est] | 0.000 % against 0.18 % |
| `an_antimony_trisulphide_burn_recovers_within_half_an_hour` | US 3,546,520 [DS] | sensitivity 0.875 within 0.03; dark rise 5 to 12 %; after 30 min under 1 % each | 0.888 and +8.4 %; then 0.995 and +0.4 % |
| `a_camera_warms_up_from_cold` | dark-current laws (Philips T12, research) [DS/FIG]; lag falls when warm [DS] | dark reading within 3 % of the law at `T(t)`; lag within 10 % of the steady lag at `T(t)`; warm lags less | passes |

The prototype (the merged engine with this section's rule in `integrate_sub_row`, `idle`, the beam switch
and the warm-up law) ran this file: all ten pass, 2.7 s together in release.

The longest, the vidicon burn, reads 120 s of frames (about 2.5 s); its recovery uses `idle`.

## Tests (`tests/camera_retention_step.rs`)

The held-charge step, on `UniformColumn`, against the same column with `retention_step_s = 0.25e-3`
(the reference of "The held-charge step"). Each case compares every row reading of every frame, and the
held charge, trap occupancies, burn and stored charge of every sub-row (`UniformColumn::element`) after
every frame. A case's bound on a reading, a held charge (share of `H`) and a stored charge (share of the
plate) is the rule's measured error plus a quarter, rounded up; the test also requires each error to be
below the first rule's error in the same case (the last column). Traps within 1e-8 of their capacity
and burn within 1e-12 everywhere. Lead oxide: light for 20 frames, then 10 dark.

| Case | Rule, measured [DER]: row / held / charge | Bound | First rule [DER] |
|---|---|---|---|
| Plumbicon R, peak white | 1.84e-5 / 1.42e-4 / 1.21e-6 | 2.3e-5 / 1.8e-4 / 1.6e-6 | 4.15e-5 / 3.93e-4 / 3.48e-6 |
| Plumbicon R, twice peak white | 2.88e-5 / 4.71e-5 / 1.54e-6 | 3.6e-5 / 5.9e-5 / 2.0e-6 | 1.87e-3 / 1.97e-3 / 1.05e-4 |
| Plumbicon G, peak white | 1.18e-5 / 1.85e-4 / 1.64e-6 | 1.5e-5 / 2.4e-4 / 2.1e-6 | 1.31e-4 / 2.24e-3 / 1.96e-5 |
| Plumbicon G, twice peak white | 9.82e-6 / 8.49e-5 / 1.75e-6 | 1.3e-5 / 1.1e-4 / 2.2e-6 | 8.21e-4 / 8.27e-3 / 1.08e-4 |
| Plumbicon B, peak white | 1.69e-6 / 1.24e-4 / 8.03e-8 | 2.2e-6 / 1.6e-4 / 1.1e-7 | 3.58e-6 / 3.61e-4 / 1.79e-7 |
| Plumbicon B, twice peak white | 3.69e-6 / 8.55e-5 / 1.77e-7 | 4.7e-6 / 1.1e-4 / 2.3e-7 | 1.35e-4 / 2.20e-3 / 6.29e-6 |
| Plumbicon with bias light R, peak white | 2.83e-5 / 1.53e-4 / 5.91e-7 | 3.6e-5 / 2.0e-4 / 7.4e-7 | 4.45e-5 / 4.14e-4 / 1.68e-6 |
| Plumbicon with bias light R, twice peak white | 1.11e-4 / 4.12e-5 / 4.39e-6 | 1.4e-4 / 5.2e-5 / 5.5e-6 | 7.35e-3 / 1.87e-3 / 2.57e-4 |
| Saticon G, 20 times peak white for 10 frames (US 4,330,733's drive), then 10 dark | 1.70e-5 / 2.64e-5 / 9.25e-6 | 2.2e-5 / 3.3e-5 / 1.2e-5 | 2.06e-4 / 2.12e-4 / 7.41e-5 |
| Vidicon G, the lamp of US 3,546,520 (3500 times peak white) for 60 frames, then 10 dark; the lamp must burn more than 1e-2 (it reaches 0.038) | 0 / - / 5.4e-11; traps 4.9e-9, burn 3.3e-13 | 1e-9 / - / 1e-10 | the same (no sub-steps in either rule) |

Tests: `lead_oxide_highlights_are_integrated_within_the_bound_of_the_converged_target` (the six plain
Plumbicon cases), `lead_oxide_with_bias_light_is_integrated_within_the_bound_of_the_converged_target`,
`a_sticking_saticon_is_integrated_within_the_bound_of_the_converged_target`,
`a_burning_vidicon_is_integrated_within_the_bound_of_the_converged_target`, and
`the_held_charge_takes_one_step_per_half_volt_of_swing`: `held_steps` over 20 frames of steady light
after 10. A Newvicon at peak white takes 0 (the merged rule). Plumbicon R, G, B at a tenth of peak
white take one step per piece (5.99 a frame, every piece swings under half a volt); at peak white at
least the swing a frame in steps, `(peak + dark) T_frame / (C HELD_STEP_V)`, and at most that plus one
per piece (measured R 11.34 for 7.30, G 21.39 for 17.47, B 11.65 for 7.63); at 100 times peak white at
most `T_frame / RETENTION_STEP_S` plus one per piece (70.47 against 72.72). The file runs in about 9 s
in release.

The existing camera tests (`camera_retention`, `camera_lag`, `camera_materials`, `camera_highlights`,
`camera_tube`, `camera_scan`, `camera_blooming`, `camera_colour`, `camera_electronics`,
`camera_geometry`) and `stripe_tube`, `stripe_camera`, `golden`, `field_sequential` and `pal` pass
unchanged with the rule, measured on an implementation of this section; no tolerance was changed.
The readings the retention tests report move by little: the XQ1020 high-key tail from 1.229 / 0.418 %
to 1.234 / 0.424 % at 60 / 200 ms (data 1.2 / 0.4 %, factor 1.3), the Saticon's sticking from 21.92 to
21.93 %, the bending test's bloom at 4 times peak white from 1427.1 to 1419.7 nA (growth to 16 times 1.72,
at least 1.3), the green low-key decay in its fourth decimal; the rest within their last printed digit.

## Decisions for the lead

1. **The H8397A's absolute level is not restored.** The proposed mechanisms cannot produce it together
   with the BC4391 family or the H8397A's own bias cut, and the H8397A's RCA equivalent contradicts it.
2. **Saticon refit, made.** The lead chose the BC4908 family (the H8397A's RCA equivalent, 30 / 50 /
   200 nA at 0 / 5 / 10 nA of bias, 800 nA beam) for the `saticon` kind. The estimate here was a larger
   charge constant (`K` about 0.17 nA s, `C` about 1.0 nF) and a bias-proof trap of about 1.5 % at
   200 nA. The fit on the scanned target, with the held charge and its 38 V knee in place, settled on
   `SATICON_CAPACITANCE_F = 0.80 nF` (`K` 0.136 nA s, inside the 0.58 to 1.16 nF data) and
   `SATICON_TRAPS` of a small fast trap that a few nA of bias keep full (capture 0.95, 0.14 nA s,
   47 ms) and a large trap that stays proportional to the light (capture 0.027, 20 nA s [est], 0.10 s),
   about 1.5 % at 50 ms [DER]; with `C` 1.0 nF the best grid missed by a factor 1.27. All nine points
   come within a factor 1.18 [DER]:

   | Bias | 30 nA | 50 nA | 200 nA |
   |---|---|---|---|
   | 0 nA | 12.08 (12.4) | 8.00 (8.7) | 3.06 (3.6) |
   | 5 nA | 5.48 (6.1) | 4.07 (4.3) | 2.25 (2.4) |
   | 10 nA | 2.96 (2.9) | 2.54 (2.2) | 1.93 (1.85) |

   (50 ms lag in %, model with the data in brackets; 60 Hz, 30 deg C.) Unfitted checks: the BC4390's
   at most 3 % and 1 % at 50 and 150 ms with 6 nA of bias read 2.20 and 0.72 % [DS]; sticking 21.9 %
   (8 to 27 %); beam-on and beam-off retention 0.16 and 2.95 %. As expected, the BC4391 levels are not
   met (4.4 to 5.6 % at 50 ms against 1.5 to 2.55 %), so `camera_materials.rs` judges that family by
   its shape only (each reading as a share of the 256-field one, within 1.7; the 200 ms tail grows
   3.3 times, data 2.5), and the H8397A bias test is replaced by the BC4908 family
   (`the_saticon_lag_falls_with_signal_and_bias_light_as_the_bc4908_shows`, factor 1.3, falling with
   signal and with bias). The H8397A Fig. 6 itself now reads 3.06 / 2.25 / 1.94 / 1.80 % at 0 / 5 / 10 /
   16 nA (data 6.7 / 3.9 / 2.0 / 0.8): the bias-proof trap the BC4908 needs at 200 nA is the floor that
   its 16 nA point does not show.
3. **The held-charge capacities and release times are [est]**, two numbers per material fitted to one
   datum each (PbO to the XQ1020 high key, Saticon to US 4,330,733's band). The PbO burn's lifetime
   (2 s) follows from that fit alone; no source times a PbO burn.
4. **The held-charge step, changed.** The first rule (0.5 ms sub-steps, `eta` at their start) made
   the default three-tube Plumbicon read about 1 fps, because lead oxide's knee is its target voltage
   (intended: the Mullard and RCA curves show incomplete collection below 45 V), so every lit element
   sub-steps. The lead approved the midpoint step of "The held-charge step". One volt per step, as
   first proposed, is the cheaper (2.8 s of CPU a frame against 3.2 s) but at peak white on red and
   blue it is further from the converged target than the first rule; half a volt is closer than the
   first rule everywhere measured, and was taken because accuracy comes first. Every existing test
   passes unchanged.

## Out of scope

- **Permanent damage** (photoflash on Sb2S3, the Newvicon's breakdown under the sun beyond f/11, the
  Saticon's 21,500 lx for 20 s): no dose can be recovered from the sources on the model's light scale.
- **The sign of a Saticon or Chalnicon retained image against target voltage** (RCA and Toshiba rules,
  opposite): the target voltage is fixed per kind; the held charge gives the positive image of a low
  field, and the opposite case (injection through lowered junction barriers, RCA 1978) has no magnitude.
- **Burn driven by infrared** as such: the scene is RGB, so the vidicon burn is driven by the light, with
  the infrared share of a tungsten scene inside the fitted rate.
- **Ageing** (PbO burn fading over hundreds of hours, blue slump, "spongy" red build-up after capping).
- **Lag against temperature** beyond dark current (PbO lags more when warm, RCA AN-5042, no numbers;
  trap and held-charge release have no temperature law).
- **Beam temperature against beam current** (RCA Fig. 11-63's "reduced beam"): one unquantified data
  point; it would change `K` with `beam_ratio` across every merged lag test.
- `docs/CAMERA.md`'s known limits and the GUI and command line (`cold_start_c`, beam switch) go with
  "Front ends".
