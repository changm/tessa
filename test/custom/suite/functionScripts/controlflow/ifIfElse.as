
function testSomething(a) {
    if (a != null) {
        if (a is Number) { print("Number"); }
    }
    else { print("else"); }
}

testSomething(10);
