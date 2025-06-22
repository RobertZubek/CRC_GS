#ifndef LORA_SX1278_H
#define LORA_SX1278_H

#include "stm32f4xx_hal.h"  

#define LORA_REG_FIFO               0x00
#define LORA_REG_OP_MODE           0x01
#define LORA_REG_FIFO_ADDR_PTR     0x0D
#define LORA_REG_FIFO_RX_BASE_ADDR 0x0F
#define LORA_REG_FIFO_RX_CURRENT   0x10
#define LORA_REG_IRQ_FLAGS         0x12
#define LORA_REG_RX_NB_BYTES       0x13
#define LORA_REG_MODEM_CONFIG1     0x1D
#define LORA_REG_MODEM_CONFIG2     0x1E
#define LORA_REG_PAYLOAD_LENGTH    0x22
#define LORA_REG_FREQ_MSB          0x06
#define LORA_REG_FREQ_MID          0x07
#define LORA_REG_FREQ_LSB          0x08

#define LORA_MODE_SLEEP            0x00
#define LORA_MODE_STDBY            0x01
#define LORA_MODE_RXCONTINUOUS     0x85
#define LORA_LONG_RANGE_MODE       0x80
#define LORA_MODE_TX      0x03


typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *nss_port;
    uint16_t nss_pin;
    GPIO_TypeDef *reset_port;
    uint16_t reset_pin;
} LoRa;

uint8_t LoRa_Init(LoRa *lora);
void LoRa_SetFrequency(LoRa *lora, long frequency);
void LoRa_ReceiveContinuous(LoRa *lora);
int LoRa_ReceivePacket(LoRa *lora, uint8_t *buffer, uint8_t maxlen);
void LoRa_Reset(LoRa *lora);
void LoRa_Send(LoRa *lora, uint8_t *data, uint8_t length);
void LoRa_WriteRegister(LoRa *lora, uint8_t addr, uint8_t value);
uint8_t LoRa_ReadRegister(LoRa *lora, uint8_t addr);



#endif
