function testThing() {
    var i:int = 10;
    var testArray:Vector.<int> = new Vector.<int>(10);
    var qd:int = testArray[--i];
    print(qd);
}

testThing();
