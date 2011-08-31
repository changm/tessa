function testContinue() {
    var x:int = 0;

    for (var i:int = 0; i < 10; i++) {
        if (i > 5) {
            continue;
        }
        
        x += i;        
    }

    print(x);
}

testContinue();
