//! Integration tests for time-varying parameter automation.

use vhsphys_core::automation::{AutomationSchedule, AutomationTrack};
use vhsphys_core::config;
use vhsphys_core::pipeline::{Chain, Session};
use vhsphys_core::video::{test_pattern, Frame};

fn mean_pixel_difference(a: &Frame, b: &Frame) -> f64 {
    assert_eq!(a.rgb.len(), b.rgb.len());
    let sum: f64 = a.rgb.iter().zip(b.rgb.iter()).map(|(&x, &y)| (x as f64 - y as f64).abs()).sum();
    sum / a.rgb.len() as f64
}

#[test]
fn automated_tracking_offset_progressively_degrades_picture() {
    let p = config::build(Some("good"), "", 1).unwrap();
    let mut schedule = AutomationSchedule::new();
    // Ramp tracking offset from 0.0 um (perfect alignment) to 25.0 um (severe mistracking)
    schedule.set_track("tracking_offset_um", AutomationTrack::linear_ramp(0.0, 0.0, 5.0, 25.0));

    let input: Vec<Frame> = (0..6).map(|_| test_pattern()).collect();
    let out = Session::run_with_automation(p, &input, schedule);
    assert_eq!(out.len(), 6);

    let pattern = test_pattern();
    // Frame 1 (near zero offset) should be close to reference
    let diff_early = mean_pixel_difference(&out[1], &pattern);
    // Frame 5 (severe offset) should be noticeably more degraded
    let diff_late = mean_pixel_difference(&out[5], &pattern);
    assert!(diff_late > diff_early + 2.0, "early diff: {diff_early}, late diff: {diff_late}");
}

#[test]
fn automated_head_clogging_attenuates_signal_progressively() {
    let p = config::build(Some("good"), "", 2).unwrap();
    let mut schedule = AutomationSchedule::new();
    // Progressive oxide buildup in head gap: 0.0 um -> 0.7 um extra spacing (Wallace spacing loss)
    schedule.set_track("head_spacing_um", AutomationTrack::smooth_ramp(0.0, 0.0, 5.0, 0.7));

    let input: Vec<Frame> = (0..6).map(|_| test_pattern()).collect();
    let out = Session::run_with_automation(p, &input, schedule);
    assert_eq!(out.len(), 6);

    let diff_between_start_and_end = mean_pixel_difference(&out[1], &out[5]);
    assert!(diff_between_start_and_end > 3.0, "clogging should alter output over time");
}

#[test]
fn automation_schedule_from_csv_and_chain_execution() {
    let csv = "\
frame, param, value, interp
0, tracking_offset_um, 0.0, linear
5, tracking_offset_um, 20.0, linear
";
    let schedule = AutomationSchedule::from_csv_str(csv).expect("valid CSV schedule");
    let p = config::build(Some("pristine"), "", 3).unwrap();
    let input: Vec<Frame> = (0..6).map(|_| test_pattern()).collect();

    let out = Chain::run_with_automation(vec![p], &input, schedule);
    assert_eq!(out.len(), 6);
}

#[test]
fn automated_crt_magnet_moves_deflection_around_screen() {
    let p = config::build(Some("pristine"), "", 4).unwrap();
    let mut schedule = AutomationSchedule::new();
    // Move magnet around the tube perimeter: angle sweeps 0 to 180 degrees at 18 cm distance
    schedule.set_track("crt_magnet_angle_deg", AutomationTrack::linear_ramp(0.0, 0.0, 5.0, 180.0));
    schedule.set_track("crt_magnet_cm", AutomationTrack::constant(18.0));

    let input: Vec<Frame> = (0..6).map(|_| test_pattern()).collect();
    let out = Session::run_with_automation(p, &input, schedule);
    assert_eq!(out.len(), 6);

    // Frame at angle 0 vs frame at angle 180 will have displaced phosphors on opposite edges
    let diff = mean_pixel_difference(&out[1], &out[5]);
    assert!(diff > 5.0, "moving magnet should distort different screen regions: diff = {diff}");
}

/// Mean absolute difference of two frames over rows `y0..y1`, in levels.
fn mean_row_difference(a: &Frame, b: &Frame, y0: usize, y1: usize) -> f64 {
    let (lo, hi) = (y0 * a.w * 3, y1 * a.w * 3);
    let sum: f64 = a.rgb[lo..hi].iter().zip(b.rgb[lo..hi].iter()).map(|(&x, &y)| (x as f64 - y as f64).abs()).sum();
    sum / (hi - lo) as f64
}

#[test]
fn slack_tape_skews_the_lines_after_the_head_switch() {
    // A slack tape is late onto the drum: the lines just after the head takes over are displaced by
    // up to 0.8 microseconds for a tension of -1 (about eleven pixels) and settle within a few lines.
    // The head takes over 6.5 lines before the vertical sync, so the displaced lines are the last
    // rows of the picture, with the end of the settling in the first rows of the next field.
    let p = config::build(Some("pristine"), "", 3).unwrap();
    let input: Vec<Frame> = (0..6).map(|_| test_pattern()).collect();
    let plain = Session::run(p.clone(), &input);
    let with_tension = |tension: f64| {
        let mut schedule = AutomationSchedule::new();
        schedule.set_track("tension", AutomationTrack::constant(tension));
        Session::run_with_automation(p.clone(), &input, schedule)
    };
    let h = plain[3].h;
    for tension in [-1.0, -0.5] {
        let slack = with_tension(tension);
        let after_switch = mean_row_difference(&plain[3], &slack[3], h - 8, h);
        let middle = mean_row_difference(&plain[3], &slack[3], 120, 400);
        assert!(after_switch > 5.0, "tension {tension}: the rows after the head switch moved by {after_switch:.2} levels");
        assert!(middle < 0.5, "tension {tension}: the middle of the picture moved by {middle:.2} levels");
    }
    // a tape that is too tight is on the drum in time
    let tight = with_tension(0.5);
    assert!(plain[3].rgb == tight[3].rgb, "a tight tape changed the picture");
}
