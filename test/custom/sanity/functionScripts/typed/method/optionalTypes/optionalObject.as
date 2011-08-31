package TestOptional {
    function testOptional(x:Object = undefined) {
        print(x);
    }
    
    testOptional();
    testOptional(new Object());
}
