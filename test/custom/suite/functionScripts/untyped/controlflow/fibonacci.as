function fib(someNumber) {
    if (someNumber == 1) {
        return 1;
    } else if (someNumber == 0) {
        return 0;
    } else {
        return fib(someNumber - 1) + fib(someNumber -2);
    }
}

print(fib(8));
