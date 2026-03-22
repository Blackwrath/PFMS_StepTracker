/*
 * TiledDisplayRenderer.h
 *
 *  Created on: 2 Mar 2026
 *      Author: james
 */

#ifndef _TILEDDISPLAYRENDERER_H_
#define _TILEDDISPLAYRENDERER_H_

//byte order is GGGl BBBBB RRRRR GGGh

#define TDR_COLOUR_BLACK 0x0000;
#define TDR_COLOUR_WHITE 0xFFFF;
#define TDR_COLOUR_BLUE  0b0001111100000000;
#define TDR_COLOUR_RED   0b0000000011111000;
#define TDR_COLOUR_GREEN 0b1110000000000111;

//extern static uint16_t displayTile[256]; //always keep in RAM
int32_t tan_approx(int32_t input);
//void generateDrawBackgroundTile(uint32_t steps, uint32_t maxsteps, uint8_t tile_x, uint8_t tile_y);
void TDR_draw_background_circle(uint32_t steps, uint32_t maxsteps);
void TDR_draw_number_sprite(uint16_t number, uint8_t startx, uint8_t starty);
void TDR_render_background_solid(uint16_t colour);
void TDR_clear_screen(void);
void TDR_draw_string(const char* s, int32_t startx, int32_t starty, uint8_t wraparound);

extern volatile unsigned char TDR_DMA_READY;
extern int TDR_TPS;

#endif /* APPLICATION_USER_LIB_TILEDDISPLAYRENDERER_H_ */
