
function testSomething() {
    var j = 18;
    var testVariable = 26;
    var loopCondition = testVariable & (1 << 0);
    var jGreaterThan = (j >= 0);
    var evaluatedCondition = jGreaterThan && loopCondition;
    print("JGreaterThan: " + jGreaterThan + " Evaluated condition: " + evaluatedCondition);
}

testSomething();
