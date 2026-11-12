# Firmware

This folder contains the source code for the Tip Etcher project

## Pico Rust firmware

Since the STM32G474 firmware is being migrated to a Raspberry Pi Pico-family board, the new firmware is in the process of being rewritten The exact board model is yet to be finalized.

Right now in Rust and uses the `no_std` environment.

Since I'm practically guessing (since there's no info ready for me right now), i'll guess we're using RP2350 and also that the entire wiring will be the same. The latter is a quick change to make, but the former will be a bit more involved if it will end up being different.
