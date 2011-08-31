package richards {
/*
    var r = new RichardsClass();
	r.runRichards();
    
    public class RichardsClass {
        public function RichardsClass() {
        }
    
        public function runRichards() {
          var tcb = new TaskControlBlock(null);
        } // end runRichards
    } // class RichardsClass
*/
    var x:TaskControlBlock = new TaskControlBlock(null);
    
    public class TaskControlBlock {
        public var state:int;

        public function TaskControlBlock(queue) {
          if (queue == null) {
            this.state = 2;
          } else {
            this.state = 5;
          }
          print(this.state);
        }
    } // class Task Control block
} // package
