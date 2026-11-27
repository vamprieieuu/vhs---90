crates_rows = [line for line in source[crates_start:crates_end].splitlines() if line.lstrip().startswith("{")]
assert len(crates_rows) == 16, f"Crates table must contain 16 source presets, got {len(crates_rows)}"
assert all(len(line.strip().lstrip("{").rstrip(",").rstrip("}").split(",")) == 44 for line in crates_rows), "Every Crates preset must define all 44 mapped source values"
vhspro_cases = set(range(55, 65))
assert vhspro_cases.issubset(style_cases), f"Missing VHSPro recipes: {sorted(vhspro_cases-style_cases)}"
vhscraft_cases = set(range(65, 73))
assert vhscraft_cases.issubset(style_cases), f"Missing VHSCraft recipes: {sorted(vhscraft_cases-style_cases)}"
style_choices = re.search(r'kVhsStyleChoices\[\]\s*=\s*"([^"]+)"', source)
assert style_choices and len(style_choices.group(1).split("|")) == 32, "VHS Style popup must expose the curated 32 entries"
assert camera_cases == set(range(2, 13)), f"Missing VHS6RIFT and FXC6/Gen 7 camera profile cases: {sorted(set(range(2,13))-camera_cases)}"
camera_choices = re.search(r'kCameraProfileChoices\[\]\s*=\s*"([^"]+)"', source)
assert camera_choices and len(camera_choices.group(1).split("|")) == 12, "Camera Profile popup must expose Off plus 11 reference profiles"
assert ntsc_cases == set(range(2, 12)), f"Missing NTSC-RS-derived look cases: {sorted(set(range(2,11))-ntsc_cases)}"

osd_position = source.find("if(osdEnabled) {")
composite_position = source.find("if(compositeStages&&master>0.0f) {")
assert osd_position >= 0 and composite_position >= 0 and osd_position < composite_position, "OSD must be composited into the source before the VHS composite signal stage"

lost_start = source.find("static constexpr float kLostSignalProfiles[13][45]={")
assert lost_start >= 0, "Missing 13x45 Lost Signal VHS profile table"
lost_end = source.find("\n    };", lost_start)
assert lost_end > lost_start, "Unterminated Lost Signal VHS profile table"
lost_rows = [line for line in source[lost_start:lost_end].splitlines() if line.lstrip().startswith("{")]
assert len(lost_rows) == 13, f"Lost Signal VHS profile table must contain 13 complete profiles, got {len(lost_rows)}"
assert all(len(line.strip().lstrip("{").rstrip(",").rstrip("}").split(",")) == 45 for line in lost_rows), "Every Lost Signal VHS profile must define all 45 mapped values"
print(f"PASS: {len(active_indices)} AE parameter slots; UI order matches all disk IDs.")
print("PASS: Phase-1 VHS Style popup is curated to 32 non-duplicate entries mapped to source-backed recipes; the full source recipe tables remain in the engine for later source-specific preset groups.")
print("PASS: all 7 audited preset families have native menu entries; Crates maps 44 recovered source fields per look, while VHSPro/VHSCraft remain source-inspired where exact tuples were not recoverable.")
print("PASS: camera profiles, NTSC-RS controls, VHS playback, decoder, frame geometry, camcorder OSD and deterministic frame-repeat controls remain wired.")