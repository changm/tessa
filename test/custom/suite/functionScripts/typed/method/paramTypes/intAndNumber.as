
package {

    function printHello(value:int, numValue:Number) {
        otherValue(value, numValue);
    }

    function otherValue(value:int, numValue:Number) {
        print(value + "," + numValue);
    }

    printHello(5, 3.14159);
    printHello(10, 2.81237);
}


