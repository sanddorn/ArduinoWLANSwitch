#include <string>
#include <ArduinoFake.h>
#include <ArduinoFake.h>
#include <HTMLHandler.h>

using namespace std;
using namespace fakeit;

class TestClass :public HTMLHandler {
public:
    void replaceString(string & original, const string & toReplace, const string replacement) {
        HTMLHandler::replaceString(original, toReplace, replacement);
    }
};

TestClass testClass;

test(replaceString_expectOk) {
    string original = "Ein schlechter Test";
    testClass.replaceString(original, "schlechter", "guter");
    assertTrue(original.compare("Ein guter Test") == 0);
}

test(replaceString_expectNothingChanged) {
    string original = "Ein schlechter Test";
    testClass.replaceString(original, "foo", "bar");
    assertTrue(original.compare("Ein schlechter Test") == 0);
}

void setup()
{
    Serial.begin(115200L);
    while(!Serial) {} // Portability for Leonardo/Micro

    Test::min_verbosity |= TEST_VERBOSITY_ASSERTIONS_ALL;
}

void loop()
{
    Test::run();
}
