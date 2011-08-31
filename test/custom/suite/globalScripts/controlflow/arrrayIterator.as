var testArray = new Array();

for (var i = 0; i < 5; i++) {
    testArray[i] = i;
}

for (arrayElement in testArray) {
    print(arrayElement);
}

