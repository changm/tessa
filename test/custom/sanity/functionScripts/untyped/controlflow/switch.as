function switchTest() {
	var x = 10;

	switch(x) {
		case 10:
			x = 20;
			break;
		case 20:
			x = 30;
			break;
		case 30:
			x = 50;
			break;
		default:
			x = 100;
			break;
	}

	print(x);
}

switchTest();
