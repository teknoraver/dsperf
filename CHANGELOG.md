# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

# [0.0.35] - 2025-06-08
### Changed
- More user friendly way to start server in both --underlay and --daas mode
- csv_format now available for --daas also
- -S and -s in --daas mode now require the remote din as parameter and not the ip/port

# [0.0.34] - 2025-06-06
### Changed
- Overlay mode now works correctly.
- New struct for managing of the args

## [0.0.33] - 2025-06-05
### Added
- Conditional compilation for DaaS overlay use

## [0.0.32] - 2025-06-05
### Changed
- New timer logic in data transfer.
- Added daas functionality

## [0.0.2]  - 2025-06-01
### Changed
- `loopback_tool` is now changed to `dperf`
- Updated blocksize and packet mode logic


## [0.0.1] - 2025-05-28
### Added
- First release of the `daas testnet` 
- First public release of the `loopback` tool for testing connectivity and throughput in a network.
- Support for **sender** and **receiver** modes, configurable via command-line options.
- Data transfer over:
  - **Underlay** (direct network)
  - **DaaS-overlay** (overlay network)
- CSV export with the following information:
  - Host name
  - Timestamp
  - Block ID
  - Layer used
  - Tool version
- Basic console logging.
