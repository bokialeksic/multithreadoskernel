#ifndef CONSOLE_HPP
#define CONSOLE_HPP
#include "../lib/hw.h"
#include "SCB.hpp"
#include "../h/syscall_c.hpp"

class ConsoleDriver {
public:
    static void init();
    static void writeToTxBuffer(char c);
    static char readFromRxBuffer();
    static void handleInterrupt();
    static int pendingCount();

    static SCB* txEmptySlots;
    static SCB* rxEmptySlots;
    static SCB* rxFullSlots;
private:
    static const int TX_BUFFER_SIZE = 200;
    static const int RX_BUFFER_SIZE = 200;
    static char txBuffer[TX_BUFFER_SIZE];
    static int txHead;
    static int txTail;
    static char rxBuffer[RX_BUFFER_SIZE];
    static int rxHead;
    static int rxTail;

    // true ako je kontroleru vec predat karakter za slanje i ceka se
    // tx prekid a false ako je kontroler idle i
    // sledeci upis u bafer mora sam da pokrene prvi transfer.
    static bool txBusy;

    // Uzima sledeci karakter iz softverskog bafera (ako ima) i salje ga
    // na hardver preko CONSOLE_TX_DATA. Poziva se i iz writeToTxBuffer
    // (da "upali" prvi transfer kad je TX idle) i iz handleInterrupt
    // (da posalje sledeci karakter kad kontroler javi da je spreman).
    // Vraca true ako je nesto stvarno poslato, false ako bafer bio prazan.
    static bool trySendNext();
};
#endif // CONSOLE_HPP

