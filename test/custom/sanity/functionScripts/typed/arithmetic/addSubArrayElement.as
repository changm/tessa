package {
  var loops:int = 15
  var nx:int = 120
  var nz:int = 120

  function run3dMorph():int {    
    var a:Array = new Array(nx*nz*3);
    a[0] = 0;
    var testOutput:Number = 0;

    for (var i:int = 0; i < 1; i++)
        testOutput += a[i];

    testOutput = testOutput - 6.7;
  }

run3dMorph();

}
