function nestedIf() {
    var x:Number = 10;
    var y:int = 30;
    var z:Number = 0;

    if (x < 20) {
        if (y > 30) {
            z = 50;
        } else {
            z = 20;
        }
    } else {
        z = 10;
    }

    print(z);
}

nestedIf();
