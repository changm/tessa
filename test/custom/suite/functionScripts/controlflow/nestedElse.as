function nestedElse() {
    var x = 10;
    var y = 30;
    var z = 0;

    if (x > 20) {
        z = 10;
    } else {
        if (y > 25 ) {
            z = 50;
        } else {
            z = 20;
        }
    }

    print(z);
}

nestedElse();
