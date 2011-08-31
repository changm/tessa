function doWhileLoop() {
	var x:int = 0;
	var sum:int = 0;

	do {
		sum += x;
		x--;
	} while (x > 0);

	print(x);
	print("sum: " + sum + " x: " + x);
}

doWhileLoop();
