function somethingRandom(message:String) {
	print(message);
}

function callOther(func:Function) {
	print("calling other");
	func("hello world");
}

callOther(somethingRandom);
