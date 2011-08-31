function definedLater() {
    var x = 10;
    print(y);

    if (x < 20) {
        var y = 30;
    }

    print(y);
}

definedLater();
