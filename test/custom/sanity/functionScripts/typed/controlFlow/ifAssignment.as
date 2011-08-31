function fannkuch(n) {
	var perm:int = 10;
	var flipsCount:int = 0;
	var k:int;

	if (!((k = perm) == 0)) {
		flipsCount++;
	}

	print(flipsCount);
}

var res=fannkuch(8);
