//! `pvc convert-units`: consolidates the four standalone unit-conversion
//! utilities `amptodB`, `dBtoamp`, `Hztopitch`, `pitchtoHz`
//! (`legacy/pvc_src/*.c`) into one `--from`/`--to` command. Every
//! conversion here reuses math already ported and oracle-verified
//! elsewhere in `pvc-core` (`units::amp_to_db`, `units::db_to_amp_exact`,
//! `response::hz_to_oppc`, `response::oppc_to_hz`) - this module is CLI
//! plumbing only.

use anyhow::{bail, Result};
use serde::Serialize;

use crate::cli::ConvertUnit;

#[derive(Serialize)]
struct Conversion {
    input: f32,
    from: &'static str,
    output: f32,
    to: &'static str,
}

fn unit_name(unit: ConvertUnit) -> &'static str {
    match unit {
        ConvertUnit::Amp => "amp",
        ConvertUnit::Db => "db",
        ConvertUnit::Hz => "hz",
        ConvertUnit::Oppc => "oppc",
    }
}

pub fn run(
    from: ConvertUnit,
    to: ConvertUnit,
    norm: f32,
    values: &[f32],
    json: bool,
) -> Result<()> {
    use ConvertUnit::*;

    for &input in values {
        let output = match (from, to) {
            (Amp, Db) => pvc_core::units::amp_to_db(input / norm),
            (Db, Amp) => pvc_core::units::db_to_amp_exact(input),
            (Hz, Oppc) => pvc_core::response::hz_to_oppc(input),
            (Oppc, Hz) => pvc_core::response::oppc_to_hz(input),
            _ => bail!(
                "unsupported conversion: {} -> {} (supported: amp<->db, hz<->oppc)",
                unit_name(from),
                unit_name(to)
            ),
        };

        if json {
            let conversion = Conversion {
                input,
                from: unit_name(from),
                output,
                to: unit_name(to),
            };
            println!("{}", serde_json::to_string(&conversion)?);
        } else {
            println!("{input} {} = {output} {}", unit_name(from), unit_name(to));
        }
    }

    Ok(())
}
