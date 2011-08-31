class TestClass {
	public function TestClass() {

	}

	public function sayHello() {
		print("hello world");
	}
}

function newInstance() {
    var instanceHere = new TestClass();
    instanceHere.sayHello();
}

newInstance();
