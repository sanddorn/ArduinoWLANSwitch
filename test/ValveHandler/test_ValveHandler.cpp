//
// Created by Nils Bokermann on 20.01.20.
//

#include <Arduino.h>
#include <cstdlib>
#include <unity.h>
#include "test_ValveHandler.h"
#include <ValveHandler.h>
#include <ArduinoLog.h>


namespace TestValveHandler {
    using namespace fakeit;

    ValveHandler *subjectUnderTest;
    Mock <AbstractCallBackHandler> abstractCallBackHandlerMock;
    Logging logging;
    static const int valveOpenPin = 1;

    static const int valveClosePin = 2;

    void setUp() {
        AbstractCallBackHandler &callBackHandler = abstractCallBackHandlerMock.get();
        subjectUnderTest = new ValveHandler(valveOpenPin, valveClosePin,
                                            std::shared_ptr<AbstractCallBackHandler>(&callBackHandler),
                                            logging);
    }

    void test_getStatus() {
        abstractCallBackHandlerMock.ClearInvocationHistory();
        ArduinoFake().ClearInvocationHistory();
        VALVESTATE state = subjectUnderTest->getStatus();
        TEST_ASSERT_EQUAL(VALVESTATE::UNKNOWN, state);
    }

    void test_getStatus_afterOpenCommand() {
        abstractCallBackHandlerMock.ClearInvocationHistory();
        ArduinoFake().ClearInvocationHistory();
        VALVESTATE state = VALVESTATE::UNKNOWN;
        When(Method(ArduinoFake(), digitalWrite).Using(valveClosePin, LOW)).AlwaysReturn();
        When(Method(ArduinoFake(), digitalWrite).Using(valveOpenPin, HIGH)).AlwaysReturn();
        When(Method(abstractCallBackHandlerMock, startCallbackTimer)).Return();
        try {
            subjectUnderTest->openValve();
            state = subjectUnderTest->getStatus();
        } catch (UnexpectedMethodCallException &e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        Verify(Method(ArduinoFake(), digitalWrite).Using(valveOpenPin, HIGH)).Once();
        Verify(Method(abstractCallBackHandlerMock, startCallbackTimer)).Once();
        TEST_ASSERT_EQUAL(VALVESTATE::OPENING, state);
    }

    void test_getStatus_afterCloseCommand() {
        abstractCallBackHandlerMock.ClearInvocationHistory();
        ArduinoFake().ClearInvocationHistory();
        VALVESTATE state = VALVESTATE::UNKNOWN;
        When(Method(ArduinoFake(), digitalWrite).Using(valveOpenPin, LOW)).AlwaysReturn();
        When(Method(ArduinoFake(), digitalWrite).Using(valveClosePin, HIGH)).AlwaysReturn();
        When(Method(abstractCallBackHandlerMock, startCallbackTimer)).Return();
        try {
            subjectUnderTest->closeValve();
            state = subjectUnderTest->getStatus();
        } catch (UnexpectedMethodCallException &e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        Verify(Method(ArduinoFake(), digitalWrite).Using(valveClosePin, HIGH)).Once();
        Verify(Method(abstractCallBackHandlerMock, startCallbackTimer)).Once();
        TEST_ASSERT_EQUAL(VALVESTATE::CLOSING, state);
    }


    void test_OpenWhileClosing() {
        abstractCallBackHandlerMock.ClearInvocationHistory();
        ArduinoFake().ClearInvocationHistory();
        When(Method(ArduinoFake(), digitalWrite).Using(valveClosePin, HIGH)).AlwaysReturn();
        When(Method(ArduinoFake(), digitalWrite).Using(valveOpenPin, HIGH)).AlwaysReturn();
        When(Method(ArduinoFake(), digitalWrite).Using(valveOpenPin, LOW)).AlwaysReturn();
        When(Method(ArduinoFake(), digitalWrite).Using(valveClosePin, LOW)).AlwaysReturn();
        When(Method(abstractCallBackHandlerMock, startCallbackTimer)).Return();
        try {
            subjectUnderTest->closeValve();
            subjectUnderTest->openValve();
        } catch (UnexpectedMethodCallException &e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        Verify(Method(ArduinoFake(), digitalWrite).Using(valveClosePin, LOW),
               Method(ArduinoFake(), digitalWrite).Using(valveOpenPin, HIGH)).Once();
    }

    void test_CloseWhileOpening() {
        abstractCallBackHandlerMock.ClearInvocationHistory();
        ArduinoFake().ClearInvocationHistory();
        When(Method(ArduinoFake(), digitalWrite).Using(valveClosePin, HIGH)).AlwaysReturn();
        When(Method(ArduinoFake(), digitalWrite).Using(valveOpenPin, HIGH)).AlwaysReturn();
        When(Method(ArduinoFake(), digitalWrite).Using(valveOpenPin, LOW)).AlwaysReturn();
        When(Method(ArduinoFake(), digitalWrite).Using(valveClosePin, LOW)).AlwaysReturn();
        When(Method(abstractCallBackHandlerMock, startCallbackTimer)).Return();
        try {
            subjectUnderTest->openValve();
            subjectUnderTest->closeValve();
        } catch (UnexpectedMethodCallException &e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        Verify(Method(ArduinoFake(), digitalWrite).Using(valveOpenPin, LOW) +
               Method(ArduinoFake(), digitalWrite).Using(valveClosePin, HIGH)).Once();
    }

    void test_callBackForOpen() {
        abstractCallBackHandlerMock.ClearInvocationHistory();
        ArduinoFake().ClearInvocationHistory();
        VALVESTATE state = VALVESTATE::UNKNOWN;
        When(Method(ArduinoFake(), digitalWrite).Using(valveClosePin, LOW)).Return();
        When(Method(ArduinoFake(), digitalWrite).Using(valveOpenPin, HIGH)).Return();
        When(Method(abstractCallBackHandlerMock, startCallbackTimer)).Return();
        When(Method(ArduinoFake(), digitalWrite).Using(valveOpenPin, LOW)).Return();
        try {
            subjectUnderTest->openValve();
            ValveHandler::callbackMethod(subjectUnderTest);
            state = subjectUnderTest->getStatus();
        } catch (UnexpectedMethodCallException &e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        Verify(Method(ArduinoFake(), digitalWrite).Using(valveOpenPin, LOW)).Once();
        TEST_ASSERT_EQUAL(VALVESTATE::OPEN, state);
    }

    void test_callBackForClose() {
        abstractCallBackHandlerMock.ClearInvocationHistory();
        ArduinoFake().ClearInvocationHistory();
        VALVESTATE state = VALVESTATE::UNKNOWN;
        When(Method(ArduinoFake(), digitalWrite).Using(valveOpenPin, LOW)).Return();
        When(Method(ArduinoFake(), digitalWrite).Using(valveClosePin, HIGH)).Return();
        When(Method(abstractCallBackHandlerMock, startCallbackTimer)).Return();
        When(Method(ArduinoFake(), digitalWrite).Using(valveClosePin, LOW)).Return();
        try {
            subjectUnderTest->closeValve();
            ValveHandler::callbackMethod(subjectUnderTest);
            state = subjectUnderTest->getStatus();
        } catch (UnexpectedMethodCallException &e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        Verify(Method(ArduinoFake(), digitalWrite).Using(valveClosePin, LOW)).Once();
        TEST_ASSERT_EQUAL(VALVESTATE::CLOSED, state);
    }

    void run_tests() {
        RUN_TEST(test_getStatus);
        RUN_TEST(test_getStatus_afterOpenCommand);
        RUN_TEST(test_getStatus_afterCloseCommand);
        RUN_TEST(test_OpenWhileClosing);
        RUN_TEST(test_CloseWhileOpening);
        RUN_TEST(test_callBackForOpen);
        RUN_TEST(test_callBackForClose);
    }
};