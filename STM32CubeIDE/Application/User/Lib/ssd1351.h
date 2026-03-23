#ifndef __SSD1351_H__
#define __SSD1351_H__

/*** Redefine if necessary ***/
#define SSD1351_SPI_PORT hspi1
extern SPI_HandleTypeDef SSD1351_SPI_PORT;

#define	SSD1351_BLACK   0x0000
#define	SSD1351_BLUE    0x001F
#define	SSD1351_RED     0xF800
#define	SSD1351_GREEN   0x07E0
#define SSD1351_CYAN    0x07FF
#define SSD1351_MAGENTA 0xF81F
#define SSD1351_YELLOW  0xFFE0
#define SSD1351_WHITE   0xFFFF
#define SSD1351_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

void SSD1351_Unselect();

void SSD1351_Init(void);
void SSD1351_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
//void SSD1351_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
void SSD1351_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void SSD1351_FillScreen(uint16_t color);
void SSD1351_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);
void SSD1351_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
//void SSD1351_InvertColors(bool invert);

// default orientation
#define SSD1351_WIDTH  128
#define SSD1351_HEIGHT 128

#endif // __SSD1351_H__
