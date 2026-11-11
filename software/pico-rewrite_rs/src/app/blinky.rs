use core::fmt::Debug;

use defmt::info;
use embedded_hal::delay::DelayNs;
use embedded_hal::digital::OutputPin;

use crate::config::BLINK_INTERVAL_MS;

pub fn run<LedPin, Timer>(mut led: LedPin, timer: &mut Timer) -> !
where
    LedPin: OutputPin,
    LedPin::Error: Debug,
    Timer: DelayNs,
{
    loop {
        info!("on!");
        led.set_high().unwrap();
        timer.delay_ms(BLINK_INTERVAL_MS);
        info!("off!");
        led.set_low().unwrap();
        timer.delay_ms(BLINK_INTERVAL_MS);
    }
}
