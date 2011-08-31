package deltablue {

public class Constraint {
    protected var value:int;
    public function Constraint() {
        this.value = 10;
    }
    
    public function addToGraph() {
        print(this.value);
    }
} 

public class BinaryConstraint extends Constraint {
    public function BinaryConstraint() {
      super();
      this.addToGraph();
    }
   
    override public function addToGraph() {
      super.addToGraph();
    }
    
} // class BinaryConstraint


new BinaryConstraint();
} // package
