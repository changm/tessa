function testUndefinedAgainstNumber() {
	var x:Number = undefined;
	if (x > 0.01) {
		print("WTF?");
	} else {
		print("Correct");
	}
}

testUndefinedAgainstNumber();
