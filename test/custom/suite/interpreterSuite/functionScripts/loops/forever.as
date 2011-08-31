function forever() {
	var x = 0;

	for (;;) {
		x++;
		if (x > 10) break;
	}

	print(x);
}

forever();
