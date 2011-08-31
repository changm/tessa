package crypto {

public class BigInteger {
    public function BigInteger():void {
    }
    
     // return bigint initialized to value
    public static function nbv(i:int):BigInteger { 
        var r:BigInteger = nbi(); 
        return r; 
    }
    
    // return new, unset BigInteger
    public static function nbi():BigInteger {
        return new BigInteger();
    }
}
    
    
function decrypt():void {
	var x:BigInteger = new BigInteger();
	var r:BigInteger = BigInteger.nbv(1); 
}

//encrypt();
decrypt();


}// package
