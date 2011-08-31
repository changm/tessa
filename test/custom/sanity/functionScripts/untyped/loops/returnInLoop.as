function testBreakLoop() {
    var i = 10;

    while (true) {
        i++;
        if (i > 20) {
            return i;
        }
    }

    return 0;
}

print(testBreakLoop());
