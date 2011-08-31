function innerIf() {
    var x:int = 10;

    for (var i:int = 0; i < 5; i++) {
        if (i % 2) {
            x += 20;
        }

        x *= 2;
    }

    print(x);
}

innerIf();
