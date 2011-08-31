package TestPackage {
	public class TestClass {
		private var message;
		private var value;

		public function TestClass() {
			message = "hello world";
			value = 0;
		}

		public function printMessage() {
			this.value++;
			print(message + value);
		}
	}

	var x = new TestClass();
	x.printMessage();
}
