function testConditions(a:Number) {
    if (a) {
        print("Exists as number" + a);
    } else {
        print("Number not converted to boolean correctly" + a);
    }
}

testConditions(10);
testConditions(15);
testConditions(-5);
testConditions(0);
