
package {

    function printHello(value:Array) {
        otherValue(value);
    }

    function otherValue(value:Array) {
        print(value);
    }


    function createArray() {
        var x:Array = [ 0, 10, 13 ];
        printHello(x);
    }

    createArray();
}


