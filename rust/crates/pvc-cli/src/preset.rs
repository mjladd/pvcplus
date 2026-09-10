//! Presets: TOML files consumed by `pvc run`, replacing the `S.*` shell
//! scripts (plan §2.2). No tool has a typed parameter schema yet (that
//! arrives per-tool in Phase 3), so a preset's `[params]`/other tables are
//! kept as generic `toml::Value`s here rather than a fixed struct.

use std::collections::BTreeMap;
use std::path::Path;

use anyhow::{bail, Context, Result};
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize, Default)]
pub struct Preset {
    pub tool: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub input: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub output: Option<String>,
    #[serde(flatten)]
    pub tables: BTreeMap<String, toml::Value>,
}

impl Preset {
    pub fn load(path: &Path) -> Result<Self> {
        let text = std::fs::read_to_string(path)
            .with_context(|| format!("reading preset {}", path.display()))?;
        toml::from_str(&text).with_context(|| format!("parsing preset {}", path.display()))
    }

    /// Applies one `--set key=value` override. An unqualified key sets
    /// `[params].key`; `section.key` targets another table; `tool`,
    /// `input`, and `output` are the top-level fields.
    pub fn apply_set(&mut self, set: &str) -> Result<()> {
        let (key, raw_value) = set
            .split_once('=')
            .ok_or_else(|| anyhow::anyhow!("--set must be KEY=VALUE, got {set:?}"))?;
        let value = parse_value(raw_value);

        if let Some((section, subkey)) = key.split_once('.') {
            return self.set_in_table(section, subkey, value);
        }

        match key {
            "tool" => self.tool = raw_value.to_string(),
            "input" => self.input = Some(raw_value.to_string()),
            "output" => self.output = Some(raw_value.to_string()),
            _ => return self.set_in_table("params", key, value),
        }
        Ok(())
    }

    fn set_in_table(&mut self, section: &str, key: &str, value: toml::Value) -> Result<()> {
        let table = self
            .tables
            .entry(section.to_string())
            .or_insert_with(|| toml::Value::Table(Default::default()));
        let toml::Value::Table(map) = table else {
            bail!("--set target {section:?} is not a table in this preset");
        };
        map.insert(key.to_string(), value);
        Ok(())
    }

    /// Builds the `pvc <tool> [flags...] <input> <output>` argument list
    /// this preset resolves to, for `Cli::try_parse_from` to parse the
    /// same way it would parse real command-line input - see
    /// `commands::run`'s own doc comment for why reusing `Cli`'s parser
    /// this way, rather than hand-mapping each preset field onto each
    /// tool's own args struct, is the whole point.
    ///
    /// Every `[params]` key must already be the snake_case spelling of a
    /// real flag name (`frames_per_sec` for `--frames-per-sec`,
    /// `shelf_low_gain` for `--shelf-low-gain`), matching clap's own
    /// kebab-case derivation - there is no separate name-mapping table,
    /// so a preset field that does not match a real flag name surfaces
    /// as a plain "unexpected argument" error from `Cli::try_parse_from`,
    /// not a silently-ignored field. A boolean `true` value becomes a
    /// bare `--flag` (for switch-style flags); `false` omits the flag
    /// entirely, which only gives the right answer for a switch whose
    /// own default is `false` - this preset format has no way to force a
    /// `true`-by-default switch back off.
    pub fn to_cli_args(&self, program: &str) -> Vec<String> {
        let mut args = vec![program.to_string(), self.tool.clone()];
        if let Some(toml::Value::Table(params)) = self.tables.get("params") {
            for (key, value) in params {
                let flag = format!("--{}", key.replace('_', "-"));
                match value {
                    toml::Value::Boolean(true) => args.push(flag),
                    toml::Value::Boolean(false) => {}
                    other => {
                        args.push(flag);
                        args.push(toml_value_to_arg(other));
                    }
                }
            }
        }
        if let Some(input) = &self.input {
            args.push(input.clone());
        }
        if let Some(output) = &self.output {
            args.push(output.clone());
        }
        args
    }
}

fn toml_value_to_arg(value: &toml::Value) -> String {
    match value {
        toml::Value::String(s) => s.clone(),
        toml::Value::Integer(i) => i.to_string(),
        toml::Value::Float(f) => f.to_string(),
        toml::Value::Boolean(b) => b.to_string(),
        other => other.to_string(),
    }
}

fn parse_value(raw: &str) -> toml::Value {
    if let Ok(b) = raw.parse::<bool>() {
        toml::Value::Boolean(b)
    } else if let Ok(i) = raw.parse::<i64>() {
        toml::Value::Integer(i)
    } else if let Ok(f) = raw.parse::<f64>() {
        toml::Value::Float(f)
    } else {
        toml::Value::String(raw.to_string())
    }
}

/// Tools with an annotated default preset available via `preset init`.
/// Grows one tool at a time as Phase 3 ports each tool - `pv` (the only
/// one so far) matches the plan's own §2.2 example verbatim.
pub const KNOWN_TOOLS: &[&str] = &["pv"];

pub fn init_template(tool: &str) -> Result<&'static str> {
    match tool {
        "pv" => Ok(PV_TEMPLATE),
        other => bail!(
            "no preset template for tool {other:?} yet (available: {})",
            KNOWN_TOOLS.join(", ")
        ),
    }
}

const PV_TEMPLATE: &str = r#"# preset for `pvc run`
tool = "pv"
input = "in.wav"
output = "out.wav"

[params]
fft = 2048               # FFT size (power of two)
window = "kaiser8"       # hamming | rectangular | blackman | bartlett | kaiser<4-12> | blackman_harris | nuttall | blackman_nuttall | flat_top
frames_per_sec = 400     # analysis frames per second (sets the hop size)
stretch = 1.0            # time-stretch factor (1.0 = unchanged)
pitch = 0.0              # pitch transposition in semitones - accepts "@path" for a time-varying control function
shelf_low_gain = 0       # low shelf EQ gain in dB
shelf_low_freq = 500     # low shelf EQ frequency in Hz
"#;

/// Example presets bundled under `examples/presets/` (relative to the
/// current working directory - none exist yet; Task 4.2 adds the first
/// batch). Returns an empty list rather than an error when the directory
/// doesn't exist.
pub fn list_examples() -> Result<Vec<std::path::PathBuf>> {
    let dir = Path::new("examples/presets");
    if !dir.is_dir() {
        return Ok(Vec::new());
    }
    let mut out = Vec::new();
    for entry in std::fs::read_dir(dir).with_context(|| format!("reading {}", dir.display()))? {
        let path = entry?.path();
        if path
            .extension()
            .is_some_and(|e| e.eq_ignore_ascii_case("toml"))
        {
            out.push(path);
        }
    }
    out.sort();
    Ok(out)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn apply_set_unqualified_key_goes_under_params() {
        let mut preset: Preset = toml::from_str(
            r#"
            tool = "pv"
            [params]
            stretch = 1.0
            "#,
        )
        .unwrap();
        preset.apply_set("stretch=1.5").unwrap();
        let params = preset.tables.get("params").unwrap().as_table().unwrap();
        assert_eq!(params.get("stretch").unwrap().as_float(), Some(1.5));
    }

    #[test]
    fn apply_set_top_level_field() {
        let mut preset = Preset {
            tool: "pv".to_string(),
            ..Default::default()
        };
        preset.apply_set("output=out2.wav").unwrap();
        assert_eq!(preset.output.as_deref(), Some("out2.wav"));
    }

    #[test]
    fn apply_set_qualified_key_targets_named_section() {
        let mut preset = Preset {
            tool: "pv".to_string(),
            ..Default::default()
        };
        preset.apply_set("shelf_eq.low_gain_db=3").unwrap();
        let section = preset.tables.get("shelf_eq").unwrap().as_table().unwrap();
        assert_eq!(section.get("low_gain_db").unwrap().as_integer(), Some(3));
    }

    #[test]
    fn init_template_for_known_tool_round_trips_as_toml() {
        let template = init_template("pv").unwrap();
        let parsed: Preset = toml::from_str(template).unwrap();
        assert_eq!(parsed.tool, "pv");
    }

    #[test]
    fn init_template_for_unknown_tool_errors() {
        assert!(init_template("nonexistent-tool").is_err());
    }

    #[test]
    fn to_cli_args_builds_flags_input_and_output_in_order() {
        let preset: Preset = toml::from_str(
            r#"
            tool = "pv"
            input = "in.wav"
            output = "out.wav"
            [params]
            fft = 2048
            stretch = 1.5
            window = "kaiser8"
            "#,
        )
        .unwrap();
        let args = preset.to_cli_args("pvc");
        assert_eq!(args[0], "pvc");
        assert_eq!(args[1], "pv");
        assert_eq!(args[args.len() - 2], "in.wav");
        assert_eq!(args[args.len() - 1], "out.wav");
        // Flag order follows BTreeMap key order (alphabetical), not
        // insertion order - assert presence, not position.
        assert!(args.windows(2).any(|w| w == ["--fft", "2048"]));
        assert!(args.windows(2).any(|w| w == ["--stretch", "1.5"]));
        assert!(args.windows(2).any(|w| w == ["--window", "kaiser8"]));
    }

    #[test]
    fn to_cli_args_bool_true_is_a_bare_flag_false_is_omitted() {
        let preset: Preset = toml::from_str(
            r#"
            tool = "sometool"
            [params]
            on_flag = true
            off_flag = false
            "#,
        )
        .unwrap();
        let args = preset.to_cli_args("pvc");
        assert!(args.contains(&"--on-flag".to_string()));
        assert!(!args.contains(&"--off-flag".to_string()));
    }

    #[test]
    fn to_cli_args_omits_input_output_when_absent() {
        let preset = Preset {
            tool: "pv".to_string(),
            ..Default::default()
        };
        let args = preset.to_cli_args("pvc");
        assert_eq!(args, vec!["pvc".to_string(), "pv".to_string()]);
    }

    #[test]
    fn pv_template_round_trips_through_cli_parsing() {
        // The template's own [params] keys must be real pv flag names -
        // this is the regression test for the shelf_eq-vs-shelf-low-gain
        // mismatch that made every `pvc run` on the bundled `pv` preset
        // fail before this test existed.
        use crate::cli::Cli;
        use clap::Parser;

        let template = init_template("pv").unwrap();
        let preset: Preset = toml::from_str(template).unwrap();
        let args = preset.to_cli_args("pvc");
        let cli = Cli::try_parse_from(&args).unwrap_or_else(|e| {
            panic!("pv template did not parse as valid pv args: {e}\nargs: {args:?}")
        });
        assert!(matches!(cli.command, crate::cli::Command::Pv(_)));
    }
}
