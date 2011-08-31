function doWhileLoop() {
	var x = 0;
	var sum = 0;

	do {
		sum += x;
		x--;
	} while (x > 0);

	print(x);
	print("sum: " + sum + " x: " + x);
}

doWhileLoop();
