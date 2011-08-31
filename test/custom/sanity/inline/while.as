
package TestAdd {
    function doSum(x:int, y:int):int {
        var sum:int;
        for (x = 0; x < y; x++) {
            sum += x + y;
        }

        return sum;
    }

    function addNumbers() {
        print(doSum(10, 18));
    }

    addNumbers();
}
