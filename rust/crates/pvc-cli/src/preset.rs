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
fft = 2048              # FFT size (power of two)
window = "kaiser8"       # hamming | rectangular | blackman | bartlett | kaiser<4-12> | blackman_harris | nuttall | blackman_nuttall | flat_top
frames_per_sec = 400     # analysis frames per second (sets the hop size)
stretch = 1.0            # time-stretch factor (1.0 = unchanged)
pitch = 0.0              # pitch transposition in semitones; accepts "@path" for a time-varying control function

[shelf_eq]
low_gain_db = 0
low_freq = 500
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
}
