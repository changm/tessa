function whileIfLoop() {
    var m:int = 1;
    var c:int = 0;

    while (m < 5) {
        if(m > 2) { 
            c++;
        }

        m++;
    }

    print(c);
}

whileIfLoop();
