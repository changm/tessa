var x = 0;

for (var i = 1; i < 10; i++) {
	x *= i;
	for (var j = 1; j < 20; j++) {
		x += j;
		var y = 10;
	}
}

print(x);
print(y);
