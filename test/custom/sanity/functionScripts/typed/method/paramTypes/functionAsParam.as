package TestParameters {
    function callFunction(x:Function) {
        x();
    }

    function passFunction() {
        var testFunction:Function = function() { print("hello"); }
        callFunction(testFunction); 
    }

    passFunction();
}

