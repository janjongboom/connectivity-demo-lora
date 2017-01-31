#ifndef DISPENSER_PARSE_KEYS_H
#define DISPENSER_PARSE_KEYS_H

#include "dot_util.h"

class ParseKeys {
public:
    static bool initializePersonalized(mDot* dot, const char* aDevAddr, const char* aNwkSKey, const char* aAppSKey, bool ack) {
        int32_t ret;

        // parse devkey
        uint8_t loriot_nwk_skey[16];
        for (uint8_t ni = 0; ni < 16; ni++)
        {
            const char hex[] = { '0', 'x', aNwkSKey[ni * 2], aNwkSKey[ni * 2 + 1], '\0' };
            loriot_nwk_skey[ni] = strtoul(hex, NULL, 16);
        }

        // parse appkey
        uint8_t loriot_app_skey[16];
        for (uint8_t ai = 0; ai < 16; ai++)
        {
            const char hex[] = { '0', 'x', aAppSKey[ai * 2], aAppSKey[ai * 2 + 1], '\0' };
            loriot_app_skey[ai] = strtoul(hex, NULL, 16);
        }

        // parse dev addr
        uint8_t loriot_dev_addr[4];
        for (uint8_t ai = 0; ai < 4; ai++)
        {
            const char hex[] = { '0', 'x', aDevAddr[ai * 2], aDevAddr[ai * 2 + 1], '\0' };
            loriot_dev_addr[ai] = strtoul(hex, NULL, 16);
        }

        // set join mode to MANUAL so the mDot doesn't have to rejoin after sleeping
        logInfo("setting join mode to MANUAL");
        if ((ret = dot->setJoinMode(mDot::MANUAL)) != mDot::MDOT_OK) {
            logError("failed to set join mode %d:%s", ret, mDot::getReturnCodeString(ret).c_str());
        }

        update_manual_config(loriot_dev_addr, loriot_nwk_skey, loriot_app_skey, 0, true, ack);

        return true;
    }

    static bool initializeOta(mDot* dot, const char* aAppEui, const char* aAppKey, bool ack) {
        int32_t ret;

        // parse app key
        uint8_t loriot_app_key[16];
        for (uint8_t ni = 0; ni < 16; ni++)
        {
            const char hex[] = { '0', 'x', aAppKey[ni * 2], aAppKey[ni * 2 + 1], '\0' };
            loriot_app_key[ni] = strtoul(hex, NULL, 16);
        }

        // parse app eui
        uint8_t loriot_app_eui[8];
        for (uint8_t ai = 0; ai < 8; ai++)
        {
            const char hex[] = { '0', 'x', aAppEui[ai * 2], aAppEui[ai * 2 + 1], '\0' };
            loriot_app_eui[ai] = strtoul(hex, NULL, 16);
        }

        // set join mode to AUTO_OTA so the mDot doesn't have to rejoin after sleeping
        logInfo("setting join mode to AUTO_OTA");
        if ((ret = dot->setJoinMode(mDot::AUTO_OTA)) != mDot::MDOT_OK) {
            logError("failed to set join mode %d:%s", ret, mDot::getReturnCodeString(ret).c_str());
        }

        update_ota_config_id_key(loriot_app_eui, loriot_app_key, 0, true, ack);

        return true;
    }
};

#endif // DISPENSER_PARSE_KEYS_H
