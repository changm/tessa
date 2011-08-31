function someBody(message:String) {
	this.x = message;
}

someBody.prototype.printMessage = function() {
	print(this.x);
}

function shadowProperty() {
    var testBody:Object = new someBody("weee prototype");
    testBody.printMessage();
}

shadowProperty();
