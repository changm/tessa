package TestPackage {
    function testMethodReturn():void {
        print(methodReturn());
    }

    function methodReturn():int {
        return 5;
    }

    testMethodReturn();
}
