function testSomething() {
		var testArray:Array = new Array(10);
		var index:int = 0;
		var x:int = 50;

		testArray[index] = 0;
		testArray[index++] ^= x;

		print(testArray[index - 1]);
}

testSomething();
