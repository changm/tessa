function someBody(message) {
	this.x = message;
}

someBody.prototype.printMessage = function() {
	print(this.x);
}

function shadowProperty() {
    var testBody = new someBody("weee prototype");
    testBody.printMessage();
}

shadowProperty();
