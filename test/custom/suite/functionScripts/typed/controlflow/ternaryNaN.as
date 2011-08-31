package {
  // Triangle intersection using barycentric coord method
  public class Triangle {
    public function Triangle():void {
    }

    public function intersect(near:Number, far:Number):Number {
        var t = 85.73323445685624;
        if (t < near || t > far) {
            return NaN;
		} else {
			return 0.0;
		}
    }
  }

  function testFunction():void {
		var x:Triangle = new Triangle();
		print(x.intersect(NaN, NaN));
	}

	testFunction();
  
}
