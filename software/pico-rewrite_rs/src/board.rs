use embedded_hal::digital::OutputPin;

use crate::hal;

pub struct BoardPins<LedPin> {
    pub led: LedPin,
}

pub fn configure(pins: hal::gpio::Pins) -> BoardPins<impl OutputPin> {
    BoardPins {
        led: pins.gpio25.into_push_pull_output(),
    }
}
