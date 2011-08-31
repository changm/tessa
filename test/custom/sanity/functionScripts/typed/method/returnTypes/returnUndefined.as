package TestPackage {
    function testMethodReturn():void {
        print(methodReturn());
    }

    function methodReturn():Object{
        return undefined;
    }

    testMethodReturn();
}
