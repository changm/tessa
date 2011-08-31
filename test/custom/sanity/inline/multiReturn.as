
package TestAdd {
    function doSum(x:int, y:int):int {
        if (x > y) {
            return x + y;
        } else {
            return x * y;
        }
    }

    function addNumbers() {
        print(doSum(10, 18));
    }

    addNumbers();
}
