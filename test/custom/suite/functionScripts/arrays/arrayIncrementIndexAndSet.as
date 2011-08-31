function testSomething() {
		var testArray = new Array(10);
		var index = 0;
		var x = 50;

		testArray[index] = 0;
		testArray[index++] ^= x;

		print(testArray[index - 1]);
}

testSomething();
