package {
    class TestClass {
	public function TestClass() {

	}

	public function sayHello():* {
		print("hello world");
		return new TestClass();
	}
}

function newInstance() {
    var instanceHere:TestClass = new TestClass();
    var otherThing:TestClass = instanceHere.sayHello();
}

newInstance();
}
