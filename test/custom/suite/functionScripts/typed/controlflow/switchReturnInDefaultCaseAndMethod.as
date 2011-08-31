function switchTest():int {
	var x:int = 10;

    switch (x) {
        case 10:
            return 50;
        default:
        return 30;
    }

    return 100;
}

print(switchTest());
