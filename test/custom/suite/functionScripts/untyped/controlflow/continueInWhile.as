function testWhileContinue() {
	var i = 10;
	var x = -5;
	while(--i >= 0) {
		if(x < 2) {
		  x++;
		  continue;
		}
	}

	print(x);
}

testWhileContinue();
