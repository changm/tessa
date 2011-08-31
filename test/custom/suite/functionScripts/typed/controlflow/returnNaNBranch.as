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
		var x = new Triangle();
        var d = x.intersect();
		if (isNaN(d) || d > far) { 
			print("Continuing");
		} else {
			print("Corret!");
		}
	
	}
	}

	var x:Scene = new Scene();
	x.intersect(NaN, NaN);
}
        
