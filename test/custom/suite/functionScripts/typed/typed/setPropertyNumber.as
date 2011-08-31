
function testSomething() {
    var this_array:Vector.<int> = new Vector.<int>(10);
    var i:int = 0;
    var someConstant:int = 20;
    this_array[i++] = someConstant + 30;
    print(this_array); 
}

testSomething();
