function testDoubleVar() {
    var sum:int = 10;

    for (var i:int = 10; i < 100; i++) {
        sum += i;
    }

    for (var i:int = 100; i < 1000; i++) {
        sum += i;
    }

    print(sum);
}

testDoubleVar();
