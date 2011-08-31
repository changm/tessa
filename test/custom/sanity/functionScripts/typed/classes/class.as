package {
    class TestClass {
	public function TestClass() {

	}

	public function sayHello() {
		print("hello world");
	}
}

function newInstance() {
    var instanceHere:TestClass = new TestClass();
    instanceHere.sayHello();
}

newInstance();
}
