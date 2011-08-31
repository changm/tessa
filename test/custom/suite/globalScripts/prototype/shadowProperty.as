function someBody(message) {
	this.x = message;
}

someBody.prototype.printMessage = function() {
	print(this.x);
}

var testBody = new someBody("weee prototype");
testBody.printMessage();
