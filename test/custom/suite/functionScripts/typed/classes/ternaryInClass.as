package deltablue {


public class BinaryConstraint {
    protected var v1:String;
    protected var v2:String;
    protected var direction:String;
    
    public function BinaryConstraint(var1:String, var2:String, strength:int) {
      this.v1 = var1;
      this.v2 = var2;
      this.chooseMethod("var");
    }

    public function returnTrue() {
        return true;
    }
    
    public function chooseMethod(mark) {
        this.direction = (returnTrue()) ? "NORTH" : "south";
    }

    public function printDirection() {
        print(this.direction);
    }
} 

function deltaBlue() {
    var x:BinaryConstraint = new BinaryConstraint("hello", "world", 3);
    x.printDirection();
}

deltaBlue();
} // package
