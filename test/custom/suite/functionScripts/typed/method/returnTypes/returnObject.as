package TestPackage {
    function testMethodReturn():void {
        print(methodReturn());
    }

    function methodReturn():Object{
        return new Object();
    }

    testMethodReturn();
}
