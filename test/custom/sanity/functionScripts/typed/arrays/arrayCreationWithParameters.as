function core_md5():Array
{
  var a:int =  1732584193;
  var b:int = -271733879;

  var resultArray:Array = Array(a, b);
  print("In core md5: " + resultArray[0]);
    return resultArray;
}

core_md5();
