package {
    function testString() {
        var testString:String = "hello " + "world";
        var otherString:String = "hello world";
        var isEqual:Boolean = testString == otherString;
        print("Is equal: " + isEqual);
    }
    
    testString();

}
