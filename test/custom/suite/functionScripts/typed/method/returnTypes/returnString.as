package TestPackage {
    function testMethodReturn():void {
        print(methodReturn());
    }

    function methodReturn():String{
        return "returned a string";
    }

    testMethodReturn();
}
