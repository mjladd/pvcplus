# Changelog

This project started tracking changes here from the first binary release onward. Earlier history lives in the git log and in [docs/dev/port-status.md](docs/dev/port-status.md).

## [Unreleased]

## [0.1.2] - 2026-09-22

### Added

- An index of every tool page, grouped by command family: [docs/tools/README.md](docs/tools/README.md).
- A QA harness under `harness/` that runs every covered `pvc` tool's own preset against a directory of your own audio files, with prerequisite chaining, multi-step chains, and fixed fixtures.
- Uninstall steps for all three install paths, and the macOS quarantine step (`xattr -d com.apple.quarantine`) for a binary downloaded with a browser.

### Changed

- `pvc --help` lists its commands under family headings, such as "Time and pitch" and "Feature extraction", each with a one-line description. `pvc <command> --help` still prints the full description and every flag.
- The man pages under `man/` carry the same one-line descriptions.

## [0.1.1] - 2026-09-11

### Added

- Downloadable binary releases for Linux (x86_64, aarch64) and macOS (Intel, Apple Silicon), built and published by cargo-dist.
