# Tetris Mint

Hackathon approved tetris cli online

## Building

Also see [docs/DEVELOP.md](docs/DEVELOP.md).

### Setup

- Make sure you have dependencies installed
  - ncurses
  - cmake

### Compiling

```bash
cmake .
make -j$(nproc)
```

Output files are put into `./bin/`

### Testing

Unit tests are compiled into a single executable.

They can be run by running `./bin/unit_tests`

## Usage

Rather than explain the usage here, just use the command help via
`./bin/tetris-mint-server -h` and `./bin/tetris-mint -h`.
