
package {

    function printHello(value:Object) {
        otherValue(value);
    }

    function otherValue(value:Object) {
        print(value);
        print(value.x);
    }

    var testObject:Object = new Object();
    testObject.x = "awesome";
    
    var otherObject:Object = new Object();
    otherObject.x = "sweet";


    printHello(testObject);
    printHello(otherObject);
}


