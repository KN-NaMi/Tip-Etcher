//! Rust port of the SSD1322 API used by the legacy C driver.

use embedded_hal::{delay::DelayNs, digital::OutputPin, spi::SpiBus};

use super::ssd1322_hw::Ssd1322HwDriver;

const COMMAND_UNLOCK: u8 = 0xFD;
const DISPLAY_OFF: u8 = 0xAE;
const DISPLAY_CLOCK_DIV: u8 = 0xB3;
const MULTIPLEX_RATIO: u8 = 0xCA;
const DISPLAY_OFFSET: u8 = 0xA2;
const START_LINE: u8 = 0xA1;
const REMAP_AND_DUAL_COM: u8 = 0xA0;
const DISABLE_IO_INPUT: u8 = 0xB5;
const FUNCTION_SELECT: u8 = 0xAB;
const ENABLE_VSL_EXTERNAL: u8 = 0xB4;
const SET_CONTRAST_CURRENT: u8 = 0xC1;
const MASTER_CONTRAST_CURRENT: u8 = 0xC7;
const DEFAULT_GRAYSCALE: u8 = 0xB9;
const SET_PHASE_LENGTH: u8 = 0xB1;
const ENHANCE_DRIVING_SCHEME: u8 = 0xD1;
const FIRST_PRE_CHARGE_VOLTAGE: u8 = 0xBB;
const SECOND_PRE_CHARGE_VOLTAGE: u8 = 0xB6;
const VCOMH: u8 = 0xBE;
const SET_DISP_MODE_NORMAL: u8 = 0xA6;
const SET_DISP_MODE_INVERTED: u8 = 0xA7;
const SET_DISP_MODE_ON: u8 = 0xA5;
const SET_DISP_MODE_OFF: u8 = 0xA4;
const SLEEP_MODE_ON: u8 = 0xAE;
const SLEEP_MODE_OFF: u8 = 0xAF;
const SET_GRAYSCALE_TABLE: u8 = 0xB8;
const ENABLE_GRAYSCALE_TABLE: u8 = 0xB9;
const SET_DEFAULT_GRAYSCALE_TAB: u8 = 0xB9;
const SET_COLUMN_ADDR: u8 = 0x15;
const SET_ROW_ADDR: u8 = 0x75;
const ENABLE_RAM_WRITE: u8 = 0x5C;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum Ssd1322Mode {
    Normal,
    Inverted,
    On,
    Off,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum Ssd1322ApiError<E> {
    InvalidGrayscaleValue,
    Driver(E),
}

pub trait Ssd1322Driver {
    type Error;

    fn drive_cs_low(&mut self) -> Result<(), Self::Error>;
    fn drive_cs_high(&mut self) -> Result<(), Self::Error>;
    fn drive_dc_low(&mut self) -> Result<(), Self::Error>;
    fn drive_dc_high(&mut self) -> Result<(), Self::Error>;
    fn drive_reset_low(&mut self) -> Result<(), Self::Error>;
    fn drive_reset_high(&mut self) -> Result<(), Self::Error>;
    fn spi_send_byte(&mut self, byte: u8) -> Result<(), Self::Error>;
    fn spi_send_array(&mut self, bytes: &[u8]) -> Result<(), Self::Error>;
    fn ms_delay(&mut self, milliseconds: u32);
}

impl<SPI, CS, DC, RESET, DELAY> Ssd1322Driver for Ssd1322HwDriver<SPI, CS, DC, RESET, DELAY>
where
    SPI: SpiBus<u8>,
    CS: OutputPin,
    DC: OutputPin,
    RESET: OutputPin,
    DELAY: DelayNs,
{
    type Error = ();

    fn drive_cs_low(&mut self) -> Result<(), Self::Error> {
        Ssd1322HwDriver::drive_cs_low(self).map_err(|_| ())
    }

    fn drive_cs_high(&mut self) -> Result<(), Self::Error> {
        Ssd1322HwDriver::drive_cs_high(self).map_err(|_| ())
    }

    fn drive_dc_low(&mut self) -> Result<(), Self::Error> {
        Ssd1322HwDriver::drive_dc_low(self).map_err(|_| ())
    }

    fn drive_dc_high(&mut self) -> Result<(), Self::Error> {
        Ssd1322HwDriver::drive_dc_high(self).map_err(|_| ())
    }

    fn drive_reset_low(&mut self) -> Result<(), Self::Error> {
        Ssd1322HwDriver::drive_reset_low(self).map_err(|_| ())
    }

    fn drive_reset_high(&mut self) -> Result<(), Self::Error> {
        Ssd1322HwDriver::drive_reset_high(self).map_err(|_| ())
    }

    fn spi_send_byte(&mut self, byte: u8) -> Result<(), Self::Error> {
        Ssd1322HwDriver::spi_send_byte(self, byte).map_err(|_| ())
    }

    fn spi_send_array(&mut self, bytes: &[u8]) -> Result<(), Self::Error> {
        Ssd1322HwDriver::spi_send_array(self, bytes).map_err(|_| ())
    }

    fn ms_delay(&mut self, milliseconds: u32) {
        Ssd1322HwDriver::ms_delay(self, milliseconds);
    }
}

pub struct Ssd1322Api<Driver> {
    driver: Driver,
}

impl<Driver> Ssd1322Api<Driver>
where
    Driver: Ssd1322Driver,
{
    pub const fn new(driver: Driver) -> Self {
        Self { driver }
    }

    pub fn into_inner(self) -> Driver {
        self.driver
    }

    pub fn command(&mut self, command: u8) -> Result<(), Driver::Error> {
        self.driver.drive_cs_low()?;
        self.driver.drive_dc_low()?;
        self.driver.spi_send_byte(command)?;
        self.driver.drive_cs_high()?;
        Ok(())
    }

    pub fn data(&mut self, data: u8) -> Result<(), Driver::Error> {
        self.driver.drive_cs_low()?;
        self.driver.drive_dc_high()?;
        self.driver.spi_send_byte(data)?;
        self.driver.drive_cs_high()?;
        Ok(())
    }

    pub fn init(&mut self) -> Result<(), Driver::Error> {
        self.driver.drive_reset_low()?;
        self.driver.ms_delay(1);
        self.driver.drive_reset_high()?;
        self.driver.ms_delay(50);

        self.command(COMMAND_UNLOCK)?;
        self.data(0x12)?;
        self.command(DISPLAY_OFF)?;
        self.command(DISPLAY_CLOCK_DIV)?;
        self.data(0x91)?;
        self.command(MULTIPLEX_RATIO)?;
        self.data(0x3F)?;
        self.command(DISPLAY_OFFSET)?;
        self.data(0x00)?;
        self.command(START_LINE)?;
        self.data(0x00)?;
        self.command(REMAP_AND_DUAL_COM)?;
        self.data(0x14)?;
        self.data(0x11)?;
        self.command(DISABLE_IO_INPUT)?;
        self.data(0x00)?;
        self.command(FUNCTION_SELECT)?;
        self.data(0x01)?;
        self.command(ENABLE_VSL_EXTERNAL)?;
        self.data(0xA0)?;
        self.data(0xFD)?;
        self.command(SET_CONTRAST_CURRENT)?;
        self.data(0xFF)?;
        self.command(MASTER_CONTRAST_CURRENT)?;
        self.data(0x0F)?;
        self.command(DEFAULT_GRAYSCALE)?;
        self.command(SET_PHASE_LENGTH)?;
        self.data(0xE2)?;
        self.command(ENHANCE_DRIVING_SCHEME)?;
        self.data(0x82)?;
        self.data(0x20)?;
        self.command(FIRST_PRE_CHARGE_VOLTAGE)?;
        self.data(0x1F)?;
        self.command(SECOND_PRE_CHARGE_VOLTAGE)?;
        self.data(0x08)?;
        self.command(VCOMH)?;
        self.data(0x07)?;
        self.command(SET_DISP_MODE_NORMAL)?;
        self.command(0xA9)?;
        self.driver.ms_delay(10);
        self.command(0xAF)?;
        self.driver.ms_delay(50);
        Ok(())
    }

    pub fn set_display_mode(&mut self, mode: Ssd1322Mode) -> Result<(), Driver::Error> {
        let command = match mode {
            Ssd1322Mode::Normal => SET_DISP_MODE_NORMAL,
            Ssd1322Mode::Inverted => SET_DISP_MODE_INVERTED,
            Ssd1322Mode::On => SET_DISP_MODE_ON,
            Ssd1322Mode::Off => SET_DISP_MODE_OFF,
        };
        self.command(command)
    }

    pub fn sleep_on(&mut self) -> Result<(), Driver::Error> {
        self.command(SLEEP_MODE_ON)
    }

    pub fn sleep_off(&mut self) -> Result<(), Driver::Error> {
        self.command(SLEEP_MODE_OFF)
    }

    pub fn set_contrast(&mut self, contrast: u8) -> Result<(), Driver::Error> {
        self.command(SET_CONTRAST_CURRENT)?;
        self.data(contrast)
    }

    pub fn set_brightness(&mut self, brightness: u8) -> Result<(), Driver::Error> {
        self.command(MASTER_CONTRAST_CURRENT)?;
        self.data(0x0F & brightness)
    }

    pub fn custom_grayscale(
        &mut self,
        grayscale_tab: &[u8; 16],
    ) -> Result<(), Ssd1322ApiError<Driver::Error>> {
        self.command(SET_GRAYSCALE_TABLE).map_err(Ssd1322ApiError::Driver)?;
        for &level in grayscale_tab {
            if level > 180 {
                return Err(Ssd1322ApiError::InvalidGrayscaleValue);
            }
            self.data(level).map_err(Ssd1322ApiError::Driver)?;
        }
        self.command(ENABLE_GRAYSCALE_TABLE)
            .map_err(Ssd1322ApiError::Driver)?;
        Ok(())
    }

    pub fn default_grayscale(&mut self) -> Result<(), Driver::Error> {
        self.command(SET_DEFAULT_GRAYSCALE_TAB)
    }

    pub fn set_window(
        &mut self,
        start_column: u8,
        end_column: u8,
        start_row: u8,
        end_row: u8,
    ) -> Result<(), Driver::Error> {
        self.command(SET_COLUMN_ADDR)?;
        self.data(start_column.wrapping_add(28))?;
        self.data(end_column.wrapping_add(28))?;
        self.command(SET_ROW_ADDR)?;
        self.data(start_row)?;
        self.data(end_row)
    }

    pub fn send_buffer(&mut self, buffer: &[u8]) -> Result<(), Driver::Error> {
        self.command(ENABLE_RAM_WRITE)?;
        self.driver.drive_cs_low()?;
        self.driver.drive_dc_high()?;
        self.driver.spi_send_array(buffer)?;
        self.driver.drive_cs_high()?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::{Ssd1322Api, Ssd1322ApiError, Ssd1322Driver, Ssd1322Mode};

    #[derive(Default)]
    struct TestDriver {
        bytes: std::vec::Vec<u8>,
    }

    impl TestDriver {
        fn new() -> Self {
            Self {
                bytes: std::vec::Vec::new(),
            }
        }

        fn output(&self) -> &[u8] {
            &self.bytes
        }
    }

    impl Ssd1322Driver for TestDriver {
        type Error = ();

        fn drive_cs_low(&mut self) -> Result<(), Self::Error> {
            Ok(())
        }

        fn drive_cs_high(&mut self) -> Result<(), Self::Error> {
            Ok(())
        }

        fn drive_dc_low(&mut self) -> Result<(), Self::Error> {
            Ok(())
        }

        fn drive_dc_high(&mut self) -> Result<(), Self::Error> {
            Ok(())
        }

        fn drive_reset_low(&mut self) -> Result<(), Self::Error> {
            Ok(())
        }

        fn drive_reset_high(&mut self) -> Result<(), Self::Error> {
            Ok(())
        }

        fn spi_send_byte(&mut self, byte: u8) -> Result<(), Self::Error> {
            self.bytes.push(byte);
            Ok(())
        }

        fn spi_send_array(&mut self, bytes: &[u8]) -> Result<(), Self::Error> {
            self.bytes.extend_from_slice(bytes);
            Ok(())
        }

        fn ms_delay(&mut self, _: u32) {}
    }

    #[test]
    fn custom_grayscale_rejects_values_outside_range() {
        let mut api = Ssd1322Api::new(TestDriver::new());
        let bad_table = [180u8; 16];
        let result = api.custom_grayscale(&bad_table);
        assert!(result.is_ok());

        let bad_table = [181u8; 16];
        let result = api.custom_grayscale(&bad_table);
        assert!(matches!(result, Err(Ssd1322ApiError::InvalidGrayscaleValue)));
    }

    #[test]
    fn set_window_offsets_columns_to_the_display_origin() {
        let mut api = Ssd1322Api::new(TestDriver::new());
        api.set_window(10, 20, 2, 7).unwrap();
        let driver = api.into_inner();
        let output = driver.output();
        assert_eq!(output, [0x15, 0x3A, 0x44, 0x75, 0x02, 0x07]);
    }

    #[test]
    fn display_mode_maps_to_the_expected_command() {
        let mut api = Ssd1322Api::new(TestDriver::new());
        api.set_display_mode(Ssd1322Mode::Inverted).unwrap();
        let driver = api.into_inner();
        let output = driver.output();
        assert_eq!(output, [0xA7]);
    }
}
