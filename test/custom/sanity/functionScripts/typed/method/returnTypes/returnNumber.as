package TestPackage {
    function testMethodReturn():void {
        print(methodReturn());
    }

    function methodReturn():Number{
        return 3.14153;
    }

    testMethodReturn();
}
