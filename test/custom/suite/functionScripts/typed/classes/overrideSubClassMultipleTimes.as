package deltablue {


public class Constraint {
    public function Constraint() {
    }
    
    public function addConstraint() {
      this.addToGraph();
    }
    
    
    public function addToGraph() {}
} 

public class BinaryConstraint extends Constraint {
    public function BinaryConstraint() {
      this.addConstraint();
    }
    
    override public function addToGraph() {
    }
} 


var firstOne:BinaryConstraint = new BinaryConstraint();
var secondOne:BinaryConstraint = new BinaryConstraint();
} // package
