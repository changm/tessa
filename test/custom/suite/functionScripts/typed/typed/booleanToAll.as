function booleanToAll() {
    var boolTrue:Boolean = true;
    var boolFalse:Boolean = false;

    var boolInt:int = boolTrue;
    var boolUint:uint= boolTrue;
    var boolString:String = boolTrue;
    var boolDouble:Number = boolTrue;
    var boolObject:Object = boolTrue;

    print(boolInt);
    print(boolUint);
    print(boolString);
    print(boolDouble);
    print(boolObject);

    boolInt = boolFalse;
    boolUint = boolFalse;
    boolString = boolFalse;
    boolDouble = boolFalse;
    boolObject = boolFalse;

    print(boolInt);
    print(boolString);
    print(boolDouble);
    print(boolObject);
    print(boolUint);

}

booleanToAll();
