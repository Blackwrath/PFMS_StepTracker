//Complete slop from Qwen3.6
/**
  * @file tlc5973.c
  * @brief TLC5973 Single-Wire LED Driver for STM32
  * @details
  *   - Uses only HAL_GPIO_WritePin for GPIO access.
  *   - Uses NOP loops for precise timing delays.
  *   - Implements timing-based protocol: tCYCLE measurement, 46-bit data, EOS.
  *   - Supports 12-bit 3-channel GS data.
  *   - No daisy-chaining support (SDO ignored).
  *
  * @note  TIMING WARNING:
  *        NOP loops are sensitive to compiler optimization.
  *        Ensure optimization level is set to O0 or use inline assembly
  *        for critical sections. This driver uses volatile loops.
  */

#include "stm32c0xx_hal.h"
#include "tlc5973.h"
#include <stdint.h>
#include <stdbool.h>

/* ================= Configuration ================= */

/**
  * @brief TLC5973 Device Handle
  */

/* ================= Private Functions ================= */

/**
  * @brief  Delays for a specified number of clock cycles using NOPs.
  *         Uses volatile loop to prevent optimization.
  */
__attribute__((optimize("O0")))
static void DelayNOP_Cycles(uint32_t cycles) {
    volatile uint32_t i;
    for (i = 0; i < cycles; i++) {
    	__ASM volatile ("nop");
    }
}

/**
  * @brief  Delays for a specified time in microseconds using NOPs.
  */
static void DelayNOP_us(uint32_t us) {
    if (us == 0) return;
    DelayNOP_Cycles(us);
}

/**
  * @brief  Transmits a single bit using the timing protocol.
  *         Structure: Edge1 -> Delay(Bit) -> Edge2
  *         Delay between Edge1 and Edge2 determines 0 or 1.
  */
static inline void TLC5973_SendBit(TLC5973_HandleTypeDef *hdev, uint8_t bit_val) {
    /* Calculate delay based on bit value */

    // Send SDI entry edge
    HAL_GPIO_WritePin(hdev->Port, hdev->Pin, GPIO_PIN_SET);
    DelayNOP_us(hdev->tEdge_ns);
    HAL_GPIO_WritePin(hdev->Port, hdev->Pin, GPIO_PIN_RESET);

    DelayNOP_us(TLC5973_DELAY_STAGE1);
    HAL_GPIO_WritePin(hdev->Port, hdev->Pin, bit_val);
    DelayNOP_us(hdev->tEdge_ns);
    HAL_GPIO_WritePin(hdev->Port, hdev->Pin, GPIO_PIN_RESET);
    DelayNOP_us(TLC5973_DELAY_STAGE3);

    //stage1, edge, and stage3 add up to tCYCLE

}

/**
  * @brief  Sends the tCYCLE measurement sequence.
  *         The TLC5973 measures the time between the first two rising edges.
  *         This sequence sets tCYCLE = hdev->tCYCLE_us.
  */
static inline void TLC5973_SendCycle(TLC5973_HandleTypeDef *hdev) {
    /* Edge 1 */
    HAL_GPIO_WritePin(hdev->Port, hdev->Pin, GPIO_PIN_SET);
    DelayNOP_us(hdev->tEdge_ns);
    HAL_GPIO_WritePin(hdev->Port, hdev->Pin, GPIO_PIN_RESET);

    /* Wait exactly tCYCLE */
    DelayNOP_us(TLC5973_DELAY_STAGE1);
    HAL_GPIO_WritePin(hdev->Port, hdev->Pin, GPIO_PIN_RESET);
    DelayNOP_us(hdev->tEdge_ns);
    HAL_GPIO_WritePin(hdev->Port, hdev->Pin, GPIO_PIN_RESET);
    DelayNOP_us(TLC5973_DELAY_STAGE3);

    /* Edge 2 */
    HAL_GPIO_WritePin(hdev->Port, hdev->Pin, GPIO_PIN_SET);
    DelayNOP_us(hdev->tEdge_ns);
    HAL_GPIO_WritePin(hdev->Port, hdev->Pin, GPIO_PIN_RESET);

    DelayNOP_us(TLC5973_DELAY_STAGE1);
    HAL_GPIO_WritePin(hdev->Port, hdev->Pin, GPIO_PIN_RESET);
    DelayNOP_us(hdev->tEdge_ns);
    HAL_GPIO_WritePin(hdev->Port, hdev->Pin, GPIO_PIN_RESET);
    DelayNOP_us(TLC5973_DELAY_STAGE3);
}

/**
  * @brief  Sends the End of Sequence (EOS).
  *         SDI held low for tH0 locks the data.
  */
static void TLC5973_SendEOS(TLC5973_HandleTypeDef *hdev) {
    /* Ensure line is low */
    HAL_GPIO_WritePin(hdev->Port, hdev->Pin, GPIO_PIN_RESET);
    DelayNOP_us(hdev->tH0_ns);
}

/**
  * @brief  Sends a stream of bits MSB first.
  */
static inline void TLC5973_SendBits(TLC5973_HandleTypeDef *hdev, uint32_t data, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        /* Extract MSB */
        uint8_t bit = (data >> (count - 1 - i)) & 0x01;
        TLC5973_SendBit(hdev, bit);
    }
}

/* ================= Public API ================= */

/**
  * @brief  Initializes the TLC5973 handle.
  * @param  hdev: Pointer to TLC5973 handle
  * @param  port: GPIO Port
  * @param  pin:  GPIO Pin
  * @param  tCYCLE_us: Desired cycle time in us (0.33 to 10.0)
  */
void TLC5973_Init(TLC5973_HandleTypeDef *hdev, GPIO_TypeDef *port, uint16_t pin, uint32_t tCYCLE_ns) {
    hdev->Port = port;
    hdev->Pin = pin;

    /* Clamp tCYCLE to valid range */
    if (tCYCLE_ns < 100) {tCYCLE_ns = 100;}
    if (tCYCLE_ns > 100000) {tCYCLE_ns = 100000;}

    hdev->tCYCLE_ns = TLC5973_DEFAULT_tCYCLE_US;
    hdev->tEdge_ns = TLC5973_DEFAULT_tEDGE_US;
    hdev->tH0_ns = TLC5973_DEFAULT_tH0_US;

    /* Ensure SDI is initially low */
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

/**
  * @brief  Writes 3-channel 12-bit GS data to the TLC5973.
  *         Sequence: tCYCLE -> Cmd(3AA) -> GS(36 bits) -> EOS
  *
  * @param  hdev: Pointer to TLC5973 handle
  * @param  ch0:  Channel 0 data (0-4095). MSB of GS stream.
  * @param  ch1:  Channel 1 data (0-4095).
  * @param  ch2:  Channel 2 data (0-4095). LSB of GS stream.
  * @note   Assumes power-on or reset has occurred, or tCYCLE is re-sent.
  *         This function re-sends tCYCLE every call to ensure synchronization.
  */
void TLC5973_WriteChannels(TLC5973_HandleTypeDef *hdev, uint16_t ch0, uint16_t ch1, uint16_t ch2) {
    /* 1. Send tCYCLE measurement sequence */
	__ASM volatile ("cpsid i" : : : "memory"); //disable interrupts
	uint64_t gs_stream = ((uint64_t)ch0 << 24) | ((uint32_t)ch1 << 12) | (uint32_t)ch2;
    TLC5973_SendCycle(hdev);

    /* 2. Send Command 0x3AA (10 bits) */
    TLC5973_SendBits(hdev, TLC5973_CMD_3AA, TLC5973_CMD_BITS);

    /* 3. Send GS Data (36 bits) */
    /* Pack channels: Ch0[11:0] Ch1[11:0] Ch2[11:0] */
    /* Shift Ch0 to bits 35-24, Ch1 to 23-12, Ch2 to 11-0 */


    TLC5973_SendBits(hdev, (uint32_t)(gs_stream >> 12), 24); /* Upper 24 bits */
    TLC5973_SendBits(hdev, (uint32_t)gs_stream, 12);        /* Lower 12 bits */

    /* 4. Send EOS */
    TLC5973_SendEOS(hdev);
    __ASM volatile ("cpsie i" : : : "memory"); //enable interrupts
}

/**
  * @brief  Writes raw 46-bit data stream (Command + GS).
  *         Useful for advanced control or debugging.
  *
  * @param  hdev: Pointer to TLC5973 handle
  * @param  data_46: 46-bit data (Command in bits 45:36, GS in bits 35:0)
  */
void TLC5973_WriteRaw(TLC5973_HandleTypeDef *hdev, uint64_t data_46) {
    TLC5973_SendCycle(hdev);

    /* Send 10 Command Bits */
    TLC5973_SendBits(hdev, (uint32_t)(data_46 >> 36), TLC5973_CMD_BITS);

    /* Send 36 GS Bits */
    TLC5973_SendBits(hdev, (uint32_t)(data_46 >> 12), 24);
    TLC5973_SendBits(hdev, (uint32_t)data_46, 12);

    TLC5973_SendEOS(hdev);
}
