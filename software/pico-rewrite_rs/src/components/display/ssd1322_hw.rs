//! Hardware-dependent SSD1322 operations corresponding to `SSD1322_HW_Driver.c`.

use embedded_hal::{delay::DelayNs, digital::OutputPin, spi::SpiBus};

pub struct Ssd1322HwDriver<SPI, CS, DC, RESET, DELAY> {
    spi: SPI,
    cs: CS,
    dc: DC,
    reset: RESET,
    delay: DELAY,
}

impl<SPI, CS, DC, RESET, DELAY> Ssd1322HwDriver<SPI, CS, DC, RESET, DELAY>
where
    SPI: SpiBus<u8>,
    CS: OutputPin,
    DC: OutputPin,
    RESET: OutputPin,
    DELAY: DelayNs,
{
    pub const fn new(spi: SPI, cs: CS, dc: DC, reset: RESET, delay: DELAY) -> Self {
        Self {
            spi,
            cs,
            dc,
            reset,
            delay,
        }
    }

    pub fn drive_cs_low(&mut self) -> Result<(), CS::Error> {
        self.cs.set_low()
    }

    pub fn drive_cs_high(&mut self) -> Result<(), CS::Error> {
        self.cs.set_high()
    }

    pub fn drive_dc_low(&mut self) -> Result<(), DC::Error> {
        self.dc.set_low()
    }

    pub fn drive_dc_high(&mut self) -> Result<(), DC::Error> {
        self.dc.set_high()
    }

    pub fn drive_reset_low(&mut self) -> Result<(), RESET::Error> {
        self.reset.set_low()
    }

    pub fn drive_reset_high(&mut self) -> Result<(), RESET::Error> {
        self.reset.set_high()
    }

    pub fn spi_send_byte(&mut self, byte: u8) -> Result<(), SPI::Error> {
        self.spi.write(core::slice::from_ref(&byte))
    }

    pub fn spi_send_array(&mut self, bytes: &[u8]) -> Result<(), SPI::Error> {
        self.spi.write(bytes)
    }

    pub fn ms_delay(&mut self, milliseconds: u32) {
        self.delay.delay_ms(milliseconds);
    }
}
