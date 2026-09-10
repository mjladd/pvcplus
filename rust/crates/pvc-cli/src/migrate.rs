//! `pvc migrate-script <path>`: parses an `S.*` shell script's own
//! variable block into a preset TOML for `pvc run` (plan §4.6).
//!
//! Every `S.*` script (see `legacy/scripts/`) shares one shape: a block
//! of shell variable assignments near the top (`name=value`, one per
//! line, each documented by a comment above or below it), followed by a
//! `# COMMAND LINE SETUP -- OFFICE USE ONLY` section that builds the
//! actual legacy command line from those variables. The line
//! `pvroutine=<name>` always opens that second section - confirmed
//! against several scripts, not just `S.plainpv` - so it doubles as
//! both "which legacy tool does this script run" and "where does the
//! variable block end."
//!
//! Only `plainpv` (-> `pv`) is mapped today, matching the plan's own
//! single named example (`pvc migrate-script scripts/S.plainpv`). A
//! script for an unmapped tool produces a clear error naming the tool,
//! not a silent no-op or a guess.

use std::collections::BTreeMap;

use crate::preset::Preset;

/// One shell variable's real preset destination and how to read its
/// value.
struct FieldMapping {
    shell_var: &'static str,
    preset_field: &'static str,
    transform: Transform,
}

enum Transform {
    /// Copies the raw value into the preset as a TOML number (integer
    /// if it parses as one, float otherwise) - the vast majority of
    /// fields, since most `S.*` variables and their `pvc` counterparts
    /// use the same units already.
    Number,
    /// Legacy `window_type` (0-3 named, 4-12 Kaiser alpha) -> `pv`'s own
    /// `--window` value (`hamming`/`rectangular`/`blackman`/`bartlett`/
    /// `kaiser<4-12>`).
    WindowType,
    /// Legacy `FILTER_TYPE` (0/1) -> `pv`'s own `--filter-type` value
    /// (`bandpass`/`reject`).
    FilterType,
}

/// `plainpv` -> `pv`'s own field mapping. Order matches `S.plainpv`'s
/// own variable block, top to bottom.
const PLAINPV_FIELDS: &[FieldMapping] = &[
    FieldMapping {
        shell_var: "FFT_length",
        preset_field: "fft",
        transform: Transform::Number,
    },
    FieldMapping {
        shell_var: "window_type",
        preset_field: "window",
        transform: Transform::WindowType,
    },
    FieldMapping {
        shell_var: "windowsize",
        preset_field: "window_size",
        transform: Transform::Number,
    },
    FieldMapping {
        shell_var: "frames_per_second",
        preset_field: "frames_per_sec",
        transform: Transform::Number,
    },
    FieldMapping {
        shell_var: "time_expansion_contraction_factor",
        preset_field: "stretch",
        transform: Transform::Number,
    },
    FieldMapping {
        shell_var: "frequency_shift_in_Hz",
        preset_field: "freq_shift",
        transform: Transform::Number,
    },
    FieldMapping {
        shell_var: "pitch_transposition_in_semitones",
        preset_field: "pitch",
        transform: Transform::Number,
    },
    FieldMapping {
        shell_var: "gain_in_decibels",
        preset_field: "gain",
        transform: Transform::Number,
    },
    FieldMapping {
        shell_var: "oscillator_resynthesis_threshold_in_dB",
        preset_field: "threshold",
        transform: Transform::Number,
    },
    FieldMapping {
        shell_var: "LOW_SHELF_EQ_gain_in_decibels",
        preset_field: "shelf_low_gain",
        transform: Transform::Number,
    },
    FieldMapping {
        shell_var: "LOW_SHELF_EQ_frequency",
        preset_field: "shelf_low_freq",
        transform: Transform::Number,
    },
    FieldMapping {
        shell_var: "HIGH_SHELF_EQ_gain_in_decibels",
        preset_field: "shelf_high_gain",
        transform: Transform::Number,
    },
    FieldMapping {
        shell_var: "HIGH_SHELF_EQ_frequency",
        preset_field: "shelf_high_freq",
        transform: Transform::Number,
    },
    FieldMapping {
        shell_var: "release_time_in_seconds",
        preset_field: "release",
        transform: Transform::Number,
    },
    FieldMapping {
        shell_var: "attack_time_in_seconds",
        preset_field: "attack",
        transform: Transform::Number,
    },
    FieldMapping {
        shell_var: "spectrum_warpshape_index",
        preset_field: "warp",
        transform: Transform::Number,
    },
    FieldMapping {
        shell_var: "FILTER_TYPE",
        preset_field: "filter_type",
        transform: Transform::FilterType,
    },
    FieldMapping {
        shell_var: "BRICKWALL_FILTER_window_low_frequency",
        preset_field: "filter_low",
        transform: Transform::Number,
    },
    FieldMapping {
        shell_var: "BRICKWALL_FILTER_window_high_frequency",
        preset_field: "filter_high",
        transform: Transform::Number,
    },
];

fn fields_for_legacy_tool(tool: &str) -> Result<(&'static str, &'static [FieldMapping]), String> {
    match tool {
        "plainpv" => Ok(("pv", PLAINPV_FIELDS)),
        other => Err(format!(
            "no migration mapping for legacy tool {other:?} yet (only \"plainpv\" -> \"pv\" \
             today) - run `pvc legacy {other}` directly, or write a preset for it by hand (see \
             docs/presets.md)"
        )),
    }
}

/// Parses `script_text`'s own variable block (every `name=value` line
/// before the first `pvroutine=` line) into an ordered map, preserving
/// each variable's own first assignment if it appears more than once
/// (matching shell's own last-wins semantics would need real
/// evaluation, e.g. of conditionals, which no `S.*` script's variable
/// block actually uses - confirmed by reading several - so this port
/// does not implement it, and instead keeps things simple as the
/// scripts themselves already are).
fn parse_variable_block(script_text: &str) -> BTreeMap<String, String> {
    let mut vars = BTreeMap::new();
    for line in script_text.lines() {
        let trimmed = line.trim();
        if trimmed.starts_with('#') || trimmed.is_empty() {
            continue;
        }
        if trimmed.starts_with("pvroutine=") {
            break;
        }
        let Some((name, value)) = trimmed.split_once('=') else {
            continue;
        };
        let name = name.trim();
        let is_identifier = !name.is_empty()
            && name
                .chars()
                .next()
                .is_some_and(|c| c.is_ascii_alphabetic() || c == '_')
            && name.chars().all(|c| c.is_ascii_alphanumeric() || c == '_');
        if !is_identifier {
            continue;
        }
        vars.entry(name.to_string())
            .or_insert_with(|| value.trim().to_string());
    }
    vars
}

fn find_pvroutine(script_text: &str) -> Option<String> {
    script_text.lines().find_map(|line| {
        line.trim()
            .strip_prefix("pvroutine=")
            .map(|v| v.trim().to_string())
    })
}

fn number_value(raw: &str) -> toml::Value {
    if let Ok(i) = raw.parse::<i64>() {
        toml::Value::Integer(i)
    } else if let Ok(f) = raw.parse::<f64>() {
        toml::Value::Float(f)
    } else {
        toml::Value::String(raw.to_string())
    }
}

fn legacy_window_type_to_name(raw: &str) -> Result<String, String> {
    let n: i64 = raw
        .parse()
        .map_err(|_| format!("window_type {raw:?} is not a number"))?;
    match n {
        0 => Ok("hamming".to_string()),
        1 => Ok("rectangular".to_string()),
        2 => Ok("blackman".to_string()),
        3 => Ok("bartlett".to_string()),
        4..=12 => Ok(format!("kaiser{n}")),
        _ => Err(format!("window_type {n} is out of the legacy 0-12 range")),
    }
}

fn legacy_filter_type_to_name(raw: &str) -> Result<String, String> {
    match raw {
        "0" => Ok("bandpass".to_string()),
        "1" => Ok("reject".to_string()),
        other => Err(format!("FILTER_TYPE {other:?} is not 0 or 1")),
    }
}

/// Migrates one `S.*` script's text into a preset TOML string, ready to
/// print or save. `script_path` is used only for the header comment
/// naming where the preset came from.
pub fn migrate_script(script_text: &str, script_path: &str) -> Result<String, String> {
    let legacy_tool = find_pvroutine(script_text).ok_or_else(|| {
        "no `pvroutine=<name>` line found - this doesn't look like one of this project's own \
         `S.*` scripts"
            .to_string()
    })?;
    let (pvc_tool, fields) = fields_for_legacy_tool(&legacy_tool)?;
    let vars = parse_variable_block(script_text);

    let mut preset = Preset {
        tool: pvc_tool.to_string(),
        input: vars.get("input_file").cloned(),
        output: vars.get("output_file").cloned(),
        ..Default::default()
    };

    let mut params = toml::map::Map::new();
    let mut migrated_vars = std::collections::HashSet::new();
    for field in fields {
        migrated_vars.insert(field.shell_var);
        let Some(raw) = vars.get(field.shell_var) else {
            continue;
        };
        let value = match field.transform {
            Transform::Number => number_value(raw),
            Transform::WindowType => toml::Value::String(legacy_window_type_to_name(raw)?),
            Transform::FilterType => toml::Value::String(legacy_filter_type_to_name(raw)?),
        };
        params.insert(field.preset_field.to_string(), value);
    }
    preset
        .tables
        .insert("params".to_string(), toml::Value::Table(params));

    let mut skipped: Vec<&str> = vars
        .keys()
        .map(String::as_str)
        .filter(|v| *v != "input_file" && *v != "output_file" && !migrated_vars.contains(v))
        .collect();
    skipped.sort_unstable();

    let mut out =
        format!("# preset for `pvc run`, migrated from {script_path} by `pvc migrate-script`\n");
    out.push_str(
        &toml::to_string_pretty(&preset).map_err(|e| format!("building preset TOML: {e}"))?,
    );
    if !skipped.is_empty() {
        out.push_str(&format!(
            "\n# Not migrated (no `pvc {pvc_tool}` equivalent): {}\n",
            skipped.join(", ")
        ));
    }
    Ok(out)
}

#[cfg(test)]
mod tests {
    use super::*;

    const S_PLAINPV: &str = r#"#!/bin/sh
output_file=/tmp/output.au
rescale_level_in_decibels=2
input_file=~/Oboe.G5.N.au
begintime=0
endtime=0
FFT_length=2048
window_type=1
windowsize=0
frames_per_second=400
output_channel=0
oscillator_resynthesis_threshold_in_dB=-96
time_expansion_contraction_factor=2
gain_in_decibels=0
frequency_shift_in_Hz=0
pitch_transposition_in_semitones=0
release_time_in_seconds=0
attack_time_in_seconds=0
spectrum_warpshape_index=0
FILTER_TYPE=0
BRICKWALL_FILTER_window_low_frequency=0
BRICKWALL_FILTER_window_high_frequency=-1
LOW_SHELF_EQ_gain_in_decibels=-0
LOW_SHELF_EQ_frequency=500
HIGH_SHELF_EQ_gain_in_decibels=-0
HIGH_SHELF_EQ_frequency=5000
print_amplitude_statistics_0_no__1_yes=1
amplitude_statistics_time_interval=1
pvroutine=plainpv
PVFLAGS="-N$FFT_length"
"#;

    #[test]
    fn migrates_plainpv_fields() {
        let out = migrate_script(S_PLAINPV, "scripts/S.plainpv").unwrap();
        let preset: Preset = toml::from_str(
            &out.lines()
                .filter(|l| !l.starts_with('#'))
                .collect::<Vec<_>>()
                .join("\n"),
        )
        .unwrap();
        assert_eq!(preset.tool, "pv");
        assert_eq!(preset.input.as_deref(), Some("~/Oboe.G5.N.au"));
        assert_eq!(preset.output.as_deref(), Some("/tmp/output.au"));
        let params = preset.tables.get("params").unwrap().as_table().unwrap();
        assert_eq!(params.get("fft").unwrap().as_integer(), Some(2048));
        assert_eq!(params.get("window").unwrap().as_str(), Some("rectangular"));
        assert_eq!(params.get("stretch").unwrap().as_integer(), Some(2));
        assert_eq!(
            params.get("filter_type").unwrap().as_str(),
            Some("bandpass")
        );
        assert_eq!(params.get("filter_high").unwrap().as_integer(), Some(-1));
    }

    #[test]
    fn migrated_preset_parses_as_real_pv_cli_args() {
        use crate::cli::Cli;
        use clap::Parser;

        let out = migrate_script(S_PLAINPV, "scripts/S.plainpv").unwrap();
        let preset: Preset = toml::from_str(
            &out.lines()
                .filter(|l| !l.starts_with('#'))
                .collect::<Vec<_>>()
                .join("\n"),
        )
        .unwrap();
        let args = preset.to_cli_args("pvc");
        Cli::try_parse_from(&args)
            .unwrap_or_else(|e| panic!("migrated preset did not parse as valid pv args: {e}"));
    }

    #[test]
    fn lists_skipped_fields_with_no_pv_equivalent() {
        let out = migrate_script(S_PLAINPV, "scripts/S.plainpv").unwrap();
        assert!(out.contains("output_channel"));
        assert!(out.contains("begintime"));
        assert!(out.contains("Not migrated"));
    }

    #[test]
    fn unmapped_tool_is_a_clear_error() {
        let text = "input_file=in.au\noutput_file=out.au\npvroutine=noisefilter\n";
        let err = migrate_script(text, "scripts/S.noisefilter").unwrap_err();
        assert!(err.contains("noisefilter"));
    }

    #[test]
    fn missing_pvroutine_is_a_clear_error() {
        let err = migrate_script("input_file=in.au\n", "scripts/not-a-real-script").unwrap_err();
        assert!(err.contains("pvroutine"));
    }
}
