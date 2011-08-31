package crypto {

public class BigInteger {
    public var s:int;
    
    public function BigInteger():void {
    }
    
    public function abs():BigInteger { 
        return (this.s) ?this:this; 
    }
    
    // r != q, this != m.  q or r may be null.
    // bnpDivRemTo
    public function divRemTo(m:BigInteger):void {
        var pm:BigInteger = m.abs();
		print("Did abs");
    }
} // class BigInteger

function encrypt():void {
	var a:BigInteger = new BigInteger();
	a.divRemTo(a);
}

encrypt();


}// package
