function testBranchNull(options:String = null) {
    if (options) {
        print("has option");
    } else {
        print("no option");
    }
}

testBranchNull();
testBranchNull("something");
