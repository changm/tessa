function innerForArray() {
    var testArray:Array = new Array(10);

    for (var i:int = 0; i < 10; i++) {
        testArray[i] = new Array(20);
        for (var j:int = 0; j < 20; j++) {
            testArray[i][j] = i * j;
        }
    }

    print(testArray[9][19]);
}

innerForArray();
