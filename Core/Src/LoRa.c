#include "LoRa.h"
#include "main.h"
#include <string.h>

#define LORA_REG_VERSION 0x42

void LoRa_WriteRegister(LoRa *lora, uint8_t addr, uint8_t value) {
    uint8_t buf[2] = { addr | 0x80, value };
    HAL_GPIO_WritePin(lora->nss_port, lora->nss_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(lora->hspi, buf, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(lora->nss_port, lora->nss_pin, GPIO_PIN_SET);
}

uint8_t LoRa_ReadRegister(LoRa *lora, uint8_t addr) {
    uint8_t tx = addr & 0x7F;
    uint8_t rx = 0;
    HAL_GPIO_WritePin(lora->nss_port, lora->nss_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(lora->hspi, &tx, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(lora->hspi, &rx, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(lora->nss_port, lora->nss_pin, GPIO_PIN_SET);
    return rx;
}

// --- Reset LoRa (bez HAL_Delay) ---
void LoRa_Reset(LoRa *lora) {
    HAL_GPIO_WritePin(lora->reset_port, lora->reset_pin, GPIO_PIN_RESET);
    for (volatile int i = 0; i < 10000; i++); 
    HAL_GPIO_WritePin(lora->reset_port, lora->reset_pin, GPIO_PIN_SET);
    // for (volatile int i = 0; i < 100000; i++); 
}


uint8_t LoRa_Init(LoRa *lora) {
    LoRa_Reset(lora);


    uint8_t version = LoRa_ReadRegister(lora, LORA_REG_VERSION);
    if (version != 0x12) {
        return 0; // brak komunikacji
    }

    // Sleep
    LoRa_WriteRegister(lora, LORA_REG_OP_MODE, LORA_LONG_RANGE_MODE | LORA_MODE_SLEEP);
    // Standby
    LoRa_WriteRegister(lora, LORA_REG_OP_MODE, LORA_LONG_RANGE_MODE | LORA_MODE_STDBY);

    LoRa_SetFrequency(lora, 433000000);

    // FIFO RX
    LoRa_WriteRegister(lora, LORA_REG_FIFO_RX_BASE_ADDR, 0x00);
    LoRa_WriteRegister(lora, LORA_REG_FIFO_ADDR_PTR, 0x00);

    LoRa_WriteRegister(lora, LORA_REG_MODEM_CONFIG1, 0x72);
    LoRa_WriteRegister(lora, LORA_REG_MODEM_CONFIG2, 0x74);

    return 1;
}

void LoRa_SetFrequency(LoRa *lora, long frequency) {
    uint64_t frf = ((uint64_t)frequency << 19) / 32000000;
    LoRa_WriteRegister(lora, LORA_REG_FREQ_MSB, (uint8_t)(frf >> 16));
    LoRa_WriteRegister(lora, LORA_REG_FREQ_MID, (uint8_t)(frf >> 8));
    LoRa_WriteRegister(lora, LORA_REG_FREQ_LSB, (uint8_t)(frf >> 0));
}

void LoRa_ReceiveContinuous(LoRa *lora) {
    LoRa_WriteRegister(lora, LORA_REG_OP_MODE, LORA_LONG_RANGE_MODE | LORA_MODE_RXCONTINUOUS);
}

int LoRa_ReceivePacket(LoRa *lora, uint8_t *buffer, uint8_t maxlen) {
    uint8_t irqFlags = LoRa_ReadRegister(lora, LORA_REG_IRQ_FLAGS);

    if (irqFlags & 0x40) { // RX Done
        uint8_t len = LoRa_ReadRegister(lora, LORA_REG_RX_NB_BYTES);
        uint8_t fifoAddr = LoRa_ReadRegister(lora, LORA_REG_FIFO_RX_CURRENT);
        LoRa_WriteRegister(lora, LORA_REG_FIFO_ADDR_PTR, fifoAddr);

        for (uint8_t i = 0; i < len && i < maxlen; i++) {
            buffer[i] = LoRa_ReadRegister(lora, LORA_REG_FIFO);
        }

        LoRa_WriteRegister(lora, LORA_REG_IRQ_FLAGS, 0xFF);
        return len;
    }

    return 0; 
}
void LoRa_Send(LoRa *lora, uint8_t *data, uint8_t length) {
    LoRa_WriteRegister(lora, LORA_REG_OP_MODE, LORA_LONG_RANGE_MODE | LORA_MODE_STDBY);
    LoRa_WriteRegister(lora, LORA_REG_FIFO_ADDR_PTR, 0x00);
    LoRa_WriteRegister(lora, LORA_REG_PAYLOAD_LENGTH, length);

    for (uint8_t i = 0; i < length; i++) {
        LoRa_WriteRegister(lora, LORA_REG_FIFO, data[i]);
    }

    LoRa_WriteRegister(lora, LORA_REG_OP_MODE, LORA_LONG_RANGE_MODE | LORA_MODE_TX);

    while ((LoRa_ReadRegister(lora, LORA_REG_IRQ_FLAGS) & 0x08) == 0);
    LoRa_WriteRegister(lora, LORA_REG_IRQ_FLAGS, 0x08); 
}
