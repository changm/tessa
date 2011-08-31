function testIterator() {
    var testArray = new Array();

    for (var i = 0; i < 5; i++) {
        testArray[i] = i;
    }

    for (var arrayElement in testArray) {
        print(arrayElement);
    }
}

testIterator();

