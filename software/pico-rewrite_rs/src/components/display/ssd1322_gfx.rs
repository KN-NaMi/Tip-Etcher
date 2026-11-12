//! Allocation-free 4-bpp drawing primitives for the SSD1322 framebuffer.

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum GfxError {
    EmptyDimensions,
    DimensionsTooLarge,
    BufferSizeOverflow,
    BufferTooSmall { required: usize, actual: usize },
    BitmapTooShort { required: usize, actual: usize },
    InvalidFontRange,
    GlyphsTooShort { required: usize, actual: usize },
    FontBitmapTooShort { required: usize, actual: usize },
    FontNotSelected,
}

/// A glyph's metrics and bit offset in a [`GfxFont`].
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct GfxGlyph {
    pub bitmap_offset: u16,
    pub width: u8,
    pub height: u8,
    pub x_advance: u8,
    pub x_offset: i8,
    pub y_offset: i8,
}

/// Borrowed bitmap and glyph tables in Adafruit GFX font format.
#[derive(Clone, Copy, Debug)]
pub struct GfxFont<'a> {
    bitmap: &'a [u8],
    glyphs: &'a [GfxGlyph],
    first: u8,
    last: u8,
    pub y_advance: u8,
}

impl<'a> GfxFont<'a> {
    /// Creates a font after checking its character range and glyph table.
    pub fn new(
        bitmap: &'a [u8],
        glyphs: &'a [GfxGlyph],
        first: u8,
        last: u8,
        y_advance: u8,
    ) -> Result<Self, GfxError> {
        if first > last {
            return Err(GfxError::InvalidFontRange);
        }

        let required = usize::from(last - first) + 1;
        if glyphs.len() < required {
            return Err(GfxError::GlyphsTooShort {
                required,
                actual: glyphs.len(),
            });
        }

        Ok(Self {
            bitmap,
            glyphs,
            first,
            last,
            y_advance,
        })
    }

    fn glyph(&self, character: char) -> Option<&GfxGlyph> {
        let code = u8::try_from(u32::from(character)).ok()?;
        if !(self.first..=self.last).contains(&code) {
            return None;
        }
        self.glyphs.get(usize::from(code - self.first))
    }
}

pub struct Ssd1322Gfx<'buffer, 'font> {
    width: usize,
    height: usize,
    buffer: &'buffer mut [u8],
    font: Option<&'font GfxFont<'font>>,
}

impl<'buffer, 'font> Ssd1322Gfx<'buffer, 'font> {
    /// Creates a drawing context over a 4-bpp framebuffer.
    pub fn new(width: usize, height: usize, buffer: &'buffer mut [u8]) -> Result<Self, GfxError> {
        if width == 0 || height == 0 {
            return Err(GfxError::EmptyDimensions);
        }
        if width > u16::MAX as usize || height > u16::MAX as usize {
            return Err(GfxError::DimensionsTooLarge);
        }

        let pixels = width
            .checked_mul(height)
            .ok_or(GfxError::BufferSizeOverflow)?;
        let required = pixels / 2 + pixels % 2;
        if buffer.len() < required {
            return Err(GfxError::BufferTooSmall {
                required,
                actual: buffer.len(),
            });
        }

        Ok(Self {
            width,
            height,
            buffer,
            font: None,
        })
    }

    pub const fn width(&self) -> usize {
        self.width
    }

    pub const fn height(&self) -> usize {
        self.height
    }

    pub fn framebuffer(&self) -> &[u8] {
        &self.buffer[..self.buffer_len()]
    }

    /// Returns the packed framebuffer slice for use by a display/API layer.
    pub fn framebuffer_mut(&mut self) -> &mut [u8] {
        let len = self.buffer_len();
        &mut self.buffer[..len]
    }

    pub fn set_font(&mut self, font: &'font GfxFont<'font>) {
        self.font = Some(font);
    }

    pub fn clear_font(&mut self) {
        self.font = None;
    }

    pub fn fill(&mut self, brightness: u8) {
        let value = brightness.min(0x0f);
        self.framebuffer_mut().fill((value << 4) | value);
    }

    pub fn draw_pixel(&mut self, x: i32, y: i32, brightness: u8) -> bool {
        self.draw_pixel_at(i64::from(x), i64::from(y), brightness)
    }

    pub fn draw_vertical_line(&mut self, x: i32, y0: i32, y1: i32, brightness: u8) {
        if x < 0 || x as usize >= self.width {
            return;
        }

        let start = y0.min(y1).max(0);
        let end = y0.max(y1).min(self.height as i32 - 1);
        for y in start..=end {
            self.draw_pixel(x, y, brightness);
        }
    }

    pub fn draw_horizontal_line(&mut self, y: i32, x0: i32, x1: i32, brightness: u8) {
        if y < 0 || y as usize >= self.height {
            return;
        }

        let start = x0.min(x1).max(0);
        let end = x0.max(x1).min(self.width as i32 - 1);
        for x in start..=end {
            self.draw_pixel(x, y, brightness);
        }
    }

    pub fn draw_line(&mut self, x0: i32, y0: i32, x1: i32, y1: i32, brightness: u8) {
        let Some((mut x0, mut y0, x1, y1)) = self.clip_line(x0, y0, x1, y1) else {
            return;
        };

        let dx = (x1 - x0).abs();
        let sx = if x0 < x1 { 1 } else { -1 };
        let dy = -(y1 - y0).abs();
        let sy = if y0 < y1 { 1 } else { -1 };
        let mut error = dx + dy;

        loop {
            self.draw_pixel_at(x0, y0, brightness);
            if x0 == x1 && y0 == y1 {
                break;
            }

            let doubled_error = 2 * error;
            if doubled_error >= dy {
                error += dy;
                x0 += sx;
            }
            if doubled_error <= dx {
                error += dx;
                y0 += sy;
            }
        }
    }

    pub fn draw_aa_line(&mut self, x0: i32, y0: i32, x1: i32, y1: i32, brightness: u8) {
        if x0 == x1 || y0 == y1 {
            self.draw_line(x0, y0, x1, y1, brightness);
            return;
        }

        let Some((mut x0, mut y0, mut x1, mut y1)) = self.clip_line(x0, y0, x1, y1) else {
            return;
        };
        if x0 == x1 || y0 == y1 {
            self.draw_line(x0 as i32, y0 as i32, x1 as i32, y1 as i32, brightness);
            return;
        }

        let steep = (y1 - y0).abs() > (x1 - x0).abs();
        if steep {
            core::mem::swap(&mut x0, &mut y0);
            core::mem::swap(&mut x1, &mut y1);
        }
        if x0 > x1 {
            core::mem::swap(&mut x0, &mut x1);
            core::mem::swap(&mut y0, &mut y1);
        }

        let gradient = (y1 - y0) * AA_ONE / (x1 - x0);
        self.plot_aa_pair(steep, x0, y0 * AA_ONE, AA_ONE / 2, brightness);
        self.plot_aa_pair(steep, x1, y1 * AA_ONE, AA_ONE / 2, brightness);

        let mut inter_y = y0 * AA_ONE + gradient;
        let start = x0 + 1;
        let end = x1 - 1;
        for x in start..=end {
            self.plot_aa_pair(steep, x, inter_y, AA_ONE, brightness);
            inter_y = inter_y.saturating_add(gradient);
        }
    }

    pub fn draw_rectangle(&mut self, x0: i32, y0: i32, x1: i32, y1: i32, brightness: u8) {
        self.draw_vertical_line(x0, y0, y1, brightness);
        self.draw_vertical_line(x1, y0, y1, brightness);
        self.draw_horizontal_line(y0, x0, x1, brightness);
        self.draw_horizontal_line(y1, x0, x1, brightness);
    }

    pub fn fill_rectangle(&mut self, x0: i32, y0: i32, x1: i32, y1: i32, brightness: u8) {
        let left = x0.min(x1).max(0);
        let right = x0.max(x1).min(self.width as i32 - 1);
        let top = y0.min(y1).max(0);
        let bottom = y0.max(y1).min(self.height as i32 - 1);

        for y in top..=bottom {
            self.draw_horizontal_line(y, left, right, brightness);
        }
    }

    pub fn draw_circle(&mut self, x: i32, y: i32, radius: u16, brightness: u8) {
        let (center_x, center_y, radius) = (i64::from(x), i64::from(y), i64::from(radius));
        let (mut offset_x, mut offset_y) = (0_i64, radius);
        let (mut decision, mut delta_x, mut delta_y) = (1 - radius, 1_i64, -2 * radius);

        while offset_x <= offset_y {
            self.plot_circle_octants(center_x, center_y, offset_x, offset_y, brightness);
            if decision >= 0 {
                offset_y -= 1;
                delta_y += 2;
                decision += delta_y;
            }
            offset_x += 1;
            delta_x += 2;
            decision += delta_x;
        }
    }

    /// Draws an 8-bpp grayscale bitmap using each source byte's high nibble.
    pub fn draw_bitmap_8bpp(
        &mut self,
        bitmap: &[u8],
        x: i32,
        y: i32,
        bitmap_width: usize,
        bitmap_height: usize,
    ) -> Result<(), GfxError> {
        let required = bitmap_width
            .checked_mul(bitmap_height)
            .ok_or(GfxError::BufferSizeOverflow)?;
        if bitmap.len() < required {
            return Err(GfxError::BitmapTooShort {
                required,
                actual: bitmap.len(),
            });
        }

        let (left, top, right, bottom) =
            self.visible_bitmap_rect(x, y, bitmap_width, bitmap_height);
        for source_y in top..bottom {
            for source_x in left..right {
                let source_index = source_y * bitmap_width + source_x;
                self.draw_pixel_at(
                    i64::from(x) + source_x as i64,
                    i64::from(y) + source_y as i64,
                    bitmap[source_index] >> 4,
                );
            }
        }
        Ok(())
    }

    /// Draws a 4-bpp bitmap packed high-nibble first across rows.
    pub fn draw_bitmap_4bpp(
        &mut self,
        bitmap: &[u8],
        x: i32,
        y: i32,
        bitmap_width: usize,
        bitmap_height: usize,
    ) -> Result<(), GfxError> {
        let pixels = bitmap_width
            .checked_mul(bitmap_height)
            .ok_or(GfxError::BufferSizeOverflow)?;
        let required = pixels / 2 + pixels % 2;
        if bitmap.len() < required {
            return Err(GfxError::BitmapTooShort {
                required,
                actual: bitmap.len(),
            });
        }

        let (left, top, right, bottom) =
            self.visible_bitmap_rect(x, y, bitmap_width, bitmap_height);
        for source_y in top..bottom {
            for source_x in left..right {
                let source_index = source_y * bitmap_width + source_x;
                let packed = bitmap[source_index / 2];
                let value = if source_index % 2 == 0 {
                    packed >> 4
                } else {
                    packed & 0x0f
                };
                self.draw_pixel_at(
                    i64::from(x) + source_x as i64,
                    i64::from(y) + source_y as i64,
                    value,
                );
            }
        }
        Ok(())
    }

    /// Draws one font glyph with `(x, y)` at the font baseline origin.
    pub fn draw_char(
        &mut self,
        character: char,
        x: i32,
        y: i32,
        brightness: u8,
    ) -> Result<(), GfxError> {
        let font = self.font.ok_or(GfxError::FontNotSelected)?;
        self.draw_glyph(font, character, i64::from(x), i64::from(y), brightness)
    }

    /// Draws baseline-positioned text; newline resets x and advances by the font line height.
    pub fn draw_text(
        &mut self,
        text: &str,
        x: i32,
        y: i32,
        brightness: u8,
    ) -> Result<(), GfxError> {
        let font = self.font.ok_or(GfxError::FontNotSelected)?;
        let start_x = i64::from(x);
        let mut cursor_x = start_x;
        let mut cursor_y = i64::from(y);

        for character in text.chars() {
            if character == '\n' {
                cursor_x = start_x;
                cursor_y += i64::from(font.y_advance);
                continue;
            }
            if character == '\r' {
                continue;
            }

            if let Some(glyph) = font.glyph(character) {
                self.draw_glyph(font, character, cursor_x, cursor_y, brightness)?;
                cursor_x += i64::from(glyph.x_advance);
            }
        }
        Ok(())
    }

    fn buffer_len(&self) -> usize {
        let pixels = self.width * self.height;
        pixels / 2 + pixels % 2
    }

    fn draw_pixel_at(&mut self, x: i64, y: i64, brightness: u8) -> bool {
        if x < 0 || y < 0 || x >= self.width as i64 || y >= self.height as i64 {
            return false;
        }

        let pixel = y as usize * self.width + x as usize;
        let byte = &mut self.buffer[pixel / 2];
        let value = brightness.min(0x0f);
        if pixel % 2 == 0 {
            *byte = (*byte & 0x0f) | (value << 4);
        } else {
            *byte = (*byte & 0xf0) | value;
        }
        true
    }

    fn draw_glyph(
        &mut self,
        font: &GfxFont<'_>,
        character: char,
        x: i64,
        y: i64,
        brightness: u8,
    ) -> Result<(), GfxError> {
        let Some(glyph) = font.glyph(character) else {
            return Ok(());
        };
        let pixel_count = usize::from(glyph.width) * usize::from(glyph.height);
        let bitmap_len = pixel_count / 8 + usize::from(pixel_count % 8 != 0);
        if pixel_count == 0 {
            return Ok(());
        }

        let bitmap_start = usize::from(glyph.bitmap_offset);
        let bitmap_end = bitmap_start
            .checked_add(bitmap_len)
            .ok_or(GfxError::BufferSizeOverflow)?;
        let bitmap =
            font.bitmap
                .get(bitmap_start..bitmap_end)
                .ok_or(GfxError::FontBitmapTooShort {
                    required: bitmap_end,
                    actual: font.bitmap.len(),
                })?;

        for bit_index in 0..pixel_count {
            if bitmap[bit_index / 8] & (0x80 >> (bit_index % 8)) != 0 {
                let glyph_x = bit_index % usize::from(glyph.width);
                let glyph_y = bit_index / usize::from(glyph.width);
                self.draw_pixel_at(
                    x + i64::from(glyph.x_offset) + glyph_x as i64,
                    y + i64::from(glyph.y_offset) + glyph_y as i64,
                    brightness,
                );
            }
        }
        Ok(())
    }

    fn plot_aa_pair(&mut self, steep: bool, x: i64, y: i64, gap: i64, brightness: u8) {
        let y_floor = y.div_euclid(AA_ONE);
        let fraction = y.rem_euclid(AA_ONE);
        let lower = aa_brightness(brightness, (AA_ONE - fraction) * gap / AA_ONE);
        let upper = aa_brightness(brightness, fraction * gap / AA_ONE);

        if steep {
            self.draw_pixel_at(y_floor, x, lower);
            self.draw_pixel_at(y_floor + 1, x, upper);
        } else {
            self.draw_pixel_at(x, y_floor, lower);
            self.draw_pixel_at(x, y_floor + 1, upper);
        }
    }

    fn plot_circle_octants(
        &mut self,
        center_x: i64,
        center_y: i64,
        offset_x: i64,
        offset_y: i64,
        brightness: u8,
    ) {
        let points = [
            (offset_x, offset_y),
            (-offset_x, offset_y),
            (offset_x, -offset_y),
            (-offset_x, -offset_y),
            (offset_y, offset_x),
            (-offset_y, offset_x),
            (offset_y, -offset_x),
            (-offset_y, -offset_x),
        ];
        for (dx, dy) in points {
            self.draw_pixel_at(center_x + dx, center_y + dy, brightness);
        }
    }

    fn visible_bitmap_rect(
        &self,
        x: i32,
        y: i32,
        bitmap_width: usize,
        bitmap_height: usize,
    ) -> (usize, usize, usize, usize) {
        let x = i64::from(x);
        let y = i64::from(y);
        let left = (-x).max(0).min(bitmap_width as i64) as usize;
        let top = (-y).max(0).min(bitmap_height as i64) as usize;
        let right = (self.width as i64 - x).max(0).min(bitmap_width as i64) as usize;
        let bottom = (self.height as i64 - y).max(0).min(bitmap_height as i64) as usize;
        (left, top, right, bottom)
    }

    fn clip_line(&self, x0: i32, y0: i32, x1: i32, y1: i32) -> Option<(i64, i64, i64, i64)> {
        let (mut x0, mut y0, mut x1, mut y1) =
            (i64::from(x0), i64::from(y0), i64::from(x1), i64::from(y1));
        let (x_max, y_max) = (self.width as i64 - 1, self.height as i64 - 1);

        loop {
            let code0 = out_code(x0, y0, x_max, y_max);
            let code1 = out_code(x1, y1, x_max, y_max);
            if code0 | code1 == 0 {
                return Some((x0, y0, x1, y1));
            }
            if code0 & code1 != 0 {
                return None;
            }

            let code = if code0 != 0 { code0 } else { code1 };
            let (x, y) = if code & OUT_BOTTOM != 0 {
                let y = y_max;
                (interpolate(x0, x1 - x0, y - y0, y1 - y0), y)
            } else if code & OUT_TOP != 0 {
                let y = 0;
                (interpolate(x0, x1 - x0, y - y0, y1 - y0), y)
            } else if code & OUT_RIGHT != 0 {
                let x = x_max;
                (x, interpolate(y0, y1 - y0, x - x0, x1 - x0))
            } else {
                let x = 0;
                (x, interpolate(y0, y1 - y0, x - x0, x1 - x0))
            };

            if code == code0 {
                (x0, y0) = (x, y);
            } else {
                (x1, y1) = (x, y);
            }
        }
    }
}

const OUT_LEFT: u8 = 1;
const OUT_RIGHT: u8 = 2;
const OUT_TOP: u8 = 4;
const OUT_BOTTOM: u8 = 8;
const AA_ONE: i64 = 1 << 16;

fn aa_brightness(brightness: u8, coverage: i64) -> u8 {
    let scaled = (i64::from(brightness.min(0x0f)) * coverage + AA_ONE / 2) / AA_ONE;
    scaled as u8
}

fn out_code(x: i64, y: i64, x_max: i64, y_max: i64) -> u8 {
    let mut code = 0;
    if x < 0 {
        code |= OUT_LEFT;
    } else if x > x_max {
        code |= OUT_RIGHT;
    }
    if y < 0 {
        code |= OUT_TOP;
    } else if y > y_max {
        code |= OUT_BOTTOM;
    }
    code
}

fn interpolate(start: i64, delta: i64, numerator: i64, denominator: i64) -> i64 {
    (i128::from(start) + i128::from(delta) * i128::from(numerator) / i128::from(denominator)) as i64
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn pixels_are_packed_high_nibble_first_and_brightness_is_clamped() {
        let mut buffer = [0; 4];
        let mut gfx = Ssd1322Gfx::new(4, 2, &mut buffer).unwrap();
        gfx.fill(0x1f);
        assert_eq!(gfx.framebuffer(), &[0xff; 4]);

        gfx.draw_pixel(0, 0, 2);
        gfx.draw_pixel(1, 0, 3);
        assert_eq!(gfx.framebuffer()[0], 0x23);
        assert!(gfx.draw_pixel(0, 1, 0x10));
        assert_eq!(gfx.framebuffer()[2], 0xff);
        assert!(!gfx.draw_pixel(-1, 0, 1));
    }

    #[test]
    fn lines_clip_to_the_framebuffer_and_include_both_endpoints() {
        let mut buffer = [0; 4];
        let mut gfx = Ssd1322Gfx::new(4, 2, &mut buffer).unwrap();
        gfx.draw_line(-5, 1, 5, 1, 7);
        assert_eq!(gfx.framebuffer(), &[0x00, 0x00, 0x77, 0x77]);

        gfx.draw_vertical_line(2, 1, -3, 4);
        assert_eq!(gfx.framebuffer()[1], 0x40);
        assert_eq!(gfx.framebuffer()[3], 0x47);
    }

    #[test]
    fn lines_with_extreme_coordinates_clip_without_overflow() {
        let mut buffer = [0; 13];
        let mut gfx = Ssd1322Gfx::new(5, 5, &mut buffer).unwrap();
        gfx.draw_line(i32::MIN, i32::MIN, i32::MAX, i32::MAX, 9);

        for coordinate in 0..5 {
            assert_eq!(pixel(gfx.framebuffer(), 5, coordinate, coordinate), 9);
        }
    }

    #[test]
    fn four_bit_bitmaps_keep_source_nibble_order_for_odd_widths() {
        let mut buffer = [0; 3];
        let mut gfx = Ssd1322Gfx::new(3, 2, &mut buffer).unwrap();
        gfx.draw_bitmap_4bpp(&[0x12, 0x34, 0x56], 0, 0, 3, 2)
            .unwrap();
        assert_eq!(gfx.framebuffer(), &[0x12, 0x34, 0x56]);
    }

    #[test]
    fn eight_bit_bitmaps_clip_and_convert_grayscale_to_four_bits() {
        let mut buffer = [0; 3];
        let mut gfx = Ssd1322Gfx::new(3, 2, &mut buffer).unwrap();
        gfx.draw_bitmap_8bpp(&[0x00, 0x1f, 0x2a, 0x34, 0x56, 0xff], -1, 0, 3, 2)
            .unwrap();
        assert_eq!(gfx.framebuffer(), &[0x12, 0x05, 0xf0]);
    }

    #[test]
    fn antialiased_lines_use_fixed_point_coverage() {
        let mut buffer = [0; 4];
        let mut gfx = Ssd1322Gfx::new(4, 2, &mut buffer).unwrap();
        gfx.draw_aa_line(0, 0, 3, 1, 15);
        assert_eq!(gfx.framebuffer(), &[0x8a, 0x50, 0x05, 0xa8]);
    }

    #[test]
    fn rectangles_and_circles_draw_clipped_pixels() {
        let mut buffer = [0; 13];
        let mut gfx = Ssd1322Gfx::new(5, 5, &mut buffer).unwrap();
        gfx.draw_rectangle(-1, -1, 2, 2, 6);
        gfx.draw_circle(3, 3, 1, 9);

        assert_eq!(pixel(gfx.framebuffer(), 5, 2, 0), 6);
        assert_eq!(pixel(gfx.framebuffer(), 5, 2, 1), 6);
        assert_eq!(pixel(gfx.framebuffer(), 5, 3, 2), 9);
        assert_eq!(pixel(gfx.framebuffer(), 5, 2, 3), 9);
        assert_eq!(pixel(gfx.framebuffer(), 5, 3, 3), 0);
    }

    #[test]
    fn malformed_glyph_bitmap_returns_an_error() {
        let glyphs = [GfxGlyph {
            bitmap_offset: 0,
            width: 8,
            height: 1,
            x_advance: 8,
            x_offset: 0,
            y_offset: 0,
        }];
        let font = GfxFont::new(&[], &glyphs, b'A', b'A', 1).unwrap();
        let mut buffer = [0; 2];
        let mut gfx = Ssd1322Gfx::new(2, 2, &mut buffer).unwrap();
        gfx.set_font(&font);

        assert_eq!(
            gfx.draw_char('A', 0, 0, 15),
            Err(GfxError::FontBitmapTooShort {
                required: 1,
                actual: 0,
            })
        );
    }

    fn pixel(buffer: &[u8], width: usize, x: usize, y: usize) -> u8 {
        let pixel = y * width + x;
        if pixel % 2 == 0 {
            buffer[pixel / 2] >> 4
        } else {
            buffer[pixel / 2] & 0x0f
        }
    }

    #[test]
    fn glyphs_render_msb_first_and_text_handles_newlines() {
        let bitmap = [0b1010_0000];
        let glyphs = [
            GfxGlyph {
                bitmap_offset: 0,
                width: 3,
                height: 1,
                x_advance: 4,
                x_offset: 0,
                y_offset: 0,
            },
            GfxGlyph {
                bitmap_offset: 0,
                width: 3,
                height: 1,
                x_advance: 4,
                x_offset: 0,
                y_offset: 0,
            },
        ];
        let font = GfxFont::new(&bitmap, &glyphs, b'A', b'B', 2).unwrap();
        let mut buffer = [0; 8];
        let mut gfx = Ssd1322Gfx::new(4, 4, &mut buffer).unwrap();
        gfx.set_font(&font);
        gfx.draw_text("A\nB", 0, 0, 0x0f).unwrap();
        assert_eq!(
            gfx.framebuffer(),
            &[0xf0, 0xf0, 0x00, 0x00, 0xf0, 0xf0, 0x00, 0x00]
        );
    }

    #[test]
    fn invalid_buffer_and_font_data_return_errors() {
        let mut small_buffer = [0; 1];
        assert_eq!(
            Ssd1322Gfx::new(4, 2, &mut small_buffer).err(),
            Some(GfxError::BufferTooSmall {
                required: 4,
                actual: 1,
            })
        );
        assert_eq!(
            GfxFont::new(&[], &[], b'A', b'B', 1).err(),
            Some(GfxError::GlyphsTooShort {
                required: 2,
                actual: 0,
            })
        );
    }
}
