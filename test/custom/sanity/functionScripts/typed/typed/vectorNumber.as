function testVector() {
    var result:Vector.<Number> = new Vector.<Number>(10);
    result[3] = 3.14159;
    print(result[3]);
    print(result);
}

testVector();
