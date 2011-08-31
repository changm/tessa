function whileBreak() {
    var x:int = 10;

    while (true) {
        x++;

        if (x > 50) {
            x = 100;
            break;
        }
    }

    print(x);
}

whileBreak();
