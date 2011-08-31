
package TestAdd {

 function testWhile(x:int) {
     var somethingElse:int = 10;
     while (somethingElse < 100)
         somethingElse++;
 }

    function doSum(x:int, y:int):int {
        var sum:int;
        for (x = 0; x < y; x++) {
            sum += x + y;
        }

        testWhile(x);
        return sum;
    }

    function addNumbers() {
        doSum(10, 18);
    }

    addNumbers();
}
