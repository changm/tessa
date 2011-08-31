
package {

    function printHello(value:int, numValue:Number, stringValue:String) {
        otherValue(value, numValue, stringValue);
    }

    function otherValue(value:int, numValue:Number, stringValue:String) {
        print(stringValue + value + numValue);
    }

    printHello(5, 3.14159, "hello");
    printHello(10, 2.81237, "awesome");
}


