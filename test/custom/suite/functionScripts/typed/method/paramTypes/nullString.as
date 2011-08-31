package TestNullString {
    function nullStringParameter(x:String) {
        print(x);
    }

    function passParam() {
        nullStringParameter(null);
    }

    passParam();
}

