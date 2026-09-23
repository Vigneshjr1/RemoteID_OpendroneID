#include "odid_uart.h"
#include "odid_mavlink.h"
#include "definitions.h"

#define ODID_UART_RX_BUFFER_SIZE  64

static uint8_t s_rxBuffer[ODID_UART_RX_BUFFER_SIZE];
static volatile bool s_rxPending;

static void ODID_UART_ReadCallback(SERCOM_USART_EVENT event, uintptr_t context)
{
    (void)context;

    if (event == SERCOM_USART_EVENT_READ_THRESHOLD_REACHED) {
        /* Parsing is intentionally deferred out of interrupt context. */
        s_rxPending = true;
    }
}

void ODID_UART_Init(void)
{
    s_rxPending = false;

    // SERCOM1 is already initialized by the system (plib_sercom0_usart.c)
    // Configure read notification: trigger when at least 1 byte available
    SERCOM0_USART_ReadThresholdSet(1);
    SERCOM0_USART_ReadNotificationEnable(true, true);
    SERCOM0_USART_ReadCallbackRegister(ODID_UART_ReadCallback, 0);
}

void ODID_UART_Tasks(void)
{
    size_t bytesAvailable;

    if (!s_rxPending && (0U == SERCOM0_USART_ReadCountGet())) {
        return;
    }

    s_rxPending = false;

    /* Drain the ring buffer in task context, including batches over 64 bytes. */
    while ((bytesAvailable = SERCOM0_USART_ReadCountGet()) > 0U) {
        if (bytesAvailable > ODID_UART_RX_BUFFER_SIZE) {
            bytesAvailable = ODID_UART_RX_BUFFER_SIZE;
        }
        size_t bytesRead = SERCOM0_USART_Read(s_rxBuffer, bytesAvailable);
        for (size_t i = 0; i < bytesRead; i++) {
            ODID_MAVLink_ProcessByte(s_rxBuffer[i]);
        }
    }
}
