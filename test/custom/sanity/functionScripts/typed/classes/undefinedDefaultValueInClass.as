package testPackage {

    public class TestClass {

        function TestClass(test:String, otherTest:String=undefined) {
            print(test);
            print(otherTest);
        }
    }

    var x = new TestClass("wee");
}

