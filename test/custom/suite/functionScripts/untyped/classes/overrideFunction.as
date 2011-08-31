package deltablue {


public class Constraint {
    public var strength;
    
    public function Constraint(strength) {
      this.strength = strength;
    }
    
    public function addConstraint() {
      this.addToGraph();
    }
    
    // functions to be overridden by children
    public function addToGraph() {}
} // class Constraint

public class BinaryConstraint extends Constraint {
    
    protected var v2;
    protected var direction;
    
    public function BinaryConstraint(var2, strength) {
      super(strength);
      this.addConstraint();
    }
    
    override public function addToGraph() {
      print("Binary add to graph");
    }
} // class BinaryConstraint


function chainTest(n) {
  var equalityConstraint = new BinaryConstraint("var2", "some strength");
}

function deltaBlue() {
  chainTest(100);
}

deltaBlue();
} // package
