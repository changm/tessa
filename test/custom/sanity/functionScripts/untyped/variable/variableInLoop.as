function definedLater() {
    var x = 10;
    var i = 0;

    for (i = 0; i < 10; i++) {
        for (var j = 0; j < 10; j++) {
            x++;
        }
    }

    print(x);
    print(j);
}

definedLater();
