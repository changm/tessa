function testBranchOnUndefined(message:String=undefined) {
    if (message) {
        print(message);
    } else {
        print(message);
    }
}

testBranchOnUndefined();
testBranchOnUndefined("should be");
