#include "../h/Console.hpp"
#include "../h/TCB.hpp"
#include "../h/MemoryAllocator.hpp"
#include "../h/syscall_c.hpp"
#include "../lib/console.h"

char ConsoleDriver::txBuffer[ConsoleDriver::TX_BUFFER_SIZE];
int ConsoleDriver::txHead = 0;
int ConsoleDriver::txTail = 0;
bool ConsoleDriver::txBusy = false;

char ConsoleDriver::rxBuffer[ConsoleDriver::RX_BUFFER_SIZE];
int ConsoleDriver::rxHead = 0;
int ConsoleDriver::rxTail = 0;

SCB* ConsoleDriver::txEmptySlots = nullptr;
SCB* ConsoleDriver::rxEmptySlots = nullptr;
SCB* ConsoleDriver::rxFullSlots = nullptr;

void ConsoleDriver::init() {
    txEmptySlots = SCB::createSemaphore(TX_BUFFER_SIZE);
    rxEmptySlots = SCB::createSemaphore(RX_BUFFER_SIZE);
    rxFullSlots = SCB::createSemaphore(0);
}

/* Uzima sledeci karakter iz softverskog bafera (ako ima) i salje ga na
 hardver. Poziva se uvek iznutra handle_trap() (ecall za putc,
 ili hardverski prekid za TX spremnost), gde su prekidi vec maskirani
 nema potrebe za dodatnom sinhronizacijom.*/
bool ConsoleDriver::trySendNext() {
    if (txHead == txTail) {
        // bafer prazan nema sta da se posalje, kontroler ostaje idle
        txBusy = false;
        return false;
    }
    if (!(*(volatile uint8*)CONSOLE_STATUS & CONSOLE_TX_STATUS_BIT)) {
        // kontroler trenutno nije spreman
        return false;
    }
    char c = txBuffer[txHead];
    txHead = (txHead + 1) % TX_BUFFER_SIZE;
    *(volatile uint8*)CONSOLE_TX_DATA = (uint8)c;
    txBusy = true;
    txEmptySlots->signal();
    return true;
}

void ConsoleDriver::writeToTxBuffer(char c) {
    txBuffer[txTail] = c;
    txTail = (txTail + 1) % TX_BUFFER_SIZE;

    // Ako kontroler trenutno nije u letu (nije mu nista predato i ceka
    // se prekid), sami pokrecemo prvi transfer  inace nikad
    // ne stize prekid koji bi izvukao ovaj karakter iz bafera.
    if (!txBusy) {
        trySendNext();
    }
}

char ConsoleDriver::readFromRxBuffer() {
    char c = rxBuffer[rxHead];
    rxHead = (rxHead + 1) % RX_BUFFER_SIZE;
    return c;
}


void ConsoleDriver::handleInterrupt() {
    uint8 status = *(volatile uint8*)CONSOLE_STATUS;

    while (status & CONSOLE_RX_STATUS_BIT) {
        char c = (char)*(volatile uint8*)CONSOLE_RX_DATA;
        rxBuffer[rxTail] = c;
        rxTail = (rxTail + 1) % RX_BUFFER_SIZE;
        rxFullSlots->signal();
        status = *(volatile uint8*)CONSOLE_STATUS;
    }

    if (status & CONSOLE_TX_STATUS_BIT) {

        trySendNext();
    }
}

int ConsoleDriver::pendingCount() {
    int diff = txTail - txHead;
    if (diff < 0) diff += TX_BUFFER_SIZE;
    return diff;
}
