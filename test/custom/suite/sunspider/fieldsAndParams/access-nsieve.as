
// The Great Computer Language Shootout
// http://shootout.alioth.debian.org/
//
// modified by Isaac Gouy

package {
	function pad(number:Number,width:Number):String{
	   var s = number.toString();
	   var prefixWidth = width - s.length;
	   if (prefixWidth>0){
	      for (var i=1; i<=prefixWidth; i++) s = " " + s;
	   }
	   return s;
	}

	function nsieve(m:int, isPrime:Array):int{
	   var i, k, count;

	   for (i=2; i<=m; i++) { isPrime[i] = true; }
	   count = 0;

	   for (i=2; i<=m; i++){
	      if (isPrime[i]) {
	         for (k=i+i; k<=m; k+=i) isPrime[k] = false;
	         count++;
	      }
	   }
	   return count;
	}

	function sieve():int {
            var res;
	    for (var i = 1; i <= 3; i++ ) {
	        var m= (1<<i)*10000;
	        var flags = new Array(m+1);
	        res=nsieve(m, flags);
	    }
	    return res;
	}

        var res:int = sieve();
	print("sieve()="+res);
	if (res==7837) {
	    print("Passed");
        } else {
            print("error sieve() expecting 7837 got "+res);
        }
}