package {  
public class Triangle {
	public function Triangle():void {

	}

	public function intersect():Number {
		return 10.11;

	}
}
public class Scene {
    
    public function Scene():void {
    }
	public function sweet():Number {
		return 10.1;
	}

    public function intersect(near:Number=undefined, far:Number=undefined):Array {
        var closest = undefined;
		var x = new Triangle();
        for (var i = 0; i < 5; i++) {
            var d = x.intersect();
			print("d is: " + d+ " and near: " + near + " and far: " + far);
            if (isNaN(d) || d > far) { 
				print("Continuing");
                continue;
			}
        }
	}
	}

	var x:Scene = new Scene();
	x.intersect(NaN, NaN);
}
        
