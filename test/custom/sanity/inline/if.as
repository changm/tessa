
package TestAdd {
    function doSum(x:int, y:int):int {
        var result:int = 0;
        if (x > y) {
            result = x + y;
        } else {
            result = x * y;
        }

        return result;
    }

    function addNumbers() {
        print(doSum(10, 18));
    }

    addNumbers();
}
