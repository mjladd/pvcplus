//! `pvc preset init/list`.

use anyhow::Result;

use crate::preset::{init_template, list_examples};

pub fn init(tool: &str) -> Result<()> {
    print!("{}", init_template(tool)?);
    Ok(())
}

pub fn list(json: bool) -> Result<()> {
    let files = list_examples()?;
    if json {
        let names: Vec<String> = files.iter().map(|p| p.display().to_string()).collect();
        println!("{}", serde_json::to_string_pretty(&names)?);
    } else if files.is_empty() {
        println!("no example presets found under examples/presets/");
    } else {
        for f in files {
            println!("{}", f.display());
        }
    }
    Ok(())
}
