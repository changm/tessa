package TestOptional {
    function testOptional(x:Number = 3.14159, y:int = 10, z:Number = 2.1723) {
        print(x);
        print(y);
        print(z);
    }
    
    testOptional();
    testOptional(2.718);
    testOptional(3132.232, 20);
    testOptional(123.12317, 50, 23877);

}
