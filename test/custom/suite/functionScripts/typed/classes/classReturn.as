package {
    class TestClass {
	public function TestClass() {

	}

	public function sayHello():void {
		print("hello world");
	}

    public function calculateSomething(value:Number) {
        return value * 10;
    }
}

function newInstance() {
    var instanceHere:TestClass = new TestClass();
    var result = instanceHere.calculateSomething(50);
    print(result);
}

newInstance();
}
