package deltablue {

public class Constraint {
    public function Constraint():void {
    }

    public function satisfy():Constraint {
        return this;
    }
} // class Constraint

public class Variable {
    function Variable() {
    }
} // class Variable

public class Planner {
    public function Planner():void {
    }
    
    public function incrementalAdd(c:Constraint):void {
        var overridden:Constraint = new Constraint();
        while (overridden != null)
            overridden = overridden.satisfy();
    }
    
    public function incrementalRemove():void {
        var u:Constraint = new Constraint();
        if (10 == 20) {
            this.incrementalAdd(u);
        }
    }
} // class Planner

// Global variable holding the current planner.
var planner:Planner = null;

function deltaBlue():void {
    planner = new Planner();
    planner.incrementalRemove();
}


deltaBlue();

} // package
