package TestPackage {
	public class TestClass {
		private var message;
		static const RANDOM_VALUE = 20;

		public function TestClass() {
			message = "hello world";
		}

		public function printMessage() {
			print(message);
			print(RANDOM_VALUE);
		}
	}

	var x = new TestClass();
	x.printMessage();
}
