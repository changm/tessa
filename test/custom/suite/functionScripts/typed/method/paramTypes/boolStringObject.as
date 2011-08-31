
package {

    function printHello(value:Boolean, message:String, testObject:Object) {
        otherValue(value, message, testObject);
    }

    function otherValue(value:Boolean, message:String, testObject:Object) {
        print(value);
        print(message);
        print(testObject);
    }

    printHello(true, "hello", new Object());
    printHello(false, "sweet", new Object());
}


