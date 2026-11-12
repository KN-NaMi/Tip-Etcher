use embedded_hal::digital::OutputPin;

use crate::hal;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(u8)]
pub enum GpioPin {
    Gpio0,
    Gpio1,
    Gpio2,
    Gpio3,
    Gpio4,
    Gpio5,
    Gpio6,
    Gpio7,
    Gpio8,
    Gpio9,
    Gpio10,
    Gpio11,
    Gpio12,
    Gpio13,
    Gpio14,
    Gpio15,
    Gpio16,
    Gpio17,
    Gpio18,
    Gpio19,
    Gpio20,
    Gpio21,
    Gpio22,
    Gpio23,
    Gpio24,
    Gpio25,
    Gpio26,
    Gpio27,
    Gpio28,
    Gpio29,
}

#[derive(Clone, Copy, Debug, Default)]
pub struct PinMap {
    pub board_button: Option<GpioPin>,
    pub oled_reset: Option<GpioPin>,
    pub oled_data_command: Option<GpioPin>,
    pub oled_chip_select: Option<GpioPin>,
    pub motor_enable: Option<GpioPin>,
    pub encoder_button: Option<GpioPin>,
    pub limit_switch_1: Option<GpioPin>,
    pub oled_spi_clock: Option<GpioPin>,
    pub oled_spi_data: Option<GpioPin>,
    pub status_led: Option<GpioPin>,
    pub encoder_a: Option<GpioPin>,
    pub encoder_b: Option<GpioPin>,
    pub tip_enable: Option<GpioPin>,
    pub voltage_select: Option<GpioPin>,
    pub motor_1_direction: Option<GpioPin>,
    pub motor_1_step: Option<GpioPin>,
    pub motor_2_direction: Option<GpioPin>,
    pub motor_2_step: Option<GpioPin>,
    pub current_sensor_i2c_clock: Option<GpioPin>,
    pub current_sensor_i2c_data: Option<GpioPin>,
}

// Assign RP GPIOs here after the target Pico board and pinout are chosen.
pub const PIN_MAP: PinMap = PinMap {
    board_button: None,
    oled_reset: None,
    oled_data_command: None,
    oled_chip_select: None,
    motor_enable: None,
    encoder_button: None,
    limit_switch_1: None,
    oled_spi_clock: None,
    oled_spi_data: None,
    status_led: None,
    encoder_a: None,
    encoder_b: None,
    tip_enable: None,
    voltage_select: None,
    motor_1_direction: None,
    motor_1_step: None,
    motor_2_direction: None,
    motor_2_step: None,
    current_sensor_i2c_clock: None,
    current_sensor_i2c_data: None,
};

pub struct BlinkyPins<LedPin> {
    pub led: LedPin,
}


//OLED: CS(port, pin), DC(port, pin), RESET(port, pin), 
//ENCODER: SIGA(port, pin), SIGB(port, pin), BTN(port, pin) 
pub struct MENU; 

pub fn configure_blinky(pins: hal::gpio::Pins) -> BlinkyPins<impl OutputPin> {
    BlinkyPins {
        led: pins.gpio25.into_push_pull_output(),
    }
}
