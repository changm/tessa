package TestPackage {
    function testMethodReturn():void {
        print(methodReturn());
    }

    function methodReturn():Boolean{
        return false;
    }

    testMethodReturn();
}
