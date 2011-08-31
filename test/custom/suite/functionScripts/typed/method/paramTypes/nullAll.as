package TestNullString {
    function nullAll(testArray:Array, testObject:Object, testString:String, testInt:int, testNumber:Number) {
        print(testArray);
        print(testObject);
        print(testString);
        print(testInt);
        print(testNumber);
    }

    function passParam() {
        nullAll(null, null, null, null, null);
    }

    passParam();
}

