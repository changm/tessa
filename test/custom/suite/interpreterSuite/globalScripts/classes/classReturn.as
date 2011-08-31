class TestClass {
	public function TestClass() {

	}

	public function sayHello():void {
		print("hello world");
	}

    public function calculateSomething(value) {
        return value * 10;
    }
}

var instanceHere = new TestClass();
var result = instanceHere.calculateSomething(50);
print(result);
