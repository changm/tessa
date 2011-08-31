package TestPackage {
	public class TestClass {
		private var message:String;
		static const RANDOM_VALUE:int = 20;

		public function TestClass() {
			message = "hello world";
		}

		public function printMessage() {
			print(message);
			print(RANDOM_VALUE);
		}
	}

	var x:TestClass = new TestClass();
	x.printMessage();
}
