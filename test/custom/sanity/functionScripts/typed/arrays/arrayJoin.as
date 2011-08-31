function testArrayJoin(o, sep):String
{
    var s:String = (sep === undefined) ? "," : String(sep)
    var out:String = ""
    for (var i:uint = 0, n:uint=uint(o.length); i < n; i++)
    {
        var x = o[i]
        if (x != null)
            out += x
        if (i+1 < n)
            out += s
    }
    return out
}

function doArrayJoin() {
     var x:Array = [0, 10, 15, 30];
    print(testArrayJoin(x, " = "));
}

doArrayJoin();
