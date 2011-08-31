function innerForLoop() {
    var x:int = 0;

    for (var i:int = 1; i < 10; i++) {
        x *= i;
        for (var j:int = 1; j < 20; j++) {
            x += j;
            var y:int = 10;
        }
    }

    print(x);
    print(y);
}

innerForLoop();
