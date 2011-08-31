package TestPackage {
    function testMethodReturn():void {
        print(methodReturn());
    }

    function methodReturn():Array{
        return new Array(5);
    }

    testMethodReturn();
}
