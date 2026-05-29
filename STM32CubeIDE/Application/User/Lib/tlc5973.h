/*
 * tlc5973.h
 *
 *  Created on: 29 Apr 2026
 *      Author: james
 */

#ifndef APPLICATION_USER_LIB_TLC5973_H_
#define APPLICATION_USER_LIB_TLC5973_H_

typedef struct {
    GPIO_TypeDef *Port;
    uint16_t Pin;

    /* Timing Configuration (in microseconds) */
    uint32_t tCYCLE_ns;   /* Base cycle time: 0.33us to 10us. Default 2.0us */
    uint32_t tEdge_ns;    /* Pulse width of SDI edges. Should be > 0.1us */
    uint32_t tH0_ns;      /* EOS hold time. Should be >= 0.5us */
} TLC5973_HandleTypeDef;

/* Default Timing Constants */
#define TLC5973_DEFAULT_tCYCLE_US  200
#define TLC5973_DEFAULT_tEDGE_US   20
#define TLC5973_DEFAULT_tH0_US     1600

#define TLC5973_DELAY_STAGE1  30
#define TLC5973_DELAY_STAGE2  TLC5973_DEFAULT_tEDGE_US
#define TLC5973_DELAY_STAGE3  150

/* Protocol Constants */
#define TLC5973_CMD_3AA            0x3AA  /* 10-bit command: 0b1110101010 */
#define TLC5973_CMD_BITS           10
#define TLC5973_GS_BITS            36
#define TLC5973_TOTAL_BITS         48

#define TLC5973_RATIO              0.15

#define TLC5973_T1_RATIO           0.45
#define TLC5973_T0_RATIO           0.55

static void DelayNOP_Cycles(uint32_t cycles);
static void DelayNOP_us(uint32_t us);
static void TLC5973_SendBit(TLC5973_HandleTypeDef *hdev, uint8_t bit_val);
static void TLC5973_SendCycle(TLC5973_HandleTypeDef *hdev);

static void TLC5973_SendEOS(TLC5973_HandleTypeDef *hdev);
static void TLC5973_SendBits(TLC5973_HandleTypeDef *hdev, uint32_t data, uint32_t count);
void TLC5973_Init(TLC5973_HandleTypeDef *hdev, GPIO_TypeDef *port, uint16_t pin, uint32_t tCYCLE_ns);
void TLC5973_WriteChannels(TLC5973_HandleTypeDef *hdev, uint16_t ch0, uint16_t ch1, uint16_t ch2);
void TLC5973_WriteRaw(TLC5973_HandleTypeDef *hdev, uint64_t data_46);


#endif /* APPLICATION_USER_LIB_TLC5973_H_ */
