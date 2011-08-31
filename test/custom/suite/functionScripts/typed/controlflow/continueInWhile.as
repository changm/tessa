function testWhileContinue() {
	var i:int = 10;
	var x:int = -5;
	while(--i >= 0) {
		if(x < 2) {
		  x++;
		  continue;
		}
	}

	print(x);
}

testWhileContinue();
