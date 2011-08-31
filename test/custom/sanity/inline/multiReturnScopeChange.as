package crypto {

public class BigInteger {
    public function BigInteger():void {
    }
    
    public static function nbv(i:int):int { 
		return 30;
    }
    
    public function modPow(e:BigInteger,m:BigInteger):int {
		var r:int = nbv(1); 
		print("in mod pow");
		return r;
    }
} // class BigInteger

function decryptBroken():void {
    var p:BigInteger = new BigInteger();
	print("Created big int");
    var result:int = p.modPow(p, p);
	print("Result is: " + result);
}

decryptBroken();

}// package
