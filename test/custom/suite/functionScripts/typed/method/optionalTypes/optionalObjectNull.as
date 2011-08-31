package TestOptional {
    function testOptional(x:Object = null) {
        print(x);
    }
    
    testOptional();
    testOptional(new Object());
}
