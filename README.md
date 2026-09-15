# BTC-STofu-Parser

BTC-STofu-Parser is a Windows C++ parser for raw Bitcoin Core block files. It reads `blk*.dat` files, parses block headers and transactions, classifies common transaction script/address forms, and writes the parsed result as JSON.

The project was imported from an internal `btc-analyzer` codebase and currently targets Visual Studio on Windows.

## What It Does

- Reads Bitcoin block files from a configured directory.
- Supports Bitcoin Core XOR-obfuscated block data through `xor.dat`.
- Parses block metadata: block hash, version, previous block hash, Merkle root, timestamp, bits, and nonce.
- Parses transactions, including version, inputs, outputs, lock time, transaction hashes, and SegWit witness data.
- Detects common Bitcoin script forms, including P2PKH, P2SH, SegWit, and Taproot-style outputs where implemented in the parser.
- Exports parsed blocks to formatted JSON files.
- Provides logging through `spdlog`.
- Contains an experimental PostgreSQL transaction database layer based on `libpqxx`.

## Repository Layout

```text
.
|-- 3rd-party/
|   |-- PostgreSQL17/
|   |-- libpqxx/
|   `-- spdlog/
|-- resources/
|   |-- blocks/
|   |-- about_blocks.txt
|   `-- config.json
|-- src/btc-parser/
|   |-- btc-parser.sln
|   |-- btc-parser.vcxproj
|   |-- main.cpp
|   |-- *Parser.cpp / *Parser.h
|   |-- JsonUtils.cpp / JsonUtils.h
|   |-- Logger.cpp / Logger.hpp
|   `-- database/
|-- notes.txt
`-- README.md
```

## Requirements

- Windows.
- Visual Studio 2022 or compatible MSVC toolchain.
- MSVC v143 platform toolset.
- Windows 10 SDK.
- C++23 support.
- x64 Release build is the currently expected working configuration.

The Visual Studio project references these dependencies:

- `nlohmann.json` 3.12.0 via NuGet.
- Boost 1.87.0 via NuGet.
- OpenSSL 1.1.1.1 via NuGet.
- `curl-vc140-static-32_64` via NuGet.
- `spdlog` from `3rd-party/spdlog`.
- `libpqxx` from `3rd-party/libpqxx`.
- PostgreSQL client libraries from `3rd-party/PostgreSQL17`.

## Build

Open the solution:

```text
src/btc-parser/btc-parser.sln
```

Recommended configuration:

```text
Release | x64
```

The historical project notes say `Release x64` is the safe default while the bundled `pqxx`/PostgreSQL dependency is present. `Debug x64` may require dependency cleanup or additional local setup.

From a Visual Studio Developer PowerShell, the build should look like:

```powershell
msbuild src/btc-parser/btc-parser.sln /p:Configuration=Release /p:Platform=x64
```

## Configuration

The application expects one command-line argument: a path to a JSON config file.

Example config:

```json
{
  "BlockDirectory": "../../resources/blocks/",
  "XorDirectory": "../../resources/xor.dat",
  "OutputDirectory": "../../output/"
}
```

Fields:

- `BlockDirectory`: directory containing `blk*.dat` files or sample block files.
- `XorDirectory`: path to `xor.dat` for Bitcoin Core XOR-obfuscated blocks. Leave empty or point to a non-used value only when parsing non-obfuscated sample data.
- `OutputDirectory`: directory where JSON files will be written.

Make sure `output` and `log` directories exist before running. The current code initializes logs under `../../log/` relative to the executable working directory.

## Usage

Place input block files in the configured block directory, then run:

```powershell
btc-parser.exe path\to\config.json
```

With the repository sample config, a Visual Studio run can use:

```text
resources/config.json
```

For every parsed input block file, the tool writes a JSON file named after the block file stem:

```text
output/<block-file-name>.json
```

## Input Data Notes

See `resources/about_blocks.txt` for the current sample-data notes.

Important behavior:

- Bitcoin Core `blk00000` / `blk00317` style files require `xor.dat` when they are stored in XOR-obfuscated form.
- The included `blk_example` sample is described as a witness-containing block and should be parsed without `xor.dat`.
- `xor.dat` handling is wired through `BitcoinReader` in `main.cpp`.

## Output

The main executable currently writes JSON. Each block output includes data such as:

- current block hash.
- block header fields.
- transactions.
- transaction hashes.
- inputs and previous transaction references.
- outputs and satoshi values.
- script signatures and script public keys.
- derived input/output addresses where the parser recognizes the script pattern.
- witness data for SegWit transactions.

## Database Layer

The repository contains an experimental PostgreSQL transaction database layer under `src/btc-parser/database`.

It includes:

- `IDatabase` and `ITransactionDatabase` interfaces.
- `TransactionDbData` model.
- `libpqxx`-based `TransactionDbPostgreSql` implementation.
- PostgreSQL query command helpers.

Current status: database functionality appears focused on transactions and should be treated as experimental until schema setup, connection configuration, and integration are documented and tested.

## Known Limitations

These are documented from the current source and project notes:

- `Release x64` is the expected build target with the current dependency layout.
- Transaction database functionality is incomplete and transaction-focused.
- The transaction hash for witness transactions is known to be incorrect in the current implementation because the hash should be generated without the SegWit marker/flag and witness data.
- Runtime memory usage can be high because parsed blocks and transactions are copied and kept in memory.
- Project structure still needs cleanup: the solution lives under `src/btc-parser` and only the database code is split into a separate subdirectory.
- There is no automated test suite in the current repository.

## Development Notes

Useful implementation areas:

- `BitcoinReader` handles binary reading and optional XOR de-obfuscation.
- `BlockFileParser` iterates over all blocks in a block file.
- `SingleBlockParser` parses one block from the reader.
- `BlockHeaderParser` parses the 80-byte Bitcoin block header.
- `TransactionParser` parses transaction inputs, outputs, SegWit data, lock time, and transaction hash metadata.
- `TransactionScriptParser` recognizes common Bitcoin script templates and extracts address-like values.
- `BlockPrinter` and `JsonUtils` serialize parsed data to text or JSON.
- `Logger` wraps `spdlog` logging.

## Roadmap

- Fix SegWit transaction hash calculation.
- Reduce memory copying and support streaming output for large block files.
- Add tests with deterministic block fixtures.
- Document and stabilize PostgreSQL schema and import flow.
- Normalize project layout and build configurations.
- Add CI for `Release x64` builds.
- Add command-line options for output format, XOR mode, and single-file parsing.

## License

MIT License. See `LICENSE`.
