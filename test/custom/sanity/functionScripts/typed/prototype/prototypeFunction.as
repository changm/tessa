function TreeNode(left:String, right:String) {
    this.left = left;
    this.right = right;
}

TreeNode.prototype.printMessage = function() {
    print(this.left);
}


function prototypeFunction() {
    var x:Object = new TreeNode("aLeft", "aRight");
    x.printMessage();
}

prototypeFunction();
