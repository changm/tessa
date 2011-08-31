function testContinue() {
    var x = 0;

    for (var i = 0; i < 10; i++) {
        if (i > 5) {
            continue;
        }
        
        x += i;        
    }

    print(x);
}

testContinue();
