
package {

    function printHello(value:int, stringValue:String, numValue:Number) {
        otherValue(value, stringValue, numValue);
    }

    function otherValue(value:int, stringValue:String, numValue:Number) {
        print(stringValue + value + numValue);
    }

    printHello(5, "hello", 3.14159);
    printHello(10, "sweet", 2.81237);
}


