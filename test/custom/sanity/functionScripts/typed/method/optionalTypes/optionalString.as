package TestOptional {
    function testOptional(x:String = "hello world") {
        print(x);
    }
    
    testOptional();
    testOptional("exists");
}
