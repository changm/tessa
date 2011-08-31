function somethingRandom(message) {
	print(message);
}

function callOther(func) {
	print("calling other");
	func("hello world");
}

callOther(somethingRandom);
