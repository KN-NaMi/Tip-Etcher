//! Hardware-independent operations required by the OLED menu renderer.

pub mod ssd1322_api;
pub mod ssd1322_hw;
pub mod ssd1322_gfx;

pub trait Display {
    type Error;

    fn clear(&mut self) -> Result<(), Self::Error>;
    fn send_buffer(&mut self) -> Result<(), Self::Error>;
}
