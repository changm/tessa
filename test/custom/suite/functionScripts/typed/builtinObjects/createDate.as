function createDate() {
    var start:Date = new Date(8000);

    // Kill some time
    var y:int = 0;
    for (var i:int = 0; i < 10000; i++) {
       y += i; 
    }

    var total:Number = new Date(120000) - start;
    print(total);
}


createDate();
