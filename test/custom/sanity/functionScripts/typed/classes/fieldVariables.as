package TestPackage {
	public class TestClass {
		private var message:String;
		private var value:int;

		public function TestClass() {
			message = "hello world";
			value = 0;
		}

		public function printMessage() {
			this.value++;
			print(message + value);
		}
	}

	var x:TestClass = new TestClass();
	x.printMessage();
}
