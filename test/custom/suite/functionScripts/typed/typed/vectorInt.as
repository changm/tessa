package TestVector {

    function testVectorInt() {
        var testVector:Vector.<int> = new Vector.<int>(10);
        testVector[0] = 10;
        testVector[5] = 30;
        print(testVector);
    }
    
    testVectorInt();
}
