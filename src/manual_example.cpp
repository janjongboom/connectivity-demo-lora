#include "dot_util.h"
#include "parse_keys.h"
#include "RadioEvent.h"

#if ACTIVE_EXAMPLE == MANUAL_EXAMPLE

#define BUILTIN_LED_ON          1
#define BUILTIN_LED_OFF         0

/////////////////////////////////////////////////////////////
// * these options must match the settings on your gateway //
// * edit their values to match your configuration         //
// * frequency sub band is only relevant for the 915 bands //
/////////////////////////////////////////////////////////////
static const char DEV_ADDR[]    = "A80BAD2C";
static const char NWK_S_KEY[]   = "BD86A216B4DE6752938FFC6D260C5649";
static const char APP_S_KEY[]   = "9A74BEA119106EEE661E20F48CD1C216";

// deepsleep consumes slightly less current than sleep
// in sleep mode, IO state is maintained, RAM is retained, and application will resume after waking up
// in deepsleep mode, IOs float, RAM is lost, and application will start from beginning after waking up
// if deep_sleep == true, device will enter deepsleep mode
static bool deep_sleep = false;

static InterruptIn btn(WAKE);

mDot* dot = NULL;

Serial pc(USBTX, USBRX);

typedef struct {
    uint16_t counter;
} ApplicationState;

static ApplicationState state;

static DigitalOut led(LED1, BUILTIN_LED_OFF); /* D2 */

static void read_counter() {
    bool res = dot->nvmRead(0x100, &state, sizeof(state));
    if (!res) {
        state.counter = 0;
    }

    logInfo("restored counter state, value is %d", state.counter);
}

static void up_counter() {
    state.counter++;

    dot->nvmWrite(0x100, &state, sizeof(state));

    logInfo("new counter value is: %d", state.counter);
}

int main() {
    // Custom event handler for automatically displaying RX data
    RadioEvent events;

    pc.baud(115200);

    mts::MTSLog::setLogLevel(mts::MTSLog::TRACE_LEVEL);

    dot = mDot::getInstance();

    int32_t ret = 0;

    // attach the custom events handler
    dot->setEvents(&events);

    if (!dot->getStandbyFlag()) {
        logInfo("mbed-os library version: %d", MBED_LIBRARY_VERSION);

        // start from a well-known state
        logInfo("defaulting Dot configuration");
        dot->resetConfig();
        dot->resetNetworkSession();

        // make sure library logging is turned on
        dot->setLogLevel(mts::MTSLog::INFO_LEVEL);

        logInfo("setting tx power to 20");
        if ((ret = dot->setTxPower(20)) != mDot::MDOT_OK) {
            logError("failed to set tx power %d:%s", ret, mDot::getReturnCodeString(ret).c_str());
        }

        logInfo("setting TX spreading factor");
        if ((ret = dot->setTxDataRate(mDot::SF_10)) != mDot::MDOT_OK) {
            logError("failed to set TX datarate %d:%s", ret, mDot::getReturnCodeString(ret).c_str());
        }

        ParseKeys::initializePersonalized(dot, DEV_ADDR, NWK_S_KEY, APP_S_KEY, true);

        // save changes to configuration
        logInfo("saving configuration");
        if (!dot->saveConfig()) {
            logError("failed to save configuration");
        }

        // display configuration
        display_config();
    } else {
        // restore the saved session if the dot woke from deepsleep mode
        // useful to use with deepsleep because session info is otherwise lost when the dot enters deepsleep
        logInfo("restoring network session from NVM");
        dot->restoreNetworkSession();
    }

    read_counter();

    while (true) {
        std::vector<uint8_t> tx_data;

        up_counter();

        tx_data.push_back(state.counter >> 8 & 0xff);
        tx_data.push_back(state.counter & 0xff);
        logInfo("sending: %d", state.counter);

        ret = dot->send(tx_data);
        if (ret == mDot::MDOT_OK) {
            std::vector<uint8_t> recv_data;
            if ((ret = dot->recv(recv_data)) != mDot::MDOT_OK) {
                logError("failed to recv %d:%s", ret, mDot::getReturnCodeString(ret).c_str());
            }

            if (recv_data.size() > 0) {
                printf("[INFO] received %d bytes:", recv_data.size());
                for (size_t ix = 0; ix < recv_data.size(); ix++) {
                    printf(" %02x", recv_data[ix]);
                }
                printf("\r\n");

                if (recv_data[0] == 1) {
                    led = BUILTIN_LED_ON;
                }
                else {
                    led = BUILTIN_LED_OFF;
                }
            }
        }
        // @todo: retry

        // ONLY ONE of the three functions below should be uncommented depending on the desired wakeup method
        //sleep_wake_rtc_only(deep_sleep);
        sleep_wake_interrupt_only(deep_sleep);
        // sleep_wake_rtc_or_interrupt(deep_sleep);
    }

    return 0;
}

#endif
