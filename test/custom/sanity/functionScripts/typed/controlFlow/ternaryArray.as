package {
  function AESEncryptCtr():int {
    var s:Array = new Array(4);
    var b:Array = new Array(4);  // 'b' is a•{02} in GF(2^8)

    for (var i:int=0; i<4; i++) {
        b[i] = s[i] & 0x80 ? s[i]<<1 ^ 0x011b : s[i]<<1;
    }
    return 10;
  }

  var cipherText:int = AESEncryptCtr();
    print(cipherText);
}
