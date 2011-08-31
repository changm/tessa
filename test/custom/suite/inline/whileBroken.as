
package TestAdd {
    function doSum(x:int, y:int):int {
        var sum:int;
        for (x = 0; x < y; x++) {
            sum += x + y;
        }

        return sum;
    }

    function addNumbers() {
		var total:int = 0;
		for (var i:int = 0; i < 10; i++) {
			total += doSum(i + 10, i);
		}

		print(total);
    }

    addNumbers();
}
