
function testSomething() {
    var j:int = 18;
    var testVariable:int = 26;
    var loopCondition:int = testVariable & (1 << 0);
    var jGreaterThan:Boolean = (j >= 0);
    var evaluatedCondition:Boolean = jGreaterThan && loopCondition;
    print("JGreaterThan: " + jGreaterThan + " Evaluated condition: " + evaluatedCondition);
}

testSomething();
