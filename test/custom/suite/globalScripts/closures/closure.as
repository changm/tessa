function foo() {
    var x = 10;
    (function() {
        print(x);
    })();
}

foo();
