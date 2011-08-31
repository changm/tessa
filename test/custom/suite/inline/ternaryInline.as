package {

function testInlineTernary() {
    testTernary();
}

function testTernary() {
    var x:int = 10;
    var y:Boolean = (x == 10) ? true : false;
    print(y);

}

testInlineTernary()

}

