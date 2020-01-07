//
// Created by Nils Bokermann on 04.01.20.
//

#include <Arduino.h>
#include <cstdlib>
#include <unity.h>

#include "../HTMLHandler/test_HTMLHandler.h"
#include "../HTMLHandler/test_replaceString.h"
#include "test_WifiConfig.h"

#ifdef UNIT_TEST

#define RUN_TEST_GROUP(TEST) \
    if (!std::getenv("TEST_GROUP") || (strcmp(#TEST, std::getenv("TEST_GROUP")) == 0)) { \
        TEST::setUp(); \
        TEST::run_tests(); \
    }

using namespace fakeit;
void setUp(void)
{
    ArduinoFakeReset();
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST_GROUP(TestWifiConfig);

    UNITY_END();

    return 0;
}

#endif
