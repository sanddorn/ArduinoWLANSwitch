#include <Arduino.h>
#include <unity.h>
#include <string>
#include <HTMLHandler.h>
#include <ArduinoLog.h>

using namespace fakeit;
using namespace std;


namespace TestReplaceString {

    class TestClass : public HTMLHandler {
    public:
        TestClass(Persistence * persistence, Logging *log) : HTMLHandler(persistence, log) {};

        void replaceString(string &original, const string &toReplace, const string replacement) {
            HTMLHandler::replaceString(original, toReplace, replacement);
        }
    };


    TestClass *subjectUnderTest;
    Logging logging;
    Mock <Persistence> persistenceMock;

    void setUp() {
        Persistence &persistence = persistenceMock.get();
        When(Method(persistenceMock, begin)).Return(true);
        subjectUnderTest = new TestClass(&persistence, &logging);
    }


    void test_replaceString_expectOk(void) {
        string original = "Ein schlechter Test";
        subjectUnderTest->replaceString(original, "schlechter", "guter");
        TEST_ASSERT_TRUE(original.compare("Ein guter Test") == 0);
    }

    void test_replaceString_expectNothingChanged(void) {
        string original = "Ein schlechter Test";
        subjectUnderTest->replaceString(original, "foo", "bar");
        TEST_ASSERT_TRUE(original.compare("Ein schlechter Test") == 0);
    }

    void test_replaceString_expectMultipleChanges(void) {
        string original = "Ein schlechter Test ist ein schlechter Test";
        subjectUnderTest->replaceString(original, "schlechter", "guter");
        TEST_ASSERT_TRUE(original.compare("Ein guter Test ist ein guter Test") == 0);
    }


    void run_tests() {

        RUN_TEST(test_replaceString_expectOk);
        RUN_TEST(test_replaceString_expectNothingChanged);
        RUN_TEST(test_replaceString_expectMultipleChanges);

    }
}