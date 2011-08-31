package crypto {

public class BigInteger {
  
    public static var dbits:int;
    public static var BI_DB:int;
    public static var BI_DM:int;
    public static var BI_DV:int;
    public static var BI_FP:int;
    public static var BI_FV:Number;
    public static var BI_F1:int;
    public static var BI_F2:int;
    
    public var array:Vector.<int>;
    public var t:int;
    public var s:int;
    
    public function BigInteger(a:*=null, b:*=null, c:*=null):void {
        this.array = new Vector.<int>(256);
        if(a != null) {
            if(a is Number) {
                print("From here");
            }
            else if(b == null && !(a is String)) {
                print("From there");
                 fromString(a,256);
            }
            else {
                print("From else");
                fromString(a,b);
            }
        }
    }
    
    public static function setupEngine(bits:int):void {
        dbits = bits;

        BI_DB = dbits;
        BI_DM = ((1<<dbits)-1);
        BI_DV = (1<<dbits);

        BI_FP = 52;
        BI_FV = Math.pow(2,BI_FP);
        BI_F1 = BI_FP-dbits;
        BI_F2 = 2*dbits-BI_FP;
    }
    
    // (public) set from string and radix
    // bnpFromString
    public function fromString(s:*,b:int):void {
        var this_array:Vector.<int> = this.array;
        this.t = 0;
        this.s = 0;
        var i:int = s.length - 1, sh:int = 0;
        var x:int = 0;
        if(sh == 0)
            this_array[this.t++] = x;
        else if(sh+k > BI_DB) {
            this_array[this.t-1] |= (x&((1<<(BI_DB-sh))-1))<<sh;
        }
    }
} // class BigInteger

var nValue:String="a5261939975948bb7a58dffe5ff54e65f0498f9175f5a09288810b8975871e99af3b5dd94057b0fc07535f5f97444504fa35169d461d0d30cf0192e307727c065168c788771c561a9400fb49175e9e6aa4e23fe11af69e9412dd23b0cb6684c4c2429bce139e848ab26d0829073351f4acd36074eafd036a5eb83359d2a698d3";

function encrypt():void {
    var bigInt:BigInteger = new BigInteger(nValue, 16);
}

encrypt();


}//epackage
