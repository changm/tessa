function testBranchOnUndefined(message=undefined) {
    if (message) {
        print(message);
    }
}

testBranchOnUndefined();
testBranchOnUndefined("should be");
