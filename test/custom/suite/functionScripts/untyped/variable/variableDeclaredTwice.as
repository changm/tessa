function testDoubleVar() {
    var sum = 10;

    for (var i = 10; i < 100; i++) {
        sum += i;
    }

    for (var i = 100; i < 1000; i++) {
        sum += i;
    }

    print(sum);
}

testDoubleVar();
