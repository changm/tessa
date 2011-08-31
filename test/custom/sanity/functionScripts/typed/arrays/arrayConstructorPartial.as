function ArrayConstructor(...args)
    {
        var n:uint = args.length
        if (n == 1 && (args[0] is Number)) {
            print("false");
        } else {
            print("Correct");
        }
    }


function stringArray() {
    var x:Object = new ArrayConstructor();
}

stringArray();
