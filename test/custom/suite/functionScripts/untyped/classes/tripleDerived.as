package TestInheritance {
	public class BaseClass {
		private var testVariable;

		public function BaseClass() {
			this.testVariable = "base class";
		}

		public function setMessage(newMessage) {
			this.testVariable = new Message();
		}

		public function printMessage() {
			print(this.testVariable);
		}
	}

	public class DerivedClass extends BaseClass {
		private var derivedVariable;
		public function DerivedClass() {
            super();
		}

		public override function printMessage() {
			print("Derived class");
		}
	}

	public class TripleDerivedClass extends DerivedClass {
		public function TripleDerivedClass() {
			super();
		}
	}

	var x = new TripleDerivedClass();
	x.printMessage();
}
