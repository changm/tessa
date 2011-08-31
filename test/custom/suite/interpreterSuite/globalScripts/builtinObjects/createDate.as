var start = new Date(8000);

// Kill some time
var y = 0;
for (var i = 0; i < 10000; i++) {
   y += i; 
}

var total = new Date(120000) - start;
print(total);
