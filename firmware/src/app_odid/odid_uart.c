#include "odid_uart.h"
#include "odid_mavlink.h"
#include "definitions.h"

#define ODID_UART_RX_BUFFER_SIZE  64

static uint8_t s_rxBuffer[ODID_UART_RX_BUFFER_SIZE];

static void ODID_UART_ReadCallback(SERCOM_USART_EVENT event, uintptr_t context)
{
    (void)context;

    if (event == SERCOM_USART_EVENT_READ_THRESHOLD_REACHED) {
        size_t bytesAvailable = SERCOM0_USART_ReadCountGet();
        if (bytesAvailable > 0) {
            if (bytesAvailable > ODID_UART_RX_BUFFER_SIZE) {
                bytesAvailable = ODID_UART_RX_BUFFER_SIZE;
            }
            size_t bytesRead = SERCOM0_USART_Read(s_rxBuffer, bytesAvailable);
            for (size_t i = 0; i < bytesRead; i++) {
                ODID_MAVLink_ProcessByte(s_rxBuffer[i]);
            }
        }
    }
}

void ODID_UART_Init(void)
{
    // SERCOM0 is already initialized by the system (plib_sercom0_usart.c)
    // Configure read notification: trigger when at least 1 byte available
    SERCOM0_USART_ReadThresholdSet(1);
    SERCOM0_USART_ReadNotificationEnable(true, true);
    SERCOM0_USART_ReadCallbackRegister(ODID_UART_ReadCallback, 0);
}

void ODID_UART_Tasks(void)
{
    // Polling fallback: read any bytes that arrived without triggering callback
    size_t bytesAvailable = SERCOM0_USART_ReadCountGet();
    if (bytesAvailable > 0) {
        if (bytesAvailable > ODID_UART_RX_BUFFER_SIZE) {
            bytesAvailable = ODID_UART_RX_BUFFER_SIZE;
        }
        size_t bytesRead = SERCOM0_USART_Read(s_rxBuffer, bytesAvailable);
        for (size_t i = 0; i < bytesRead; i++) {
            ODID_MAVLink_ProcessByte(s_rxBuffer[i]);
        }
    }
}
