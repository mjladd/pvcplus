//! `pvc fn gen1..gen6`: dispatches to `pvc_core::gen`'s ported generators
//! and writes the resulting table via `pvc_io::write_control_file`.

use anyhow::{ensure, Result};

use crate::cli::FnCommand;

pub fn run(cmd: FnCommand) -> Result<()> {
    match cmd {
        FnCommand::Gen1 {
            length,
            open,
            points,
            output,
        } => {
            // pvc_core::gen1 panics below two points - validated here
            // instead, so bad user input gets an actionable error rather
            // than a Rust panic/backtrace (plan §2.1: "errors are
            // actionable").
            ensure!(
                points.len() >= 2,
                "gen1: need at least two --point breakpoints, got {}",
                points.len()
            );
            let table = pvc_core::gen1(length, !open, &points);
            pvc_io::write_control_file(&output, &table)?;
        }
        FnCommand::Gen2 {
            length,
            closed,
            sine,
            cosine,
            output,
        } => {
            let table = pvc_core::gen2(length, closed, &sine, &cosine);
            pvc_io::write_control_file(&output, &table)?;
        }
        FnCommand::Gen3 {
            length,
            open,
            values,
            output,
        } => {
            ensure!(
                values.len() >= 2,
                "gen3: need at least two --value entries, got {}",
                values.len()
            );
            let table = pvc_core::gen3(length, !open, &values);
            pvc_io::write_control_file(&output, &table)?;
        }
        FnCommand::Gen4 {
            length,
            open,
            points,
            output,
        } => {
            ensure!(
                points.len() >= 2,
                "gen4: need at least two --point breakpoints, got {}",
                points.len()
            );
            let table = pvc_core::gen4(length, !open, &points);
            pvc_io::write_control_file(&output, &table)?;
        }
        FnCommand::Gen5 {
            length,
            closed,
            partials,
            output,
        } => {
            let table = pvc_core::gen5(length, closed, &partials);
            pvc_io::write_control_file(&output, &table)?;
        }
        FnCommand::Gen6 { length, output } => {
            let table = pvc_core::gen6(length);
            pvc_io::write_control_file(&output, &table)?;
        }
    }
    Ok(())
}
